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
 * $Id: vcontrast.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vcontrast -- contrast enhancement

\par Description
 contrast enhancement.


\par Usage

        <code>vcontrast

        \param -in     input image
        \param -out    output image
        \param -type   Contrast stretching method 
                       (0: any repn, 1: ubyte only, 2: map to range). Default: 0
        \param -alpha  Contrast stretching factor. Default: 2
        \param -background  Background threshold. Default: 0.01

\par Examples

\par Known bugs
none.

\file vcontrast.c
\author G.Lohmann, MPI-CBS, 2004
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <via.h>

VDictEntry OPDict[] = {
  { "any", 0 },
  { "all", 1 },
  { "ubyte", 2 },
  { "short", 3 },
  { "range", 4 },
  { "equalize", 5 },
  { NULL }
};

extern VImage VHistoEqualize(VImage,VImage,VFloat);
extern VImage VContrastShort(VImage,VImage,VFloat,VFloat);
extern VImage VContrastAny(VImage,VImage,VFloat,VFloat);


int main (int argc, char **argv)
{
  static VShort type   = 1;
  static VFloat low    = 0.01;
  static VFloat high   = 0.01;
  static VFloat alpha  = 3;
  static VFloat beta   = 4;
  static VFloat background = 0.1;
  static VOptionDescRec options[] = {
    { "type", VShortRepn, 1, &type,VOptionalOpt, OPDict,"Contrast stretching method" },
    { "low", VFloatRepn, 1, &low,VOptionalOpt, NULL,"Lower threshold " },
    { "high", VFloatRepn, 1, &high,VOptionalOpt, NULL,"Upper threshold" },
    { "alpha", VFloatRepn, 1, &alpha,VOptionalOpt, NULL,"factor " },
    { "background", VFloatRepn, 1, &background,VOptionalOpt, NULL,"background threshold" },
    { "beta", VFloatRepn, 1, &beta,VOptionalOpt, NULL,"parameter for histogram equalization" }
  };

  FILE *in_file, *out_file;
  VAttrList list;
  VImage src=NULL,dest=NULL;
  VAttrListPosn posn;
  char prg[50];	
  sprintf(prg,"vcontrast V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv, &in_file, &out_file);

  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL))) exit (1);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    switch (type) {
    case 0:
      dest = VContrast(src,NULL,VUByteRepn,alpha,background);
      break;

    case 1:
      dest = VContrastAny(src,NULL,low,high);
      break;

    case 2:
      dest = VContrastUByte(src,NULL,low,high);
      break;

    case 3:
      dest = VContrastShort(src,NULL,low,high);
      break;

    case 4:
      dest = VMapImageRange(src,NULL,VUByteRepn); 
      break;

    case 5:
      dest = VHistoEqualize(src,NULL,beta);
      break;

    default:
      VError(" illegal type");
    }

    if (dest == NULL) VError(" dest NULL");
    VSetAttrValue (& posn, NULL, VImageRepn, dest);
  }


  VHistory(VNumber(options),options,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done\n",argv[0]);
  return 0;
}



