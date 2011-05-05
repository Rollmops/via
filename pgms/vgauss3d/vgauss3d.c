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
 * $Id: vgauss3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vgauss3d - 3D Gauss filter

\par Description
3D Gauss filter for noise cleaning


\par Usage

        <code>vgauss3d</code>

        \param -in     input image
        \param -out    output image
        \param -sigma  sigma. Default: 1.5


\par Examples
<br>

\par Known bugs
none.

\file vgauss3d.c
\author G.Lohmann, MPI-CBS, 2004
*/





/*
** 3D Gauss filter
**
** G.Lohmann, MPI-CBS
*/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <via.h>



int 
main (int argc,char *argv[])
{  
  static VFloat sigma = 1.5;
  static VOptionDescRec  options[] = {
    {"sigma",VFloatRepn,1,(VPointer) &sigma,VOptionalOpt,NULL,"standard deviation"}
  };
  FILE *in_file,*out_file;
  VAttrList list=NULL;
  VAttrListPosn posn;
  VImage src=NULL,dest=NULL;
  char prg[50];	
  sprintf(prg,"vgauss3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);
  
  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,&out_file);

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);

    dest = VFilterGauss3d (src,NULL,(double)sigma);
    VSetAttrValue (& posn, NULL,VImageRepn,dest);
  }
  if (src == NULL) VError(" no input image found");

  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
