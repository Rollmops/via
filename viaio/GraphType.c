/*
 *  $Id: GraphType.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains methods for the graph (VGraph) type.
 */

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/os.h"
#include "viaio/VGraph.h"

/*
 *  Table of methods.
 */

/* Later in this file: */
static VDecodeMethod VGraphDecodeMethod;
static VEncodeAttrMethod VGraphEncodeAttrMethod;
static VEncodeDataMethod VGraphEncodeDataMethod;

/* Used in Type.c to register this type: */
VTypeMethods VGraphMethods = {
  (VCopyMethod *) VCopyGraph,		/* copy a VGraph */
  (VDestroyMethod *) VDestroyGraph,	/* destroy a VGraph */
  VGraphDecodeMethod,			/* decode a VGraph's value */
  VGraphEncodeAttrMethod,		/* encode a VGraph's attr list */
  VGraphEncodeDataMethod		/* encode a VGraph's binary data */
};

/*
 *  VGraphDecodeMethod
 *
 *  The "decode" method registered for the "Graph" type.
 *  Convert an attribute list plus binary data to a VGraph object.
 */

static VPointer VGraphDecodeMethod (VStringConst name, VBundle b)
{
  VGraph graph;
  VLong nnodes=0, nfields, node_repn, useWeights;
  VAttrList list;
  VLong idx, nadj;
  int length;
  size_t len;
  VNode n;
  VPointer p, ptr;
  VAdjacency adj;

#define Extract(name, dict, locn, required)	\
	VExtractAttr (b->list, name, dict, VLongRepn, & locn, required)

  /* Extract the required attribute values for Graph. */
  if (! Extract (VNGraphNodesAttr, NULL, nnodes, FALSE) ||
      ! Extract (VRepnAttr, VNumericRepnDict, node_repn, TRUE) ||
      ! Extract (VNNodeFieldsAttr, NULL, nfields, TRUE) ||
      ! Extract (VNNodeWeightsAttr, NULL, useWeights, TRUE))
    return NULL;

  if (nnodes <= 0) nnodes = 1024;
  if (nnodes <= 0 || nfields <= 0) {
    VWarning ("VGraphReadDataMethod: Bad Graph file attributes");
    return NULL;
  }

  /* Create the Graph data structure. */
  graph = VCreateGraph ((int) nnodes, (int) nfields,
			(VRepnKind) node_repn, (int) useWeights);
  if (! graph)
    return NULL;

  /* Give it whatever attributes remain: */
  list = VGraphAttrList (graph);
  VGraphAttrList (graph) = b->list;
  b->list = list;
    
  length = b->length;
  if (length == 0) return graph;
  p = b->data;

#define unpack(repn, cnt, dest) \
    ptr = dest; \
    if (VUnpackData(repn, cnt, p, VMsbFirst, & len, & ptr, 0) == 0) return 0; \
    p = (char *) p + len; length -= len; len = length; \
    if (length < 0) goto Fail;
  len = length;

  while (length > 0) {

    /* Get the index : */
    unpack(VLongRepn, 1, &idx);
    graph->table[idx-1] = n = VCalloc(1, VNodeSize(graph));
    if (idx > graph->lastUsed) graph->lastUsed = idx;

    /* Get the number of adjacencies : */
    unpack(VLongRepn, 1, &nadj);
	
    /* Unpack the adjacencies : */
    while (nadj--)  {
      adj = VMalloc(sizeof(VAdjRec));
      unpack(VLongRepn, 1, &adj->id);
      if (graph->useWeights)  {
	unpack(VFloatRepn, 1, &adj->weight);
      } else
	adj->weight = 0.0;
      adj->next = n->base.head; n->base.head = adj;
    };	    

    /* Unpack the node itself: */
    if (graph->useWeights) {
      unpack(VFloatRepn, 1, &(n->base.weight));
    } else
      n->base.weight = 0.0;
    unpack(graph->node_repn, graph->nfields, n->data);
  }
  return graph;

 Fail:
  VWarning ("VGraphDecodeMethod: %s graph has wrong data length", name);
  VDestroyGraph (graph);
  return NULL;
#undef Extract
}


