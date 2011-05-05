/*
**  display the image histogram.
**  input images of any repn type are allowed.
**
** G.Lohmann, MPI-CBS.
*/


/* From the Vista library: */
#include <viaio/VX.h>
#include <viaio/VImageView.h>
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>

/* From the standard C library: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define WIDTH  1200
#define HEIGHT  600


/*
**  DisplayHistogram
*/
VBoolean 
DisplayHistogram(float *histo,VLong nvals,VRepnKind repn,double xmin,double xmax,
		 double average,double sigma)
{
  VImage image;
  int nbands = 1;
  int i,nrows,ncols;
  int r0,c0,xlen,ylen;
  int margin = 40;
  int right_margin = 40;
  int xstep,ystep,modu1,modu2;
  int ymax;
  int r1,r2,c1;
  char str[80];
  float nels,imax,u,d;
  Widget widget;


  imax = 0;
  nels = 0;
  for (i=0; i<nvals; i++) {
    if (histo[i] > imax) imax = i;
    nels += histo[i];
  }

  ymax = 0;
  for (i=0; i<nvals; i++) {
    histo[i] = (histo[i] * 100.0) / nels;
    if (histo[i] > ymax) ymax = (int) histo[i];
  }
  ymax += 2;


  /* get display size */
  ncols = 4 * nvals + margin + right_margin;
  if (ncols > WIDTH) 
    ncols = 3 * nvals + margin + right_margin;
  if (ncols > WIDTH) 
    ncols = 2 * nvals + margin + right_margin;
  if (ncols > WIDTH) 
    ncols = nvals + margin + right_margin;

  if (ncols < WIDTH) ncols= WIDTH;

  nrows = 512;


  widget = VXGetApplicationShell();
  XtVaSetValues(widget,XmNwidth,ncols + 100,XmNheight,nrows + 50,NULL);

  /* Display a white background image: */
  image = VCreateImage(nbands,nrows,ncols,VBitRepn);
  if (!image) VError("Error creating image");
  VFillImage(image,VAllBands,1);

  VXClearLines ();
  sprintf(str,"mean: %g  sigma: %g\n",average,sigma);
  VXDisplayMessage (TRUE,str);
  VXSetImage (image, 0, 1.0, 0, 0);

  /* set style */
  VXSetLineColor("black");
  VXSetTextColor("black");
  /*
  VXSetTextFont("times_roman10");
  */


  r0 = nrows - margin;      /* origin of coordinate axes */
  c0 = margin;

  xlen = ncols - margin - right_margin;    /* length of x axis */
  ylen = nrows - margin - right_margin;    /* length of y axis */

  xstep = xlen / nvals;   /* tics on x-axis */
  ystep = ylen / ymax;    /* tics on y-axis */


  /* 
  ** x-axis 
  */
  VXDrawLine(r0,c0-10,r0,c0 + xlen);

  r1 = r0;
  c1 = c0;
  if (xstep == 2) modu1 = 20;
  else if (xstep == 1) modu1 = 50;
  else modu1 = 10;
  if (xstep >= 3)
    modu2 = 1;
  else
    modu2 = 5;


  if (nvals == 256) modu1 = 20;
  else modu1 = 20;

  d = xmax - xmin;

  for (i=0; i<nvals; i++) {
    VXDrawLine(r1,c1,r1+8,c1);
    u = xmin + (float)i * (xmax - xmin)/(float)(nvals-1);

    if (nvals != 256) {
      if (d < 0.01) sprintf(str,"%.3f",u);
      else if (d < 1) sprintf(str,"%.3f",u);
      else if (d < 11) sprintf(str,"%.3f",u);
      else if (d < 110) sprintf(str,"%.2f",u);
      else if (d < 501) sprintf(str,"%.0f",u);
      else if (d < 1010) sprintf(str,"%.0f",u);
      else sprintf(str,"%.0f",u);
    }
    else 
      sprintf(str,"%.0f",u);

    VXDrawLine(r1,c1,r1+4,c1);
    if (i%modu1 == 0)
      VXDrawText(str,r1+25,c1-5);

    c1 += xstep;
  }


  /* 
  ** y axis 
  */
  VXDrawLine(r0+10,c0,r0 - ylen,c0);
  
  r1 = r0;
  c1 = c0;
  if (ymax < 30) {
    modu1 = 5;
    modu2 = 1;
  }
  else {
    modu1 = 10;
    modu2 = 2;
  }

  for (i=0; i<=ymax; i++) {
    if (i % modu1 == 0) {
      VXDrawLine(r1,c1-8,r1,c1);
      sprintf(str,"%d",i);
      VXDrawText(str,r1+5,c1-30);
      
    }
    else if (i % modu2 == 0) {
      VXDrawLine(r1,c1-4,r1,c1);
    }
    r1 -= ystep;
  }
  VXDrawText("% ",r1 + ystep / 2,c1-25);

  /*
  ** draw histogram
  */
  VXSetLineWidth((double) 2.0);
  r1 = r0;
  c1 = c0;
  for (i=0; i<nvals; i++) {
    r2 = r1 - histo[i] * ystep;
    VXDrawLine(r1,c1,r2,c1);
    c1 += xstep;
  }

  return TRUE;
}

