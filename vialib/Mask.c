/*! \file
  Masking a raster image

Pixels that are not covered by a given mask
are set to zero.

*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>


/*!
\fn VImage VMask (VImage src, VImage dest, VImage mask)
\param src  input image (any repn)
\param dest output image (any repn)
\param mask mask image (bit repn)
*/
VImage 
VMask (VImage src, VImage dest, VImage mask)
{
  int nbands,nrows,ncols,b,r,c;
  VDouble v;
  
  if (VPixelRepn(mask) != VBitRepn) VError("mask pixel repn must be bit");

  nbands = VImageNBands (src);
  nrows  = VImageNRows (src);
  ncols  = VImageNColumns (src);
  
  dest = VCopyImage(src,dest,VAllBands);

  v = 0;
  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	if (VPixel(mask,b,r,c,VBit) == 0)
	  VSetPixel(dest,b,r,c,v);
      }
    }
  }

  return dest;
}



/*!
\fn VImage VMaskRange (VImage src, VImage dest, VDouble xmin, VDouble xmax)
\brief pixels beyond the given grey value range [xmin,xmax] are set to zero.
\param src  input image (any repn)
\param dest output image (any repn)
\param xmin min grey value
\param xmax max grey value
*/
VImage 
VMaskRange (VImage src, VImage dest, VDouble xmin, VDouble xmax)
{
  int i,npixels;
  VFloat u,v;
  
  npixels  = VImageNPixels (src);
  dest = VCopyImage(src,dest,VAllBands);

  v = 0;
  for (i=0; i<npixels; i++) {
    u = VGetPixelValue (dest,i);
    if (u < xmin || u > xmax)
      VSetPixelValue (dest,i,v);
  }

  return dest;
}
