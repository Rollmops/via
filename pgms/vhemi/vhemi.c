/****************************************************************
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2004, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id: vhemi.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vhemi -- select one hemisphere

\par Description
select one hemisphere.
vhemi accepts both raster images and graphs as input.

\par Usage

        <code>vhemi</code>

        \param -in    input image
        \param -out   output image
        \param -hemi  Hemisphere to be selected (left | right). Default: left


\par Examples
<br>
\par Known bugs
none.

\file vhemi.c
\author G.Lohmann, MPI-CBS, 2004
*/


#include <stdio.h>
#include <stdlib.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via.h>



VDictEntry HEMIDict[] = {
  { "left", 0 },
  { "right", 1 },
  { NULL }
};


extern VGraph VGraphHemi(VGraph,VLong);
extern void VImageHemi(VImage *,VLong);

int main (int argc,char *argv[])
{
  static VLong hemi = 0;
  static VOptionDescRec options[] = {
    { "hemi", VLongRepn, 1, & hemi, VOptionalOpt, HEMIDict,
      "Hemisphere to retain" }
  };
  FILE *in_file, *out_file;
  VAttrList list;
  VAttrListPosn posn;
  VImage isrc=NULL;
  VGraph gsrc=NULL,dest=NULL;
  char prg[50];	
  sprintf(prg,"vhemi V%s", getVersion());
  fprintf (stderr, "%s\n", prg);
  
  /* Parse command line arguments and identify files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,
		   & in_file, & out_file);
  
  /* Read the grey input file: */
  list = VReadFile (in_file, NULL);
  if (! list) VError("Error reading input image");
  fclose(in_file);


  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) == VImageRepn) {
      VGetAttrValue (& posn, NULL, VImageRepn, & isrc);
      VImageHemi(&isrc,hemi);
      VSetAttrValue (& posn, NULL,VImageRepn,isrc);
    }
    else if (VGetAttrRepn (& posn) == VGraphRepn) {
      VGetAttrValue (& posn, NULL, VGraphRepn, & gsrc);
      dest = VGraphHemi(gsrc,hemi);
      VSetAttrValue (& posn, NULL,VGraphRepn,dest);
    }
  }

  /* Write out the results: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file,list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}


/*
 *  Zero out one hemisphere of a raster image
 */
void
VImageHemi(VImage *src,VLong hemi)
{
  int nbands,nrows,ncols;
  int b,r,c,c0,c1,half;

  ncols  = VImageNColumns (*src);
  nrows  = VImageNRows (*src);
  nbands = VImageNBands (*src);

  half = ncols/2;

  if (hemi == 1) {
    c0 = 0;
    c1 = half;
  }
  else {
    c0 = half;
    c1 = ncols;
  }
 
  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=c0; c<c1; c++) {
	VSetPixel((*src),b,r,c,(VDouble) 0);
      }
    }
  }
}


/*
** test adjacency
*/
VBoolean
VGraphIsAdjacent(VGraph graph,int i,int j)
{
  VNode node;
  VAdjacency neighb;
  VNodeBaseRec base;

  node   = (VNode) VGraphGetNode(graph,i);
  base   = node->base;
  neighb = base.head;
  while ((neighb != NULL)) {
    if (neighb->id == j) return TRUE;
    neighb = neighb->next;
  }

  return FALSE;
}



VGraph
VGraphHemi(VGraph src,VLong hemi)
{
  VString str;
  VGraph dest=NULL;
  int i,j,n,ncols,half=0;
  float x,y,z,val=0;
  VNode node,node1;
  int *table;

  half = 80;   /* standard coordinate for inter-hemisperic cleft */

  if (VGetAttr (VGraphAttrList (src), "ca", NULL,VStringRepn, (VPointer) & str) == VAttrFound) {
    sscanf(str,"%f %f %f",&x,&y,&z);
    ncols = (int) x;
    half  = ncols/2;
  }

  table = (int *) VMalloc(sizeof(int) * (src)->lastUsed + 1);
  for (i=0; i<= src->lastUsed; i++) table[i] = 0;

  n = (src)->lastUsed/2;
  dest = VCreateGraph(n,VGraphNFields(src),VNodeRepn(src),src->useWeights);
 
  for (i=1; i<=(src)->lastUsed; i++) {
    node = (VNode) VGraphGetNode (src,i);
    if (node == 0) continue;
    VReadNode((src),node,&x,&y,&z,&val);

    if ((hemi == 0 && x <= half) || (hemi == 1 && x > half)) {
      n = VGraphAddNode(dest,(VNode) node);
      table[i] = n;
    }
  }


  for (i=1; i<=(src)->lastUsed; i++) {
    node = (VNode) VGraphGetNode (src,i);
    if (node == 0) continue;

    for (j=1; j<=(src)->lastUsed; j++) {
      node1 = (VNode) VGraphGetNode (src,j);
      if (node1 == 0) continue;
      if (table[i] == 0 || table[j] == 0) continue;

      if (VGraphIsAdjacent(src,i,j) == TRUE) {
	VGraphLinkNodes(dest,table[i],table[j]);
      }
    }
  }
  VFree(table);
  return dest;
}

