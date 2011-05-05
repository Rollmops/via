/*! \file 
3d curvature computations.

For each surface voxel, a curvature feature is computed.
Surface voxels are defined by a threshold.
The algorithm is based on discrete approximations to
second-order derivatives and the Weingarten equations.

\par Reference:
O. Monga, S. Benayoun (1995).
"Using partial derivatives of 3D images to extract typical surface features",
Computer Vision and Image Understanding,
Vol.61, No.2, pp.171--189.

\par Author:
Gabriele Lohmann, MPI-CBS
*/



/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define CONVEX  1
#define CONCAVE 2
#define SADDLE  3
#define FLAT    4

/*
**  types of derivatives
*/
#define COL      1
#define ROW      2
#define BAND     3
#define COL2     4
#define ROW2     5
#define BAND2    6
#define COL_ROW  7
#define COL_BAND 8
#define ROW_BAND 9

#define epsilon 0.003
#define ABS(x) ((x) > 0 ? (x) : -(x))

#define MSIZE 7    /* size of mask */


double
VDeriv(VImage src,int b,int r,int c,double mask[MSIZE][MSIZE][MSIZE],int wsize)
{
  int bb,rr,cc,b0,b1,r0,r1,c0,c1;
  int wn2,i,j,k;
  double sum;
  int nbands,nrows,ncols;

  wn2 = wsize / 2;

  nbands  = VImageNBands (src);
  nrows   = VImageNRows (src);
  ncols   = VImageNColumns (src);

  sum = 0;
  b0 = (b > wn2) ? b0 = b-wn2 : 0;
  b1 = (b < nbands - wn2) ? b1 = b + wn2 : nbands - 1;
  i = (b > wn2) ? 0 : wn2 - b;
  for (bb=b0; bb <= b1; bb++) {

    r0 = (r > wn2) ? r0 = r-wn2 : 0;
    r1 = (r < nrows - wn2) ? r1 = r + wn2 : nrows - 1;
    j = (r > wn2) ? 0 : wn2 - r;
    for (rr=r0; rr <= r1; rr++) {

      c0 = (c > wn2) ? c0 = c-wn2 : 0;
      c1 = (c < ncols - wn2) ? c1 = c + wn2 : ncols - 1;
      k = (c > wn2) ? 0 : wn2 - c;
      for (cc=c0; cc <= c1; cc++) {

        sum += VGetPixel(src,bb,rr,cc) * mask[i][j][k];
        k++;
      }
      j++;
    }
    i++;
  }
  return sum;
}

/*
** 1st and 2nd derivative of a gaussian
*/
void
vderiv_gaussian(double sigma,double *gauss,double *deriv1,double *deriv2,int n)
{  
  int i,wn2;
  double c,x,y,sum1,sum2,norm;
  double pi = 3.14159265;

  c = 1.0 / (sqrt((double) 2.0 * pi) * sigma);
  wn2 = n / 2;

  sum1 = 0;
  for (i=0; i<n; i++) {
    x = (double) (i - wn2);
    y = c * exp(- (x * x) / (2.0 * sigma * sigma));
    gauss[i] = y;
    sum1 += gauss[i];
  }
  norm = sum1;
  sum2 = 0;
  for (i=0; i<n; i++) {
    gauss[i] /= norm;
    sum2 += gauss[i];
  }

  /* D1 */
  sum1 = 0; sum2 = 0;
  for (i=0; i<n; i++) {
    x = (double) (i - wn2);
    y = - x * c * exp(- (x * x) / (2.0 * sigma * sigma));
    deriv1[i] = y / (sigma * sigma);
    sum1 += deriv1[i];
    sum2 += x * deriv1[i];
  }
  norm = sum2;
  norm = 1.0;
  sum1 = 0; sum2 = 0;
  for (i=0; i<n; i++) {
    x = (double) (i - wn2);
    deriv1[i] /= norm;
    sum1 += deriv1[i];
    sum2 += x * deriv1[i];
  }

  /* D2 */
  sum1 = 0; sum2 = 0;
  for (i=0; i<n; i++) {
    x = (double) (i - wn2);
    y = c * exp(- (x * x) / (2.0 * sigma * sigma)) * ((x * x) / (sigma * sigma) - 1.0);
    deriv2[i] = y / (sigma * sigma);
    sum1 += deriv2[i];
    sum2 += deriv2[i] * x * x * 0.5;
  }

  norm = sum2;
  norm = 1.0;

  sum1 = 0; sum2 = 0;
  for (i=0; i<n; i++) {
    x = (double) (i - wn2);
    if (ABS(norm) > 0.00001) deriv2[i] /= norm;
    sum1 += deriv2[i];
    sum2 += 0.5 * x * x * deriv2[i];
  }
}


