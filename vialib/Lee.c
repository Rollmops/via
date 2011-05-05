/*! \file
3D Lee filter

\par Author:
Gabriele Lohmann, MPI-CBS
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C library: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

/*
** this macro performs the sigma filtering (2D)
*/
#define Lee2d(type)                                         \
{                                                           \
    type *src_pp,*dest_pp;                                  \
    src_pp = (type *) VPixelPtr(src,band,0,0);              \
    dest_pp = (type *) VPixelPtr(result,band,0,0);          \
    for (row = 0; row < nrows; row++) {                     \
      for (col = 0; col < ncols; col++) {                   \
	val1 = (long) *src_pp++;                            \
        if (!((ignoreval >= 0)&&(val1 == ignoreval))) {     \
	sum = 0;                                            \
        dlen = 0;                                           \
        rwm=row-wn2;rwp=row+wn2;cwm=col-wn2;cwp=col+wn2;    \
        if (row < wn2) {rstart=0;rend=rwp;}                 \
        if ((row >= wn2)&&(row<nr2)) {rstart=rwm;rend=rwp;} \
        if (row>=nr2) {rstart=rwm;rend=nr1;}                \
        if (col < wn2) {cstart=0;cend=cwp;}                 \
        if ((col >= wn2)&&(col<nc2)) {cstart=cwm;cend=cwp;} \
        if (col>=nc2) {cstart=cwm;cend=nc1;}                \
	for (r = rstart; r <= rend; r++) {                  \
	  for (c = cstart; c <= cend; c++) {                \
            val2 = (long) VPixel(src,band,r,c,type);        \
        if (ignoreval == -1) {                              \
            if ((double) ABS(val1 - val2) <= sigma) {       \
	      sum += val2;                                  \
	      dlen++;                                       \
	    }                                               \
   	   }                                                \
        if ((ignoreval >= 0)&&(val2 != ignoreval)) {        \
            if ((double) ABS(val1 - val2) <= sigma) {       \
	      sum += val2;                                  \
	      dlen++;                                       \
	    }                                               \
   	   }                                                \
	  }                                                 \
    	}                                                   \
	 *dest_pp++ = (type) (sum / dlen);                  \
       }                                                    \
	 else *dest_pp++ = (type) 0;                        \
      }                                                     \
    }                                                       \
}

/*
** this macro performs the sigma filtering (3D)
*/
#define Lee3d(type)                                           \
{                                                             \
    type *src_pp,*dest_pp;                                    \
    src_pp = (type *) VPixelPtr(src,0,0,0);                   \
    dest_pp = (type *) VPixelPtr(result,0,0,0);               \
    for (band = 0; band < nbands; band++) {                   \
     for (row = 0; row < nrows; row++) {                      \
      for (col = 0; col < ncols; col++) {                     \
        y = (long) *src_pp++;                                 \
        if (!((ignoreval >= 0)&&(y == ignoreval))) {          \
	sum = 0;                                              \
        dlen = 0;                                             \
        rwm=row-wn2;rwp=row+wn2;cwm=col-wn2;cwp=col+wn2;      \
        bwm=band-wn2;bwp=band+wn2;                            \
        if (band < wn2) {bstart=0;bend=bwp;}                  \
        if ((band >= wn2)&&(band<nb2)) {bstart=bwm;bend=bwp;} \
        if (band>=nb2) {bstart=bwm;bend=nb1;}                 \
        if (row < wn2) {rstart=0;rend=rwp;}                   \
        if ((row >= wn2)&&(row<nr2)) {rstart=rwm;rend=rwp;}   \
        if (row>=nr2) {rstart=rwm;rend=nr1;}                  \
        if (col < wn2) {cstart=0;cend=cwp;}                   \
        if ((col >= wn2)&&(col<nc2)) {cstart=cwm;cend=cwp;}   \
        if (col>=nc2) {cstart=cwm;cend=nc1;}                  \
        for (b = bstart; b <= bend; b++) {                    \
         for (r = rstart; r <= rend; r++) {                   \
          for (c = cstart; c <= cend; c++) {                  \
            pixval = (long) VPixel(src,b,r,c,type);           \
        if (ignoreval == -1) {                                \
            if ((double)ABS(pixval - y) <= sigma) {           \
	      sum += pixval;                                  \
	      dlen++;                                         \
	    }                                                 \
   	   }                                                  \
        if ((ignoreval >= 0)&&(pixval != ignoreval)) {        \
            if ((double)ABS(pixval - y) <= sigma) {           \
	      sum += pixval;                                  \
	      dlen++;                                         \
	    }                                                 \
   	   }                                                  \
	  }                                                   \
    	 }                                                    \
    	}                                                     \
         *dest_pp++ = (type) (sum / dlen);                    \
       }                                                      \
	 else *dest_pp++ = (type) 0;                          \
      }                                                       \
     }                                                        \
    }                                                         \
}




