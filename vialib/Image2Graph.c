/*! \file
  Convert a raster image to graph representation

Each foreground voxel is represented by a node in the output graph.
Optionally, only border voxels are used.
Optionally, 26-adjacent voxels are connected by arcs.
Voxels of grey value '0' are assumed to be background voxels.

\par Author:
 Gabriele Lohmann, MPI-CBS
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <viaio/Vlib.h>
#include <via.h>


#define ABS(x) ((x) < 0 ? -(x) : (x))



/*!
 \fn VGraph VImage2Graph (VImage src, VGraph dest,VDouble xmin,VDouble xmax,
     VBoolean link,VBoolean border)
 \brief Convert a raster image to graph representation
 \param src   input image (any repn)
 \param dest  output graph
 \param xmin  voxels with values below 'xmin' are assumed to be background.
 \param xmax  voxels with values above 'xmax' are assumed to be background.
   If both xmin and xmax equal zero then these thresholds are ignored.
 \param link  if 'TRUE', then 26-adjacent voxels are linked by an arc.
 \param border if 'TRUE', convert only border voxels.
*/
VGraph
VImage2Graph (VImage src,VGraph dest,VDouble xmin,VDouble xmax,VBoolean link,VBoolean border)
{
  VImage tmp=NULL,tmp1=NULL,border_image=NULL;
  int band,row,col,b,r,c;
  int i,k,npoints;
  int nbands,ncols,nrows,npixels;
  SNode node1,node2;
  int nid=0,n1=0,n2=0;
  int nlinks = 0;
  int b0,b1,r0,r1,c0,c1;
  double u=0;
  VDouble zmax,zmin;
  VDouble tiny=1.0e-5;
  VBit *bin_pp;


  /*
  ** reset thresholds, if needed
  */
  if (ABS(xmin) < tiny && ABS(xmax) < tiny) {
    xmin = VRepnMinValue(VDoubleRepn);
    xmax = VRepnMaxValue(VDoubleRepn);
  }


  /*
  ** get image dimensions
  */
  nrows  = VImageNRows (src);
  ncols  = VImageNColumns (src);
  nbands = VImageNBands (src);
  npixels = nbands * nrows * ncols;

  zmin = VRepnMinValue(VShortRepn);
  zmax = VRepnMaxValue(VShortRepn);


  /*
  ** get border voxels, only border voxels are converted to graph nodes
  */
  if (VPixelRepn(src) != VBitRepn) {
    tmp1 = VCreateImage(nbands,nrows,ncols,VBitRepn);
    VFillImage(tmp1,VAllBands,0);

    for (b=0; b<nbands; b++) {
      for (r=0; r<nrows; r++) {
	for (c=0; c<ncols; c++) {
	  u = VReadPixel(src,b,r,c);
	  if (ABS(u) < tiny) continue;
	  if (u > xmax || u < xmin) continue;
	  VPixel(tmp1,b,r,c,VBit) = 1;
	}
      }
    }
  }
  else
    tmp1 = VCopyImage(src,NULL,VAllBands);


  /*
  ** extract border voxels
  */
  if (border == TRUE)
    border_image = VBorderImage3d (tmp1,NULL);
  else
    border_image = VCopyImage(tmp1,NULL,VAllBands);

  VDestroyImage(tmp1);


  /*
  ** create temporary image
  */
  if (link) {
    tmp = VCreateImage(nbands,nrows,ncols,VLongRepn);
    if (tmp == NULL) VError(" error creating temporary image");
    VFillImage(tmp,VAllBands,0);
  }

  /*
  ** allocate graph structure
  */
  bin_pp = VImageData(border_image);
  npoints = 0;
  for (i=0; i<VImageNPixels(border_image); i++) {
    if (*bin_pp > 0) npoints++;
    bin_pp++;
  }
  npoints++;

  if (dest == NULL)
    dest = VCreateGraph(npoints,5,VShortRepn,FALSE);

  /*
  ** create nodes
  */
  node1 = (SNode) VMalloc(sizeof(SNodeRec));
  node1->base.hops    = 0;
  node1->base.visited = 0;
  node1->base.head    = 0;
  node1->base.weight  = 0;

  k = 0;
  for (band=0; band<nbands; band++) {
    for (row=0; row<nrows; row++) {
      for (col=0; col<ncols; col++) {
	if (VPixel(border_image,band,row,col,VBit) == 0) continue;
	u = VReadPixel(src,band,row,col);

	u = (VShort) (u + 0.5); /* clipping */
	if (u > zmax) u = zmax;  
	if (u < zmin) u = zmin;

	node1->type  = 4;
	node1->band  = band;
	node1->row   = row;
	node1->col   = col;
	node1->label = (VShort) u;
	n1 = VGraphAddNode(dest,(VNode) node1);
	if (link) VPixel(tmp,band,row,col,VLong) = n1;
	k++;
      }
    }
  }

  /*
  ** get links between nodes
  */
  if (link) {
    node2 = (SNode) malloc(sizeof(SNodeRec));
    node2->base.hops    = 0;
    node2->base.visited = 0;
    node2->base.head    = 0;
    node2->base.weight  = 0;
    nlinks = 0;

    for (nid=1; nid<=dest->lastUsed; nid++) {
      node1 = (SNode) VGraphGetNode (dest,nid);
      if (node1 == 0) continue;
    
      band = node1->band;
      row  = node1->row;
      col  = node1->col;
      n1   = nid;

      b0 = (band < 1) ? 0 : band - 1;
      b1 = (band >= nbands - 1) ? nbands - 1 : band + 1;
      for (b=b0; b<=b1; b++) {

	r0 = (row < 1) ? 0 : row - 1;
	r1 = (row >= nrows - 1) ? nrows - 1 : row + 1;
	for (r=r0; r<=r1; r++) {

	  c0 = (col < 1) ? 0 : col - 1;
	  c1 = (col >= ncols - 1) ? ncols - 1 : col + 1;
	  for (c=c0; c<=c1; c++) {
	      
	    if ((b == band) && (r == row) && (c == col)) continue;
	    if (VPixel(border_image,b,r,c,VBit) == 0) continue;

	    n2 = (int) VPixel(tmp,b,r,c,VLong);
	    VGraphLinkNodes(dest,n1,n2);
	    nlinks++;
	  }
	}
      }
    }
    VDestroyImage(tmp);
  }
  VDestroyImage(border_image);

  VGraphAttrList (dest) = VCopyAttrList (VImageAttrList (src));

  VSetAttr(VGraphAttrList(dest),"nbands",NULL,VShortRepn,VImageNBands(src));
  VSetAttr(VGraphAttrList(dest),"nrows",NULL,VShortRepn,VImageNRows(src));
  VSetAttr(VGraphAttrList(dest),"ncolumns",NULL,VShortRepn,VImageNColumns(src));

  return dest;
}

