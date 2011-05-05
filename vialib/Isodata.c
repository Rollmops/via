/*! \file
Isodata clustering


\par Author:
Gabriele Lohmann, MPI-CBS
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))


/*
 *  Histo
 *
 *  This macro computes the 1-dim image histogram of all bands
 */
#define Histo(type)                                      \
{                                                        \
  for (m=0; m<256; m++) histo[m] = 0;                    \
  for (band = 0; band < nbands; band++) {		 \
    for (row = 0; row < nrows; row++) {			 \
      for (col = 0; col < ncols; col++) {		 \
	pixval = (VLong) VPixel(src,band,row,col,type);	 \
        if (pixval == ignore) continue;                  \
	if (pixval < 0)   pixval = 0;			 \
	if (pixval > 255) pixval = 255;			 \
	histo[pixval]++;				 \
      }							 \
    }							 \
  }							 \
}

/*
 *  Output
 *
 *  This macro applies the output lut to the image
 */
#define Lutapply(type)                                        \
{ \
  for (i=0; i<256; i++) { \
    dmin = 100000.0; \
    j0 = 0; \
    for (j=0; j<nclusters; j++) { \
      if (ABS((double) i - center[j]) < dmin) { \
	dmin = ABS((double)i - center[j]); \
	j0 = j; \
      } \
    } \
    lut[i] = (VUByte) j0; \
  } \
  for (band = 0; band < nbands; band++) {		      \
    for (row=0; row< nrows; row++) {		              \
      for (col=0; col < ncols; col++) {	                      \
        pixval = (VLong) VPixel(src,band,row,col,type);	      \
        if (pixval < 0)   pixval = 0;			      \
        if (pixval > 255) pixval = 255;			      \
        VPixel(dest,band,row,col,VUByte) = lut[pixval];       \
      }							      \
    }							      \
  }                                                           \
}

/*
**  sort the array: Numerical recipes, p. 330
*/
static void
piksrt(double *array, VLong n) 
{
  int i,j;
  double a=0;

  for (j=1; j<n; j++) {
    a = array[j];
    i = j-1;
    while (i >= 0 && array[i] > a) {
      array[i+1] = array[i];
      i--;
    }
    array[i+1] = a;
  }
}



void Isodata3d(double *center, double *histo, VLong nclusters)
{
  int i,j,jmin[256],count;
  double cx=0,xnorm;
  double dx,dmin,sum,diff;

/*  let initial cluster centers be evenly distributed between 0...255  */
  cx = (int) (255.0 / (float) (nclusters + 1));
  for (i=0; i<nclusters; i++) center[i] = (i + 1) * cx;

  diff = 1000000000.0;
  count = 0;

  while (diff > 1.0 && count < 15) {

    for (i=0; i<256; i++) {
      dmin = 256.0 * 256.0;
      for (j=0; j<nclusters; j++) {
	dx = (double) i - center[j];
	dx *= dx;
	if (dx < dmin) {
	  jmin[i] = j;  dmin = dx;
	}
      }
    }

    diff = 0;
    for (i=0; i<nclusters; i++) {
      xnorm = 0; sum = 0;
      for (j=0; j<256; j++)
        if (jmin[j] == i) xnorm += histo[j];
      for (j=0; j<256; j++)
        if (jmin[j] == i) sum += (double) j * histo[j];
      if (xnorm > 0) sum /= xnorm;
      else sum = 0;
      dx = sum - center[i];
      center[i] = sum;
      dx *= dx;
      diff += dx;
    }
    count++;
  }

  piksrt(center,nclusters);
}



/*!
\fn VImage VIsodataImage3d (VImage src,VImage dest,VLong nclusters,VLong ignore)
\param src   input image (any repn)
\param dest  output image (ubyte repn)
\param nclusters number of clusters
\param ignore grey value to be ignored in clustering (e.g. '0')
*/
VImage
VIsodataImage3d (VImage src,VImage dest,VLong nclusters,VLong ignore)
{
  VLong   pixval;
  double  *histo;
  double  dmin;
  int nbands,nrows,ncols;
  int band,row, col, i, j,j0=0, m;
  VRepnKind dest_repn;
  double *center;
  VUByte lut[256];

  /* Ensure that "c" is legal: */
  if (nclusters <= 1 || nclusters > 255 ) {
    VError ("VIsodataImage: illegal number of clusters (%d)", nclusters);
  }

  /* Choose an appropriate destination representation: */
  dest_repn = VUByteRepn;
  nbands = VImageNBands (src);
  nrows  = VImageNRows (src);
  ncols  = VImageNColumns (src);

  if (dest == NULL)
    dest = VCreateImage (nbands,nrows,ncols,VUByteRepn);
  if (! dest) return NULL;

  histo  = (double *) VMalloc(256 * sizeof(double));
  center = (double *) VMalloc(nclusters * sizeof(double));

  /* Get histogram and perform clustering */
  switch (VPixelRepn(src)) {
    
  case VBitRepn:
    VError("VIsodataImage3d: illegal pixel repn");
    break;
    
  case VUByteRepn:
    Histo(VUByte);
    Isodata3d(center,histo,nclusters);
    Lutapply(VUByte);
    break;
    
  case VSByteRepn:
    Histo(VSByte);
    Isodata3d(center,histo,nclusters);
    Lutapply(VSByte);
    break;
    
  case VShortRepn:
    Histo(VShort);
    Isodata3d(center,histo,nclusters);
    Lutapply(VShort);
    break;
    
  case VLongRepn:
    Histo(VLong);
    Isodata3d(center,histo,nclusters);
    Lutapply(VLong);
    break;
    
  case VFloatRepn:
    Histo(VFloat);
    Isodata3d(center,histo,nclusters);
    Lutapply(VFloat);
    break;
    
  case VDoubleRepn:
    Histo(VDouble);
    Isodata3d(center,histo,nclusters);
    Lutapply(VDouble);

  default:
    VWarning(" no legal image representation found in input file");
  }

  /* Successful completion: */
  VCopyImageAttrs (src, dest);
  return dest;
}







