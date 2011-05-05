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
 * $Id: vcurvature.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vcurvature -- 3d curvature computations

\par Description
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

\par Usage

        <code>vcurvature</code>

        \param -in      input image
        \param -out     output image
        \param -t       iso-value. Default: 0.5
        \param -ctype   type of output (class | mean | gauss). Default: class
        \param -border  Whether to process only border voxels (true | false)

\par Examples

\par Known bugs
none.

\file vcurvature.c
\author G.Lohmann, MPI-CBS, 2004
*/



/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <via.h>


VDictEntry TYPDict[] = {
  { "class", 0 },
  { "mean",  1 },
  { "gauss", 2 },
  { "sum",   3 },
  { NULL }
};

int main (int argc, char **argv)
{
  static VFloat threshold = 0.5;
  static VLong type = 0;
  static VBoolean border = TRUE;
  static VOptionDescRec options[] = {
    { "t", VFloatRepn, 1,(VPointer) & threshold,
      VOptionalOpt, NULL,"iso-value" },
    { "border", VBooleanRepn, 1,(VPointer) & border,
      VOptionalOpt, NULL,"Whether to process only border voxels" },
    { "ctype", VLongRepn, 1,(VPointer) & type,
      VOptionalOpt, TYPDict,"type of output: class,mean,gauss" }
  };

  FILE *in_file, *out_file;
  VAttrList list;
  VImage src=NULL, dest=NULL;
  VAttrListPosn posn;
  char prg[50];	
  sprintf(prg,"vcurvature V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file, &out_file);

  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL))) exit (1);

  /* Process each image: */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    dest = VCurvature (src,NULL,threshold,type,border);
    if (dest == NULL) exit (1);
    VSetAttrValue (& posn, NULL, VImageRepn, dest);
  }

  /* Write the results to the output file: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done.\n",argv[0]);
  return 0;
}
