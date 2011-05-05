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
 * $Id: vderiche3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vderiche3d - 3D Deriche edge detection

\par Description
Perform a Deriche edge detection, optionally followed by
a non-maxima suppression.

\par Reference:
R. Deriche. Fast algorithms for low-level vision.
IEEE Transactions on Pattern Analysis and Machine Intelligence,
1(12):78-88, January 1990.

\par Usage

        <code>vderiche3d</code>

        \param -in     input image
        \param -out    output image
        \param -alpha  Edge parameter. Default: 1
	\param -nonmax Whether non-maximum suppression is performed (true | false). Default: false


\par Examples
<br>

\par Known bugs
none.

\file vderiche3d.c
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


int main (int argc, char **argv)
{
  static VFloat alpha = 1.0;
  static VBoolean flag_nonmax = FALSE;

  static VOptionDescRec options[] = {
    { "alpha", VFloatRepn, 1, &alpha,
      VOptionalOpt, NULL,"Edge parameter" },
    { "nonmax", VBooleanRepn, 1, &flag_nonmax,
      VOptionalOpt, 0,"Whether non-maximum suppression is performed" },
  };

  FILE *in_file, *out_file;
  VAttrList list;
  VImage src=NULL,dest=NULL;
  VImage gradb=NULL,gradr=NULL,gradc=NULL;
  VAttrListPosn posn;
  char prg[50];	
  sprintf(prg,"vderiche3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv,
		   &in_file, &out_file);

  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL))) exit (1);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    VDeriche3d (src,alpha,&gradb,&gradr,&gradc);

    if (flag_nonmax == FALSE) {
      dest = VMagnitude3d(gradb,gradr,gradb,NULL);
    }
    else {
      dest = VNonmaxSuppression(gradb,gradr,gradc,NULL);
    }

    if (dest == NULL) VError(" dest NULL");
    VSetAttrValue (& posn, NULL, VImageRepn, dest);
  }


  VHistory(VNumber(options),options,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done\n",argv[0]);
  return 0;
}



