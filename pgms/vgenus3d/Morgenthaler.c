/*
 *  get 3D Euler number using Morgenthaler's algorithm
 * Lit:
 *   D.G. Morgenthaler: "Three-dimensional topology: the genus",
 *   Computer Science Center, Univ. of Maryland, College Park,
 *   Tech. Report TR-980, 1980.
 *
 * G.Lohmann, MPI-CNS, Oct. 1997.
 */


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>

/* From the standard C library: */
#include <stdio.h>
#include <stdlib.h>

#define NOT(x) ((x) > 0 ? 0 : 1)



int
VGenusMorgenthaler (VImage src,VShort neighb)
{
  int b,r,c,nbands,ncols,nrows,npixels;
  VBit *tmpbuf;
  VBit x,x1,x2,x3,x4,x5,x6,x7;
  int psi1,psi2,psi3,psi4,euler;
  int i;
  VImage tmp;

  nbands = VImageNBands (src) + 1;
  nrows  = VImageNRows (src) + 1;
  ncols  = VImageNColumns (src) + 1;
  npixels = nbands * ncols * nrows;

  if (nbands < 3 || nrows < 3 || ncols < 3)
    VError(" image too small, minimum image size is 3 x 3 x 3");

  if (neighb != 6 && neighb != 26)
    VError(" illegal parameter value n: use n=6 or n=26");

  tmp = VCreateImage(nbands,nrows,ncols,VBitRepn);

  tmpbuf  = VPixelPtr(tmp,0,0,0);

  if (neighb == 6)
    for (i=0; i<npixels; i++) *tmpbuf++ = 0;
  else
    for (i=0; i<npixels; i++) *tmpbuf++ = 1;

  for (b=0; b<nbands-1; b++) {
    for (r=0; r<nrows-1; r++) {
      for (c=0; c<ncols-1; c++) {
	x = VPixel(src,b,r,c,VBit);
	if (neighb == 6)
	  VPixel(tmp,b,r,c,VBit) = x;
	else
	  VPixel(tmp,b,r,c,VBit) = NOT(x);
      }
    }
  }

  psi1 = 0;
  psi2 = 0;
  psi3 = 0;
  psi4 = 0;

  for (b=0; b<nbands-1; b++) {
    for (r=0; r<nrows-1; r++) {
      for (c=0; c<ncols-1; c++) {

	x = * (VBit *) VPixelPtr(tmp, b, r, c);
	if (x == 0) continue;

	x1 = * (VBit *) VPixelPtr(tmp, b, r+1, c);
	x2 = * (VBit *) VPixelPtr(tmp, b, r, c+1);
	x3 = * (VBit *) VPixelPtr(tmp, b, r+1, c+1);
	x4 = * (VBit *) VPixelPtr(tmp, b+1, r, c);
	x5 = * (VBit *) VPixelPtr(tmp, b+1, r+1, c);
	x6 = * (VBit *) VPixelPtr(tmp, b+1, r, c+1);
	x7 = * (VBit *) VPixelPtr(tmp, b+1, r+1, c+1);

	/* pattern 1: */
	if (x) psi1++;

	/* pattern 2: */
	if (x && x1) psi2++;
	if (x && x2) psi2++; 
	if (x && x4) psi2++; 

	/* pattern 3: */
	if (x && x1 && x2 && x3) psi3++;
	if (x && x1 && x4 && x5) psi3++;
	if (x && x2 && x4 && x6) psi3++;
	
	/* pattern 4: */
	if (x && x1 && x2 && x3 && x4 && x5 && x6 && x7) psi4++;

      }
    }
  }

  euler = psi1 - psi2 + psi3 - psi4;
  VDestroyImage(tmp);

  return euler;
}