void getmask(double *gaussian,double *deriv, double *deriv2,
	     int type,double mask[MSIZE][MSIZE][MSIZE],int n)
{
  int i,j,k;

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      for (k=0; k<n; k++) {

	switch(type) {

	case COL:
	  mask[i][j][k] = 
	    gaussian[i] * gaussian[j] * deriv[k];
	  break;

	case ROW:
	  mask[i][j][k] = 
	    deriv[j] * gaussian[k] * gaussian[i];
	  break;

	case BAND:
	  mask[i][j][k] = 
	    deriv[i] * gaussian[j] * gaussian[k];
	  break;

	case COL2:
	  mask[i][j][k] = 
	    deriv2[k] * gaussian[i] * gaussian[j];
	  break;

	case ROW2:
	  mask[i][j][k] = 
	    deriv2[j] * gaussian[k] * gaussian[i];
	  break;

	case BAND2:
	  mask[i][j][k] = 
	    deriv2[i] * gaussian[k] * gaussian[j];
	  break;

	case COL_ROW:
	  mask[i][j][k] = 
	    deriv[k] * gaussian[i] * deriv[j];
	  break;

	case COL_BAND:
	  mask[i][j][k] = 
	    deriv[k] * gaussian[j] * deriv[i];
	  break;

	case ROW_BAND:
	  mask[i][j][k] = 
	    deriv[i] * gaussian[k] * deriv[j];
	  break;

	default:
	  fprintf(stderr," illegal mask type \n");
	}
      }
    }
  }
}

/*
** check if border point
*/
int
Border(VImage src,int b,int r,int c,VFloat threshold)
{
  int nbands,nrows,ncols;

  nbands  = VImageNBands (src);
  nrows   = VImageNRows (src);
  ncols   = VImageNColumns (src);

  if (b > 0)
    if (VGetPixel(src,b-1,r,c) < threshold) return 1;
  
  if (r > 0)
    if (VGetPixel(src,b,r-1,c) < threshold) return 1;
  
  if (c > 0)
    if (VGetPixel(src,b,r,c-1) < threshold) return 1;

  if (b < nbands - 1)
    if (VGetPixel(src,b+1,r,c) < threshold) return 1;

  if (r < nrows - 1)
    if (VGetPixel(src,b,r+1,c) < threshold) return 1;

  if (c < ncols - 1)
    if (VGetPixel(src,b,r,c+1) < threshold) return 1;
  
  return 0;
}


