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
 * $Id: vskel2d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vskel2d - 2D skeletonization

\par Description
2D skeletonization. The inpout image must be binary.

\par Reference
A.Manzanera,T.Bernard,F.Preteux,B.Longuet: 
"nD skeletonization: a unified mathematical framework",
Journal of Electronic Engineering, Vol. 11-1, Jan. 2002, pp.25-37.

\par Usage

        <code>vskel2d</code>

        \param -in      input image
        \param -out     output image

\par Examples
<br>
\par Known bugs
none.

\file vskel2d.c
\author G.Lohmann, MPI-CBS, 2004
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via.h>



int main (int argc,char *argv[])
{
  FILE *in_file, *out_file;
  VAttrList list;
  VAttrListPosn posn;
  VImage src=NULL, dest=NULL;
  char prg[50];	
  sprintf(prg,"vskel2d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments and identify files: */
  VParseFilterCmd (0,NULL, argc, argv, & in_file, & out_file);
  

  /* Read the input file: */
  list = VReadFile (in_file, NULL);
  if (! list) exit (1);
  fclose(in_file);


  /* For each attribute read... */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {

    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    dest = VSkel2d(src,NULL);
    if (! dest) exit (1);

    VSetAttrValue (& posn, NULL, VImageRepn, dest);
    VDestroyImage (src);
  }
  

  /* output */
  VHistory(0,NULL,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);  
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
