/*
**  display the image histogram.
**  input images of any repn type are allowed.
**
** G.Lohmann, MPI-CBS.
** 
** $Id: ImageHistogram.c 3629 2009-08-20 17:04:30Z proeger $
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))


float
*ImageHistogram (VImage src,VLong ignore,VLong *nnvals,
		 double *ymin,double *ymax,double *mean,double *sigma)
{
  int i,band,row,col,nvals;
  int nbands,nrows,ncols;
  float *histo;
  double xmin,xmax,u,v;
  double sum1,sum2,nx;

  nbands = VImageNBands(src);
  nrows = VImageNRows(src);
  ncols = VImageNColumns(src);

  if (VPixelRepn(src) != VUByteRepn) {
    xmin = VPixelMaxValue(src);
    xmax = VPixelMinValue(src);

    for (i=0; i<VImageNPixels(src); i++) {
      u = VGetPixelValue (src,i);
      if (u > -0.00001 && u < 0.00001) continue; 
      if (u < xmin) xmin = u;
      if (u > xmax) xmax = u;
    }

    u = xmax - xmin;
    if (ABS(u) < 0.0001) VError(" err in ImageHisto");

    if (u < 1) {
      xmin = VFloor(xmin);
      xmax = VCeil(xmax);
      nvals = 101;
    }
    else if (u < 11) {
      xmin = VFloor(xmin);
      xmax = VCeil(xmax);
      nvals = 101;
    }
    else if (u < 501) {
      xmin = 10.0 * VRint(xmin/10.0);
      xmax = 10.0 * VRint(xmax/10.0);
      if (xmin > 0) xmin = 0;
      nvals = 201;
    }
    else if (u < 1010) {
      xmin = 100.0 * VRint(xmin/100.0);
      xmax = 100.0 * VRint(xmax/100.0);
      if (xmin > 0) xmin = 0;
      nvals = 201;
    }
    else {
      xmin = 200.0 * VRint(xmin/200.0);
      xmax = 200.0 * VRint(xmax/200.0);
      if (xmin > 0 && xmin < 100) xmin = 0;
      nvals = 401;
    }

    *ymin = xmin;
    *ymax = xmax;
  }
  else {
    xmin = 0;
    xmax = 255;
    *ymin = xmin;
    *ymax = xmax;
    nvals = 256;
  }


  *nnvals = nvals;

  histo = (float *) VMalloc(sizeof(float) * nvals);
  for (i=0; i<nvals; i++) histo[i] = 0;

  sum1 = sum2 = nx = 0;
  for (band = 0; band < nbands; band++) {
    for (row = 0; row < nrows; row++) {
      for (col = 0; col < ncols; col++) {
	v = VGetPixel(src,band,row,col);
	if (v > -0.00001 && v < 0.00001) continue; 
	u = (float)(nvals-1) * (v-xmin)/(xmax-xmin);

	i = (int) VRint(u);

	if (i < 0) i = 0;
	if (i >= nvals) i = nvals-1;
	if (i != ignore) {
	  histo[i]++;
	  sum1 += v;
	  sum2 += v*v;
	  nx++;
	}
      }
    }
  }
  *mean = sum1 / nx;
  *sigma = sqrt((double)((sum2 - nx * (*mean) * (*mean)) / (nx - 1.0)));


  return histo;
}
