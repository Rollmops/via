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
 * $Id: vscale3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vscale3d - 3D scaling

\par Description
3D scaling.

\par Usage

        <code>vscale3d</code>

        \param -in      input image
        \param -out     output image
        \param -xscale  zoom factor in column-direction. Default: 1
        \param -yscale  zoom factor in row-direction. Default: 1
        \param -zscale  zoom factor in slice-direction. Default: 1
        \param -type    Type of interpolation (linear | NN | spline). Default: linear

\par Examples
<br>
\par Known bugs
none.

\file vscale3d.c
\author G.Lohmann, MPI-CBS, 2004
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/os.h>

#include <via.h>



VDictEntry ITYPDict[] = {
  { "trilinear", 0 },
  { "NN", 1 },
  { "spline", 2 },
  { NULL }
};

VDictEntry FIXDict[] = {
  { "cacp", 0 },
  { "center", 1 },
  { NULL }
};


int main (int argc, char *argv[])
{
  static VFloat   zscale = 1.0;
  static VFloat   yscale = 1.0;
  static VFloat   xscale = 1.0;

  static VShort   zdim = 0;
  static VShort   ydim = 0;
  static VShort   xdim = 0;

  static VShort   type    = 0;
  static VShort   fixtype = 0;
  static VBoolean mat     = TRUE;

  static VOptionDescRec  options[] = {
    {"matrix",VBooleanRepn,1,(VPointer) &mat, VOptionalOpt,NULL,"Whether to adapt matrix size"},
    {"fix",VShortRepn,1,(VPointer) &fixtype, VOptionalOpt,NULL,"Point to keep fixed"},
    {"zscale",VFloatRepn,1,(VPointer) &zscale, VOptionalOpt,NULL,"zoom factor in slice-direction"},
    {"yscale",VFloatRepn,1,(VPointer) &yscale, VOptionalOpt,NULL,"zoom factor in row-direction"},
    {"xscale",VFloatRepn,1,(VPointer) &xscale, VOptionalOpt,NULL,"zoom factor in column-direction"},

    {"zdim",VShortRepn,1,(VPointer) &zdim, VOptionalOpt,NULL,"matrix size in slice-direction"},
    {"ydim",VShortRepn,1,(VPointer) &ydim, VOptionalOpt,NULL,"matrix size in row-direction"},
    {"xdim",VShortRepn,1,(VPointer) &xdim, VOptionalOpt,NULL,"matrix size in column-direction"},

    { "type", VShortRepn, 1, & type, VOptionalOpt,ITYPDict,
      "Type of interpolation (trilinear | NN | spline)" }
  };

  FILE *in_file, *out_file;
  VAttrList list;
  VAttrListPosn posn;
  VImage src=NULL,dest=NULL;
  int dst_nbands,dst_nrows,dst_ncols;
  char buf[40];
  VString str;
  float x,y,z,fix_x,fix_y,fix_z;
  float scale[3],shift[3];
  char prg[50];	
  sprintf(prg,"vscale3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv, & in_file, & out_file);

  if (xdim > 0 || ydim > 0 || zdim > 0) mat = FALSE;


  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL)))  exit (EXIT_FAILURE);
  fclose(in_file);


  /* Scale each object: */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);

    /* matrix dimensions change */
    if (mat) {
      dst_nbands = VFloor((float)VImageNBands(src) * zscale);
      dst_nrows  = VFloor((float)VImageNRows(src) * yscale);
      dst_ncols  = VFloor((float)VImageNColumns(src) * xscale);
      fix_x = fix_y = fix_z = 0;
      shift[0] = shift[1] = shift[2] = 0;
    }

    /* matrix dimensions remain fixed */
    else {

      if (xdim > 0 && ydim > 0 && zdim > 0) {
	dst_nbands = zdim;
	dst_nrows  = ydim;
	dst_ncols  = xdim;

	xscale = (float) xdim / (float)VImageNColumns(src);
	yscale = (float) ydim / (float)VImageNRows(src);
	zscale = (float) zdim / (float)VImageNBands(src);
	fix_x = fix_y = fix_z = 0;
      }
      else {
	dst_nbands = VImageNBands(src);
	dst_nrows  = VImageNRows(src);
	dst_ncols  = VImageNColumns(src);
      

	if (VGetAttr (VImageAttrList (src), "fixpoint", NULL,
		      VStringRepn, (VPointer) & str) == VAttrFound && fixtype == 0) {
	  sscanf(str,"%f %f %f",&fix_x,&fix_y,&fix_z);
	}
	else {
	  fix_x = (float)dst_ncols  * 0.5;
	  fix_y = (float)dst_nrows  * 0.5;
	  fix_z = (float)dst_nbands * 0.5;
	}
	shift[0] = (1.0 - zscale) * fix_z;
	shift[1] = (1.0 - yscale) * fix_y;
	shift[2] = (1.0 - xscale) * fix_x;
      }
    }

    fprintf(stderr," output matrix size: %3d %3d %3d\n",dst_nbands,dst_nrows,dst_ncols);
    fprintf(stderr,"    scaling factors: %.2f %.2f %.2f\n",xscale,yscale,zscale);
    /* fprintf(stderr,"   point kept fixed: %.2f %.2f %.2f\n",fix_x,fix_y,fix_z); */

    scale[0] = zscale;
    scale[1] = yscale;
    scale[2] = xscale;


    switch (type) {
    case 0:   /* trilinear interpolation resampling */
      dest = VTriLinearScale3d (src,NULL,dst_nbands,dst_nrows,dst_ncols,shift,scale);
      break;

    case 1:   /* nearest neighbour resampling */
      dest = VNNScale3d (src,NULL,dst_nbands,dst_nrows,dst_ncols,shift,scale);
      break;

    case 2:   /* cubic spline resampling */
      dest = VCubicSplineScale3d (src,dest,dst_nbands,dst_nrows,dst_ncols,shift,scale);
      break;

    default:
      VError("illegal interpolation type");
    }   
    if (! dest) exit (EXIT_FAILURE);




    /* 
    ** update header info 
    */
    if (VGetAttr (VImageAttrList (src), "voxel", NULL,
		  VStringRepn, (VPointer) & str) == VAttrFound) {
      sscanf(str,"%f %f %f",&x,&y,&z);
      x /= xscale;
      y /= yscale;
      z /= zscale;

      sprintf(buf,"%.2f %.2f %.2f",x,y,z);
      VSetAttr(VImageAttrList (dest),"scaled_voxel",NULL,VStringRepn,(VString)buf);


      /* update ca */
      if (VGetAttr (VImageAttrList (src), "ca", NULL,
		    VStringRepn, (VPointer) & str) == VAttrFound) {
	sscanf(str,"%f %f %f",&x,&y,&z);
	x = x * xscale + fix_x * (1.0 - xscale);
	y = y * yscale + fix_y * (1.0 - yscale);
	z = z * zscale + fix_z * (1.0 - zscale);
	sprintf(str,"%.3f %.3f %.3f",x,y,z);
	VSetAttr(VImageAttrList(dest),"ca",NULL,VStringRepn,str);
      }

      /* update cp */
      if (VGetAttr (VImageAttrList (src), "cp", NULL,
		    VStringRepn, (VPointer) & str) == VAttrFound) {
	sscanf(str,"%f %f %f",&x,&y,&z);
	x = x * xscale + fix_x * (1.0 - xscale);
	y = y * yscale + fix_y * (1.0 - yscale);
	z = z * zscale + fix_z * (1.0 - zscale);
	sprintf(str,"%.3f %.3f %.3f",x,y,z);
	VSetAttr(VImageAttrList(dest),"cp",NULL,VStringRepn,str);
      }

      /* update fixpoint */
      if (VGetAttr (VImageAttrList (src), "fixpoint", NULL,
		    VStringRepn, (VPointer) & str) == VAttrFound) {
	sscanf(str,"%f %f %f",&x,&y,&z);
	x = x * xscale + fix_x * (1.0 - xscale);
	y = y * yscale + fix_y * (1.0 - yscale);
	z = z * zscale + fix_z * (1.0 - zscale);
	sprintf(str,"%.3f %.3f %.3f",x,y,z);
	VSetAttr(VImageAttrList(dest),"fixpoint",NULL,VStringRepn,str);
      }

      /* update extent */
      if (VGetAttr (VImageAttrList (src), "extent", NULL,
		    VStringRepn, (VPointer) & str) == VAttrFound) {
	sscanf(str,"%f %f %f",&x,&y,&z);
	x = x * xscale;
	y = y * yscale;
	z = z * zscale;
	sprintf(str,"%.3f %.3f %.3f",x,y,z);
	VSetAttr(VImageAttrList(dest),"extent",NULL,VStringRepn,str);
      }
    }
    VSetAttrValue (& posn, NULL, VImageRepn, dest);
  }


  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
