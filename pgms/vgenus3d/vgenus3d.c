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
 * $Id: vgenus3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vgenus3d - 3D Euler characteristic 

\par Description
Compute the 3D Euler characteristic of a 3D binary raster image.

\par Reference:
 C.N. Lee, A. Rosenfeld (1987).
 ICCV 1987, London, pp.567-571.

\par Usage

        <code>vgenus3d</code>

        \param -in     input image
        \param -n      neighbourhood system: 6 or 26. Default: 26

\par Examples
<br>

\par Known bugs
none.

\file vgenus3d.c
\author G.Lohmann, MPI-CBS, 2004
*/



/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via.h>

/* From the standard C library: */
#include <stdio.h>
#include <stdlib.h>



int main (int argc,char *argv[])
{
  static VShort neighb = 26;
  static VOptionDescRec  options[] = {
    {"n",VShortRepn,1,(VPointer) &neighb,
       VOptionalOpt,NULL,"neighbourhood system: 6 or 26"},
  };
  FILE *in_file;
  VAttrList list;
  VAttrListPosn posn;
  VImage src;
  int euler = 0;
  char prg[50];	
  sprintf(prg,"vgenus3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  
  /* Parse command line arguments and identify files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,& in_file, NULL);
  
  /* Read the input file: */
  list = VReadFile (in_file, NULL);
  if (! list) exit (1);
 
  /* For each attribute read... */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {

    if (VGetAttrRepn (& posn) != VImageRepn)
      continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    euler = VGenusLee(src, neighb);

    fprintf(stderr," euler number: %d \n",euler);
  }
  return 0;
}


