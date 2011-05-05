/*! \file
 NonMaxima Supression by gradient interpolation

This function is used in conjunction with edge detection.
The input into this function is the output of a 3D edge
detection procedure (e.h. VCanny3d or VDeriche3d).
It produces an output image containing the gradient magnitude.
Nonmaxima edges are suppressed.

\par Authors:
Alex Seidel, (TU Muenchen) and
Gabriele Lohmann, MPI-CBS
*/


#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>
#include <viaio/os.h>

#include <stdio.h>
#include <math.h>

#define TYPE VFloat
#define SQR(x) ((x)*(x))

enum {
  xgrad,
  ygrad,
  zgrad,
  mgrad
};

enum {
  no,
  yes
};
extern VImage vgrad3d(VImage,VImage,VLong,VShort);

enum {
  plane1=1,
  plane2=2,
  plane3=3
  };

int GetPlane(TYPE, TYPE, TYPE );
int IntersectWithPlane(int plane,TYPE, TYPE, TYPE, VDouble *Sa, VDouble *Sb); 
double Interpolate(VImage dx,VImage dy,VImage dz,int Quarter,VDouble Sa, VDouble Sb);         

int NBands,NRows,NColumns;
int band,row,column;

/*!
\fn VImage VNonmaxSuppression(VImage dx, VImage dy, VImage dz,VImage dest)
\param dx  input gradient in column direction (float repn)
\param dy  input gradient in row direction (float repn)
\param dz  input gradient in slice direction (float repn)
\param dest   output image (ubyte repn)
*/
VImage VNonmaxSuppression(VImage dx, VImage dy, VImage dz,VImage dest)
{
  TYPE Voxeldx,Voxeldy,Voxeldz;
  int plane,Quarter;
  long RemovedVoxels=0,VoxelCount=0;
  VDouble maximum=0,magakt,mag1,Sa,Sb,value;
  VUByte base = 0;

  NBands=VImageNBands(dx);  
  NRows=VImageNRows(dx);
  NColumns=VImageNColumns(dx);

  dest = VSelectDestImage ("VNonmaxSuppression", NULL, NBands, NRows, NColumns, VFloatRepn);
  for (band=0 ; band < NBands;band++)
    {
      for (column = 0 ; column < NColumns ; column++)
	for (row = 0 ; row < NRows ; row++) 
	  {
	    Voxeldx=VPixel(dx,band,row,column,TYPE);
	    Voxeldy=VPixel(dy,band,row,column,TYPE);
	    Voxeldz=VPixel(dz,band,row,column,TYPE);
	    magakt=sqrt(Voxeldx*Voxeldx+Voxeldy*Voxeldy+Voxeldz*Voxeldz);
	    if (magakt==0) VPixel(dest,band,row,column,VFloat)=0;
	    else
	      {
		VoxelCount++;
		/* determine the plane parallel to the given gradient and where the current point lies */
		plane=GetPlane(Voxeldx,Voxeldy,Voxeldz); 
          
		/* calculates the point of intersection (Sa,Sb), 
		   returns a code for the quarter it lies in also */  
		Quarter=IntersectWithPlane(plane,Voxeldx,Voxeldy,Voxeldz,&Sa,&Sb);
          
		/* calculates a interpolated value from the neighbourhood */
		mag1=Interpolate(dx,dy,dz,Quarter,Sa,Sb);         
		if (mag1 <= magakt) 
		  {
		    value=magakt+base;  
		    if (value > maximum){
		      maximum=value;
		    }
		  }
		else 
		  { 
		    value=base;
		    RemovedVoxels++;
		  }
		VPixel(dest,band,row,column,VFloat)=value; 
	      }
	  }
    }
    

  VCopyImageAttrs (dx, dest);
  return dest;
}

/* The GetPlane is determing the plane which is used to 
   intersect with the vector of the gradient*/
int GetPlane(TYPE xscomp, TYPE yscomp, TYPE zscomp)
{
  VDouble xcomp,ycomp,zcomp;
   
  xcomp = (double) xscomp;
  ycomp = (double) yscomp;
  zcomp = (double) zscomp;
  if (xcomp==0.0) xcomp=0.000001;
  if (zcomp==0.0) zcomp=0.000001;


  if (fabs(ycomp/zcomp)>1)
    if (fabs(ycomp/xcomp)>1)  
      return plane2;
    else
      return plane3;
  else 
    if (fabs(xcomp/zcomp)<1)  
      return plane1;  
    else
      return plane3;
}

/* Calculates the point of intersection of the gradientvector with the choosen plane then
   returns the number of the quarter wich was involved */
int IntersectWithPlane(int plane,TYPE dx,TYPE dy,TYPE dz,VDouble *Sa,VDouble *Sb)
{
  VDouble lamda;
  switch(plane)
    {
    case plane1:  lamda = -1.0/dz;
      *Sa=lamda*dx;
      *Sb=lamda*dy;
      if ((*Sa)>=0) 
	if ((*Sb)>=0) return 3;
	else    return 2;
      else
	if ((*Sb)>=0) return 4;
	else    return 1;
    case plane2:  lamda = -1.0/dy;
      *Sa=lamda*dx;
      *Sb=lamda*dz;
      if ((*Sa)>=0) 
	if ((*Sb)>=0) return 7;
	else    return 6;
      else
	if ((*Sb)>=0) return 8;
	else    return 5;
    case plane3:  lamda =     1.0/dx;
      *Sa=lamda*dy;
      *Sb=lamda*dz;
      if ((*Sa)>=0) 
	if ((*Sb)>=0) return 11;
	else    return 12;
      else
	if ((*Sb)>=0) return 10;
	else    return 9;
    }
  return 0;
} 

