/*! \file
  Prune small branches in a graph.



\par Author:
Gabriele Lohmann, MPI-CBS
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/VGraph.h>
#include <viaio/mu.h>


/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <via.h>



#define ABS(x) ((x) > 0 ? (x) : -(x))


#define VNodeTestDestroy(node) ((node)->type < 0)
#define VNodeSetDestroy(node) \
if ((node)->type > 0) (node)->type = - (node)->type;
#define VNodeClearDestroy(node) \
if ((node)->type < 0) (node)->type = - (node)->type;


/*
** count number of adjacent nodes
*/
int
VGraphNumNeighbours(VGraph graph, SNode node)
{
  VAdjacency neighb;
  VNodeBaseRec base;
  int nadj;

  base   = node->base;
  neighb = base.head;

  nadj = 0;
  while ((neighb != NULL)) {
    neighb = neighb->next;
    nadj++;
  }
  return nadj;
}


/*
** get length of branch
*/
void
BranchLength(VGraph graph, SNode node, int mlen, int *len)
{
  VAdjacency neighb;
  VNodeBaseRec base;
  SNode adj;
  int nadj = 0;

  if (node == NULL) return;

  if (VNodeTestVisit(node)) return;
  if (VNodeTestDestroy(node)) return;
  VNodeSetVisit(node);
  (*len)++;
  if (*len > mlen) return;

  base = node->base;
  neighb = base.head;

  while ((neighb != NULL)) {
    adj = (SNode) VGraphGetNode(graph,neighb->id);
    neighb = neighb->next;
    if (VNodeTestVisit(adj)) continue;
    nadj = VGraphNumNeighbours(graph,adj);

     /* break, if junction or endpoint reached: */
    if ((nadj > 2) || (nadj <= 1 && (*len) > 1)) return; 
    BranchLength(graph,adj,mlen,len);
  }
}


/*
** prune a branch starting from an endpoint, and go "inwards"
** until a junction or another endpoint is reached.
** Mark all nodes along the way for deletion.
*/
void
BranchPrune(VGraph graph, SNode node,int *len)
{
  VAdjacency neighb;
  VNodeBaseRec base;
  SNode adj;
  int nadj = 0;

  if (node == NULL) return;

  if (VNodeTestVisit(node)) return;
  if (VNodeTestDestroy(node)) return;
  VNodeSetVisit(node);
  VNodeSetDestroy(node);
  (*len)++;

  base = node->base;
  neighb = base.head;

  while ((neighb != NULL)) {
    adj = (SNode) VGraphGetNode(graph,neighb->id);
    neighb = neighb->next;
    if (VNodeTestVisit(adj)) continue;

    nadj = VGraphNumNeighbours(graph,adj);
    if (nadj > 2) return;               /* junction point reached */

    if ((nadj <= 1) && ((*len) > 1)) return; /* another endpoint */
    BranchPrune(graph,adj,len);
  }
}


/*!
\fn VGraph VGraphPrune(VGraph src,int minlength)
\brief Graph pruning.
\param src   input graph. Nodes must of type 'SNode'.
\param minlength branches that are shorter than <minlength> are cut off.
*/
VGraph
VGraphPrune(VGraph src,int minlength)
{
  VGraph dest;
  SNode node,node0,node1,node2;
  int i,j,n,nadj,nnodes,nlinks,len;
  VAdjacency neighb;
  VNodeBaseRec base;
  int mlen;

  mlen = minlength;

  /*
  ** prune inwards starting at endpts
  */
  for (i=1; i<=src->lastUsed; i++) {
    node = (SNode) VGraphGetNode (src,i);
    if (node == NULL) continue;

    nadj = VGraphNumNeighbours(src,node); /* count number of neighbours */

    if (nadj == 0)  {    /* isolated point */
      VNodeSetVisit(node);
      VNodeSetDestroy(node);
    }

    else if (nadj == 1)  {    /* if only one neighbour, it's an endpoint */

      /* measure length of path from endpoint to nearest junction */

      VGraphClearVisit(src);
      len = 0;
      BranchLength(src,node,mlen,&len);

      /* if too small, prune */
      if (len < minlength) {
	VGraphClearVisit(src);
	len = 0;
	BranchPrune(src,node,&len);
      }
    }
  }

  /*
  ** create output graph
  */
  n = VGraphNNodes (src);
  dest = VCreateGraph(n,5,VShortRepn,FALSE);

  nnodes = 0;
  for (i=1; i<=src->lastUsed; i++) {
    node0 = (SNode) VGraphGetNode (src,i);
    if (node0 == NULL) continue;
    if (VNodeTestDestroy(node0)) continue;

    node2 = (SNode) VMalloc(sizeof(SNodeRec));
    node2->base.hops    = 0;
    node2->base.visited = 0;
    node2->base.head    = 0;
    node2->base.weight  = 0;

    node2->type  = 4;
    node2->band  = node0->band;
    node2->row   = node0->row;
    node2->col   = node0->col;
    node2->label = node0->label;
    n = VGraphAddNode(dest,(VNode) node2);
    nnodes++;
  }

  /*
  ** get links between nodes
  */
  nlinks = 0;
  for (i=1; i<=dest->lastUsed; i++) {
    node0 = (SNode) VGraphGetNode (dest,i);
    if (node0 == 0) continue;

    n = VGraphLookupNode (src,(VNode) node0);
    node1 = (SNode) VGraphGetNode (src,n);

    base   = node1->base;
    neighb = base.head;
    
    while ((neighb != NULL)) {

      node2 = (SNode) VGraphGetNode(src,neighb->id);
      neighb = neighb->next;
      if (VNodeTestDestroy(node2)) continue;
      j = VGraphLookupNode (dest,(VNode) node2);
      VGraphLinkNodes(dest,i,j);

      nlinks++;
    }
  }

  /*
  ** restore src graph
  */    
  for (i=1; i<=src->lastUsed; i++) {
    node0 = (SNode) VGraphGetNode (src,i);
    if (node0 == NULL) continue;
    if (VNodeTestDestroy(node0))
      VNodeClearDestroy(node0);
  }

  /*
  ** output
  */
  VGraphAttrList (dest) = VCopyAttrList (VImageAttrList (src));

  return dest;
}
