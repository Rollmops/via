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
 * $Id: vquickmorph3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vquickmorph3d - 3D morphological operators using distance transforms

\par Description
3D morphological operators using distance transforms.

\par Usage

        <code>vquickmorph3d</code>

        \param -in     input image
        \param -out    output image
        \param -op     type of operation (dilate | erode | open | close). Default: dilate
        \param -radius radius of spherical structuring element. Default: 3


\par Examples
<br>
\par Known bugs
none.

\file vquickmorph3d.c
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


VDictEntry OPDict[] = {
  { "dilate", 0 },
  { "erode", 1 },
  { "open", 2 },
  { "close", 3 },
  { NULL }
};


int main (int argc, char **argv)
{
  static VDouble radius = 3.0;
  static VLong op = 0;
  static VOptionDescRec options[] = {
    { "op", VLongRepn, 1, &op, VOptionalOpt, OPDict,"type of operation" },
    { "radius", VDoubleRepn, 1, & radius, VOptionalOpt, NULL,
      "radius of structuring sphere" }
  };
  FILE *in_file, *out_file;
  VAttrList list;
  VImage src, dest=NULL;
  VAttrListPosn posn;
  char prg[50];	
  sprintf(prg,"vquickmorph3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv, &in_file, &out_file);


  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);


  /* Perform morphological filtering */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn)
      continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    switch (op) {
    case 0:
      dest = VDTDilate(src,NULL,radius);
      if (dest == NULL) exit (1);
      break;

    case 1:
      dest = VDTErode(src,NULL,radius);
      if (dest == NULL) exit (1);
      break;

    case 2:
      dest = VDTOpen(src,NULL,radius);
      if (dest == NULL) exit (1);
      break;

    case 3:
      dest = VDTClose(src,NULL,radius);
      if (dest == NULL) exit (1);
      break;

    default:
      ;
    }

    VSetAttrValue (& posn, NULL, VImageRepn, dest);
    VDestroyImage (src);
    break;
  }

  /* Write the results to the output file: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done.\n",argv[0]);
  return 0;
}

