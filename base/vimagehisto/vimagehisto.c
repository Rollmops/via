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
 * $Id: vimagehisto.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vimagehisto - display the image histogram.

\par Description
display the image histogram.
input images of any pixel representation are allowed.

\par Usage

        <code>vimagehisto</code>

        \param -in          input image

\par Examples
<br>
\par Known bugs
none.

\file vimagehisto.c
\author G.Lohmann, MPI-CBS, 2004.
*/


/* From the Vista library: */
#include <viaio/VX.h>
#include <viaio/VImageView.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>


/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


extern VBoolean DisplayHistogram(float *,VLong,VRepnKind,double,double,double,double);
extern float *ImageHistogram (VImage,VLong,VLong *,double *,double *,double *,double *);


void QuitCallback (VPointer client_data)
{
  exit (0);
}


int main (int argc, char **argv)
{
  static VStringConst resources[] = {
    "*messageAreaNLines: 2",
    NULL
  };
  static VLong nvals  = 256;
  static VLong ignore = 0;
  FILE *in_file;
  VAttrList list;
  VImage src=NULL;
  VAttrListPosn posn;
  float *histo;
  double xmin,xmax;
  double mean=0,sigma=0;
  char prg[50];	
  sprintf(prg,"vimagehisto V%s", getVersion());
  fprintf (stderr, "%s\n", prg);


  VXInit ("Histogram", resources, & argc, argv);

  /* Parse command line arguments: */
  VParseFilterCmd (0,NULL, argc, argv,&in_file, NULL);

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  /* Process : */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    histo = ImageHistogram (src,ignore,&nvals,&xmin,&xmax,&mean,&sigma);
    DisplayHistogram(histo,nvals,VPixelRepn(src),xmin,xmax,mean,sigma);
    break;
  }
  VXAddMenu ("Quit", "Quit", "<Key>Q", QuitCallback, NULL,NULL);

  VXAppMainLoop ();
  exit(0);
}


