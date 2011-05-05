/*
 *  $Id: Dither.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains routines for dithering images.
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

/* From the standard C library: */
#undef __OPTIMIZE__
#include <math.h>

/* File identification string: */
VRcsId ("$Id: Dither.c 3177 2008-04-01 14:47:24Z karstenm $");

/* Later in this file: */
static void DitherBit (VImage, VImage, int, int, int, int *, int *);
static void DitherFloat (VImage, VImage, int, int, int, int *, int *,
			 VBoolean);
static void DitherInt (VImage, VImage, int, int, int, int *, int *, VBoolean);


/*
 *  VDither
 *
 *  Dither an image to produce one whose pixel values are drawn from a limited
 *  set. The calling interface is a bit weird, in order to accommodate as
 *  efficiently as possible common circumstances of, for example, the
 *  VImageView widget. Arguments have the following meaning:
 *
 *	- a destination image must be provided, and it must have VUByte pixels.
 *	  It cannot be the same as the source image. It can have any
 *	  number of bands; that number is denoted here by dest_nbands.
 *
 *	- the band argument specifies the first band of the source image
 *	  to be dithered; altogether dest_nbands consecutive bands will be
 *	  dithered
 *
 *	- top, left, height, width specify a rectangular region of the
 *	  source image to be dithered; it must not extend beyond the image
 *
 *	- nvalues is a vector of dest_nbands values specifying the number
 *	  of pixel values to be used in each dithered destination band.
 *	  For example, the first band is rendered with pixel values in
 *	  the range [0, nvalues[0] - 1].
 *
 *	- if absolute is TRUE, absolute source pixel values are dithered;
 *	  otherwise actual (signed) source pixel values are dithered.
 *
 *  The dithering algorithm used is due to Floyd & Steinberg.
 */

VBoolean VDither (VImage src, VImage dest, VBand band,
		  int top, int left, int height, int width,
		  int nvalues[], VBooleanPromoted absolute)
{
    int i, *col_index, size;

    /* Check parameters: */
    if (! dest) {
	VWarning ("VDither: No destination image");
	return FALSE;
    }
    if (VPixelRepn (dest) != VUByteRepn) {
	VWarning ("VDither: Destination image doesn't have VUByte pixels");
	return FALSE;
    }
    if (dest == src) {
	VWarning ("VDither: Source and destination images must differ");
	return FALSE;
    }
    if (band == VAllBands) {
	if (VImageNBands (src) != VImageNBands (dest)) {
	    VWarning ("VDither: Source and destination have different"
		      " numbers of bands");
	    return FALSE;
	}
	band = 0;
    } else {
	if (band < 0 || band + VImageNBands (dest) > VImageNBands (src)) {
	    VWarning ("VDither: Band (%d) out of range", band);
	    return FALSE;
	}
    }
    if (top < 0 || left < 0 || height < 1 || width < 1 ||
	top + height > VImageNRows (src) ||
	left + width > VImageNColumns (src)) {
	VWarning ("VDither: Source region exceeds image");
	return FALSE;
    }
    for (i = 0; i < VImageNBands (dest); i++)
	if (nvalues[i] < 2 || nvalues[i] > 256) {
	    VWarning ("VDither: nvalues (%d) not in [2,256]", nvalues[i]);
	    return FALSE;
	}

    /* Initialize an index vector for sampling columns of the input image: */
    col_index = VMalloc (VImageNColumns (dest) * sizeof (int));
    size = VPixelSize (src);
    for (i = 0; i < VImageNColumns (dest); i++)
	col_index[i] = ((width * i) / VImageNColumns (dest) + left) * size;

    /* Invoke a type-dependent dithering routine: */
    switch (VPixelRepn (src)) {

    case VBitRepn:
	DitherBit (src, dest, band, top, height, nvalues, col_index);
	break;

    case VFloatRepn:
    case VDoubleRepn:
	DitherFloat (src, dest, band, top, height, nvalues, col_index,
		     absolute);
	break;

    default:
	DitherInt (src, dest, band, top, height, nvalues, col_index,
		   absolute);
    }

    VFree (col_index);

    return TRUE;
}


/*
 *  DitherBit
 *
 *  Dither a Bit image. This amounts to resampling the image, and mapping
 *  pixel values {0,1} to {0, nvalues-1}.
 */

