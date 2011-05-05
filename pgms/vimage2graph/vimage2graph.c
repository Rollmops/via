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
 * $Id: vimage2graph.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vimage2graph - convert a raster image to graph representation

\par Description
The input image must be a Vista raster image of arbitrary pixel
representation. The output will be a graph where each pixel
is representated by one node. The pixel value will be attached
to the node. 


\par Usage

        <code>vimage2graph</code>

        \param -in      input image
        \param -out     output graph
        \param -min     minimal voxel value (voxel below this value are not
                        converted). Default: -32000
        \param -max     maximal voxel value (voxel above this value are not
                        converted). Default: 32000
        \param -link    Whether to link nodes in graph (true | false). 
                        If set to 'true' then 26-adjacent voxels will be linked
                        by an edge. Default: true


\par Examples
<br>
\par Known bugs
none.

\file vimage2graph.c
\author G.Lohmann, MPI-CBS, 2004
*/


#include <stdio.h>
#include <stdlib.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via.h>



int main (int argc,char *argv[])
{
  static VDouble xmin = -32000;
  static VDouble xmax = 32000;
  static VBoolean link = TRUE;
  static VOptionDescRec options[] = {
    { "min", VDoubleRepn, 1, & xmin, VOptionalOpt, NULL, "min" },
    { "max", VDoubleRepn, 1, & xmax, VOptionalOpt, NULL, "max" },
    { "link", VBooleanRepn, 1, & link, VOptionalOpt, NULL,
      "Whether to link nodes in graph" },
  };
  FILE *in_file, *out_file;
  VAttrList list;
  VAttrListPosn posn;
  VImage src=NULL;
  VGraph dest=NULL;
  char prg[50];	
  sprintf(prg,"vimage2graph V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments and identify files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,
		   & in_file, & out_file);
  
  /* Read the input file: */
  list = VReadFile (in_file, NULL);
  if (! list) exit (1);
  
  /* For each attribute read... */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {

    if (VGetAttrRepn (& posn) != VImageRepn)  continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    dest = VImage2Graph (src,NULL,xmin,xmax,link,TRUE);
    if (! dest) VError(" vimage2graph failed");
   
    VSetAttrValue (& posn, NULL, VGraphRepn, dest);
    VDestroyImage(src);
    break;
  }
  
  /* Write out the results: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