VDouble Interpolate (VImage dx, VImage dy, VImage dz,int QuarterOfPlane,VDouble Sa,VDouble Sb)
{
  int bandm1,bandp1,rowm1,rowp1,columnp1,columnm1;
  VDouble G1,G2,G3,G4,H1,H2,H3,H4,G,H;

  Sa=fabs(Sa);
  Sb=fabs(Sb);
  
  
  /* Clipping */
  if (band==0)   bandm1=band;
  else bandm1=band-1;

  if (band>=NBands-1) bandp1=band;
  else bandp1=band+1;

  if (row==0)    rowm1=row;
  else rowm1=row-1;

  if (row>=NRows-1) rowp1=row;
  else rowp1=row+1;

  if (column==0)   columnm1=column;
  else columnm1=column-1;

  if (column>=NColumns-1) columnp1=column;      
  else columnp1=column+1;

  /* 
   * now choosing the proper neighbours for the interpolation formula
   */
  switch(QuarterOfPlane)
    {
    case 1:
      G1= sqrt(SQR(VPixel(dx,bandm1,row,column,TYPE))+
	       SQR(VPixel(dy,bandm1,row,column,TYPE))+
	       SQR(VPixel(dz,bandm1,row,column,TYPE)));

      H1= sqrt(SQR(VPixel(dx,bandp1,row,column,TYPE))+
	       SQR(VPixel(dy,bandp1,row,column,TYPE))+
	       SQR(VPixel(dz,bandp1,row,column,TYPE)));

      G2= sqrt(SQR(VPixel(dx,bandm1,row,columnm1,TYPE))+
	       SQR(VPixel(dy,bandm1,row,columnm1,TYPE))+
	       SQR(VPixel(dz,bandm1,row,columnm1,TYPE)));

      H2= sqrt(SQR(VPixel(dx,bandp1,row,columnp1,TYPE))+
	       SQR(VPixel(dy,bandp1,row,columnp1,TYPE))+
	       SQR(VPixel(dz,bandp1,row,columnp1,TYPE)));

      G3= sqrt(SQR(VPixel(dx,bandm1,rowm1,columnm1,TYPE))+
	       SQR(VPixel(dy,bandm1,rowm1,columnm1,TYPE))+
	       SQR(VPixel(dz,bandm1,rowm1,columnm1,TYPE)));

      H3= sqrt(SQR(VPixel(dx,bandp1,rowp1,columnp1,TYPE))+
	       SQR(VPixel(dy,bandp1,rowp1,columnp1,TYPE))+
	       SQR(VPixel(dz,bandp1,rowp1,columnp1,TYPE)));

      G4= sqrt(SQR(VPixel(dx,bandm1,rowm1,column,TYPE))+
	       SQR(VPixel(dy,bandm1,rowm1,column,TYPE))+
	       SQR(VPixel(dz,bandm1,rowm1,column,TYPE)));

      H4= sqrt(SQR(VPixel(dx,bandp1,rowp1,column,TYPE))+
	       SQR(VPixel(dy,bandp1,rowp1,column,TYPE))+
	       SQR(VPixel(dz,bandp1,rowp1,column,TYPE)));
      break;

    case 2:
      G1= sqrt(VPixel(dx,bandm1,row,column,TYPE)*VPixel(dx,bandm1,row,column,TYPE)+
	       VPixel(dy,bandm1,row,column,TYPE)*VPixel(dy,bandm1,row,column,TYPE)+
	       VPixel(dz,bandm1,row,column,TYPE)*VPixel(dz,bandm1,row,column,TYPE));
      H1= sqrt(VPixel(dx,bandp1,row,column,TYPE)*VPixel(dx,bandp1,row,column,TYPE)+
	       VPixel(dy,bandp1,row,column,TYPE)*VPixel(dy,bandp1,row,column,TYPE)+
	       VPixel(dz,bandp1,row,column,TYPE)*VPixel(dz,bandp1,row,column,TYPE));
      G2= sqrt(VPixel(dx,bandm1,row,columnp1,TYPE)*VPixel(dx,bandm1,row,columnp1,TYPE)+
	       VPixel(dy,bandm1,row,columnp1,TYPE)*VPixel(dy,bandm1,row,columnp1,TYPE)+
	       VPixel(dz,bandm1,row,columnp1,TYPE)*VPixel(dz,bandm1,row,columnp1,TYPE));
      H2= sqrt(VPixel(dx,bandp1,row,columnm1,TYPE)*VPixel(dx,bandp1,row,columnm1,TYPE)+
	       VPixel(dy,bandp1,row,columnm1,TYPE)*VPixel(dy,bandp1,row,columnm1,TYPE)+
	       VPixel(dz,bandp1,row,columnm1,TYPE)*VPixel(dz,bandp1,row,columnm1,TYPE)); 
      G3= sqrt(VPixel(dx,bandm1,rowm1,columnp1,TYPE)*VPixel(dx,bandm1,rowm1,columnp1,TYPE)+
	       VPixel(dy,bandm1,rowm1,columnp1,TYPE)*VPixel(dy,bandm1,rowm1,columnp1,TYPE)+
	       VPixel(dz,bandm1,rowm1,columnp1,TYPE)*VPixel(dz,bandm1,rowm1,columnp1,TYPE));
      H3= sqrt(VPixel(dx,bandp1,rowp1,columnm1,TYPE)*VPixel(dx,bandp1,rowp1,columnm1,TYPE)+
	       VPixel(dy,bandp1,rowp1,columnm1,TYPE)*VPixel(dy,bandp1,rowp1,columnm1,TYPE)+
	       VPixel(dz,bandp1,rowp1,columnm1,TYPE)*VPixel(dz,bandp1,rowp1,columnm1,TYPE));
      G4= sqrt(VPixel(dx,bandm1,rowm1,column,TYPE)*VPixel(dx,bandm1,rowm1,column,TYPE)+
	       VPixel(dy,bandm1,rowm1,column,TYPE)*VPixel(dy,bandm1,rowm1,column,TYPE)+
	       VPixel(dz,bandm1,rowm1,column,TYPE)*VPixel(dz,bandm1,rowm1,column,TYPE));
      H4= sqrt(VPixel(dx,bandp1,rowp1,column,TYPE)*VPixel(dx,bandp1,rowp1,column,TYPE)+
	       VPixel(dy,bandp1,rowp1,column,TYPE)*VPixel(dy,bandp1,rowp1,column,TYPE)+
	       VPixel(dz,bandp1,rowp1,column,TYPE)*VPixel(dz,bandp1,rowp1,column,TYPE));
      break;

    case 3:
      G1= sqrt(VPixel(dx,bandm1,row,column,TYPE)*VPixel(dx,bandm1,row,column,TYPE)+
	       VPixel(dy,bandm1,row,column,TYPE)*VPixel(dy,bandm1,row,column,TYPE)+
	       VPixel(dz,bandm1,row,column,TYPE)*VPixel(dz,bandm1,row,column,TYPE));
      H1= sqrt(VPixel(dx,bandp1,row,column,TYPE)*VPixel(dx,bandp1,row,column,TYPE)+
	       VPixel(dy,bandp1,row,column,TYPE)*VPixel(dy,bandp1,row,column,TYPE)+
	       VPixel(dz,bandp1,row,column,TYPE)*VPixel(dz,bandp1,row,column,TYPE));
      G2= sqrt(VPixel(dx,bandm1,row,columnp1,TYPE)*VPixel(dx,bandm1,row,columnp1,TYPE)+
	       VPixel(dy,bandm1,row,columnp1,TYPE)*VPixel(dy,bandm1,row,columnp1,TYPE)+
	       VPixel(dz,bandm1,row,columnp1,TYPE)*VPixel(dz,bandm1,row,columnp1,TYPE));
      H2= sqrt(VPixel(dx,bandp1,row,columnm1,TYPE)*VPixel(dx,bandp1,row,columnm1,TYPE)+
	       VPixel(dy,bandp1,row,columnm1,TYPE)*VPixel(dy,bandp1,row,columnm1,TYPE)+
	       VPixel(dz,bandp1,row,columnm1,TYPE)*VPixel(dz,bandp1,row,columnm1,TYPE)); 
      G3= sqrt(VPixel(dx,bandm1,rowp1,columnp1,TYPE)*VPixel(dx,bandm1,rowp1,columnp1,TYPE)+
	       VPixel(dy,bandm1,rowp1,columnp1,TYPE)*VPixel(dy,bandm1,rowp1,columnp1,TYPE)+
	       VPixel(dz,bandm1,rowp1,columnp1,TYPE)*VPixel(dz,bandm1,rowp1,columnp1,TYPE));
      H3= sqrt(VPixel(dx,bandp1,rowm1,columnm1,TYPE)*VPixel(dx,bandp1,rowm1,columnm1,TYPE)+
	       VPixel(dy,bandp1,rowm1,columnm1,TYPE)*VPixel(dy,bandp1,rowm1,columnm1,TYPE)+
	       VPixel(dz,bandp1,rowm1,columnm1,TYPE)*VPixel(dz,bandp1,rowm1,columnm1,TYPE));
      G4= sqrt(VPixel(dx,bandm1,rowp1,column,TYPE)*VPixel(dx,bandm1,rowp1,column,TYPE)+
	       VPixel(dy,bandm1,rowp1,column,TYPE)*VPixel(dy,bandm1,rowp1,column,TYPE)+
	       VPixel(dz,bandm1,rowp1,column,TYPE)*VPixel(dz,bandm1,rowp1,column,TYPE));
      H4= sqrt(VPixel(dx,bandp1,rowm1,column,TYPE)*VPixel(dx,bandp1,rowm1,column,TYPE)+
	       VPixel(dy,bandp1,rowm1,column,TYPE)*VPixel(dy,bandp1,rowm1,column,TYPE)+
	       VPixel(dz,bandp1,rowm1,column,TYPE)*VPixel(dz,bandp1,rowm1,column,TYPE));
      break;      

    case 4:
      G1= sqrt(VPixel(dx,bandm1,row,column,TYPE)*VPixel(dx,bandm1,row,column,TYPE)+
	       VPixel(dy,bandm1,row,column,TYPE)*VPixel(dy,bandm1,row,column,TYPE)+
	       VPixel(dz,bandm1,row,column,TYPE)*VPixel(dz,bandm1,row,column,TYPE));
      H1= sqrt(VPixel(dx,bandp1,row,column,TYPE)*VPixel(dx,bandp1,row,column,TYPE)+
	       VPixel(dy,bandp1,row,column,TYPE)*VPixel(dy,bandp1,row,column,TYPE)+
	       VPixel(dz,bandp1,row,column,TYPE)*VPixel(dz,bandp1,row,column,TYPE));
      G2= sqrt(VPixel(dx,bandm1,row,columnm1,TYPE)*VPixel(dx,bandm1,row,columnm1,TYPE)+
	       VPixel(dy,bandm1,row,columnm1,TYPE)*VPixel(dy,bandm1,row,columnm1,TYPE)+
	       VPixel(dz,bandm1,row,columnm1,TYPE)*VPixel(dz,bandm1,row,columnm1,TYPE));
      H2= sqrt(VPixel(dx,bandp1,row,columnp1,TYPE)*VPixel(dx,bandp1,row,columnp1,TYPE)+
	       VPixel(dy,bandp1,row,columnp1,TYPE)*VPixel(dy,bandp1,row,columnp1,TYPE)+
	       VPixel(dz,bandp1,row,columnp1,TYPE)*VPixel(dz,bandp1,row,columnp1,TYPE)); 
      G3= sqrt(VPixel(dx,bandm1,rowp1,columnm1,TYPE)*VPixel(dx,bandm1,rowp1,columnm1,TYPE)+
	       VPixel(dy,bandm1,rowp1,columnm1,TYPE)*VPixel(dy,bandm1,rowp1,columnm1,TYPE)+
	       VPixel(dz,bandm1,rowp1,columnm1,TYPE)*VPixel(dz,bandm1,rowp1,columnm1,TYPE));
      H3= sqrt(VPixel(dx,bandp1,rowm1,columnp1,TYPE)*VPixel(dx,bandp1,rowm1,columnp1,TYPE)+
	       VPixel(dy,bandp1,rowm1,columnp1,TYPE)*VPixel(dy,bandp1,rowm1,columnp1,TYPE)+
	       VPixel(dz,bandp1,rowm1,columnp1,TYPE)*VPixel(dz,bandp1,rowm1,columnp1,TYPE));
      G4= sqrt(VPixel(dx,bandm1,rowp1,column,TYPE)*VPixel(dx,bandm1,rowp1,column,TYPE)+
	       VPixel(dy,bandm1,rowp1,column,TYPE)*VPixel(dy,bandm1,rowp1,column,TYPE)+
	       VPixel(dz,bandm1,rowp1,column,TYPE)*VPixel(dz,bandm1,rowp1,column,TYPE));
      H4= sqrt(VPixel(dx,bandp1,rowm1,column,TYPE)*VPixel(dx,bandp1,rowm1,column,TYPE)+
	       VPixel(dy,bandp1,rowm1,column,TYPE)*VPixel(dy,bandp1,rowm1,column,TYPE)+
	       VPixel(dz,bandp1,rowm1,column,TYPE)*VPixel(dz,bandp1,rowm1,column,TYPE));
      break;

    case 5:
      G1= sqrt(VPixel(dx,band,rowm1,column,TYPE)*VPixel(dx,band,rowm1,column,TYPE)+
	       VPixel(dy,band,rowm1,column,TYPE)*VPixel(dy,band,rowm1,column,TYPE)+
	       VPixel(dz,band,rowm1,column,TYPE)*VPixel(dz,band,rowm1,column,TYPE));
      H1= sqrt(VPixel(dx,band,rowp1,column,TYPE)*VPixel(dx,band,rowp1,column,TYPE)+
	       VPixel(dy,band,rowp1,column,TYPE)*VPixel(dy,band,rowp1,column,TYPE)+
	       VPixel(dz,band,rowp1,column,TYPE)*VPixel(dz,band,rowp1,column,TYPE));
      G2= sqrt(VPixel(dx,band,rowm1,columnm1,TYPE)*VPixel(dx,band,rowm1,columnm1,TYPE)+
	       VPixel(dy,band,rowm1,columnm1,TYPE)*VPixel(dy,band,rowm1,columnm1,TYPE)+
	       VPixel(dz,band,rowm1,columnm1,TYPE)*VPixel(dz,band,rowm1,columnm1,TYPE));
      H2= sqrt(VPixel(dx,band,rowp1,columnp1,TYPE)*VPixel(dx,band,rowp1,columnp1,TYPE)+
	       VPixel(dy,band,rowp1,columnp1,TYPE)*VPixel(dy,band,rowp1,columnp1,TYPE)+
	       VPixel(dz,band,rowp1,columnp1,TYPE)*VPixel(dz,band,rowp1,columnp1,TYPE)); 
      G3= sqrt(VPixel(dx,bandm1,rowm1,columnm1,TYPE)*VPixel(dx,bandm1,rowm1,columnm1,TYPE)+
	       VPixel(dy,bandm1,rowm1,columnm1,TYPE)*VPixel(dy,bandm1,rowm1,columnm1,TYPE)+
	       VPixel(dz,bandm1,rowm1,columnm1,TYPE)*VPixel(dz,bandm1,rowm1,columnm1,TYPE));
      H3= sqrt(VPixel(dx,bandp1,rowp1,columnp1,TYPE)*VPixel(dx,bandp1,rowp1,columnp1,TYPE)+
	       VPixel(dy,bandp1,rowp1,columnp1,TYPE)*VPixel(dy,bandp1,rowp1,columnp1,TYPE)+
	       VPixel(dz,bandp1,rowp1,columnp1,TYPE)*VPixel(dz,bandp1,rowp1,columnp1,TYPE));
      G4= sqrt(VPixel(dx,bandm1,rowm1,column,TYPE)*VPixel(dx,bandm1,rowm1,column,TYPE)+
	       VPixel(dy,bandm1,rowm1,column,TYPE)*VPixel(dy,bandm1,rowm1,column,TYPE)+
	       VPixel(dz,bandm1,rowm1,column,TYPE)*VPixel(dz,bandm1,rowm1,column,TYPE));
      H4= sqrt(VPixel(dx,bandp1,rowp1,column,TYPE)*VPixel(dx,bandp1,rowp1,column,TYPE)+
	       VPixel(dy,bandp1,rowp1,column,TYPE)*VPixel(dy,bandp1,rowp1,column,TYPE)+
	       VPixel(dz,bandp1,rowp1,column,TYPE)*VPixel(dz,bandp1,rowp1,column,TYPE));
      break;

    case 6:
      G1= sqrt(VPixel(dx,band,rowm1,column,TYPE)*VPixel(dx,band,rowm1,column,TYPE)+
	       VPixel(dy,band,rowm1,column,TYPE)*VPixel(dy,band,rowm1,column,TYPE)+
	       VPixel(dz,band,rowm1,column,TYPE)*VPixel(dz,band,rowm1,column,TYPE));
      H1= sqrt(VPixel(dx,band,rowp1,column,TYPE)*VPixel(dx,band,rowp1,column,TYPE)+
	       VPixel(dy,band,rowp1,column,TYPE)*VPixel(dy,band,rowp1,column,TYPE)+
	       VPixel(dz,band,rowp1,column,TYPE)*VPixel(dz,band,rowp1,column,TYPE));
      G2= sqrt(VPixel(dx,band,rowm1,columnp1,TYPE)*VPixel(dx,band,rowm1,columnp1,TYPE)+
	       VPixel(dy,band,rowm1,columnp1,TYPE)*VPixel(dy,band,rowm1,columnp1,TYPE)+
	       VPixel(dz,band,rowm1,columnp1,TYPE)*VPixel(dz,band,rowm1,columnp1,TYPE));
      H2= sqrt(VPixel(dx,band,rowp1,columnm1,TYPE)*VPixel(dx,band,rowp1,columnm1,TYPE)+
	       VPixel(dy,band,rowp1,columnm1,TYPE)*VPixel(dy,band,rowp1,columnm1,TYPE)+
	       VPixel(dz,band,rowp1,columnm1,TYPE)*VPixel(dz,band,rowp1,columnm1,TYPE)); 
      G3= sqrt(VPixel(dx,bandm1,rowm1,columnp1,TYPE)*VPixel(dx,bandm1,rowm1,columnp1,TYPE)+
	       VPixel(dy,bandm1,rowm1,columnp1,TYPE)*VPixel(dy,bandm1,rowm1,columnp1,TYPE)+
	       VPixel(dz,bandm1,rowm1,columnp1,TYPE)*VPixel(dz,bandm1,rowm1,columnp1,TYPE));
      H3= sqrt(VPixel(dx,bandp1,rowp1,columnm1,TYPE)*VPixel(dx,bandp1,rowp1,columnm1,TYPE)+
	       VPixel(dy,bandp1,rowp1,columnm1,TYPE)*VPixel(dy,bandp1,rowp1,columnm1,TYPE)+
	       VPixel(dz,bandp1,rowp1,columnm1,TYPE)*VPixel(dz,bandp1,rowp1,columnm1,TYPE));
      G4= sqrt(VPixel(dx,bandm1,rowm1,column,TYPE)*VPixel(dx,bandm1,rowm1,column,TYPE)+
	       VPixel(dy,bandm1,rowm1,column,TYPE)*VPixel(dy,bandm1,rowm1,column,TYPE)+
	       VPixel(dz,bandm1,rowm1,column,TYPE)*VPixel(dz,bandm1,rowm1,column,TYPE));
      H4= sqrt(VPixel(dx,bandp1,rowp1,column,TYPE)*VPixel(dx,bandp1,rowp1,column,TYPE)+
	       VPixel(dy,bandp1,rowp1,column,TYPE)*VPixel(dy,bandp1,rowp1,column,TYPE)+
	       VPixel(dz,bandp1,rowp1,column,TYPE)*VPixel(dz,bandp1,rowp1,column,TYPE));
      break;

    case 7:

      G1= sqrt(VPixel(dx,band,rowm1,column,TYPE)*VPixel(dx,band,rowm1,column,TYPE)+
	       VPixel(dy,band,rowm1,column,TYPE)*VPixel(dy,band,rowm1,column,TYPE)+
	       VPixel(dz,band,rowm1,column,TYPE)*VPixel(dz,band,rowm1,column,TYPE));
      H1= sqrt(VPixel(dx,band,rowp1,column,TYPE)*VPixel(dx,band,rowp1,column,TYPE)+
	       VPixel(dy,band,rowp1,column,TYPE)*VPixel(dy,band,rowp1,column,TYPE)+
	       VPixel(dz,band,rowp1,column,TYPE)*VPixel(dz,band,rowp1,column,TYPE));
      G2= sqrt(VPixel(dx,band,rowm1,columnp1,TYPE)*VPixel(dx,band,rowm1,columnp1,TYPE)+
	       VPixel(dy,band,rowm1,columnp1,TYPE)*VPixel(dy,band,rowm1,columnp1,TYPE)+
	       VPixel(dz,band,rowm1,columnp1,TYPE)*VPixel(dz,band,rowm1,columnp1,TYPE));
      H2= sqrt(VPixel(dx,band,rowp1,columnm1,TYPE)*VPixel(dx,band,rowp1,columnm1,TYPE)+
	       VPixel(dy,band,rowp1,columnm1,TYPE)*VPixel(dy,band,rowp1,columnm1,TYPE)+
	       VPixel(dz,band,rowp1,columnm1,TYPE)*VPixel(dz,band,rowp1,columnm1,TYPE)); 
      G3= sqrt(VPixel(dx,bandp1,rowm1,columnp1,TYPE)*VPixel(dx,bandp1,rowm1,columnp1,TYPE)+
	       VPixel(dy,bandp1,rowm1,columnp1,TYPE)*VPixel(dy,bandp1,rowm1,columnp1,TYPE)+
	       VPixel(dz,bandp1,rowm1,columnp1,TYPE)*VPixel(dz,bandp1,rowm1,columnp1,TYPE));
      H3= sqrt(VPixel(dx,bandm1,rowp1,columnm1,TYPE)*VPixel(dx,bandm1,rowp1,columnm1,TYPE)+
	       VPixel(dy,bandm1,rowp1,columnm1,TYPE)*VPixel(dy,bandm1,rowp1,columnm1,TYPE)+
	       VPixel(dz,bandm1,rowp1,columnm1,TYPE)*VPixel(dz,bandm1,rowp1,columnm1,TYPE));
      G4= sqrt(VPixel(dx,bandp1,rowm1,column,TYPE)*VPixel(dx,bandp1,rowm1,column,TYPE)+
	       VPixel(dy,bandp1,rowm1,column,TYPE)*VPixel(dy,bandp1,rowm1,column,TYPE)+
	       VPixel(dz,bandp1,rowm1,column,TYPE)*VPixel(dz,bandp1,rowm1,column,TYPE));
      H4= sqrt(VPixel(dx,bandm1,rowp1,column,TYPE)*VPixel(dx,bandm1,rowp1,column,TYPE)+
	       VPixel(dy,bandm1,rowp1,column,TYPE)*VPixel(dy,bandm1,rowp1,column,TYPE)+
	       VPixel(dz,bandm1,rowp1,column,TYPE)*VPixel(dz,bandm1,rowp1,column,TYPE));
      break;

    case 8:
      G1= sqrt(VPixel(dx,band,rowm1,column,TYPE)*VPixel(dx,band,rowm1,column,TYPE)+
	       VPixel(dy,band,rowm1,column,TYPE)*VPixel(dy,band,rowm1,column,TYPE)+
	       VPixel(dz,band,rowm1,column,TYPE)*VPixel(dz,band,rowm1,column,TYPE));
      H1= sqrt(VPixel(dx,band,rowp1,column,TYPE)*VPixel(dx,band,rowp1,column,TYPE)+
	       VPixel(dy,band,rowp1,column,TYPE)*VPixel(dy,band,rowp1,column,TYPE)+
	       VPixel(dz,band,rowp1,column,TYPE)*VPixel(dz,band,rowp1,column,TYPE));
      G2= sqrt(VPixel(dx,band,rowm1,columnm1,TYPE)*VPixel(dx,band,rowm1,columnm1,TYPE)+
	       VPixel(dy,band,rowm1,columnm1,TYPE)*VPixel(dy,band,rowm1,columnm1,TYPE)+
	       VPixel(dz,band,rowm1,columnm1,TYPE)*VPixel(dz,band,rowm1,columnm1,TYPE));
      H2= sqrt(VPixel(dx,band,rowp1,columnp1,TYPE)*VPixel(dx,band,rowp1,columnp1,TYPE)+
	       VPixel(dy,band,rowp1,columnp1,TYPE)*VPixel(dy,band,rowp1,columnp1,TYPE)+
	       VPixel(dz,band,rowp1,columnp1,TYPE)*VPixel(dz,band,rowp1,columnp1,TYPE)); 
      G3= sqrt(VPixel(dx,bandp1,rowm1,columnm1,TYPE)*VPixel(dx,bandp1,rowm1,columnm1,TYPE)+
	       VPixel(dy,bandp1,rowm1,columnm1,TYPE)*VPixel(dy,bandp1,rowm1,columnm1,TYPE)+
	       VPixel(dz,bandp1,rowm1,columnm1,TYPE)*VPixel(dz,bandp1,rowm1,columnm1,TYPE));
      H3= sqrt(VPixel(dx,bandm1,rowp1,columnp1,TYPE)*VPixel(dx,bandm1,rowp1,columnp1,TYPE)+
	       VPixel(dy,bandm1,rowp1,columnp1,TYPE)*VPixel(dy,bandm1,rowp1,columnp1,TYPE)+
	       VPixel(dz,bandm1,rowp1,columnp1,TYPE)*VPixel(dz,bandm1,rowp1,columnp1,TYPE));
      G4= sqrt(VPixel(dx,bandp1,rowm1,column,TYPE)*VPixel(dx,bandp1,rowm1,column,TYPE)+
	       VPixel(dy,bandp1,rowm1,column,TYPE)*VPixel(dy,bandp1,rowm1,column,TYPE)+
	       VPixel(dz,bandp1,rowm1,column,TYPE)*VPixel(dz,bandp1,rowm1,column,TYPE));
      H4= sqrt(VPixel(dx,bandm1,rowp1,column,TYPE)*VPixel(dx,bandm1,rowp1,column,TYPE)+
	       VPixel(dy,bandm1,rowp1,column,TYPE)*VPixel(dy,bandm1,rowp1,column,TYPE)+
	       VPixel(dz,bandm1,rowp1,column,TYPE)*VPixel(dz,bandm1,rowp1,column,TYPE));
      break;

    case 9:
      G1= sqrt(VPixel(dx,band,row,columnp1,TYPE)*VPixel(dx,band,row,columnp1,TYPE)+
	       VPixel(dy,band,row,columnp1,TYPE)*VPixel(dy,band,row,columnp1,TYPE)+
	       VPixel(dz,band,row,columnp1,TYPE)*VPixel(dz,band,row,columnp1,TYPE));
      H1= sqrt(VPixel(dx,band,row,columnm1,TYPE)*VPixel(dx,band,row,columnm1,TYPE)+
	       VPixel(dy,band,row,columnm1,TYPE)*VPixel(dy,band,row,columnm1,TYPE)+
	       VPixel(dz,band,row,columnm1,TYPE)*VPixel(dz,band,row,columnm1,TYPE));
      G2= sqrt(VPixel(dx,band,rowm1,columnp1,TYPE)*VPixel(dx,band,rowm1,columnp1,TYPE)+
	       VPixel(dy,band,rowm1,columnp1,TYPE)*VPixel(dy,band,rowm1,columnp1,TYPE)+
	       VPixel(dz,band,rowm1,columnp1,TYPE)*VPixel(dz,band,rowm1,columnp1,TYPE));
      H2= sqrt(VPixel(dx,band,rowp1,columnm1,TYPE)*VPixel(dx,band,rowp1,columnm1,TYPE)+
	       VPixel(dy,band,rowp1,columnm1,TYPE)*VPixel(dy,band,rowp1,columnm1,TYPE)+
	       VPixel(dz,band,rowp1,columnm1,TYPE)*VPixel(dz,band,rowp1,columnm1,TYPE)); 
      G3= sqrt(VPixel(dx,bandm1,rowm1,columnp1,TYPE)*VPixel(dx,bandm1,rowm1,columnp1,TYPE)+
	       VPixel(dy,bandm1,rowm1,columnp1,TYPE)*VPixel(dy,bandm1,rowm1,columnp1,TYPE)+
	       VPixel(dz,bandm1,rowm1,columnp1,TYPE)*VPixel(dz,bandm1,rowm1,columnp1,TYPE));
      H3= sqrt(VPixel(dx,bandp1,rowp1,columnm1,TYPE)*VPixel(dx,bandp1,rowp1,columnm1,TYPE)+
	       VPixel(dy,bandp1,rowp1,columnm1,TYPE)*VPixel(dy,bandp1,rowp1,columnm1,TYPE)+
	       VPixel(dz,bandp1,rowp1,columnm1,TYPE)*VPixel(dz,bandp1,rowp1,columnm1,TYPE));
      G4= sqrt(VPixel(dx,bandm1,row,columnp1,TYPE)*VPixel(dx,bandm1,row,columnp1,TYPE)+
	       VPixel(dy,bandm1,row,columnp1,TYPE)*VPixel(dy,bandm1,row,columnp1,TYPE)+
	       VPixel(dz,bandm1,row,columnp1,TYPE)*VPixel(dz,bandm1,row,columnp1,TYPE));
      H4= sqrt(VPixel(dx,bandp1,row,columnm1,TYPE)*VPixel(dx,bandp1,row,columnm1,TYPE)+
	       VPixel(dy,bandp1,row,columnm1,TYPE)*VPixel(dy,bandp1,row,columnm1,TYPE)+
	       VPixel(dz,bandp1,row,columnm1,TYPE)*VPixel(dz,bandp1,row,columnm1,TYPE));
      break;

    case 10:
      G1= sqrt(VPixel(dx,band,row,columnp1,TYPE)*VPixel(dx,band,row,columnp1,TYPE)+
	       VPixel(dy,band,row,columnp1,TYPE)*VPixel(dy,band,row,columnp1,TYPE)+
	       VPixel(dz,band,row,columnp1,TYPE)*VPixel(dz,band,row,columnp1,TYPE));
      H1= sqrt(VPixel(dx,band,row,columnm1,TYPE)*VPixel(dx,band,row,columnm1,TYPE)+
	       VPixel(dy,band,row,columnm1,TYPE)*VPixel(dy,band,row,columnm1,TYPE)+
	       VPixel(dz,band,row,columnm1,TYPE)*VPixel(dz,band,row,columnm1,TYPE));
      G2= sqrt(VPixel(dx,band,rowm1,columnp1,TYPE)*VPixel(dx,band,rowm1,columnp1,TYPE)+
	       VPixel(dy,band,rowm1,columnp1,TYPE)*VPixel(dy,band,rowm1,columnp1,TYPE)+
	       VPixel(dz,band,rowm1,columnp1,TYPE)*VPixel(dz,band,rowm1,columnp1,TYPE));
      H2= sqrt(VPixel(dx,band,rowp1,columnm1,TYPE)*VPixel(dx,band,rowp1,columnm1,TYPE)+
	       VPixel(dy,band,rowp1,columnm1,TYPE)*VPixel(dy,band,rowp1,columnm1,TYPE)+
	       VPixel(dz,band,rowp1,columnm1,TYPE)*VPixel(dz,band,rowp1,columnm1,TYPE)); 
      G3= sqrt(VPixel(dx,bandp1,rowm1,columnp1,TYPE)*VPixel(dx,bandp1,rowm1,columnp1,TYPE)+
	       VPixel(dy,bandp1,rowm1,columnp1,TYPE)*VPixel(dy,bandp1,rowm1,columnp1,TYPE)+
	       VPixel(dz,bandp1,rowm1,columnp1,TYPE)*VPixel(dz,bandp1,rowm1,columnp1,TYPE));
      H3= sqrt(VPixel(dx,bandm1,rowp1,columnm1,TYPE)*VPixel(dx,bandm1,rowp1,columnm1,TYPE)+
	       VPixel(dy,bandm1,rowp1,columnm1,TYPE)*VPixel(dy,bandm1,rowp1,columnm1,TYPE)+
	       VPixel(dz,bandm1,rowp1,columnm1,TYPE)*VPixel(dz,bandm1,rowp1,columnm1,TYPE));
      G4= sqrt(VPixel(dx,bandp1,row,columnp1,TYPE)*VPixel(dx,bandp1,row,columnp1,TYPE)+
	       VPixel(dy,bandp1,row,columnp1,TYPE)*VPixel(dy,bandp1,row,columnp1,TYPE)+
	       VPixel(dz,bandp1,row,columnp1,TYPE)*VPixel(dz,bandp1,row,columnp1,TYPE));
      H4= sqrt(VPixel(dx,bandm1,row,columnm1,TYPE)*VPixel(dx,bandm1,row,columnm1,TYPE)+
	       VPixel(dy,bandm1,row,columnm1,TYPE)*VPixel(dy,bandm1,row,columnm1,TYPE)+
	       VPixel(dz,bandm1,row,columnm1,TYPE)*VPixel(dz,bandm1,row,columnm1,TYPE));
      break;  
  
    case 11:
      G1= sqrt(VPixel(dx,band,row,columnp1,TYPE)*VPixel(dx,band,row,columnp1,TYPE)+
	       VPixel(dy,band,row,columnp1,TYPE)*VPixel(dy,band,row,columnp1,TYPE)+
	       VPixel(dz,band,row,columnp1,TYPE)*VPixel(dz,band,row,columnp1,TYPE));
      H1= sqrt(VPixel(dx,band,row,columnm1,TYPE)*VPixel(dx,band,row,columnm1,TYPE)+
	       VPixel(dy,band,row,columnm1,TYPE)*VPixel(dy,band,row,columnm1,TYPE)+
	       VPixel(dz,band,row,columnm1,TYPE)*VPixel(dz,band,row,columnm1,TYPE));
      G2= sqrt(VPixel(dx,band,rowp1,columnp1,TYPE)*VPixel(dx,band,rowp1,columnp1,TYPE)+
	       VPixel(dy,band,rowp1,columnp1,TYPE)*VPixel(dy,band,rowp1,columnp1,TYPE)+
	       VPixel(dz,band,rowp1,columnp1,TYPE)*VPixel(dz,band,rowp1,columnp1,TYPE));
      H2= sqrt(VPixel(dx,band,rowm1,columnm1,TYPE)*VPixel(dx,band,rowm1,columnm1,TYPE)+
	       VPixel(dy,band,rowm1,columnm1,TYPE)*VPixel(dy,band,rowm1,columnm1,TYPE)+
	       VPixel(dz,band,rowm1,columnm1,TYPE)*VPixel(dz,band,rowm1,columnm1,TYPE)); 
      G3= sqrt(VPixel(dx,bandp1,rowp1,columnp1,TYPE)*VPixel(dx,bandp1,rowp1,columnp1,TYPE)+
	       VPixel(dy,bandp1,rowp1,columnp1,TYPE)*VPixel(dy,bandp1,rowp1,columnp1,TYPE)+
	       VPixel(dz,bandp1,rowp1,columnp1,TYPE)*VPixel(dz,bandp1,rowp1,columnp1,TYPE));
      H3= sqrt(VPixel(dx,bandm1,rowm1,columnm1,TYPE)*VPixel(dx,bandm1,rowm1,columnm1,TYPE)+
	       VPixel(dy,bandm1,rowm1,columnm1,TYPE)*VPixel(dy,bandm1,rowm1,columnm1,TYPE)+
	       VPixel(dz,bandm1,rowm1,columnm1,TYPE)*VPixel(dz,bandm1,rowm1,columnm1,TYPE));
      G4= sqrt(VPixel(dx,bandp1,row,columnp1,TYPE)*VPixel(dx,bandp1,row,columnp1,TYPE)+
	       VPixel(dy,bandp1,row,columnp1,TYPE)*VPixel(dy,bandp1,row,columnp1,TYPE)+
	       VPixel(dz,bandp1,row,columnp1,TYPE)*VPixel(dz,bandp1,row,columnp1,TYPE));
      H4= sqrt(VPixel(dx,bandm1,row,columnm1,TYPE)*VPixel(dx,bandm1,row,columnm1,TYPE)+
	       VPixel(dy,bandm1,row,columnm1,TYPE)*VPixel(dy,bandm1,row,columnm1,TYPE)+
	       VPixel(dz,bandm1,row,columnm1,TYPE)*VPixel(dz,bandm1,row,columnm1,TYPE));
      break;
  
    case 12:
      G1= sqrt(VPixel(dx,band,row,columnp1,TYPE)*VPixel(dx,band,row,columnp1,TYPE)+
	       VPixel(dy,band,row,columnp1,TYPE)*VPixel(dy,band,row,columnp1,TYPE)+
	       VPixel(dz,band,row,columnp1,TYPE)*VPixel(dz,band,row,columnp1,TYPE));
      H1= sqrt(VPixel(dx,band,row,columnm1,TYPE)*VPixel(dx,band,row,columnm1,TYPE)+
	       VPixel(dy,band,row,columnm1,TYPE)*VPixel(dy,band,row,columnm1,TYPE)+
	       VPixel(dz,band,row,columnm1,TYPE)*VPixel(dz,band,row,columnm1,TYPE));
      G2= sqrt(VPixel(dx,band,rowp1,columnp1,TYPE)*VPixel(dx,band,rowp1,columnp1,TYPE)+
	       VPixel(dy,band,rowp1,columnp1,TYPE)*VPixel(dy,band,rowp1,columnp1,TYPE)+
	       VPixel(dz,band,rowp1,columnp1,TYPE)*VPixel(dz,band,rowp1,columnp1,TYPE));
      H2= sqrt(VPixel(dx,band,rowm1,columnm1,TYPE)*VPixel(dx,band,rowm1,columnm1,TYPE)+
	       VPixel(dy,band,rowm1,columnm1,TYPE)*VPixel(dy,band,rowm1,columnm1,TYPE)+
	       VPixel(dz,band,rowm1,columnm1,TYPE)*VPixel(dz,band,rowm1,columnm1,TYPE)); 
      G3= sqrt(VPixel(dx,bandm1,rowp1,columnp1,TYPE)*VPixel(dx,bandm1,rowp1,columnp1,TYPE)+
	       VPixel(dy,bandm1,rowp1,columnp1,TYPE)*VPixel(dy,bandm1,rowp1,columnp1,TYPE)+
	       VPixel(dz,bandm1,rowp1,columnp1,TYPE)*VPixel(dz,bandm1,rowp1,columnp1,TYPE));
      H3= sqrt(VPixel(dx,bandp1,rowm1,columnm1,TYPE)*VPixel(dx,bandp1,rowm1,columnm1,TYPE)+
	       VPixel(dy,bandp1,rowm1,columnm1,TYPE)*VPixel(dy,bandp1,rowm1,columnm1,TYPE)+
	       VPixel(dz,bandp1,rowm1,columnm1,TYPE)*VPixel(dz,bandp1,rowm1,columnm1,TYPE));
      G4= sqrt(VPixel(dx,bandm1,row,columnp1,TYPE)*VPixel(dx,bandm1,row,columnp1,TYPE)+
	       VPixel(dy,bandm1,row,columnp1,TYPE)*VPixel(dy,bandm1,row,columnp1,TYPE)+
	       VPixel(dz,bandm1,row,columnp1,TYPE)*VPixel(dz,bandm1,row,columnp1,TYPE));
      H4= sqrt(VPixel(dx,bandp1,row,columnm1,TYPE)*VPixel(dx,bandp1,row,columnm1,TYPE)+
	       VPixel(dy,bandp1,row,columnm1,TYPE)*VPixel(dy,bandp1,row,columnm1,TYPE)+
	       VPixel(dz,bandp1,row,columnm1,TYPE)*VPixel(dz,bandp1,row,columnm1,TYPE));
      break;
    }
  /*Now we can interpolate*/  
  G=(1-Sa)*(1-Sb)*G1+Sa*(1-Sb)*G2+Sa*Sb*G3+(1-Sa)*Sb*G4;
  H=(1-Sa)*(1-Sb)*H1+Sa*(1-Sb)*H2+Sa*Sb*H3+(1-Sa)*Sb*H4;
  if (G<H) return H;
  else return G;
}

