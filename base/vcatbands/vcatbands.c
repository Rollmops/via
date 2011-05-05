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
 * $Id: vcatbands.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vcatbands - concatenate image bands into a single image

\par Description
vcatbands concatenates one or more images to produce a single, multi-band image. All
images read from all input files are combined to produce a single  image,  which  is
written to the output file.
<br>
The input images must all have the same number of rows, number of columns, and pixel
representation.


\par Usage

        <code>vcatbands</code>

        \param -in      Specifies one or more Vista data files containing images.
        \param -out     output image
        \param -name    Specifies the name to be given the created image. Default: ``image''.

\par Examples
<br>
\par Known bugs
none.

\file vcatbands.c
\author Arthur Pope, UBC Laboratory for Computational Intelligence
*/



/*
 *
 *  The vcatbands program, which concatenates multiple image bands into
 *  a single image.
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


/* Later in this file: */
static void BadImage (int, int, VStringConst);

/* Command line options: */
static VArgVector in_files;			/* list of input files */
static VStringConst out_filename;
static VBoolean in_found, out_found;
static VStringConst object_name = "image";
static VLong frame_interp = VBandInterpNone;
static VLong viewpoint_interp = VBandInterpNone;
static VLong color_interp = VBandInterpNone;
static VLong component_interp = VBandInterpNone;
static VDictEntry frame_dict[] = {
    { "none",		VBandInterpNone },
    { NULL }
};
VDictEntry viewpoint_dict[] = {
    { "none",		VBandInterpNone },
    { "stereo_pair",	VBandInterpStereoPair },
    { NULL }
};
VDictEntry color_dict[] = {
    { "none",		VBandInterpNone },
    { "rgb",		VBandInterpRGB },
    { NULL }
};
VDictEntry component_dict[] = {
    { "none",		VBandInterpNone },
    { "complex",	VBandInterpComplex },
    { "gradient",	VBandInterpGradient },
    { "intensity",	VBandInterpIntensity },
    { "orientation",	VBandInterpOrientation },
    { NULL }
};
static VLong nframes = 1, nviewpoints = 1, ncolors = 1, ncomponents = 1;
static VBoolean nframes_found, nviewpoints_found;
static VBoolean ncolors_found, ncomponents_found;
static VOptionDescRec options[] = {
    { "in", VStringRepn, 0, & in_files, & in_found, NULL,
	  "Files containing source image(s)" },
    { "out", VStringRepn, 1, & out_filename, & out_found, NULL,
	  "File to contain created image" },
    { "name", VStringRepn, 1, & object_name, VOptionalOpt, NULL,
	  "Name to be given created image" },
    { "frame_interp", VLongRepn, 1, & frame_interp, VOptionalOpt,
	  frame_dict, "Frame interpretation" },
    { "nframes", VLongRepn, 1, & nframes, & nframes_found, NULL,
	  "Number of frames" },
    { "viewpoint_interp", VLongRepn, 1, & viewpoint_interp, VOptionalOpt,
	  viewpoint_dict, "Viewpoint interpretation" },
    { "nviewpoints", VLongRepn, 1, & nviewpoints, & nviewpoints_found, NULL,
	  "Number of viewpoints" },
    { "color_interp", VLongRepn, 1, & color_interp, VOptionalOpt,
	  color_dict, "Color interpretation" },
    { "ncolors", VLongRepn, 1, & ncolors, & ncolors_found, NULL,
	  "Number of colors" },
    { "component_interp", VLongRepn, 1, & component_interp, VOptionalOpt,
	  component_dict, "Component interpretation" },
    { "ncomponents", VLongRepn, 1, & ncomponents, & ncomponents_found, NULL,
	  "Number of components" }
};

/* Global variables: */
static VImage **images;				/* images from each file */
static int *nimages;				/* number from each file */
static VBoolean errorDetected = FALSE;		/* whether error detected */


/*
 *  Program entry point.
 */

