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
 * $Id: vdelsmall.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vdelsmall -- delete small components in a labelled image

\par Description
delete small components in a labelled image.
The input image must be of type "ubyte" or "short".
The output image will be of type "bit".

\par Usage

        <code>vdelsmall</code>

        \param -in     input image
        \param -out    output image
        \param -msize  Components of size less than <msize> are removed. Default: 10

\par Examples

\par Known bugs
none.

\file vdelsmall.c
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



int main (int argc, char **argv)
{  
  static VShort msize = 3;
  static VOptionDescRec  options[] = {
    {"msize",VShortRepn,1,(VPointer) &msize,VOptionalOpt,NULL,"size threshold"},
  };
  FILE *in_file, *out_file;
  VAttrList list;
  VImage src=NULL, dest=NULL;
  VAttrListPosn posn;
  char prg[50];	
  sprintf(prg,"vdelsmall V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,&out_file);


  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);


  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);

    dest = VDeleteSmall (src,NULL,(int)msize);
    if (dest == NULL) VError("destination image is NULL");

    VSetAttrValue (& posn, NULL, VImageRepn, dest);
    VDestroyImage (src);
  }

  /* Write the results to the output file: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done .\n",argv[0]);
  exit(0);
}


