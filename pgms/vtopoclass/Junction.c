
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>



/* component labelling */

typedef struct {
  short b;   /* band   */
  short r;   /* row    */
  short c;   /* column */
} Voxel;

typedef struct {
  Voxel *A;
  int front,rear;
} Queue;

Queue queue;
int QueueSize;

int msize;
typedef int BOOLEAN;
VImage ldest;

#define ABS(x) ((x) > 0 ? (x) : -(x))

#define TRUE  1
#define FALSE 0

void QueueClear(Queue *pQ)
{
  pQ->front = 0;
  pQ->rear  = 0;
}

BOOLEAN
QueueEmpty(Queue *pQ) 
{
  return (pQ->front == pQ->rear);
}


BOOLEAN
deQueue(Queue *pQ, Voxel *pe)
{
  if (pQ->front == pQ->rear) {
    fprintf(stderr," deQueue: empty\n");
    return FALSE;
  }
  else {
    (*pe) = pQ->A[(pQ->rear)++];
    return TRUE;
  }
}

BOOLEAN 
enQueue(Queue *pQ, Voxel e)
{
  if (pQ->front > msize) msize = pQ->front;
  
  if (pQ->front < QueueSize - 1) {
    pQ->A[(pQ->front)++] = e;
    return TRUE;
  }
  else if (pQ->rear > 2) {
    pQ->A[--(pQ->rear)] = e;
    return TRUE;
  }
  else {
    QueueSize += QueueSize * 0.2;
    fprintf(stderr," realloc: %d\n",QueueSize);
    queue.A = (Voxel *) VRealloc(queue.A,sizeof(Voxel) * QueueSize);
    pQ->A[(pQ->front)++] = e;
    return TRUE;
  }
}


int
CountComponents(VImage src)
{
  int nbands,nrows,ncols,npixels;
  Voxel v,vv;
  int label,n;
  int b0,b1,r0,r1,c0,c1,b,r,c,bb,rr,cc;
  
  nbands  = VImageNBands(src);
  nrows   = VImageNRows(src);
  ncols   = VImageNColumns(src);
  npixels = nbands * nrows * ncols;

  label = 0;

  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	if (VPixel(src,b,r,c,VBit) == 0) continue;
	if (VPixel(ldest,b,r,c,VUByte) > 0) continue;

	QueueClear(&queue);

	label++;
	n = 1;

	v.b = b;
	v.r = r;
	v.c = c;
	if (enQueue(&queue,v) == FALSE) VError(" error in enQueue");
	VPixel(ldest,b,r,c,VUByte) = label;

	while (! QueueEmpty(&queue)) {

	  deQueue(&queue,&v);

	  b0 = (v.b > 0) ? v.b - 1 : 0;
	  b1 = (v.b + 1 < nbands) ? v.b + 1 : nbands - 1;
	  for (bb=b0; bb<=b1; bb++) {

	    r0 = (v.r > 0) ? v.r - 1 : 0;
	    r1 = (v.r + 1 < nrows) ? v.r + 1 : nrows - 1;
	    for (rr=r0; rr<=r1; rr++) {

	      c0 = (v.c > 0) ? v.c - 1 : 0;
	      c1 = (v.c + 1 < ncols) ? v.c + 1 : ncols - 1;
	      for (cc=c0; cc<=c1; cc++) {

		if (bb == v.b && rr == v.r && cc == v.c) continue;
		if (VPixel(src,bb,rr,cc,VBit) == 0) continue;
		if (VPixel(ldest,bb,rr,cc,VUByte) > 0) continue;
		vv.b = bb;
		vv.r = rr;
		vv.c = cc;
		if (enQueue(&queue,vv) == FALSE) VError(" error in enQueue");
		VPixel(ldest,bb,rr,cc,VUByte) = label;
		n++;
	      }
	    }
	  }
	}
      }
    }
  }
  return label;
}

/*
** correct topological labelling. Some surface points must be relabelled
** as junctions.
*/
VImage
VJunction(VImage src,VImage topo)
{
  VImage tmp1,tmp2;
  int nbands,nrows,ncols;
  int b,r,c,bb,rr,cc,bbb,rrr,ccc;
  VBit *bit_pp,*bin_pp,u;
  int i,npix;

  nbands = VImageNBands(src);
  nrows = VImageNRows(src);
  ncols = VImageNColumns(src);

  ldest = VCreateImage(5,5,5,VUByteRepn);
  tmp1 = VCreateImage(5,5,5,VBitRepn);
  tmp2 = VCreateImage(5,5,5,VBitRepn);
  npix = 5 * 5 * 5;

  QueueSize = npix;
  queue.A = (Voxel *) VMalloc(sizeof(Voxel) * QueueSize);

  for (b=2; b<nbands-2; b++) {
    for (r=2; r<nrows-2; r++) {
      for (c=2; c<ncols-2; c++) {

	if (VPixel(topo,b,r,c,VUByte) != 6) continue;

	bit_pp = (VBit *) VPixelPtr(tmp1,0,0,0);
	for (i=0; i<npix; i++) *bit_pp++ = 0;

	bit_pp = VPixelPtr(tmp2,0,0,0);
	for (i=0; i<npix; i++) *bit_pp++ = 0;

	for (bb=b-1; bb<=b+1; bb++) {
	  for (rr=r-1; rr<=r+1; rr++) {
	    for (cc=c-1; cc<=c+1; cc++) {

	      if (ABS(bb-b) + ABS(rr-r) + ABS(cc-c) > 2) continue;
	      u = VPixel(src,bb,rr,cc,VBit);
	      VPixel(tmp1,bb-b+1,rr-r+1,cc-c+1,VBit) = ~ u;
	    }
	  }
	}
	VPixel(tmp1,2,2,2,VBit) = 0;

	for (bb=b-1; bb<=b+1; bb++) {
	  for (rr=r-1; rr<=r+1; rr++) {
	    for (cc=c-1; cc<=c+1; cc++) {
	      if (VPixel(topo,bb,rr,cc,VUByte) != 6) continue;

	      bit_pp = VPixelPtr(tmp2,0,0,0);
	      for (i=0; i<npix; i++) *bit_pp++ = 0;

	      for (bbb=bb-1; bbb<=bb+1; bbb++) {
		for (rrr=rr-1; rrr<=rr+1; rrr++) {
		  for (ccc=cc-1; ccc<=cc+1; ccc++) {

		    if (ABS(bbb-bb) + ABS(rrr-rr) + ABS(ccc-cc) > 2) continue;
		    u = VPixel(src,bbb,rrr,ccc,VBit);
		    VPixel(tmp2,bbb-bb+1,rrr-rr+1,ccc-cc+1,VBit) = ~ u;
		  }
		}
	      }
	      VPixel(tmp2,2,2,2,VBit) = 0;

	      bit_pp = (VBit *) VPixelPtr(tmp1,0,0,0);
	      bin_pp = (VBit *) VPixelPtr(tmp2,0,0,0);
	      for (i=0; i<npix; i++) {
		*bin_pp++ = *bit_pp | *bit_pp;
		bin_pp++;
		bit_pp++;
	      }

	      if (CountComponents(tmp2) > 2) {
		VPixel(topo,b,r,c,VUByte) = 7;
		goto skip;
	      }
	    }
	  }
	}
      skip: ;
      }
    }
  }
  return topo;
}