/*
 *  VGraphEncodeAttrMethod
 *
 *  The "encode_attrs" method registered for the "Graph" type.
 *  Encode an attribute list value for a VGraph object.
 */

static VAttrList VGraphEncodeAttrMethod (VPointer value, size_t *lengthp)
{
  VGraph graph = value;
  VAttrList list;
  size_t len, nadj;
  int i, slong, sfloat, spriv;
  VNode n;
  VAdjacency adj;

  /* Temporarily prepend several attributes to the edge set's list: */
  if ((list = VGraphAttrList (graph)) == NULL)
    list = VGraphAttrList (graph) = VCreateAttrList ();
  VPrependAttr (list, VRepnAttr, VNumericRepnDict,
		VLongRepn, (VLong) graph->node_repn);
  VPrependAttr (list, VNNodeFieldsAttr, NULL, VLongRepn,
		(VLong) graph->nfields);
  VPrependAttr (list, VNGraphNodesAttr, NULL, VLongRepn,
		(VLong) graph->nnodes);
  VPrependAttr (list, VNNodeWeightsAttr, NULL, VLongRepn,
		(VLong) graph->useWeights);

  /* Compute the file space needed for the Graph's binary data: */
  len = 0;
  slong = VRepnPrecision (VLongRepn) / 8;
  sfloat = VRepnPrecision (VFloatRepn) / 8;
  spriv = graph->nfields * VRepnPrecision (graph->node_repn) / 8;

  for (i = 1; i <= graph->size; i++) {
    n = VGraphGetNode(graph, i); if (n == 0) continue;

    /* Count the number of adjacencies : */
    for (adj = n->base.head, nadj = 0; adj; adj = adj->next) nadj++;

    /* each node contains:
     * an index and the number of adjacencies
     * the private data area
     * the list of adjacencies
     * optionally reserve space for weights
     */
    len += 2 * slong + nadj * slong + spriv;
    if (graph->useWeights) len += (nadj+1) * sfloat;
  };
  *lengthp = len;

  return list;
}


/*
 *  VGraphEncodeDataMethod
 *
 *  The "encode_data" method registered for the "Graph" type.
 *  Encode the edge and point fields for a VGraph object.
 */

static VPointer VGraphEncodeDataMethod (VPointer value, VAttrList list,
					size_t length, VBoolean *free_itp)
{
  VGraph graph = value;
  VAttrListPosn posn;
  VNode n;
  size_t len;
  VPointer p, ptr;
  VAdjacency adj;
  int i, nadj;

#define pack(repn, cnt, dest) \
    if (! VPackData (repn, cnt, dest, VMsbFirst, &len, &p, NULL)) return NULL; \
    p = (char *) p + len; length -= len; len = length;
    
  /* Remove the attributes prepended by the VGraphEncodeAttrsMethod: */
  for (VFirstAttr (list, & posn);
       strcmp (VGetAttrName (& posn), VRepnAttr) != 0; VDeleteAttr (& posn));
  VDeleteAttr (& posn);

  /* Allocate a buffer for the encoded data: */
  if (length == 0)  {
    *free_itp = FALSE;
    return value;			/* we may return anything != 0 here */
  };
    
  p = ptr = VMalloc (length);
  len = length;

  /* Pack each node: */
  for (i = 1; i <= graph->size; i++) {

    n = VGraphGetNode(graph, i); if (n == 0) continue;

    /* Count the number of adjacencies : */
    for (adj = n->base.head, nadj = 0; adj; adj = adj->next) nadj++;
	
    /* Pack the header */
    pack(VLongRepn, 1, &i);
    pack(VLongRepn, 1, &nadj);

    /* Pack the adjacencies : */
    for (adj = n->base.head; adj; adj = adj->next)  {
      pack(VLongRepn, 1, &adj->id);
      if (graph->useWeights) { pack(VFloatRepn, 1, &adj->weight); };
    };
	    
    /* Pack the node itself: */
    if (graph->useWeights) { pack(VFloatRepn, 1, &(n->base.weight)); };
    pack(graph->node_repn, graph->nfields, n->data);
  }

  *free_itp = TRUE;
  return ptr;
}

