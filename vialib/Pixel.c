/*
** Contrast enhancement
**
** Author:
**  G.Lohmann <lohmann@cns.mpg.de>, May 1998
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/VGraph.h>
#include <viaio/mu.h>

#include <stdio.h>
#include <stdlib.h>
#include <via.h>



VFloat
VReadPixel(VImage src,int b,int r,int c)
{
  switch(VPixelRepn(src)) {
  case VUByteRepn:
    return (VFloat) VPixel(src,b,r,c,VUByte);
  case VShortRepn:
    return (VFloat) VPixel(src,b,r,c,VShort);
  case VFloatRepn:
    return (VFloat) VPixel(src,b,r,c,VFloat);
  case VBitRepn:
    return (VFloat) VPixel(src,b,r,c,VBit);
  case VSByteRepn:
    return (VFloat) VPixel(src,b,r,c,VSByte);
  case VDoubleRepn:
    return (VFloat) VPixel(src,b,r,c,VDouble);
  }
}


void
VWritePixel(VImage src,int b,int r,int c,VFloat val)
{
  switch(VPixelRepn(src)) {
  case VUByteRepn:
    VPixel(src,b,r,c,VUByte) = val;
    return;
  case VShortRepn:
    VPixel(src,b,r,c,VShort) = val;
    return;
  case VFloatRepn:
    VPixel(src,b,r,c,VFloat) = val;
    return;
  case VBitRepn:
    VPixel(src,b,r,c,VBit) = val;
    return;
  case VSByteRepn:
    VPixel(src,b,r,c,VSByte) = val;
    return;
  case VDoubleRepn:
    VPixel(src,b,r,c,VDouble) = val;
    return;
  }
}





VFloat 
VGetPixelValue (VImage image, int i)
{
  VPointer p = (VPointer) VPixelIndex (image, i);

  switch (VPixelRepn (image)) {

  case VUByteRepn:
    return (VFloat) * (VUByte *) p;

  case VShortRepn:
    return (VFloat) * (VShort *) p;

  case VBitRepn:
    return (VFloat) * (VBit *) p;

  case VFloatRepn:
    return (VFloat) * (VFloat *) p;

  case VSByteRepn:
    return (VFloat) * (VFloat *) p;

  default:
    ;
  }
  return 0.0;
}


void 
VSetPixelValue (VImage image, int i, VFloat value)
{
  VPointer p = VPixelIndex (image, i);

  switch (VPixelRepn (image)) {

  case VUByteRepn:
    * (VUByte *) p = value;
  break;

  case VShortRepn:
    * (VShort *) p = value;
  break;

  case VBitRepn:
    * (VBit *) p = value;
  break;

  case VFloatRepn:
    * (VFloat *) p = value;
  break;

  case VSByteRepn:
    * (VFloat *) p = value;
  break;

  default:
    ;
  }
}




void
VReadNode(VGraph graph,VNode node,float *x,float *y,float *z,float *val)
{
  VPointer p;
  size_t size;

  *x = *y = *z = 0;
  if (node == NULL) return;

  switch (VNodeRepn(graph)) {
  case VShortRepn:
    size = 2;
    p = node->data;
    p += size;
    *x = (VFloat) * (VShort *) p;
    p += size;
    *y = (VFloat) * (VShort *) p;
    p += size;
    *z = (VFloat) * (VShort *) p;
    p += size;
    *val = (VFloat) * (VShort *) p;
    break;

  case VFloatRepn:
    size = 4;
    p = node->data;
    p += size;
    *x = (VFloat) * (VFloat *) p;
    p += size;
    *y = (VFloat) * (VFloat *) p;
    p += size;
    *z = (VFloat) * (VFloat *) p;
    p += size;
    *val = (VFloat) * (VFloat *) p;
    break;
    
  default:
    ;
  }
}


void
VWriteNode(VGraph graph,VNode node,float x,float y,float z,float val)
{
  VPointer p;
  size_t size;

  if (node == NULL) return;

  switch (VNodeRepn(graph)) {
  case VShortRepn:
    size = 2;
    p = node->data;
    p += size;
     * (VShort *) p = x;
    p += size;
    * (VShort *) p = y;
    p += size;
    * (VShort *) p = z;
    p += size;
    * (VShort *) p = val;
    break;

  case VFloatRepn:
    size = 4;
    p = node->data;
    p += size;
    * (VFloat *) p = x;
    p += size;
    * (VFloat *) p = y;
    p += size;
    * (VFloat *) p = z;
    p += size;
    * (VFloat *) p = val;
    break;
    
  default:
    ;
  }
}

