/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/VImage.h>
#include <stdio.h>


/*
 *  Select slices from an image
 */

VImage 
VSelSlices (VImage src,VImage dest,VShort first,VShort last)
{
  int nbands,nrows,ncols,nbands_neu,npixels;
  int i,b,bn;
  VRepnKind repn;

  nbands = VImageNBands(src);
  nrows  = VImageNRows(src);
  ncols  = VImageNColumns(src);
  repn   = VPixelRepn(src);

  if (first < 0) VError("VSelSlices: parameter <first> must be positive");
  if (last < 0) last = nbands-1;
  if (last >= nbands) last = nbands-1;


  if (first > last)
    VError("VSelSlices: <first> must be less than <last>.");

  nbands_neu = last - first + 1;
  if (dest == NULL)
    dest = VCreateImage(nbands_neu,nrows,ncols,repn);

  npixels = nrows * ncols;

#define SelSlices(type) \
{ \
  type *src_pp, *dest_pp; \
  bn = 0; \
  for (b=first; b<=last; b++) { \
    src_pp  = VPixelPtr(src,b,0,0); \
    dest_pp = VPixelPtr(dest,bn,0,0); \
    for (i=0; i<npixels; i++) *dest_pp++ = *src_pp++; \
    bn++; \
  } \
}

      
  /* Instantiate the macro once for each source pixel type: */
  switch (repn) {
  case VBitRepn:	SelSlices (VBit);	break;
  case VUByteRepn:	SelSlices (VUByte);	break;
  case VSByteRepn:	SelSlices (VSByte);	break;
  case VShortRepn:	SelSlices (VShort);	break;
  case VLongRepn:	SelSlices (VLong);	break;
  case VFloatRepn:	SelSlices (VFloat);	break;
  case VDoubleRepn:	SelSlices (VDouble);	break;
  default: ;
  }


  /* Let the destination inherit any attributes of the source image: */
  VCopyImageAttrs (src, dest);
  return dest;
}
