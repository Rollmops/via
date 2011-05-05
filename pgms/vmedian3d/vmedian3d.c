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
 * $Id: vmedian3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vmedian3d - 3D median filter

\par Description
 3D median filter

\par Usage

        <code>vmedian3d</code>

        \param -in      input image
        \param -out     output image
        \param -dim     kernel size (must be an odd integer). Default: 3

\par Examples
<br>
\par Known bugs
none.

\file vmedian3d.c
\author G.Lohmann, MPI-CBS, 2004
*/


#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>

extern VImage VMedianImage3d (VImage,VImage,int,VBoolean);



int 
main (int argc,char *argv[])
{  
  static VShort dim = 3;
  static VBoolean ignore = TRUE;
  static VOptionDescRec  options[] = {
    {"dim",VShortRepn,1,(VPointer) &dim,VOptionalOpt,NULL,"kernel dim"},
    {"ignore",VBooleanRepn,1,(VPointer) &ignore,VOptionalOpt,NULL,
        "whether to ignore zero voxels"}
  };
  FILE *in_file,*out_file;
  VAttrList list=NULL;
  VAttrListPosn posn;
  VImage src=NULL,dest=NULL;
  char prg[50];	
  sprintf(prg,"vmedian3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);
  
  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,&out_file);
  if (dim < 3) VError(" <dim> must be >= 3");
  if (dim %2 == 0) VError(" <dim> must be odd");

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    if (VImageNBands(src) < dim) 
      VError("Input image has fewer slices (%d) than <dim> (%d)",VImageNBands(src),dim);

    dest = VMedianImage3d (src,NULL,(int)dim,ignore);
    VSetAttrValue (& posn, NULL,VImageRepn,dest);
  }
  if (src == NULL) VError(" no input image found");


  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
