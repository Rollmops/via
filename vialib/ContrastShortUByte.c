/****************************************************************
 *
 * ContrastShortUByte.c
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Thomas Arnold, 2002, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * $Id: ContrastShortUByte.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/option.h>
#include <viaio/mu.h>



/*------------------------------------------------------------------------------

LinearContrast
==============

Src     source image with voxels of type T
Min     minimum value of contrasted image
Max     maximum value of contrasted image
Dest    contrasted image with voxels of type T (created)
Black   amount of lower histogram that is set to black (0 <= Black <= 1, default 0.01)
White   amount of upper histogram that is set to white (0 <= White <= 1, default 0.01)

Note: This function returns an error message for images of type VLong, VFloat
      and VDouble. These types require a sparse histogram, which is not yet
      implemented.

------------------------------------------------------------------------------*/

VImage 
VContrastShortUByte (VImage Src, int Min, int Max, float Black, float White)
{

   VImage Dest = NULL;    /* destination image     */
   int Voxels;            /* number of voxels      */
   long  min, max;        /* min/max voxel values  */
   long  size;            /* size of histogram     */
   float* hist;           /* image histogram       */
   long  lower, upper;    /* histogram borders     */
   float scale;           /* linear scaling factor */
   float xmin,xmax,sum1,sum2,ave,sigma,nx,sum,u,v,slope;
   float background = 1;
   int   nbands,nrows,ncols;
   float sig2,alpha,norm,sumw,w,d;
   int   i,j,k,ws;
   float *histo;
   float *lut;
   VString str;
   int type=0;  /* contrast stretching method: 0=linear, 1=histo equalization */


   VShort *src;     /* source data pointer      */
   VUByte *dest;    /* destination data pointer */
   float value;     /* voxel value              */

   long n;   /* index */


   /* check image representation */
   if (VPixelRepn (Src) != VShortRepn)
   {
      VError ("LinearContrast: Source image doesnt have VShortRepn");
      return;
   }

   /* get source image size */
   Voxels = VImageNPixels (Src);

   /* create contrasted image */
   Dest = VCreateImage (VImageNBands (Src), VImageNRows (Src), VImageNColumns (Src), VUByteRepn);
   VImageAttrList (Dest) = VCopyAttrList (VImageAttrList (Src));

   /* create histogram */
   min  = (long) VPixelMinValue (Src);
   max  = (long) VPixelMaxValue (Src);
   size = (long) (VPixelMaxValue (Src) - VPixelMinValue (Src) + 1);
   hist = (float *)VMalloc(sizeof(float)*size);
   hist -= min;

   histo = (float *)VMalloc(sizeof(float)*size);
   histo -= min;
   lut   = (float *)VMalloc(sizeof(float)*size);
   lut -= min;

   /*
    * estimate background grey values, assume that background is dark
    */
   ncols  = VImageNColumns(Src);
   nrows  = VImageNRows(Src);
   nbands = VImageNBands(Src);

   sum1 = sum2 = nx = 0;
   n = 12;
   if (ncols < n) n=ncols/2;
   if (nrows < n) n=nrows/2;
   for (i=1; i<n; i++) {
     for (j=1; j<n; j++) {
       value = VGetPixel(Src,0,i,j);
       sum1 += value;
       sum2 += (value*value);
       nx++;
     }
   }
   ave = sum1/nx;
   sigma = sqrt((double)((sum2 - nx * ave * ave) / (nx - 1.0)));
   background = ave + 2.0*sigma;

   /*
    * get image histogram
    */
   for (n = min; n <= max; n++) hist[n] = histo[n] = lut[n] = 0;
   src = VPixelPtr (Src, 0, 0, 0);
   for (n = 0; n < Voxels; n++) {
     j = *(src++);
     if (type == 1 && j > background) hist[j]++;
     if (type == 0) hist[j]++;
   }
   
   /*
    * histogram equalization with Gaussian blurring of the histogram
    */
   sigma = 90;
   ws    = (int)(2.5*sigma);
   sig2  = 2.0 * sigma * sigma;
    
   for (j=min; j<=max; j++) {
     sum = sumw = 0;
     for (k=j-ws; k<=j+ws; k++) {
       if (k < min || k >= max) continue;
       d = j-k;
       w =  exp((double)(- d*d/sig2));
       sum  += w*hist[k];
       sumw += w;
     }
     if (sumw > 0 && j > background) histo[j] = sum / sumw;
   }

   sum = 0;
   for (j=min; j<=max; j++) sum += histo[j];
   for (j=min; j<=max; j++) histo[j] /= sum;
  
   for (i=min; i<=max; i++) {
     sum = 0;
     for (j=min; j<=i; j++) sum += histo[j];
     if (sum < 0) sum = 0;
     if (sum > 1) sum = 1;
     lut[i] = sum * 255.0f;
   }

   sum = 0;
   for (n=min; n<=max; n++) sum += hist[n];
   for (n=min; n<=max; n++) hist[n] /= sum;

   xmin = 0;
   sum  = 0;
   for (n=min; n<max; n++) {
     if (n > background) sum += hist[n];
     if (sum > Black) break;
   }

   xmin = n;
   xmax = 255.0;
   sum = 0;
   for (n=max; n>min; n--) {
     if (n > background) sum += hist[n];
     if (sum > White) break;
   }
   xmax = n; 
   slope = 255.0 / (xmax - xmin);

   /*
    * get transfer function
    * histo equalization plus linear contrast stretch
    */
   for (n=min; n<max; n++) {
     value = n;
     v = slope * (value - xmin);
     u = lut[n];
     lut[n] = (double)(0.0*u  + 1.0*v);
   }

   /*
    * apply contrast
    */
   src  = VPixelPtr (Src,  0, 0, 0);
   dest = VPixelPtr (Dest, 0, 0, 0);
   for (n = 0; n < Voxels; n++)
     {
       value = (float) *(src++);
       j = (int) value;
       v = ((double)lut[j]);

       j = (int)(v + 0.5);
       if (j < 0)    j = 0;
       if (j > 255)  j = 255;
       *(dest++) = (VUByte)j;
     }

   /* clean-up */
   hist += min;
   VFree(hist);

   /* return Dest */
   return Dest; 

} /* LinearContrast */
