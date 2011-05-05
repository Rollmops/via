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
 * $Id: vcrop.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vcrop - crops an image

\par Description
vcrop  crops  a  file of images using a cropping rectangle specified by command line
options.
<p>
The cropping rectangle must be unambiguously specified by using  any  two  of  -top,
-bottom, and -height, plus any two of -left, -right, and -width.
<p>
The cropping rectangle may fall partially or completely outside an image it is crop­
ping; in that case, the area outside is filled with zeros.

\par Usage

        <code>vcrop</code>

        \param -in      input image
        \param -out     output image
        \param -top     Specifies the first (top) row of the cropping rectangle.
        \param -bottom  Specifies the last (bottom) row of the cropping rectangle.
        \param -left    Specifies the first (leftmost) column of the cropping rectangle.
        \param -right   Specifies the last (rightmost) column of the cropping rectangle.
        \param -height  Specifies the height of the cropping rectangle.
        \param -width   Specifies the width of the cropping rectangle.

\par Examples
<br>
\par Known bugs
none.

\file vcrop.c
\author Ralph Horstmann, UBC Laboratory for Computational Intelligence
*/



/*
 * 
 * The vcrop program crops an image an creates a new image.
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
 *  Author: Ralph Horstmann, UBC Laboratory for Computational Intelligence
 */

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"


int main(int argc, char *argv[])
{
    static VLong left, right, top, bottom, height, width;
    static VBoolean top_found, bottom_found, height_found;
    static VBoolean left_found, right_found, width_found;
    static VOptionDescRec  options[] = {
	{"left", VLongRepn, 1, & left, & left_found, NULL,
	     "First (leftmost) column of cropped region" },
	{"right", VLongRepn, 1, & right, & right_found, NULL,
	     "Last (rightmost) column of cropped region" },
	{"width", VLongRepn, 1, & width, & width_found, NULL,
	     "Width of cropped region in columns" },
	{"top", VLongRepn, 1, & top, & top_found, NULL,
	     "First (top) row of cropped region" },
	{"bottom", VLongRepn, 1, & bottom, & bottom_found, NULL,
	     "Last (bottom) row of cropped region"},
	{"height", VLongRepn, 1, & height, & height_found, NULL,
	     "Height of cropped region in rows"}
    };
    FILE *in_file, *out_file;
    VAttrList list;
    VAttrListPosn posn;
    VImage src, result;
    int nimages = 0;
    char prg[50];	
    sprintf(prg,"vcrop V%s", getVersion());
    fprintf (stderr, "%s\n", prg);


    /* Parse command line arguments: */
    VParseFilterCmd (VNumber (options), options, argc, argv,
		     & in_file, & out_file);
    
    /* Require exactly two of -top, -bottom, -height: */
    if (top_found + bottom_found + height_found != 2)
	VError ("Specify two of -top, -bottom, and -height");
    if (! top_found)
	top = bottom - height + 1;
    else if (! height_found)
	height = bottom - top + 1;
    if (height <= 0)
	VError ("Height of cropped region must be positive");
	
    /* Require exactly two of -left, -right, -width: */
    if (left_found + right_found + width_found != 2)
	VError ("Specify two of -left, -right, and -width");
    if (! left_found)
	left = right - width + 1;
    else if (! width_found)
	width = right - left + 1;
    if (width <= 0)
	VError ("Width of cropped region must be positive");

    /* Read the input file: */
    if (! (list = VReadFile (in_file, NULL)))
	exit (EXIT_FAILURE);

    /* Replace each image with a cropped version: */
    for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
	if (VGetAttrRepn (& posn) != VImageRepn)
	    continue;
	VGetAttrValue (& posn, NULL, VImageRepn, & src);
	result = VCropImage (src, NULL, VAllBands, top, left, height, width);
	if (! result)
	    exit (EXIT_FAILURE);
	VSetAttrValue (& posn, NULL, VImageRepn, result);
	VDestroyImage (src);
	nimages++;
    }

    /* Write the output file: */
    if (! VWriteFile (out_file, list))
	exit (EXIT_FAILURE);

    fprintf(stderr, "%s: Cropped %d image%s.\n",
	    argv[0], nimages, nimages == 1 ? "" : "s");
    return EXIT_SUCCESS;
}
