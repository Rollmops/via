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
 * $Id: vcanny3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vcanny3d -- 3D edge detection

\par Description
Perform a 3D Canny edge detction, optionally followed by
 a non-maxima suppression.

\par Reference:
J. Canny (1986). "A computational approach to edge detection",
IEEE-PAMI, Vol.8, No.6. pp.679--698.

\par Usage

	<code>vcanny2d

	\param -in     input image
	\param -out    output image
	\param -dim    size of filter mask. Default: 5
	\param -nonmax  Whether non-maximum suppression is performed [ true | false ]. Default: false

\par Examples

\par Known bugs
none.

\file vcanny3d.c
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
  static VShort dim = 5;
  static VBoolean flag_nonmax = FALSE;

  static VOptionDescRec options[] = {
    { "dim", VShortRepn, 1, &dim,
      VOptionalOpt, NULL,"Size of filter mask" },
    { "nonmax", VBooleanRepn, 1, &flag_nonmax,
      VOptionalOpt, 0,"Whether non-maximum suppression is performed" },
  };

  FILE *in_file, *out_file;
  VAttrList list;
  VImage src=NULL,dest=NULL,xdest=NULL;
  VImage gradb=NULL,gradr=NULL,gradc=NULL;
  VAttrListPosn posn;
  char prg[50];	
  sprintf(prg,"vcanny3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file, &out_file);

  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL))) exit (1);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    VCanny3d (src,(int)dim,&gradb,&gradr,&gradc);

    if (flag_nonmax == FALSE) {
      xdest = VMagnitude3d(gradb,gradr,gradc,NULL);
    }
    else {
      xdest = VNonmaxSuppression(gradb,gradr,gradc,NULL);
    }
    dest  = VContrast(xdest,NULL,VUByteRepn,(VFloat)3.0,(VFloat)0.01);
    VDestroyImage(xdest);

    if (dest == NULL) VError(" dest NULL");
    VSetAttrValue (& posn, NULL, VImageRepn, dest);
  }

  VHistory(VNumber(options),options,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done\n",argv[0]);
  return 0;
}



