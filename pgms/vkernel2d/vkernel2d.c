/****************************************************************
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2004, <lipsia@cbs.mpg.de>
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
 * $Id: vkernel2d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vkernel2d - construct convolution kernels

\par Description
construct convolution kernels to be used in 'vconvolve2d'
or 'vconvolve3d'. The kernels can be used for texture analysis
as proposed by Laws et al.

\par Reference
R.M. Haralick, L.G. Shapiro: Computer and Robot Vision. Vol.1.
Addison-Wesley, 1992. (pp. 467-468).

\par Usage

        <code>vkernel2d</code>

        \param -in      input image
        \param -out     output image
        \param -k1      kernel 1 (l3|e3|s3|l5|e5|s5|w5|r5). Default: l3
        \param -k2      kernel 2 (l3|e3|s3|l5|e5|s5|w5|r5). Default: l3


\par Examples
<br>
\par Known bugs
none.

\file vkernel2d.c
\author G.Lohmann, MPI-CBS, 2004
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>


/* 
** laws filter masks (Haralick, p. 468)
*/
float l3[3]={1,2,1};
float e3[3]={-1,0,1};
float s3[3]={-1,2,-1};

float l5[5]={1,4,6,4,1};
float e5[5]={-1,-2,0,2,1};
float s5[5]={-1,0,2,0,-1};
float w5[5]={-1,2,0,-2,1};
float r5[5]={1,-4,6,-4,1};



int main (int argc, char **argv)
{
    static VString k1 = "l3";
    static VString k2 = "l3";
    static VOptionDescRec options[] = {
      {"k1",VStringRepn,1,(VPointer) &k1,
              VOptionalOpt,NULL,"kernel 1 (l3,e3,s3,l5,e5,s5,w5,r5)"},
      {"k2",VStringRepn,1,(VPointer) &k2,
              VOptionalOpt,NULL,"kernel 2 (l3,e3,s3,l5,e5,s5,w5,r5)"}
    };
    FILE *out_file;
    VAttrList list;
    VImage dest=NULL;
    int i,j,n1,n2;
    float x,kernel1[5],kernel2[5];
    char prg[50];	
    sprintf(prg,"vkernel2d V%s", getVersion());
    fprintf (stderr, "%s\n", prg);

    /* Parse command line arguments: */
    VParseFilterCmd (VNumber (options), options, argc, argv,NULL,&out_file);

    
    /* k1 */
    if (strcmp(k1,"l3") == 0) {
      n1 = 3;
      for (i=0; i<n1; i++) kernel1[i] = l3[i];
    }
    else if (strcmp(k1,"e3") == 0) {
      n1 = 3;
      for (i=0; i<n1; i++) kernel1[i] = e3[i];
    }
    else if (strcmp(k1,"s3") == 0) {
      n1 = 3;
      for (i=0; i<n1; i++) kernel1[i] = s3[i];
    }

    else if (strcmp(k1,"l5") == 0) {
      n1 = 5;
      for (i=0; i<n1; i++) kernel1[i] = l5[i];
    }
    else if (strcmp(k1,"e5") == 0) {
      n1 = 5;
      for (i=0; i<n1; i++) kernel1[i] = e5[i];
    }
    else if (strcmp(k1,"s5") == 0) {
      n1 = 5;
      for (i=0; i<n1; i++) kernel1[i] = s5[i];
    }

    else if (strcmp(k1,"w5") == 0) {
      n1 = 5;
      for (i=0; i<n1; i++) kernel1[i] = w5[i];
    }
    else if (strcmp(k1,"r5") == 0) {
      n1 = 5;
      for (i=0; i<n1; i++) kernel1[i] = r5[i];
    }
    else
      VError(" illegal k1");
 
 

    /* k2 */
    if (strcmp(k2,"l3") == 0) {
      n2 = 3;
      for (i=0; i<n2; i++) kernel2[i] = l3[i];
    }
    else if (strcmp(k2,"e3") == 0) {
      n2 = 3;
      for (i=0; i<n2; i++) kernel2[i] = e3[i];
    }
    else if (strcmp(k2,"s3") == 0) {
      n2 = 3;
      for (i=0; i<n2; i++) kernel2[i] = s3[i];
    }

    else if (strcmp(k2,"l5") == 0) {
      n2 = 5;
      for (i=0; i<n2; i++) kernel2[i] = l5[i];
    }
    else if (strcmp(k2,"e5") == 0) {
      n2 = 5;
      for (i=0; i<n2; i++) kernel2[i] = e5[i];
    }
    else if (strcmp(k2,"s5") == 0) {
      n2 = 5;
      for (i=0; i<n2; i++) kernel2[i] = s5[i];
    }

    else if (strcmp(k2,"w5") == 0) {
      n2 = 5;
      for (i=0; i<n2; i++) kernel2[i] = w5[i];
    }
    else if (strcmp(k2,"r5") == 0) {
      n2 = 5;
      for (i=0; i<n2; i++) kernel2[i] = r5[i];
    }
    else
      VError(" illegal k2");
 

    for (i=0; i<n1; i++) fprintf(stderr," %7.2f ",kernel1[i]);
    fprintf(stderr,"\n");
    for (i=0; i<n2; i++) fprintf(stderr," %7.2f ",kernel2[i]);
    fprintf(stderr,"\n\n");


    dest = VCreateImage(1,n1,n2,VFloatRepn);
    VFillImage(dest,VAllBands,0);

    /* make mask */
    for (i=0; i<n1; i++) {
      for (j=0; j<n2; j++) {
	x = kernel1[i] * kernel2[j];
	VPixel(dest,0,i,j,VFloat) = x;
      }
    }

    
    /* Write the results to the output file: */
    list = VCreateAttrList();
    VHistory(VNumber(options),options,prg,&list,&list);
    VAppendAttr(list,"kernel",NULL,VImageRepn,dest);
    if (! VWriteFile (out_file, list)) exit (1);
    fprintf (stderr, "%s: done.\n",argv[0]);
    return 0;
}
