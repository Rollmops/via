/*! \file
  Delete small components

The input image must be of type "ubyte" or "short".
The output image will be of type "bit" and contain
only those pixels which belong to components of size
larger or equal to "msize".
The maximum number of classes allowed is currently
set to 10000.

\par Author:
Gabriele Lohmann, MPI-CBS
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXVAL 10000

/*!
\fn VImage VDeleteSmall (VImage src,VImage dest,int msize)
\param src  input image (ubyte or short repn)
\param dest output image (bit repn)
\param msize minimal size, components with fewer voxels than 'msize' are deleted.
*/
VImage
VDeleteSmall (VImage src,VImage dest,int msize)
{
  int i,j,npixels,nbands,nrows,ncols;
  long table[MAXVAL];
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

  dest = VSelectDestImage("VDeleteSmall",dest,nbands,nrows,ncols,VBitRepn);
  if (! dest) VError("Error creating destination image");

  /*
  ** get table of component sizes
  */
  for (i=0; i<MAXVAL; i++) table[i] = 0;

  switch (repn) {
  case VUByteRepn:
    byte_pp = (VUByte *) VPixelPtr(src,0,0,0);
    for (j=0; j<npixels; j++) {
      i = (int) *byte_pp++;
      if (i >= MAXVAL) {
	i = MAXVAL - 1;
	VWarning("label exceeds range: max number of labels supported: %d",MAXVAL);
      }
      if (i != 0) table[i]++;
    }
    break;

  case VShortRepn:

    short_pp = (VShort *) VPixelPtr(src,0,0,0);
    for (j=0; j<npixels; j++) {
      i = (int) *short_pp++;
      if (i >= MAXVAL) {
	i = MAXVAL - 1;
	VWarning("label exceeds range: max number of labels supported: %d",MAXVAL);
      }
      if (i != 0) table[i]++;
    }
    break;

  default:
    ;
  }

  /*
  ** delete small components
  */
  switch (repn) {

  case VUByteRepn:
    bit_pp = (VBit *) VPixelPtr(dest,0,0,0);
    byte_pp = (VUByte *) VPixelPtr(src,0,0,0);
    for (j=0; j<npixels; j++) {
      i = (int) *byte_pp++;
      *bit_pp++ = (table[i] >= msize) ? 1 : 0;
    }
    break;

  case VShortRepn:
    bit_pp = (VBit *) VPixelPtr(dest,0,0,0);
    short_pp = (VShort *) VPixelPtr(src,0,0,0);
    for (j=0; j<npixels; j++) {
      i = (int) *short_pp++;
      *bit_pp++ = (table[i] >= msize) ? 1 : 0;
    }
    break;

  default:
    ;
  }

  VCopyImageAttrs (src, dest);
  return dest;
}