static void DitherBit (VImage src, VImage dest, int src_band, int top,
		       int height, int *nvalues, int *col_index)
{
    int dest_band, max_dest_pixel, row, col;
    char *src_row;
    VUByte *dest_pixelp = VPixelPtr (dest, 0, 0, 0);

    /* For each band to be dithered: */
    for (dest_band = 0; dest_band < VImageNBands (dest);
	 src_band++, dest_band++) {

	max_dest_pixel = nvalues[dest_band] - 1;

	/* For each row of the destination image: */
	for (row = 0; row < VImageNRows (dest); row++) {

	    /* Select the corresponding row of the source image: */
	    src_row = VPixelPtr (src, src_band, (row * height) /
				 VImageNRows (dest) + top, 0);

	    /* For each pixel of the destination row: */
	    for (col = 0; col < VImageNColumns (dest); col++)

		/* Set the pixel according to the corresponding 
		   source pixel value: */
		*dest_pixelp++ = (* (VBit *) (src_row + col_index[col])) ?
		    max_dest_pixel : 0;
	}
    }
}


/* 
 *  DitherFloat
 *
 *  Dither an image having floating-point pixel values.
 */

static void DitherFloat (VImage src, VImage dest, int src_band, int top,
			 int height, int *nvalues, int *col_index,
			 VBoolean absolute)
{
    int dest_band, row, col, dest_pixel, max_dest_pixel;
    size_t error_row_size;
    char *src_row;
    VUByte *dest_pixelp = VPixelPtr (dest, 0, 0, 0);
    VDouble *error_row, left_err, upleft_err, up_err;
    VDouble src_pixel, max_dest_pixel_flt;
    VBoolean bin_dither;

    /* Allocate memory for a row-length vector of error terms: */
    error_row_size = (VImageNColumns (dest) + 1) * sizeof (VDouble);
    error_row = VMalloc (error_row_size);

    /* For each band to be dithered: */
    for (dest_band = 0; dest_band < VImageNBands (dest);
	 src_band++, dest_band++) {

	/* Precompute some loop-invariant subexpressions: */
	max_dest_pixel = nvalues[dest_band] - 1;
	max_dest_pixel_flt = (double) max_dest_pixel;
	bin_dither = (nvalues[dest_band] == 2);

	/* Initialize a vector of errors to propagate from the previous row: */

	/*
	 *  ASSUMPTION: Floating point 0.0 is binary all-bits-zero.
	 *
	 *  This is true of the IEEE standard floating point format, but it
	 *  isn't assured by the ANSI C standard.
	 */

	memset (error_row, 0, error_row_size);

	/* For each row of the destination image: */
	for (row = 0; row < VImageNRows (dest); row++) {

	    /* Select the corresponding row of the source image: */
	    src_row = VPixelPtr (src, src_band,
				 (row * height) / VImageNRows (dest) + top, 0);
	    left_err = upleft_err = 0.0;

	    /* For each pixel of the destination row: */
	    for (col = 0; col < VImageNColumns (dest); col++) {

		/* Fetch a source pixel value: */
		if (VPixelRepn (src) == VFloatRepn)
		    src_pixel = * (VFloat *) (src_row + col_index[col]);
		else src_pixel = * (VDouble *) (src_row + col_index[col]);

		/* Ensure it isn't NaN or Inf: */
		if (! finite (src_pixel))
		    src_pixel = 0.0;

		/* Translate the source pixel to the range [0,1]: */
		if (absolute) {
		    if (src_pixel < 0.0)
			src_pixel = -src_pixel;
		    if (src_pixel > 1.0)
			src_pixel = 1.0;
		} else {
		    if (src_pixel < -1.0)
			src_pixel = 0.0;
		    else if (src_pixel > 1.0)
			src_pixel = 1.0;
		    else src_pixel = src_pixel * 0.5 + 0.5;
		}

		/* Add error terms from adjacent pixels: */
		up_err = error_row[col];
		src_pixel += upleft_err * 0.0625 + up_err * 0.1875 +
			     error_row[col + 1] * 0.3125 + left_err * 0.4375;
		upleft_err = up_err;

		/* Compute the destination pixel's value: */
		if (bin_dither) {

		    /* Special case of dithering to a binary image: */
		    if (src_pixel >= 0.5) {
			*dest_pixelp++ = 1;
			src_pixel -= 1.0;
		    } else *dest_pixelp++ = 0;

		} else {

		    /* General case: */
		    dest_pixel = src_pixel * max_dest_pixel_flt + 0.5;
		    if (dest_pixel > max_dest_pixel) {
			*dest_pixelp++ = max_dest_pixel;
			src_pixel -= 1.0;
		    } else if (dest_pixel < 0)
			*dest_pixelp++ = 0;
		    else {
			*dest_pixelp++ = dest_pixel;
			src_pixel -= dest_pixel / max_dest_pixel_flt;
		    }
		}

		/* Propagate the remaining error to the next row and column: */
		error_row[col] = left_err = src_pixel; 
	    }
	}
    }

    VFree (error_row);
}


/* 
 *  DitherInt
 *
 *  Dither an image having integer pixel values.
 */

