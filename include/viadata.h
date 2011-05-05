
#include <viaio/VImage.h>
#include <viaio/VGraph.h>
#include <viaio/Volumes.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
  \struct SNode
  \brief data struct for representing nodes in a graph.
  \param VShort <b> col</b> column index
  \param VShort <b>row</b> row index
  \param VShort <b>band</b> slice (band) index
  \param VShort <b>label</b> node label
  
 \par Author:
 Gabriele Lohmann, MPI-CBS
*/
typedef struct SNodeStruct {
  VNodeBaseRec base;
  VShort type;
  VShort col;
  VShort row; 
  VShort band;
  VShort label;
} SNodeRec, *SNode;



/*!
  \struct FNode
  \brief data struct for representing nodes in a graph.
  \param VFloat <b>x</b> column index
  \param VFloat <b>y</b> row index
  \param VFloat <b>z</b> slice (band) index
  \param VFloat <b>val</b> node label
  
 \par Author:
 Gabriele Lohmann, MPI-CBS
*/
typedef struct FNodeStruct {
  VNodeBaseRec base;
  VFloat type;
  VFloat x;
  VFloat y; 
  VFloat z;
  VFloat label;
} FNodeRec, *FNode;


/*!
  \struct VoxelList
  \brief data struct for representing voxels.
  \param short <b>b</b> slice (band) index
  \param short <b>r</b> row index
  \param short <b>c</b> column index
  
 \par Author:
 Gabriele Lohmann, MPI-CBS
*/
typedef struct {
  short b;
  short r;
  short c;
} Voxel, *VoxelList;



/*!
  \struct PixelList
  \brief data struct for representing pixels.
  \param short <b>r</b> row index
  \param short <b>c</b> column index
  
 \par Author:
 Gabriele Lohmann, MPI-CBS
*/
typedef struct {
  short r;
  short c;
} Pixel, *PixelList;


/*!
  \struct VPoint
  \brief data struct for representing labelled voxels. The label is a float value. 
  \param short <b>b</b> slice (band) index
  \param short <b>r</b> row index
  \param short <b>c</b> column index
  \param float <b>val</b> label value
  
 \par Author:
 Gabriele Lohmann, MPI-CBS
*/
typedef struct {
  short b;
  short r;
  short c;
  float val;
} VPoint;


/*!
  \struct XPoint
  \brief data struct for representing 3D points.
  \param float <b>x</b> slice (band) index
  \param float <b>y</b> row index
  \param float <b>z</b> column index
  
 \par Author:
 Gabriele Lohmann, MPI-CBS
*/
typedef struct pointStruct{
  float x;
  float y;
  float z;
} XPoint;



/*
** access to a pixel
*/
#define VPixelIndex(image, i) \
   ((VPointer) ((char *) (VImageData(image) + (i) * VPixelSize (image))))

/*
** fast implementation of rounding
*/
#define VRintPos(x) ((int)((x) + 0.5))
#define VRintNeg(x) ((int)((x) - 0.5))
#define VRint(x)  ((x) >= 0 ? VRintPos(x) : VRintNeg(x))

#define VFloor(x) ((x) >= 0 ? (int)((x+0.000001)) : (int)((x) - 0.999999))

#define VCeilPos(x) (VRint((x)) == (x) ? (x) : (int)(x+0.99999))
#define VCeilNeg(x) (VRint((x)) == (x) ? (x) : (int)(x+0.00001))
#define VCeil(x) ((x) >= 0 ? VCeilPos(x) : VCeilNeg(x))


/*
** some other stuff
*/
#define VSqr(x) ((x) * (x))
#define VAbs(x) ((x) > 0 > ? (x) : -(x))


#ifdef __cplusplus
}                               /* extern "C" */
#endif                          /* __cplusplus */

