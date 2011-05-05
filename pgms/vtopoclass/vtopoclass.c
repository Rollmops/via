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
 * $Id: vtopoclass.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vtopoclass - topological classification

\par Description

Each foreground voxel of a 3D binary input image receives a
label indicating its topological type. Possible types are
the following:

Output codes:
<ul>
<li>  1:  interior point
<li>  2:  isolated point
<li>  3:  border point
<li>  4:  curve point
<li>  5:  curves junction
<li>  6:  surface point
<li>  7:  surface/curve junction
<li>  8:  surfaces junction
<li>  9:  surfaces/curve junction
</ul>

\par Reference
G. Malandain, G. Bertrand, N. Ayache:
"Topological Segmentation of discrete surfaces",
Intern. Journal of Computer Vision, 10:2, pp. 183-197, 1993

\par Usage

        <code>vtopoclass</code>

        \param -in      input image
        \param -out     output image

\par Examples
<br>
\par Known bugs
none.

\file vtopoclass.c
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


int main (int argc, char **argv)
{
  FILE *in_file, *out_file;
  VAttrList list;
  VImage src, dest;
  VAttrListPosn posn;
  char prg[50];	
  sprintf(prg,"vtopoclass V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (0,NULL, argc, argv,&in_file, &out_file);
  
  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL)))  exit (1);

  /* Performs Median filtering on each image: */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    dest = VTopoclass (src, NULL);
    if (dest == NULL) exit (1);

    VSetAttrValue (& posn, NULL, VImageRepn, dest);
    VDestroyImage (src);
  }

  /* Write the results to the output file: */
  VHistory(0,NULL,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done.\n",argv[0]);
  return 0;
}
