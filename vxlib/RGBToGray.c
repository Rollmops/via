/*
 *  $Id: RGBToGray.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains routines for converting an image from 
 *  RBG representaion to a single band of gray shades.
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
#include "viaio/os.h"
#include "viaio/VImage.h"

/* From the standard C libaray: */
#include <math.h>

/* File identification string: */
VRcsId ("$Id: RGBToGray.c 3177 2008-04-01 14:47:24Z karstenm $");


/*
 *  VRGBImageToGray
 *
 *  Combine the 3 bands of a RGB image into a single band of gray shades.
 */

VImage VRGBImageToGray (VImage src, VImage dest, VBand band)
{
  int src_band, dest_nbands, dest_band, npixels, i;

  /* Require a single-component RGB image: */
  if (VImageNComponents (src) != 1 ||
      VImageColorInterp (src) != VBandInterpRGB) {
    VWarning ("VRGBImageToGray: Image isn't single-component RGB");
    return NULL;
  }
  if (band == VAllBands) {
    dest_nbands = VImageNBands (src) / 3;
    src_band = 0;
  } else if (band >= 0 && band + 2 < VImageNBands (src)) {
    dest_nbands = 1;
    src_band = band;
  } else {
    VWarning ("VRGBImageToGray: Band %d referenced in image of %d band(s)",
	      band, VImageNBands (src));
    return NULL;
  }
    
  /* Create or check the destination image: */
  dest = VSelectDestImage ("VRGBImageToGray", dest, dest_nbands,
			   VImageNRows (src), VImageNColumns (src),
			   VPixelRepn (src));
  if (dest == NULL)
    return NULL;

  /*
   * Take weighted average of the 3 bands:
   */
#define RGBToGray(type)						\
    {									\
        type *rpixs = (type *) VPixelPtr (src, src_band, 0, 0);		\
        type *gpixs = (type *) VPixelPtr (src, src_band + 1, 0, 0);	\
        type *bpixs = (type *) VPixelPtr (src, src_band + 2, 0, 0);	\
	type *dest_pixels = (type *) VPixelPtr (dest, dest_band, 0, 0);	\
        for (i = 0; i < npixels; i++)					\
            *dest_pixels++ = 0.299 * *rpixs++				\
	                   + 0.587 * *gpixs++				\
                           + 0.114 * *bpixs++;				\
    }

  /* Perform pixel averaging according to pixel representation: */
  npixels = VImageNRows (src) * VImageNColumns (src);
  for (dest_band = 0; dest_band < dest_nbands; dest_band++, src_band += 3)
    switch (VPixelRepn (src)) {
    case VBitRepn:      RGBToGray(VBit);    break;
    case VUByteRepn:    RGBToGray(VUByte);  break;
    case VSByteRepn:    RGBToGray(VSByte);  break;
    case VShortRepn:    RGBToGray(VShort);  break;
    case VLongRepn:	    RGBToGray(VLong);   break;
    case VFloatRepn:    RGBToGray(VFloat);  break;
    case VDoubleRepn:   RGBToGray(VDouble); break;
    default: break;
    }
    
  /* Adopt the band interpretation information from the source image but
     delete any color interpretation: */
  if (band == VAllBands) {
    if (VImageAttrList (dest))
      VDestroyAttrList (VImageAttrList (dest));
    VImageAttrList (dest) = VCopyAttrList (VImageAttrList (src));
    VExtractAttr (VImageAttrList (dest), VColorInterpAttr, NULL,
		  VBitRepn, NULL, FALSE);
    VImageNFrames (dest) = VImageNFrames (src);
    VImageNViewpoints (dest) = VImageNViewpoints (src);
    VImageNColors (dest) = VImageNComponents (dest) = 1;
  } else VCopyImageAttrs (src, dest);

  return dest;

#undef RGBToGray
}
