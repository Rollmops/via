/****************************************************************
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
 * $Id: vselbands.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vselbands -- select slices from an image.

\par Description
vselbands copies selected slices from its input image to produce a corresponding
output image.

\par Usage

        <code>vselbands</code>

        \param -in      input image
        \param -out     output image
        \param -first   First slice to be selected. Default: 0
        \param -last    Last slice to be selected. If set to '-1' then
                        the last slice in the image is taken


\par Examples
<p>
Select the first 10 slices from an image:
<br>
<code>vselbands -in image.v -out result.v -first 0 -last 9</code>
<br>
<p>
Select the 5th slice from an image:
<br>
<code>vselbands -in image.v -out result.v -first 4 -last 4</code>
<br>
<p>
Select all slices except the first two slices:
<br>
<code>vselbands -in image.v -out result.v -first 2 -last -1</code>
<br>


\par Known bugs
none.

\file vselbands.c
\author originally by Arthur Pope (UBC), rewritten by G.Lohmann (MPI-CBS).
*/





/*
 *  Program: vselbands
 *
 *  Select slices from an image. This program replaces
 *  an older version of 'vselbands' which sometimes
 *  produced errors.
 *
 *  G.Lohmann, MPI-CBS
 */

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>

/* From the standard C library: */
#include <stdio.h>

/* Later in this file: */
extern VImage VSelSlices (VImage,VImage,VShort,VShort);


int 
main (int argc,char *argv[])
{
  static VShort first = 0;
  static VShort last = -1;
  static VOptionDescRec options[] = {
    { "first", VShortRepn, 1, (VPointer) & first,
      VOptionalOpt, NULL, "First slice" },
    { "last", VShortRepn, 1, (VPointer) & last,
      VOptionalOpt, NULL, "Last slice " }
  };

  FILE *in_file, *out_file;
  VAttrList list;
  VAttrListPosn posn;
  VImage src=NULL, result=NULL;
  char prg[50];	
  sprintf(prg,"vselbands V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments and identify files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,& in_file, & out_file);


  /* Read the input file: */
  list = VReadFile (in_file, NULL);
  if (! list) exit (1);


  /* process */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {

    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);
    result = VSelSlices (src, NULL,first,last);
    if (! result) exit (1);
    VSetAttrValue (& posn, NULL, VImageRepn, result);
    VDestroyImage (src);
  }


  /* Write out the results: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}

