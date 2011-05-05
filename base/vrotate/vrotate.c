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
 * $Id: vrotate.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vrotate - rotate an image by an arbitrary angle

\par Description
vrotate rotates an image by an arbitrary angle. 
vrotate  rotates each input image by the specified angle, producing a file of output
images. The rotation is done by first rotating the image by a multiple of 90 degrees
(using  flips  and  transpositions)  to an angle nearest to the specified angle, and
then by completing the rotation using Alan Paeth's three-shear method. 
<br>
Because each pixel in the rotated image is a weighted average of  the  corresponding
pixels  in the original image, the set of colors (or gray shades) used by the origi­
nal image will not be preserved in the rotated image.
<br>
In general, the edges of the rotated image will not be parallel to the vertical  and
horizontal  axes.  In  such cases, the output image will be the smallest rectangular
region containing the whole rotated image. Portions of the output image not  covered
by the rotated image will be filled with zeros.

\par Usage

        <code>vrotate</code>

        \param -in      input image
        \param -out     output image
        \param -angle   Specifies  the  angle  of rotation in degrees. Positive values rotate
	counterclockwise; negative ones rotate clockwise. Default: 90.

\par Examples
<br>
\par Known bugs
none.

\file vrotate.c
\author Daniel Ko, UBC Laboratory for Computational Intelligence
*/




/*
 *  $Id: vrotate.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *  The vrotate program, which rotates an image by an arbitrary angle.
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
#include "via.h"

/* From the standard C libaray: */
#include <math.h>

/* File identification string: */
VRcsId ("$Id: vrotate.c 3629 2009-08-20 17:04:30Z proeger $");


int main (int argc, char **argv)
{
    static VDouble angle = 90.0;
    static VOptionDescRec options[] = {
	{ "angle", VDoubleRepn, 1, & angle, VOptionalOpt, NULL,
	      "Angle of rotation (degrees CCW)" }
    };
    FILE *in_file, *out_file;
    VAttrList list;
    VImage src, result;
    VAttrListPosn posn;
    int nimages = 0;
    double angle_rads;
    char prg[50];	
    sprintf(prg,"vrotate V%s", getVersion());
    fprintf (stderr, "%s\n", prg);

    /* Parse command line arguments: */
    VParseFilterCmd (VNumber (options), options, argc, argv,
		     & in_file, & out_file);

    /* Convert angle to radians: */
    angle_rads = angle * M_PI / 180.0;

    /* Read source image(s): */
    if (! (list = VReadFile (in_file, NULL)))
	exit (EXIT_FAILURE);

    /* Rotate each image: */
    for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
	if (VGetAttrRepn (& posn) != VImageRepn)
	    continue;
	VGetAttrValue (& posn, NULL, VImageRepn, & src);

	result = VRotateImage (src, NULL, VAllBands, angle_rads);
	if (result == NULL)
	    exit (EXIT_FAILURE);
	
	VSetAttrValue (& posn, NULL, VImageRepn, result);
	VDestroyImage (src);
	nimages++;
    }

    /* Write the results to the output file: */
    VHistory(VNumber(options),options,prg,&list,&list);
    if (! VWriteFile (out_file, list))
	exit (EXIT_FAILURE);

    fprintf (stderr, "%s: Rotated %d image%s by %g degrees.\n",
	     argv[0], nimages, nimages == 1 ? "" : "s", (double) angle);
    return EXIT_SUCCESS;
}
