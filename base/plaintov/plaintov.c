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
 * $Id: plaintov.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief plaintov - converts file from Vista plain format to Vista format

\par Description
The plaintov program, which converts file from Vista plain format to Vista format.
Useful for specifying convolution kernels.

Plain file format allows  a  small  image  to  be  created
entirely using a text editor because it represents pixel values in ASCII. In a Vista
data file, those pixel values are encoded in binary.

\par Usage

        <code>plaintov</code>

        \param -in      input image
        \param -out     output image
        \param -name    Specifies  the  name  to  be  given  the  converted  image.

\par Examples
<br>
\par Known bugs
none.

\file plaintov.c
\author Daniel Ko, UBC Laboratory for Computational Intelligence
*/



/*
 *
 *  The plaintov program, which converts file from Vista plain
 *  format to Vista format
 */

/*
 *  Copyright 1993, 1994 University of British Columbia
 *
 *  Permission to use, copy, modify, distribute, and sell this software and its
 *  documentation for any purpose is hereby granted without fee, provided that
 *  the above copyright notice appears in all copies and that both that
 *  copyright notice and this permission notice appear in supporting
 *  documentation. UBC makes no representations about the suitability of this
 *  software for any purpose. It is provided "as is" without express or
 *  implied warranty.
 *
 *  Author: Daniel Ko, UBC Laboratory for Computational Intelligence
 */

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"


/* Command line options: */
static VStringConst object_name = "image";
static VOptionDescRec options[] = {
    { "name", VStringRepn, 1, & object_name, VOptionalOpt, NULL,
	  "Name to be given created image" }
};


/*
 *  Program entry point.
 */

int main (int argc, char **argv)
{
    FILE *in_file, *out_file;
    VImage image;
    VAttrList list = VCreateAttrList ();
    char prg[50];	
    sprintf(prg,"plaintov V%s", getVersion());
    fprintf (stderr, "%s\n", prg);


    VParseFilterCmd (VNumber (options), options, argc, argv,
		     & in_file, & out_file);
    
    if (! (image = VReadPlain (in_file)))
	exit (EXIT_FAILURE);

    VHistory(VNumber(options),options,prg,&list,&list);
    VAppendAttr (list, object_name, NULL, VImageRepn, image);
    if (! VWriteFile (out_file, list))
	exit (EXIT_FAILURE);

    fprintf (stderr, "%s: Converted 1 file from Vista plain format"
	     " to Vista format.\n", argv[0]);

    return EXIT_SUCCESS;
}