/*
 *  VCreateGraph
 *
 *  Allocates memory for a VGraph structure and initializes its fields.
 *    Initially, this contains an empty node table, so each node must still
 *    be initialized.
 *  Returns a pointer to the graph if successful, NULL otherwise.
 */

VGraph VCreateGraph (int nnodes, int nfields, VRepnKind repn, int useW)
{
  VGraph graph;

  /* Check parameters: */
  if (nnodes < 1  || nfields < 1)
    VWarning ("VCreateGraph: Invalid number of nodes or fields.");

  /* Allocate memory for the VGraph, and the node table: */
  graph = VMalloc (sizeof (VGraphRec));
  if (graph == NULL) return NULL;

  graph->table = VCalloc(nnodes, sizeof(VNode));
  if (graph->table == NULL) {
    VFree(graph);
    return NULL;
  };

  /* Initialize the VGraph: */
  graph->nnodes = nnodes;
  graph->nfields = nfields;
  graph->node_repn = repn;
  graph->attributes = VCreateAttrList ();
  graph->lastUsed = 0;
  graph->size = nnodes;
  graph->useWeights = useW;
  graph->iter = 0;

  return graph;
}

static void VDestroyNodeSimple(VGraph graph, int i)
     /* simple deletion: just look at structures of this node */
{
  VAdjacency p, q;
  VNode n;
    
  n = VGraphGetNode(graph, i); if (n == 0) return;
    
  /* destroy adjacency list */
  for (p = n->base.head; p; p = q)  {
    q = p->next; VFree(p);
  };
  VFree(n);

  VGraphGetNode(graph, i) = 0;
}

/*
 *  VDestroyGraph
 *
 *  Frees memory occupied by a graph.
 */

void VDestroyGraph (VGraph graph)
{
  int i;

  /* destroy each node */
  for (i = 1; i <= graph->size; i++) VDestroyNodeSimple(graph, i);
    
  /* destroy the table */
  VFree (graph->table);
  graph->table = 0; graph->size = 0;	/* again, make it sure */
  VDestroyAttrList (graph->attributes);
  VFree (graph);
}

/*
 *   VCopyNodeShallow
 *
 *   Copy a node excluding links
 *
 */
 
static VNode VCopyNodeShallow (VGraph graph, VNode src)
{
  VNode dst;
    
  if (src == 0) return 0;
  dst = VCalloc(1, VNodeSize(graph));
  memcpy(dst, src, VNodeSize(graph));
  dst->base.head = 0;
  return dst;
}

/*
 *   VCopyNodeDeep
 *
 *   Copy a node including links
 *
 */
 
static VNode VCopyNodeDeep(VGraph graph, VNode src)
{
  VNode dst;
  VAdjacency o, n;
  int cnt;
    
  if (src == 0) return 0;

  /* allocate and copy base part */
  dst = VCalloc(1, VNodeSize(graph));
  dst->base.hops = src->base.hops;
  dst->base.visited = src->base.visited;
  dst->base.weight = src->base.weight;
  dst->base.head = 0;
    
  /* copy all adjacencies */
  for (o = src->base.head; o; o = o->next)  {
    n = VMalloc(sizeof(VAdjRec));
    n->id = o->id; n->weight = o->weight;
    n->next = dst->base.head;
    dst->base.head = n;
  };

  /* copy private area */
  cnt = (graph->nfields * VRepnPrecision(graph->node_repn)) / 8;
  memcpy(dst->data, src->data, cnt);
  return dst;
}

/*
 *  VCopyGraph
 *
 *  Copy a VGraph object.
 *  Note that no compaction is performed, since this would require
 *  a recalculation of all indices,
 */

