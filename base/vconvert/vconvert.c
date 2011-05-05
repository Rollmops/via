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
 * $Id: vconvert.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vconvert -  converts images from one pixel representation to another.

\par Description
vconvert  converts Vista images to a specified pixel representation.  The images are
read from an input file, converted to the representation specified by the -repn com­
mand-line option, then written to an output file.
<br>
vconvert  knows  several  ways  of  mapping  input pixels to output pixels.  You can
choose the mapping you want by means of a -map command line option.   If  you  don't
specify  that  option,  however,  then  you'll get a default mapping that is usually
appropriate. The default mapping is one that retains an image's appearance as  accu­
rately  as  possible  -- i.e., a particular shade of gray in the input image will be
mapped to nearly the same shade in the output image. Alternatively, you can use -map
to specify some other mapping that may change the brightness or contrast of an image
while converting it to its new representation.
<br>
The following describes, in some detail, the various mappings supported by vconvert.
Here the full range of pixel values capable of being represented in the source image
is denoted [Smin,Smax]; the actual range of pixel values present in the source image
is  denoted  [Amin,Amax]; and the full range of pixel values capable of being repre­
sented in the destination image is denoted [Dmin,Dmax]. (For float and  double  pix­
els,  the ranges [Smin,Smax] and [Dmin,Dmax] are taken to be [-1,+1].) Define Sabs =
max ( |Smin|, |Smax| ), and similarly define Aabs. Then the  various  mappings  sup­
ported by vconvert are:
range | copy | opt1 | opt2 | op3 | linear.


\par Usage

        <code>vconvert</code>

       \param -in      Specifies one or more Vista data files containing images.
       \param -out     output image
       \param -repn    Specifies the pixel representation to which  images  should  
                       be  converted( bit | ubyte | sbyte | short | long | float | double).  
                       Default: ubyte.
       \param  -map
                       Specifies  the  mapping of input pixel values to output pixel values
                       (range | copy | opt1 | opt2 | op3 | linear). Default: range.

       \param -a       Specify the values a and b to be used when -map linear 
                       is the chosen mapping.

       \param -b       Specify the values a and b to be used when -map linear 
                       is the chosen mapping.

\par Examples
<br>
\par Known bugs
none.

\file vconvert.c
\author Arthur Pope, UBC Laboratory for Computational Intelligence
*/


/*
 *
 *  The vconvert program, which converts images from one pixel representation
 *  to another.
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
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"



/* We support these methods of converting pixel values: */
enum {
    rangeMethod,		/* map source range to destination range */
    copyMethod,			/* just copy it without mapping */
    opt1Method,			/* map actual range of source pixel values */
    opt2Method,			/*   to full destination range */
    opt3Method,
    linearMethod		/* explicitly specified linear mapping */
};

/* Keywords for representing conversion methods: */
static VDictEntry method_dict[] = {
    { "range", rangeMethod }, 
    { "copy", copyMethod },
    { "opt1", opt1Method }, 
    { "opt2", opt2Method }, 
    { "opt3", opt3Method },
    { "linear", linearMethod },
    { NULL }
};

int main (int argc, char **argv)
{
    static VLong repn = VUByteRepn;
    static VLong method = rangeMethod;
    static VDouble a = 1.0, b = 0.0;
    static VOptionDescRec options[] = {
	{ "repn", VLongRepn, 1, & repn, VOptionalOpt, VNumericRepnDict,
	      "Desired pixel representation" },
        { "map", VLongRepn, 1, & method, VOptionalOpt, method_dict,
	      "Pixel value mapping" },
        { "a", VDoubleRepn, 1, & a, VOptionalOpt, NULL,
	      "Multiplication factor for -map linear" },
        { "b", VDoubleRepn, 1, & b, VOptionalOpt, NULL,
	      "Addition term for -map linear" }
    };
    FILE *in_file, *out_file;
    VAttrList list;
    VImage src, result;
    int nimages = 0;
    VAttrListPosn posn;
    char prg[50];	
    sprintf(prg,"vconvert V%s", getVersion());
    fprintf (stderr, "%s\n", prg);


    /* Parse command line arguments: */
    VParseFilterCmd (VNumber (options), options, argc, argv,
		     & in_file, & out_file);

    /* Read source image(s): */
    if (! (list = VReadFile (in_file, NULL)))
	exit (EXIT_FAILURE);

    /* Convert each source image to the specified representation: */
    for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
	if (VGetAttrRepn (& posn) != VImageRepn)
	    continue;
	VGetAttrValue (& posn, NULL, VImageRepn, & src);

	switch (method) {

	case rangeMethod:
	    result = VConvertImageRange (src, NULL, VAllBands, (int) repn);
	    break;

	case copyMethod:
	    result = VConvertImageCopy (src, NULL, VAllBands, (int) repn);
	    break;

	case linearMethod:
	    result = VConvertImageLinear (src, NULL, VAllBands, (int) repn,
					  (double) a, (double) b);
	    break;

	default:
	    result = VConvertImageOpt (src, NULL, VAllBands, (int) repn,
				       method - opt1Method + 1);
	}
	VSetAttrValue (& posn, NULL, VImageRepn, result);
	VDestroyImage (src);
	nimages++;
    }

    /* Write the results to the output file: */
    if (! VWriteFile (out_file, list))
	exit (EXIT_FAILURE);
    
    fprintf (stderr, "%s: Converted %d image%s to %s representation.\n",
	     argv[0], nimages, nimages == 1 ? "" : "s",
	     VRepnName ((int) repn));
    return EXIT_SUCCESS;
}
