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
 * $Id: vistat.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/* From the V library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))


VImage
VImStats(VImage src)
{
  int b,r,c,n,i;
  double v,sum1,sum2,nx,xmin,xmax,tiny=1.0e-6;
  double mean,sigma;

  xmin = VRepnMaxValue(VDoubleRepn);
  xmax = VRepnMinValue(VDoubleRepn);

  sum1 = sum2 = nx = 0;
  for (b=0; b<VImageNBands(src); b++) {
    for (r=0; r<VImageNRows(src); r++) {
      for (c=0; c<VImageNColumns(src); c++) {
	v = VGetPixel(src,b,r,c);
	if (v < xmin) xmin = v;
	if (v > xmax) xmax = v;
	if (ABS(v) < tiny) continue;
	sum1 += v;
	sum2 += v*v;
	nx++;
      }
    }
  }
  if (nx < 1) VError(" no non-zero voxels found");
  mean = sum1/nx;
  sigma = sqrt((sum2 - nx * mean * mean) / (nx - 1.0));

  
  fprintf(stderr,"\n image stats:\n");
  fprintf(stderr," min = %g, max= %g\n",xmin,xmax);
  fprintf(stderr," mean= %g, std= %g\n\n",mean,sigma);
}




int main (int argc, char **argv)
{
  FILE *in_file;
  VAttrList list;
  VAttrListPosn posn;
  VImage image;
  char prg[50];	
  sprintf(prg,"vistat V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments and identify the input and output files: */
  VParseFilterCmd (NULL, 0, argc, argv, & in_file,NULL);

  /* Read the input file: */
  if (! (list = VReadFile (in_file, NULL)))
    exit (EXIT_FAILURE);

  /* For each image in the input file: */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & image);
    VImStats(image);
  }
}
