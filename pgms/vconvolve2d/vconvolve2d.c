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
 * $Id: vconvolve2d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vconvolve2d -- 2D convolution filter

\par Description
 2D convolution filter, 'vkernel2d' can be used to construct
 a convolution kernel.


\par Usage

        <code>vconvolve2d</code>

        \param -in      input image
        \param -out     output image
        \param -kernel  Convolution kernel. Required. 

\par Examples
<br>

\par Known bugs
none.

\file vconvolve2d.c
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
#include <via.h>


int main (int argc, char **argv)
{
  static VString filename = "";
  static VOptionDescRec options[] = {
    { "kernel", VStringRepn, 1, &filename,
      VRequiredOpt, NULL,"Convolution kernel" }
  };

  FILE *in_file, *out_file, *fp;
  VAttrList list,list1;
  VImage src=NULL,dest=NULL,kernel=NULL;
  VImage xsrc=NULL,xdest=NULL;
  VAttrListPosn posn;
  char prg[50];	
  sprintf(prg,"vconvolve2d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file, &out_file);

  
  fp = VOpenInputFile (filename, TRUE);
  list1 = VReadFile (fp, NULL);
  fclose(fp);

  for (VFirstAttr (list1,&posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & kernel);
    if (VPixelRepn(kernel) != VFloatRepn)
      VError("Convolution kernel must be of type float");
    break;
  }
  if (kernel == NULL) VError(" no kernel found");


  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);


  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    xsrc = VConvertImageCopy(src,NULL,VAllBands,VFloatRepn);
    if (xsrc == NULL) VError(" xsrc= 0");
    xdest = VConvolve2d (xsrc,NULL,kernel);

    if (xdest == NULL) VError(" dest NULL");

    dest = VContrast(xdest,NULL,VUByteRepn,(VFloat) 3.0,(VFloat) 0.001);
    /* dest = VMapImageRange(xdest,NULL,VUByteRepn); */
    VSetAttrValue (& posn, NULL, VImageRepn, dest);

    VDestroyImage(xsrc);
    VDestroyImage(xdest);
  }


  VHistory(VNumber(options),options,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done\n",argv[0]);
  return 0;
}



