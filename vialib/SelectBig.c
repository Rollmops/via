/*! \file
  Select most frequent voxel label

The input image must be of type "ubyte" or "short".
The output image will be of type "bit" and contain
only those pixels which belong to the most frequent
class. The maximum number of classes allowed is currently
set to 8192.

\par Author:
Gabriele Lohmann, MPI-CBS
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <stdio.h>
#include <stdlib.h>


#define MAXVAL 20000

/*!
\fn VImage VSelectBig (VImage src,VImage dest)
\param src  input image (ubyte or short repn)
\param dest output image (bit repn)
*/
VImage
VSelectBig (VImage src,VImage dest)
{
  int i,j,i0,npixels,nbands,nrows,ncols;
  long table[MAXVAL],maxsize;
  VRepnKind repn;
  VShort *short_pp;
  VUByte *byte_pp;
  VBit *bit_pp;

  nrows  = VImageNRows (src);
  ncols  = VImageNColumns (src);
  nbands = VImageNBands (src);
  npixels = nbands * nrows * ncols;
  repn   = VPixelRepn(src);
  if ((repn != VUByteRepn) && (repn != VShortRepn))
    VError("vselbig: input image representation must be either ubyte or short");

  dest = VSelectDestImage("VSelectBig",dest,nbands,nrows,ncols,VBitRepn);
  if (! dest) VError("Error creating destination image");

  /*
  ** get histogram of component sizes
  */
  for (i=0; i<MAXVAL; i++) table[i] = 0;

  switch (repn) {
  case VUByteRepn:
    byte_pp = (VUByte *) VImageData(src);
    for (j=0; j<npixels; j++) {
      i = (int) *byte_pp++;
      if (i >= MAXVAL) {
	i = MAXVAL - 1;
	VWarning("label exceeds range: max number of labels supported: %d",MAXVAL);
      }
      table[i]++;
    }
    break;

  case VShortRepn:

    short_pp = (VShort *) VImageData(src);
    for (j=0; j<npixels; j++) {
      i = (int) *short_pp++;
      if (i >= MAXVAL) {
	i = MAXVAL - 1;
	VWarning("label exceeds range: max number of labels supported: %d",MAXVAL);
      }
      table[i]++;
    }
    break;

  default:
    ;
  }

  /*
  ** get largest component
  */
  maxsize = 0;
  i0 = -1;
  for (i=1; i<MAXVAL; i++) {
    if (table[i] > maxsize) {
      i0 = i;
      maxsize = table[i];
    }
  }
  if (i0 < 0)
    VError(" input image is zero.");


  /*
  ** delete all but the largest component
  */

  switch (repn) {

  case VUByteRepn:
    bit_pp = (VBit *) VImageData(dest);
    byte_pp = (VUByte *) VImageData(src);
    for (j=0; j<npixels; j++) {
      i = (int) *byte_pp++;
      *bit_pp++ = (i == i0) ? 1 : 0;
    }
    break;

  case VShortRepn:
    bit_pp = (VBit *) VImageData(dest);
    short_pp = (VShort *) VImageData(src);
    for (j=0; j<npixels; j++) {
      i = (int) *short_pp++;
      *bit_pp++ = (i == i0) ? 1 : 0;
    }
    break;

  default:
    ;
  }

  VCopyImageAttrs (src, dest);
  return dest;
}