VGraph VCopyGraph (VGraph src)
{
  VGraph dst;
  int i;
    
  dst = VCreateGraph (src->size, src->nfields, src->node_repn, src->useWeights);

  /* copy each used node in table */
  for (i = 1; i <= src->size; i++)
    dst->table[i-1] = VCopyNodeDeep(src, VGraphGetNode(src, i));

  dst->nnodes = src->nnodes;
  dst->lastUsed = src->lastUsed;
 
  if (VGraphAttrList (dst))
    VDestroyAttrList (VGraphAttrList (dst));
  if (VGraphAttrList (src))
    VGraphAttrList (dst) = VCopyAttrList (VGraphAttrList (src));
  return dst;
}


/*
 *  VReadGraphs
 *
 *  Read a Vista data file, extract the graphs from it, and return a list
 *  of them.
 */

int VReadGraphs (FILE *file, VAttrList *attrs, VGraph **graphs)
{
  return VReadObjects(file, VGraphRepn, attrs, (VPointer **)graphs);
}


/*
 *  VWriteGraphs
 *
 *  Write a list of graphs to a Vista data file.
 */

VBoolean VWriteGraphs (FILE *file, VAttrList attrs,
		       int n, VGraph graphs[])
{
  return VWriteObjects(file, VGraphRepn, attrs, n, (VPointer *)graphs);
}

/*
 *  VGraphLookupNode
 *
 *  Find a node in a Vista graph structure.
 *  Return reference to this node.
 */

int VGraphLookupNode (VGraph graph, VNode node)
{
  int n = (graph->nfields * VRepnPrecision(graph->node_repn)) / 8;
  int i;
    
  for (i = 1; i <= graph->lastUsed; i++)  {
    if (VGraphNodeIsFree(graph, i)) continue;
    if (memcmp(node->data, VGraphGetNode(graph, i)->data, n) == 0)
      return i;
  };
  return 0;
}

static int growGraph (VGraph graph)
     /* note that we grow just a pointer table */
{
  int newsize = (graph->size * 3) / 2;
  VNode *t = VCalloc(newsize, sizeof(VNode));
  if (t == 0) return 0;
  memcpy(t, graph->table, graph->size * sizeof(VNode));
  VFree(graph->table); graph->table = t;
  graph->size = newsize; graph->nnodes = newsize; return newsize;
}

/*
 *  VGraphAddNode
 *
 *  Add a node to a Vista graph structure.
 *  Return reference to this node.
 */

int VGraphAddNode (VGraph graph, VNode node)
{
  int i = VGraphLookupNode (graph, node);
  if (i) return i;
  if (graph->lastUsed == graph->size)
    if (growGraph(graph) == 0) return 0;
  graph->table[graph->lastUsed++] = VCopyNodeShallow(graph, node);
  return graph->lastUsed;
}

/*
 *  VGraphAddNodeAt
 *
 *  Add a node to a Vista graph structure at a specific position.
 *  Note that we do not check for duplicates.
 *  Return reference to this node.
 */

int VGraphAddNodeAt (VGraph graph, VNode node, int position)
{
  VDestroyNodeSimple(graph, position);
  VGraphGetNode(graph, position) = VCopyNodeShallow(graph, node);
  if (position > graph->lastUsed) graph->lastUsed = position;
  return position;
}

/*
 *  VGraphLinkNodes
 *
 *  Make a link between to nodes.
 *  Return TRUE if successful.
 */

int VGraphLinkNodes (VGraph graph, int a, int b)
{
  VNode n;
  VAdjacency adj,neighb;
  VNodeBaseRec base;
    
  if (VGraphNodeIsFree(graph, a) || VGraphNodeIsFree(graph, b)) return FALSE;
  n = VGraphGetNode(graph, a);

  /* check if link already exists, updated by G.L. Mar. 2004 */
  base   = n->base;
  neighb = base.head;
  while ((neighb != NULL)) {
    if (neighb->id == b) return TRUE;
    neighb = neighb->next;
  }

  /* if not, append it to adj-list */
  adj = VMalloc(sizeof(VAdjRec));
  adj->id = b; adj->weight = 0;
  adj->next = n->base.head;
  n->base.head = adj;
  return TRUE;
}    