/*!
\fn VImage VCurvature (VImage src,VImage dest,VFloat threshold,VLong type)
\brief compute curvature features at each surface point
\param src     input image  (any repn)
\param dest    output image (float repn)
\param threshold binarization into 'foreground' and 'background'.
\param type    type of operation. Possible types are:
\param border  whether to process ony border voxels
<ul>
<li> 0: classification into  convex(0), concave(1) or saddle(2)
<li> 1: mean curvature
<li> 2: gaussian curvature
</ul> 
*/
VImage
VCurvature (VImage src,VImage dest,VFloat threshold,VLong type,VBoolean border)
{
  int b,r,c;
  int nbands,nrows,ncols;
  double f,fx,fy,fz,fxx,fyy,fzz,fxy,fxz,fyz;
  double u,v,w,x,norm;
  double fx0,fx2,fy0,fy2,fz0,fz2;
  double fu,fv,fuu,fvv,fuv,h;
  double E,F,G,H,L,M,N,H2;
  double s0,s1,s2,k0,k1,k2;
  double deriv[MSIZE],deriv2[MSIZE],gaussian[MSIZE];
  double maskCol[MSIZE][MSIZE][MSIZE];
  double maskRow[MSIZE][MSIZE][MSIZE];
  double maskBand[MSIZE][MSIZE][MSIZE];
  double maskCol2[MSIZE][MSIZE][MSIZE];
  double maskRow2[MSIZE][MSIZE][MSIZE];
  double maskBand2[MSIZE][MSIZE][MSIZE];
  double maskColRow[MSIZE][MSIZE][MSIZE];
  double maskRowBand [MSIZE][MSIZE][MSIZE];
  double maskColBand [MSIZE][MSIZE][MSIZE];
  int curvature_class;
  int wsize=MSIZE;
  double sigma=1;
  double tiny=1.0e-5;


  vderiv_gaussian(sigma,gaussian,deriv,deriv2,wsize);
  getmask(gaussian,deriv,deriv2,(int) COL,maskCol,wsize);
  getmask(gaussian,deriv,deriv2,(int) ROW,maskRow,wsize);
  getmask(gaussian,deriv,deriv2,(int) BAND,maskBand,wsize);
  getmask(gaussian,deriv,deriv2,(int) COL2,maskCol2,wsize);
  getmask(gaussian,deriv,deriv2,(int) ROW2,maskRow2,wsize);
  getmask(gaussian,deriv,deriv2,(int) BAND2,maskBand2,wsize);
  getmask(gaussian,deriv,deriv2,(int) COL_ROW,maskColRow,wsize);
  getmask(gaussian,deriv,deriv2,(int) ROW_BAND,maskRowBand,wsize);
  getmask(gaussian,deriv,deriv2,(int) COL_BAND,maskColBand,wsize);

  nbands  = VImageNBands (src);
  nrows   = VImageNRows (src);
  ncols   = VImageNColumns (src);

  dest = VSelectDestImage("VCurvature",dest,nbands,nrows,ncols,VFloatRepn);
  if (! dest) VError(" err creating dest image");
  VFillImage(dest,VAllBands,0);
  

  for (b=1; b<nbands-1; b++) {
    for (r=1; r<nrows-1; r++) {
      for (c=1; c<ncols-1; c++) {

	f = VGetPixel(src,b,r,c);
	if (f < threshold) continue;
	
	if (border) { /* process only border voxels */
	  if (Border(src,b,r,c,threshold) == 0) continue; 
	}

	fx = VDeriv(src,b,r,c,maskCol,wsize);
	fy = VDeriv(src,b,r,c,maskRow,wsize);
	fz = VDeriv(src,b,r,c,maskBand,wsize);

	norm = (fx * fx + fy * fy + fz * fz);
	if (norm < 0.001) continue;


	fxx = VDeriv(src,b,r,c,maskCol2,wsize);
	fyy = VDeriv(src,b,r,c,maskRow2,wsize);
	fzz = VDeriv(src,b,r,c,maskBand2,wsize);

	fxy = VDeriv(src,b,r,c,maskColRow,wsize);
	fxz = VDeriv(src,b,r,c,maskColBand,wsize);
	fyz = VDeriv(src,b,r,c,maskRowBand,wsize);

	/*
	E = 1.0 + (fx * fx) / (fz * fz);
	F = (fx * fy) / (fz * fz);
	G = 1.0 + (fy * fy) / (fz * fz);
	H = (fx * fx + fy * fy + fz * fz) / (fz * fz);
	H2 = sqrt((double) H) * fz * fz * fz;

	L = - (fxx * fz * fz - 2.0 * fxz * fx * fz + fzz * fx * fx) / H2;
	M = (fyz * fx * fz + fxz * fy * fz - fzz * fx * fy - fxy * fz * fz) / H2;
	N = - (fyy * fz * fz - 2.0 * fyz * fy * fz + fzz * fy * fy) / H2;

	s0 = (G * L - 2.0 * F * M + E * N) / (2.0 * (E * G - F * F));
	k0 = (L * N - M * M ) / (E * G - F * F);
	*/

	s2 = k2 = 0;

	/* Gauss curvature: */
	if (type == 0 || type == 2 || type == 3) {
	  k2  = fx * fx * (fyy * fzz - fyz * fyz) + 2.0 * fy * fz * (fxz * fxy - fxx * fyz);
	  k2 += fy * fy * (fxx * fzz - fxz * fxz) + 2.0 * fx * fz * (fyz * fxy - fyy * fxz);
	  k2 += fz * fz * (fxx * fyy - fxy * fxy) + 2.0 * fx * fy * (fxz * fyz - fzz * fxy);
	  h   = (fx*fx + fy*fy + fz*fz);
	  if (ABS(h) > 1.0e-4) k2 /= (h * h);
	  else k2 = 0;
	}

	/* mean curvature */
	if (type == 0 || type == 1 || type == 3) {
	  s2  = fx * fx * (fyy + fzz) - 2.0 * fy * fz * fyz;
	  s2 += fy * fy * (fxx + fzz) - 2.0 * fx * fz * fxz; 
	  s2 += fz * fz * (fxx + fyy) - 2.0 * fx * fy * fxy;
	  h   = sqrt(fx*fx + fy*fy + fz*fz);
	  h   = 2.0 * h * h * h;
	  if (ABS(h) > 1.0e-4) s2 /= h;
	  else s2 = 0;
	}

	switch(type) {

	case 0:  /* curvature classification */
	  if (s2 >= 0) curvature_class = CONCAVE;        /* concave */
	  else curvature_class = CONVEX;                 /* convex  */
	  if (k2 <= -tiny) curvature_class = SADDLE;    /* saddle  */
	  if (ABS(k2) < tiny && ABS(s2) < tiny) curvature_class = FLAT;

	  VPixel(dest,b,r,c,VFloat) = curvature_class;
	  break;

	case 1:   /* mean curvature */
	  VPixel(dest,b,r,c,VFloat) = s2;
	  break;

	case 2:   /* gaussian curvature */
	  if (k2 < -10) k2 = -10;
	  if (k2 >  10) k2 =  10;
	  VPixel(dest,b,r,c,VFloat) = k2;
	  break;

	case 3:   /* sum of curvatures */
	  h = sqrt(s2*s2 + k2*k2);
	  VPixel(dest,b,r,c,VFloat) = h;
	  break;


	default:
	  ;
	}
      }
    }
  }

  VCopyImageAttrs (src,dest);
  return dest;
}


