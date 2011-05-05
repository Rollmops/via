/*
 *  $Id: ImageDpy.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains routines that aid the display of images.
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
#include "viaio/os.h"
#include "viaio/VImage.h"

/* File identification string: */
VRcsId ("$Id: ImageDpy.c 3177 2008-04-01 14:47:24Z karstenm $");


/*
 *  VImageWindowSize
 * 
 *  Compute the dimensions of a window that will display a correctly-
 *  proportioned image and not exceed specified limits.
 */

void VImageWindowSize (VImage image, int max_width, int max_height,
		       int *width, int *height)
{
    VFloat pixel_aspect = 1.0, image_aspect, window_aspect;

    /* Find the image's pixel aspect ratio: */
    if (VGetAttr (VImageAttrList (image), VPixelAspectRatioAttr, NULL,
		  VFloatRepn, & pixel_aspect) == VAttrBadValue)
	VWarning ("VImageWindowSize: Image has bad %s attribute",
		  VPixelAspectRatioAttr);

    /* Compute the width and height of the rectangle within the window
       that will contain the image: */
    image_aspect = pixel_aspect *
	(float) VImageNColumns (image) / (float) VImageNRows (image);
    window_aspect = (float) max_width / (float) max_height;
    if (image_aspect != window_aspect) {
	if (image_aspect > window_aspect) {

	    /* Image is a wider rectangle than window. */
	    *width = max_width;
	    *height = max_width / image_aspect;

	} else {

	    /* Image is taller rectangle than window. */
	    *width = max_height * image_aspect;
	    *height = max_height;
	}
    } else {
	*width = max_width;
	*height = max_height;
    }
}