/*
 *  VGraphUnlinkNodes
 *
 *  unlinks two nodes.
 *  Return TRUE if successful.
 */

int VGraphUnlinkNodes (VGraph graph, int a, int b)
{
  VNode n;
  VAdjacency adj, prev;
    
  if (VGraphNodeIsFree(graph, a) || VGraphNodeIsFree(graph, b)) return FALSE;
  n = VGraphGetNode(graph, a);
  prev = 0;
  for (adj = n->base.head; adj; adj = adj->next)  {
    if (adj->id == (unsigned int)b)  {
      if (prev)
	prev->next = adj->next;
      else
	n->base.head = adj->next;
      VFree(adj);
      return TRUE;
    };
    prev = adj;
  };
  return FALSE;
}    

static VNode seqNode(VGraph graph, int i)
{
  while (i <= graph->lastUsed)  {
    VNode n = VGraphGetNode(graph, i);
    if (n)  { graph->iter = i; return n; };
    i++;
  };
  return 0;
}

VPointer VGraphFirstNode(VGraph graph)
{
  return graph? seqNode(graph, 1): 0;
}

VPointer VGraphNextNode(VGraph graph)
{
  return graph? seqNode(graph, graph->iter+1): 0;
}

void VGraphClearVisit(VGraph graph)
{
  VNode n;
  int i;
    
  if (graph == 0) return;
  for (i = 1; i <= graph->lastUsed; i++)  {
    if (VGraphNodeIsFree(graph, i)) continue;
    n = VGraphGetNode(graph, i);
    VNodeClearVisit(&n->base);
  };
}

/*
 *  VGraphResizeFields
 *
 *  Grow private data area of each node to newfields.
 *  Return TRUE if successful.
 */

int VGraphResizeFields (VGraph graph, int newfields)
{
  VNode o, n;
  int i;
  int nsize = sizeof(VNodeBaseRec) + (newfields * VRepnPrecision(graph->node_repn)) / 8;
  int osize = VNodeSize(graph);
  if (newfields <= graph->nfields) return TRUE;
  for (i = 1; i <= graph->lastUsed; i++)  {
    if (VGraphNodeIsFree(graph, i)) continue;
    o = VGraphGetNode(graph, i);
    n = VCalloc(1, nsize);
    memcpy(n, o, osize);
    VGraphGetNode(graph, i) = n; VFree(o);
  };
  graph->nfields = newfields;
  return TRUE;
}    

static int firstUnvisitedNode(VGraph graph)
{
  int i;
    
  for (i = 1; i <= graph->lastUsed; i++)  {
    VNode n = VGraphGetNode(graph, i);
    if (n && n->base.hops == 0) return i;
  };
  return 0;
}

static void blockConnection(VGraph graph, int i, int j)
{
  VAdjacency adj;
  VNode n = VGraphGetNode(graph, i);
  if (n == 0) return;
  for (adj = n->base.head; adj; adj = adj->next)
    if (adj->id == (unsigned int)j) { adj->weight = -1; return; };
}

/*
 *  VGraphVisitNodesFrom
 *
 *  Visits all node in a graph connected to node i
 */

int VGraphVisitNodesFrom(VGraph graph, int i)
{
  VAdjacency adj;
  VNode n, p;
  int cycles = 0;
    
  if (graph == 0 || (n = VGraphGetNode(graph, i)) == 0) return 0;
  if (n->base.hops > 0) return 1;
  n->base.hops++;

  for (adj = n->base.head; adj; adj = adj->next)  {
    p = VGraphGetNode(graph, adj->id);
    if (p && p->base.hops > 0) continue;
    cycles += VGraphVisitNodesFrom(graph, adj->id);
  };
  return cycles;
}

/*
 *  VGraphClearHops
 *
 *  Clears the hops field in a graph
 */

void VGraphClearHops(VGraph graph)
{
  int i;
    
  if (graph == 0) return;
  for (i = 1; i <= graph->lastUsed; i++)  {
    VNode n = VGraphGetNode(graph, i);
    if (n) n->base.hops = 0;
  };
}

