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
 * $Id: vflip.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vflip - performs one or more flip operations

\par Description
vflip flips images horizontally and/or vertically.
<p>
A  horizontal  flip  exchanges the left and right sides of an image. A vertical flip
exchanges the top and bottom.

\par Usage

        <code>vflip</code>

        \param -in          input image
        \param -out         output image
        \param -horizontal  Flip horizontally. Default: false
        \param -vertical    Flip vertically. Default: false

\par Examples
<br>
\par Known bugs
none.

\file vflip.c
\author Daniel Ko, UBC Laboratory for Computational Intelligence
*/


/*
 *  The vflip program, which performs one or more flip operations
 *  on Vista images.
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



int main (int argc, char **argv)
{
    static VBoolean horizontal_flip = FALSE, vertical_flip = FALSE;
    static VOptionDescRec options[] = {
	{ "horizontal", VBooleanRepn, 1, & horizontal_flip, VOptionalOpt, NULL,
	      "Flip horizontally" },
	{ "vertical", VBooleanRepn, 1, & vertical_flip, VOptionalOpt, NULL,
	      "Flip vertically" }
    };
    FILE *in_file, *out_file;
    VAttrList list;
    VAttrListPosn posn;
    VImage image;
    int nimages = 0;
    char prg[50];	
    sprintf(prg,"vflip V%s", getVersion());
    fprintf (stderr, "%s\n", prg);


    /* Parse command line arguments: */
    VParseFilterCmd (VNumber (options), options, argc, argv,
		     & in_file, & out_file);

    /* Default to horizontal flip if no flipping is specified: */
    if (! horizontal_flip && ! vertical_flip)
	horizontal_flip = TRUE;

    /* Read source image(s): */
    if (! (list = VReadFile (in_file, NULL)))
	exit (EXIT_FAILURE);
    
    /* Perform flip operations on each image: */
    for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
	
	if (VGetAttrRepn (& posn) != VImageRepn)
	    continue;
	VGetAttrValue (& posn, NULL, VImageRepn, & image);
	
	/* First flip horizontal: */
	if (horizontal_flip && ! VFlipImage (image, image, VAllBands, FALSE))
	    exit (EXIT_FAILURE);
	
	/* Then flip vertical: */
	if (vertical_flip && ! VFlipImage (image, image, VAllBands, TRUE))
	    exit (EXIT_FAILURE);

	nimages++;
    }

    /* Write the results to the output file: */
    if (! VWriteFile (out_file, list))
	exit (EXIT_FAILURE);

    fprintf (stderr, "%s: Flipped %d image(s) %s%s%s.\n",
	     argv[0], nimages,
	     horizontal_flip ? "horizontally" : "",
	     (horizontal_flip && vertical_flip) ? " and " : "",
	     vertical_flip ? "vertically" : "");

    return EXIT_SUCCESS;
}
