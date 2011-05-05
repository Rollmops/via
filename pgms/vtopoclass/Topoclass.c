/*
** Topologocal classification
**
** Lit:
**
** G. Malandain, G. Bertrand, N. Ayache:
** "Topological Segmentation of discrete surfaces"
** Intern. Journal of Computer Vision, 10:2, pp. 183-197, 1993
**
**
** Output codes:
**
**  1:  interior point 
**  2:  isolated point
**  3:  border point
**  4:  curve point
**  5:  curves junction
**  6:  surface point
**  7:  surface/curve junction
**  8:  surfaces junction
**  9:  surfaces/curve junction
**
**
** Author: G.Lohmann <lohmann@cns.mpg.de>, May 1996
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>

extern VImage VTopoclass (VImage, VImage);
extern int VNumComp26(VBit []);
extern int VANumComp6(VBit []);
extern int VBNumComp6(VBit []);
extern VImage VJunction(VImage,VImage);


VImage
VTopoclass(VImage src, VImage dest)
{
  int b=0,r=0,c=0,i,u=0;
  int bb,rr,cc,n;
  int b0,b1,r0,r1,c0;
  int nbands,nrows,ncols;
  int c1,c2,c3;
  VBit adj[27];
  VUByte v;

  nbands = VImageNBands(src);
  nrows  = VImageNRows(src);
  ncols  = VImageNColumns(src);

  dest = VCreateImage(nbands,nrows,ncols,VUByteRepn);

  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	VPixel(dest,b,r,c,VUByte) = 0;
	if (VPixel(src,b,r,c,VBit) == 0) continue;
	
	/*
	** get number of connected components in N26
	*/
	i = 0;
	for (bb=b-1; bb<=b+1; bb++) {
	  for (rr=r-1; rr<=r+1; rr++) {
	    for (cc=c-1; cc<=c+1; cc++) {
	      if (bb >= 0 && bb < nbands
		  && rr >= 0 && rr < nrows 
		  && cc >= 0 && cc < ncols)
		u = VPixel(src,bb,rr,cc,VBit);
	      else
		u = 0;
	      adj[i++] = u;
	    }
	  }
	}
	adj[13] = 0;
	c1 = VNumComp26(adj);

	/*
	** get number of connected components in -N18
	** 6-adjacent to the center voxel
	*/
	for (i=0; i<27; i++) {
	  u = adj[i];
	  adj[i] = (u > 0) ? 0 : 1;
	}

	adj[13] = 0;
	adj[0]  = 0;
	adj[2]  = 0;
	adj[6]  = 0;
	adj[8]  = 0;
	adj[18] = 0;
	adj[20] = 0;
	adj[24] = 0;
	adj[26] = 0;

	c2 = VANumComp6(adj);
	c3 = VBNumComp6(adj);

	if (c2 == 0)                    /* interior point */
	  VPixel(dest,b,r,c,VUByte) = 1;

	else if (c1 == 0)               /* isolated point */
	  VPixel(dest,b,r,c,VUByte) = 2;

	else if (c2 == 1 &&  c1 == 1)   /* border point */
	  VPixel(dest,b,r,c,VUByte) = 3;

	else if (c2 == 1 &&  c1 == 2)   /* curve point */
	  VPixel(dest,b,r,c,VUByte) = 4;

	else if (c2 == 1 &&  c1 > 2)    /* curves junction */
	  VPixel(dest,b,r,c,VUByte) = 5;

/*
	else if (c2 == 2 &&  c1 == 1 && c3 > 2)
	  VPixel(dest,b,r,c,VUByte) = 8;
	  */
	else if (c2 == 2 &&  c1 == 1)   /* surface point */
	  VPixel(dest,b,r,c,VUByte) = 6;

	else if (c2 == 2 &&  c1 >= 2)   /* surface/curve junction */
	  VPixel(dest,b,r,c,VUByte) = 7;

	else if (c2 > 2 &&  c1 == 1)    /* surfaces junction */
	  VPixel(dest,b,r,c,VUByte) = 8;

	else if (c2 > 2 &&  c1 >= 2)    /* surfaces/curve junction */
	  VPixel(dest,b,r,c,VUByte) = 9;

	else                            /* undefined */
	  VPixel(dest,b,r,c,VUByte) = 10;

/*
	n =  VPixel(dest,b,r,c,VUByte);
	fprintf(stderr," %2d %2d %2d : %d %d %d, %d\n",b,r,c,c1,c2,c3,n);
	*/

      }
    }
  }


  /*
  ** get missed curves junctions
  */
  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {

	if (VPixel(dest,b,r,c,VUByte) == 4) {
	  n = 0;

	  b0 = (b < 1) ? 0 : b - 1;
	  b1 = (b >= nbands - 1) ? nbands - 1 : b + 1;
	  for (bb=b0; bb<=b1; bb++) {

	    r0 = (r < 1) ? 0 : r - 1;
	    r1 = (r >= nrows - 1) ? nrows - 1 : r + 1;
	    for (rr=r0; rr<=r1; rr++) {

	      c0 = (c < 1) ? 0 : c - 1;
	      c1 = (c >= ncols - 1) ? ncols - 1 : c + 1;
	      for (cc=c0; cc<=c1; cc++) {

		v = VPixel(dest,bb,rr,cc,VUByte);
		if (v == 3 || v == 4 || v == 5) n++;
	      }
	    }
	  }
	  if (n > 3) {
	    VPixel(dest,b,r,c,VUByte) = 5;
	  }
	}
      }
    }
  }


  VJunction(src,dest);
  
  VCopyImageAttrs (src, dest);
  return dest;
}