/*
 *  VGraphNCycles
 *
 *  Returns number of cycles in a graph
 */

int VGraphNCycles (VGraph graph)
{
  int cycles = 0;

  VGraphClearHops(graph);
  while (1)  {
    /* get the first unvisited node */
    int n = firstUnvisitedNode(graph);
    if (n == 0) return cycles;
    cycles += VGraphVisitNodesFrom(graph, n);
  };
}
    
/*
 *  VGraphExtractNodes
 *
 *  Copies nodes from a graph in which the visited flag is set
 *  and places them into a new graph
 */

VGraph VGraphExtractNodes (VGraph src)
{
  VGraph dst;
  VAdjacency adj;
  VNode n;
  int i, j;
    
  /* create a destination graph much like src */
  /* 1000 nodes is just a guess */
  dst = VCreateGraph(1000, src->nfields, src->node_repn, src->useWeights);

  /* copy selected nodes from src */
  for (i = j = 1; i <= src->lastUsed; i++)  {
    n = VGraphGetNode(src, i);
    if (n && n->base.hops) dst->table[j] = VCopyNodeShallow(src, n);
  };

  /* set number of nodes used */
  dst->nnodes = src->nnodes;
  dst->lastUsed = j;
 
  /* now link nodes in new graph */
  for (i = 1; i <= dst->lastUsed; i++) {
    n = VGraphGetNode(dst, i);
    if (n == 0) continue;
    	    
    j = VGraphLookupNode(src, n);
    if (j == 0) continue;
    n = VGraphGetNode(src, j);
    for (adj = n->base.head; adj; adj = adj->next)  {
      n = VGraphGetNode(src, adj->id);
      j = VGraphLookupNode(dst, n);
      if (j) VGraphLinkNodes(dst, i, j);
    };
  };
    
  if (VGraphAttrList (dst))
    VDestroyAttrList (VGraphAttrList (dst));
  if (VGraphAttrList (src))
    VGraphAttrList (dst) = VCopyAttrList (VGraphAttrList (src));
  return dst;
}
    
/*
 *  VGraphToggleNodesFrom
 *
 *  Visits all node in a graph connected to node i and toggles the hops field
 *  note that the visit field must have been cleared before
 *
 */

void VGraphToggleNodesFrom(VGraph graph, int i)
{
  VAdjacency adj;
  VNode n, p;
    
  /* find a valid starting point */
  if (graph == 0 || (n = VGraphGetNode(graph, i)) == 0) return;

  /* mark this node and toggle the hops field */
  if (n->base.visited == 1) return;
  n->base.visited = 1;
  n->base.hops = n->base.hops? 0: 1;

  /* now look at the neighbors */
  for (adj = n->base.head; adj; adj = adj->next)  {
    p = VGraphGetNode(graph, adj->id);
    if (p && p->base.visited) continue;
    VGraphToggleNodesFrom(graph, adj->id);
  };
}

void VDestroyNode(VGraph graph, int i)
     /* complex deletion: look at all connected structures of this node */
{
  VAdjacency p, q;
  VNode n;
    
  n = VGraphGetNode(graph, i); if (n == 0) return;
    
  /* destroy adjacency list */
  for (p = n->base.head; p; p = q)  {
    /* remove connection from other node to this node */
    VGraphUnlinkNodes(graph, p->id, i);
    q = p->next; VFree(p);
  };
  VFree(n);

  VGraphGetNode(graph, i) = 0;
}

static void VGraphRemoveNodes(VGraph graph)
     /* remove all nodes in which the visited flag is set */
{
  int i;
    
  for (i = 1; i <= graph->lastUsed; i++)  {
    VNode n = VGraphGetNode(graph, i);
    if (n && n->base.visited) VDestroyNode(graph, i);
  };
}
    
/*
 *  VGraphDestroyNodesFrom
 *
 *  Destroys nodes from a graph in which the hops field is set
 */

void VGraphDestroyNodesFrom(VGraph graph, int i)
{
  VGraphToggleNodesFrom(graph, i);
  VGraphRemoveNodes(graph);
}