static void DitherInt (VImage src, VImage dest, int src_band, int top,
		       int height, int *nvalues, int *col_index,
		       VBoolean absolute)
{
    int dest_band, nvals, nvalues_minus_1, row, col, error_row_size, shift;
    char *src_row;
    VUByte *dest_pixelp;
    VLong *error_row, left_err, upleft_err, up_err, upright_err;
    VLong src_pixel, dest_pixel;
    VBoolean do_shifts;

    /* Allocate memory for a row-length vector of error terms: */
    error_row_size = (VImageNColumns (dest) + 1) * sizeof (VLong);
    error_row = VMalloc (error_row_size);

    /* For each band to be dithered: */
    for (dest_band = 0; dest_band < VImageNBands (dest);
	 src_band++, dest_band++) {
	nvals = nvalues[dest_band];
	nvalues_minus_1 = nvals - 1;

	/* Some multiplies and divides can be replaced by shifts if nvals
	   is one greater than a power of two: */
	shift = ffs (nvalues_minus_1) - 1;
	do_shifts = (1 << shift) == nvalues_minus_1;

	/* Initialize a vector of errors to propagate from the previous row: */
	if (! (VPixelRepn (src) == VUByteRepn && nvals == 256))
	    memset (error_row, 0, error_row_size);
	dest_pixelp = VPixelPtr (dest, dest_band, 0, 0);

	/* For each row of the destination image: */
	for (row = 0; row < VImageNRows (dest); row++) {

	    /* Select the corresponding row of the source image: */
	    src_row = src->band_index[src_band]
		[(row * height) / VImageNRows (dest) + top];
	    left_err = upleft_err = 0;

	    /* For each pixel of the destination row: */
	    for (col = 0; col < VImageNColumns (dest); col++) {

		/* Fetch a source pixel value and normalize it so that it
		   lies in the range [0,65535]: */
		if (VPixelRepn (src) == VUByteRepn) {
		    src_pixel = * (VUByte *) (src_row + col_index[col]);
		    if (nvals == 256) {

			/* Handle a common case requiring simple copying. */
			*dest_pixelp++ = src_pixel;
			continue;
		    }

		} else {

		    switch (VPixelRepn (src)) {

		    case VSByteRepn:
			src_pixel =
			    (* (VSByte *) (src_row + col_index[col])) << 1;
			break;
	
		    case VShortRepn:
			src_pixel = (* (VShort *) (src_row + col_index[col])) /
			    (1 << 8);
			break;
	
		    case VLongRepn:
			src_pixel = (* (VLong *) (src_row + col_index[col])) /
			    (1 << 23);
			break;

		    default:
			break;
		    }

		    /* src_pixel is now in the range [-256,255] */

		    if (absolute) {
			if (src_pixel < 0)
			    src_pixel = -src_pixel;
		    } else src_pixel = (src_pixel + 256) >> 1;

		}

		/* src_pixel is now in the range [0,256] */

		/* Add error terms from adjacent pixels: */
		up_err = error_row[col];
		upright_err = error_row[col + 1];
		src_pixel += (upleft_err + (up_err << 1) + up_err +
			      (upright_err << 2) + upright_err +
			      (left_err << 3) - left_err) / 16;;
		upleft_err = up_err;

		/* Compute the destination pixel's value: */
		if (nvals == 2) {

		    /* Dither to a monochrome image: */
		    if (src_pixel >= 128) {
			*dest_pixelp++ = 1;
			src_pixel -= 255;
		    } else *dest_pixelp++ = 0;

		} else if (do_shifts) {
		    
		    /* Special case common due to minGrayShades value (17)
		       in Colormap.c: */
		    dest_pixel = ((src_pixel << shift) + 128) / 255;
		    if (dest_pixel > nvalues_minus_1) {
			*dest_pixelp++ = nvalues_minus_1;
			src_pixel -= 255;
		    } else if (dest_pixel < 0) {
			*dest_pixelp++ = 0;
		    } else {
			*dest_pixelp++ = dest_pixel;
			src_pixel -= (dest_pixel * 255) >> shift;
		    }

		} else {

		    /* General case: */
		    dest_pixel = (src_pixel * nvalues_minus_1 + 128) / 255;
		    if (dest_pixel > nvalues_minus_1) {
			*dest_pixelp++ = nvalues_minus_1;
			src_pixel -= 255;
		    } else if (dest_pixel < 0) {
			*dest_pixelp++ = 0;
		    } else {
			*dest_pixelp++ = dest_pixel;
			src_pixel -= (dest_pixel * 255) / nvalues_minus_1;
		    }
		}

		/* Propagate the remaining error to the next row and column: */
		error_row[col] = left_err = src_pixel;
	    }
	}
    }

    VFree (error_row);
}
