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
 * $Id: vscale2d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vscale2d - 2D scaling

\par Description
2D scaling.

\par References
Michael J. Aramini.
"Efficient image magnification by bicubic spline interpolation".
http://members.bellatlantic.net/~vze2vrva/

\par Usage

        <code>vscale2d</code>

        \param -in      input image
        \param -out     output image
        \param -type    interpolation method (0:bilinear, 1:bicubic). Default: 0
        \param -zoom    zoom factor. Default: 2

\par Examples
<br>
\par Known bugs
none.

\file vscale2d.c
\author G.Lohmann, MPI-CBS, 2004
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
  static VShort type = 0;
  static VFloat zoom = 2.0;
  static VOptionDescRec  options[] = {
    {"type",VShortRepn,1,(VPointer) &type,VOptionalOpt,NULL,
     "interpolation method (0:bilinear, 1:bicubic)"},
    {"zoom",VFloatRepn,1,(VPointer) &zoom,VOptionalOpt,NULL,"zoom"}
  };
  FILE *in_file,*out_file;
  VAttrList list=NULL;
  VAttrListPosn posn;
  VImage src=NULL,dest=NULL;
  int dest_nrows,dest_ncols;
  float shift[2],scale[2];
  char prg[50];	
  sprintf(prg,"vscale2d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,&out_file);

  if (zoom <= 0) VError(" zoom must be positive");

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);

    switch (type) {
    case 0:
      shift[0] = shift[1] = 0;
      scale[0] = scale[1] = zoom;
      dest_nrows = (int) ((float)VImageNRows(src) * zoom + 0.5);
      dest_ncols = (int) ((float)VImageNColumns(src) * zoom + 0.5);
      dest = VBiLinearScale2d (src,dest,dest_nrows,dest_ncols,shift,scale);
      break;
    case 1:
      dest = VBicubicScale2d(src,dest,zoom);
      break;
    default:
      VError("illegal type");
    }
    VSetAttrValue (& posn, NULL,VImageRepn,dest);
  }

  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
