/*
 *  $Id: Adjust.c 3629 2009-08-20 17:04:30Z proeger $
 * 
 *  This file contains a routine for adjustint image brightness and contrast.
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
#include "viaio/os.h"
#include "viaio/VImage.h"

/* From the standard C library: */
#include <math.h>

extern double rint(double);


/*
 *  Some macros used in adjusting brightness and contrast.
 *  Macro AdjustByCalc modified by FK (18.02.96) - to avoid gcc optimization problem
 */

/* Apply brightness and contrast adjustment to a pixel value: */
#define Adjust(x) (((pow ((double) x, exponent) - 0.5) * factor) + 0.5)

/* Apply adjustment to each pixel: */
#define AdjustByCalc(type, round) 					\
    {									\
	type *src_pp = (type *) src_pixels;				\
	type *dest_pp = (type *) VPixelPtr (dest, 0, 0, 0);		\
	while (npixels-- > 0) {						\
	    s = *src_pp++;						\
	    t = (s > 0.0) ? -s : s;					\
	    t = Adjust (t / min_value) * min_value;			\
	    if (s > 0.0) t = -t;					\
	    t = (t < min_value? min_value : (t > max_value? max_value : t)); \
	    *dest_pp++ = round(t);					\
	}								\
    }
	
/* Apply adjustment to each pixel using table lookup: */
#define AdjustByLookup(type, offset)					\
    {									\
	type *src_pp = (type *) src_pixels;				\
	type *dest_pp = (type *) VPixelPtr (dest, 0, 0, 0);		\
	while (npixels-- > 0)						\
	    *dest_pp++ = (type) table[*src_pp++ + offset];		\
    }

#define Nothing


/*
 *  VAdjustImage
 *
 *  Adjust the brightness and/or contrast of an image.
 */

VImage VAdjustImage (VImage src, VImage dest, VBand band,
		     double brightness, double contrast)
{
    int npixels, i, table[256];
    VPointer src_pixels;
    double exponent, factor, min_value, max_value, s, t;

    /* Locate the source pixels: */
    if (! VSelectBand ("VAdjustImage", src, band, & npixels, & src_pixels))
        return NULL;

    /* Locate the destination pixels: */
    dest = VSelectDestImage ("VAdjustImage", dest,
			     band == VAllBands ? VImageNBands (src) : 1,
			     VImageNRows (src), VImageNColumns (src),
			     VPixelRepn (src));
    if (! dest)
        return NULL;

    /* Precompute some things: */
    exponent = pow (2.0, -brightness);
    factor = pow (2.0, contrast);
    min_value =
	VIsFloatPtRepn (VPixelRepn (src)) ? -1.0 : VPixelMinValue (src);
    max_value =
	VIsFloatPtRepn (VPixelRepn (src)) ? 1.0 : VPixelMaxValue (src);


    /* Adjust brightness and contrast of each pixel: */
    switch ((int) VPixelRepn (src)) {

    case VBitRepn:
    break;
    
    case VUByteRepn:
    
	for (i = 0, s = 0.0; s <= max_value; s++) {
	    t = Adjust (s / max_value) * max_value;
	    table[i++] = (t < 0.0 ? 0 :
			  (t > max_value ? max_value : rint (t)));
	}
	if (VPixelRepn (src) == VBitRepn) {
	    AdjustByLookup (VBit, 0);
	} else {
	    AdjustByLookup (VUByte, 0);
	}
	break;

    case VSByteRepn:
  
	for (i = 0, s = min_value; s <= max_value; s++) {
	    t = (s < 0) ? -128.0 : 128.0;
	    t = Adjust (s / t) * t;
	    table[i++] = (t < min_value ? min_value :
			  (t > max_value ? max_value : rint (t)));
	    
	}
	AdjustByLookup (VSByte, 128);
	break;
	
   case VShortRepn:
    
	AdjustByCalc (VShort, rint);
	break;
	
    case VLongRepn:
	AdjustByCalc (VLong, rint);
	break;

    case VFloatRepn:
	AdjustByCalc (VFloat, Nothing);
	break;
	
    case VDoubleRepn:
	AdjustByCalc (VDouble, Nothing);
	
    }
    
    
    VCopyImageAttrs (src, dest);
    return dest;
}
