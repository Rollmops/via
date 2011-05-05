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
 * $Id: vinvert.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vinvert - inverts an image

\par Description
 The vinvert program inverts an image (swapping black and white).
 vinvert inverts each input image to produce a corresponding output image. The opera­
 tion is suitable for swapping black and white in a grey-scale image.
<p>
 Each output pixel is computed from the corresponding input pixel by
<p>
 out-pixel = M - in-pixel
<p>
 where M is the maximum value allowed by the pixel representation, or 1 in  the  case
 of float or double pixels. Input pixel values must lie in the range [0,M].
<p>
 Each output image has the same number of bands, rows and columns, and the same pixel
 representation, as the corresponding input image.

\par Usage

        <code>vinvert</code>

        \param -in      input image(s)
        \param -out     output image

\par Examples
<br>
\par Known bugs
none.

\file vinvert.c
\author Arthur Pope, UBC Laboratory for Computational Intelligence
*/



/*
 *
 *  The vinvert program inverts an image (swapping black and white).
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
 *  Author: Arthur Pope, UBC Laboratory for Computational Intelligence
 */

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"



int main (int argc, char *argv[])
{
    FILE *in_file, *out_file;
    VAttrList list;
    VAttrListPosn posn;
    VImage image;
    int nimages = 0;
    char prg[50];	
    sprintf(prg,"vinvert V%s", getVersion());
    fprintf (stderr, "%s\n", prg);

    /* Parse command line arguments and identify the input and output files: */
    VParseFilterCmd (NULL, 0, argc, argv, & in_file, & out_file);

    /* Read the input file: */
    if (! (list = VReadFile (in_file, NULL)))
	exit (EXIT_FAILURE);

    /* Replace each image with its inverse: */
    for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
	if (VGetAttrRepn (& posn) != VImageRepn)
	    continue;
	VGetAttrValue (& posn, NULL, VImageRepn, & image);
	if (! VInvertImage (image, image, VAllBands))
	    exit (EXIT_FAILURE);
	nimages++;
    }

    /* Write the output file: */
    VHistory(0,NULL,prg,&list,&list);
    if (! VWriteFile (out_file, list))
	exit (EXIT_FAILURE);

    fprintf (stderr, "%s: Inverted %d image%s.\n",
	     argv[0], nimages, nimages == 1 ? "" : "s");
    return EXIT_SUCCESS;
}