/*!
\fn VImage VLeeImage (VImage src,VImage result,VLong wsize,VDouble sigma,VLong dim,VLong ignoreval)
\param src   input image 
\param result  output image 
\param wsize window size
\param sigma sigma
\param dim dimension (0=2D, 1=3D)
\param ignoreval grey value to be ignored
*/

VImage
VLeeImage (VImage src,VImage result,VLong wsize,VDouble sigma,VLong dim,VLong ignoreval)
{
  VRepnKind result_repn;
  long sum,val1,val2,pixval,y;
  long b,r,c,band,row,col,nbands,nrows,ncols;
  long wn2,dlen;
  long cstart,cend,rstart,rend,nr2,nr1,nc2,nc1,rwp,cwp,rwm,cwm;
  long bstart,bend,nb2,nb1,bwp,bwm;

  nrows = VImageNRows (src);
  ncols = VImageNColumns (src);
  nbands = VImageNBands (src);
  result_repn = VPixelRepn (src);

  /* Ensure that "wsize" is legal: */
  if (wsize <= 0 || wsize%2 == 0) {
    VWarning ("VLee2dImage: illegal value wsize (%d)", wsize);
    return NULL;
  }
  /* Ensure that "wsize" is less or equal than nbands for 3D */
  if ((wsize > nbands)&&(dim==1)) {
    VWarning ("VBoxImage: wsize (%d) is greater than nbands (%d) for 3D", wsize,nbands);
    return NULL;
  }
  if (result == NULL)
    result = VCreateImage (nbands,nrows,ncols,result_repn);
  if (! result) return NULL;

  dlen = 0;
  wn2 = floor((double) wsize / 2.0);
  nb2=nbands-wn2;
  nr2=nrows-wn2;
  nc2=ncols-wn2;
  nb1=nbands-1;
  nr1=nrows-1;
  nc1=ncols-1;

  if (dim==0) { 
    for (band = 0; band < nbands; band++) {
      switch (result_repn) {

      case VBitRepn:
        Lee2d(VBit);
        break;

      case VUByteRepn:
        Lee2d(VUByte);
        break;

      case VSByteRepn:
        Lee2d(VSByte);
        break;

      case VShortRepn:
        Lee2d(VShort);
        break;

      case VLongRepn:
        Lee2d(VLong);
        break;

      case VFloatRepn:
        Lee2d(VFloat);
        break;

      case VDoubleRepn:
        Lee2d(VDouble);
      }
    }
  }
  if (dim==1) { 
    switch (result_repn) {

    case VBitRepn:
      Lee3d(VBit);
      break;

    case VUByteRepn:
      Lee3d(VUByte);
      break;

    case VSByteRepn:
      Lee3d(VSByte);
      break;

    case VShortRepn:
      Lee3d(VShort);
      break;

    case VLongRepn:
      Lee3d(VLong);
      break;

    case VFloatRepn:
      Lee3d(VFloat);
      break;

    case VDoubleRepn:
      Lee3d(VDouble);
    }
  }

  VCopyImageAttrs (src, result);
  return result;
}
