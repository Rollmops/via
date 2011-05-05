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
 * $Id: vimage2volumes.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vimage2volumes - convert a raster image to volume representation

\par Description
vimage2volumes - convert a raster image to volume representation

\par Reference
The volume respresentation is described in:<br>
G. Lohmann (1998). "Volumetric Image Analysis",
John Wiley & Sons, Chichester, England.

\par Usage

        <code>vimage2volumes</code>

        \param -in      input raster image
        \param -out     output volume set

\par Examples
<br>
\par Known bugs
none.

\file vimage2volumes.c
\author G.Lohmann, MPI-CBS, 2004
*/



/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>



int 
main (int argc, char *argv[])
{
  FILE *in_file, *out_file;
  VAttrList list;
  VImage src=NULL;
  Volumes dest=NULL;
  VAttrListPosn posn;
  char prg[50];	
  sprintf(prg,"vimage2volumes V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (0,NULL, argc, argv, &in_file, &out_file);

  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL)))  exit (1);

  /* Performs conversion on each image: */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn)  continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    dest = VImage2Volumes (src);

    if (dest == NULL) VError("Error: dst=0");
    VSetAttrValue (& posn, NULL, VolumesRepn, dest);
  }

  /* Write the results to the output file: */
  VHistory(0,NULL,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done.\n",argv[0]);
  return 0;
}