int main (int argc, char *argv[])
{
    FILE *f;
    VStringConst in_filename;
    VAttrList list;
    int i, j, n, nrows, ncolumns;
    VRepnKind repn;
    VImage *all_images, result;
    VBand *all_bands;
    char prg[50];	
    sprintf(prg,"vcatbands V%s", getVersion());
    fprintf (stderr, "%s\n", prg);


    /* Parse command line arguments: */
    if (! VParseCommand (VNumber (options), options, & argc, argv) ||
        ! VIdentifyFiles (VNumber (options), options, "in", & argc, argv, 0) ||
	! VIdentifyFiles (VNumber (options), options, "out", & argc, argv, 1))
	goto Usage;
    if (argc > 1) {
	VReportBadArgs (argc, argv);
Usage:	VReportUsage (argv[0], VNumber (options), options, NULL);
	exit (EXIT_FAILURE);
    }

    /* Default band interpretation attributes: */
    if (color_interp != VBandInterpNone && ! ncolors_found)
	switch (color_interp) {

	case VBandInterpRGB:
	    ncolors = 3;
	}
    if (viewpoint_interp != VBandInterpNone && ! nviewpoints_found)
	switch (viewpoint_interp) {

	case VBandInterpStereoPair:
	    nviewpoints = 2;
	}
    if (component_interp != VBandInterpNone && ! ncomponents_found)
	switch (component_interp) {

	case VBandInterpComplex:
	    ncomponents = 2;
	    break;

	case VBandInterpGradient:
	    VError ("-ncomponents 2 or 3 required for gradient images");
	}

    /* Allocate storage for keeping track of all input images: */
    nimages = VMalloc (in_files.number * sizeof (int));
    images = VMalloc (in_files.number * sizeof (VImage *));

    /* For each input file: */
    for (i = 0; i < in_files.number; i++) {
	in_filename = ((VStringConst *) in_files.vector)[i];

	/* Read its images: */
	f = VOpenInputFile (in_filename, TRUE);
	if (! (nimages[i] = VReadImages (f, & list, & images[i])))
	    exit (EXIT_FAILURE);
	fclose (f);
	VDestroyAttrList (list);
    }

    /* Ensure that all images have the same size and pixel representation: */
    nrows = VImageNRows (images[0][0]);
    ncolumns = VImageNColumns (images[0][0]);
    repn = VPixelRepn (images[0][0]);
    for (i = 0; i < in_files.number; i++)
	for (j = 0; j < nimages[i]; j++) {
	    if (VImageNRows (images[i][j]) != nrows)
		BadImage (i, j, "number of rows");
	    if (VImageNColumns (images[i][j]) != ncolumns)
		BadImage (i, j, "number of columns");
	    if (VPixelRepn (images[i][j]) != repn)
		BadImage (i, j, "pixel representation");
	}
    if (errorDetected)
	VError ("(Expected %d rows x %d columns of %s pixels)",
		  nrows, ncolumns, VRepnName (repn));

    /* Combine all bands of all images into a single image: */
    for (i = n = 0; i < in_files.number; i++)
	n += nimages[i];
    all_images = VMalloc (n * sizeof (VImage));
    all_bands = VMalloc (n * sizeof (VBand));
    for (i = n = 0; i < in_files.number; i++)
	for (j = 0; j < nimages[i]; j++) {
	    all_images[n] = images[i][j];
	    all_bands[n] = VAllBands;
	    n++;
	}
    
    if (! (result = VCombineBands (n, all_images, all_bands, (VImage) NULL)))
	exit (EXIT_FAILURE);

    /* Set band interpretation attributes: */
    n = nframes_found ? nframes :
	 VImageNBands (result) / (nviewpoints * ncolors * ncomponents);
    if (VImageNBands (result) != n * nviewpoints * ncolors * ncomponents)
	VError ("No. bands (%d) conflicts with no. "
		"of frames, etc. (%d %d %d %d)", VImageNBands (result),
		nframes, nviewpoints, ncolors, ncomponents);
    VSetBandInterp (result, frame_interp, n, viewpoint_interp, nviewpoints,
		    color_interp, ncolors, component_interp, ncomponents);

    /* Put it on an attribute list with the specified name: */
    list = VCreateAttrList ();
    VAppendAttr (list, object_name, NULL, VImageRepn, result);

    /* Open and write the output file: */
    if (! VWriteFile (VOpenOutputFile (out_filename, TRUE), list))
	exit (EXIT_FAILURE);

    fprintf (stderr, "%s: %d image(s) concatenated into a %d-band image.\n",
	     argv[0], n, VImageNBands (result));

    return EXIT_SUCCESS;
}


/*
 *  BadImage
 *
 *  Report a problem with one of the input images.
 */

static void BadImage (int file_idx, int image_idx, VStringConst msg)
{
    VStringConst filename, imagename;

    /* Get a name for the file: */
    filename = ((VStringConst *) in_files.vector)[file_idx];
    if (strcmp (filename, "-") == 0)
	filename = "<stdin>";

    /* Get a name for the image: */
    if (VGetAttr (VImageAttrList (images[file_idx][image_idx]),
		  VNameAttr, NULL, VStringRepn, & imagename) == VAttrFound)
	VWarning ("\"%s\" in %s has the wrong %s", imagename, filename, msg);
    else VWarning ("Image %d in %s has the wrong %s",
		   image_idx + 1, filename, msg);

    errorDetected = TRUE;
}
