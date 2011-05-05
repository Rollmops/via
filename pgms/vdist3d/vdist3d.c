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
 * $Id: vdist3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vdist3d -- 3D distance transform

\par Description
3D distance transform.
For each background voxel, the length of the shortest
3D path to the nearest foreground voxel is computed.
The chamfer distance metric is an approximation to the
Euclidian distance.

\par References
Toyofumi Saito, Jun-Ichiro Toriwaki,
New algorithms for euclidean distance transformation of a n-dimensional
picture with applications,
Pattern Recognition, Vol. 27, No. 11, pp 1551-1565, 1994
<br>
G. Borgefors (1984).
"Distance Transforms in arbitrary dimensions",
CVGIP 27, pp.321-345.

\par Usage

        <code>vdist3d</code>

        \param -in      input image
        \param -out     output image
        \param -repn    Output pixel repn (short | float). Default: float
        \param -metric  type of metric (chamfer | euclidean). Default: euclidean

\par Examples
<br>
\par Known bugs
none.

\file vdist3d.c
\author G.Lohmann, MPI-CBS, 2004
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <via.h>
#include <viaio/option.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>



VDictEntry MetricDict[] = {
  { "euclidean", 0 },
  { "chamfer", 1 },
  { NULL }
};

VDictEntry RepnDict[] = {
  { "short", 0 },
  { "float", 1 },
  { NULL }
};


int main (int argc, char **argv)
{
  static VLong metric = 0;
  static VLong irepn  = 1;
  static VOptionDescRec options[] = {
    { "repn", VLongRepn, 1, &irepn, VOptionalOpt, RepnDict,
      "Output pixel repn (short,float)" },
    { "metric", VLongRepn, 1, &metric, VOptionalOpt, MetricDict,
      "type of metric: chamfer, euclidean" }
  };

  FILE *in_file, *out_file;
  VAttrList list;
  VImage src, dest=NULL;
  VAttrListPosn posn;
  VRepnKind repn;
  char prg[50];	
  sprintf(prg,"vdist3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options,argc,argv,&in_file,&out_file);

  if (irepn == 0) repn = VShortRepn;
  else if (irepn == 1) repn = VFloatRepn;
  else VError(" <repn> must be either 'short' or 'float' ");

  
  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL))) exit (1);

  /* Process */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    if (metric == 0) 
      dest = VEuclideanDist3d(src,NULL,repn);
    else
      dest = VChamferDist3d(src,NULL,repn);

    if (dest == NULL) exit (1);
    VSetAttrValue (& posn, NULL, VImageRepn, dest);
    VDestroyImage(src);
  }


  /* Write the results to the output file: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done.\n",argv[0]);
  return 0;
}

