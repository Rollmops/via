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
 * $Id: volumeselect.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief volumeselect - select volumes from a set of volumes


\par Description
Select volumes from a set of volumes.
The output image will contain the selected volumes.


\par Usage

        <code>volumeselect</code>

        \param -in      input volume set
        \param -out     output volume set
        \param -feature type of feature (size | circularity | compactness). Default: size
        \param -type    type of operation (range | min | max). Default: range
        \param -min     min value. Default: 0
        \param -max     max value. Default: 1


\par Examples
<br>
\par Known bugs
none.

\file volumeselect.c
\author G.Lohmann, MPI-CBS, 2004
*/



/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>


enum FEATURE {SIZE, CIRCULARITY, COMPACTNESS};

VDictEntry FEATUREDict[] = {
  { "size", 0 },
  { "circularity", 1 },
  { "compactness", 2 },
  { NULL }
};


VDictEntry TypeDict[] = {
  { "range", 0 },
  { "min", 1 },
  { "max", 2 },
  { NULL }
};


extern Volumes VolumeSelect(Volumes,VLong,VLong,double,double);

int 
main (int argc, char *argv[])
{
  static VLong   feature = 0;
  static VLong   type    = 0;
  static VDouble minval = 0;
  static VDouble maxval = 1.0;
  static VOptionDescRec options[] = {
    { "feature", VLongRepn, 1,(VPointer) & feature, 
      VOptionalOpt, FEATUREDict,"type of feature" },
    { "type", VLongRepn, 1,(VPointer) & type, 
      VOptionalOpt, TypeDict,"type of operation: range, min, max" },
    { "min", VDoubleRepn, 1,(VPointer) & minval, 
      VOptionalOpt, NULL,"min value" },
    { "max", VDoubleRepn, 1,(VPointer) & maxval, 
      VOptionalOpt, NULL,"max value" }
  };
  FILE *in_file,*out_file;
  VAttrList list;
  Volumes src=NULL,dest=NULL;
  VImage src_image=NULL,dst_image=NULL,tmp=NULL;
  VAttrListPosn posn;
  int nl=0;
  char prg[50];	
  sprintf(prg,"volumeselect V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file,&out_file);


  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL))) exit (1);

  /* Performs conversion on each image: */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {

    switch (VGetAttrRepn (& posn)) {

    case VImageRepn:

      VGetAttrValue (& posn, NULL, VImageRepn, & src_image);

      if (VPixelRepn(src_image) == VBitRepn) {
	tmp = VLabelImage3d(src_image,NULL,26,VShortRepn,&nl);
	src = VImage2Volumes(tmp);
      }
      else {
	src  = VImage2Volumes(src_image);
      }
      dest = VolumeSelect(src,feature,type,minval,maxval);
      if (dest == NULL) VError("no volumes selected");
      dst_image = Volumes2Image(dest,VPixelRepn(src_image));
      VSetAttrValue (& posn, NULL, VImageRepn, dst_image);
      break;

    case VolumesRepn:

      VGetAttrValue (& posn, NULL, VolumesRepn, & src);
      dest = VolumeSelect(src,feature,type,minval,maxval);
      if (dest == NULL) VError("no volumes selected");
      VSetAttrValue (& posn, NULL, VolumesRepn, dest);
      break;

    default:
      continue;
    }
  }

  /* Write the results to the output file: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done.\n",argv[0]);
  return 0;
}


/*
** select volumes
*/
Volumes
VolumeSelect(Volumes src,VLong feature,VLong type,double minval,double maxval)
{
  Volumes dest;
  Volume u,v,v0;
  double size;
  double mean[3];
  double pi = 3.14159265;
  double rmax;
  double bordersize;
  double value = 0;
  int nvol,nsel;
  int nbands,nrows,ncols;
  double sum1, sum2, ave, sigma;
  VDouble xmin,xmax;

  /*
  ** create new Volume set
  */
  nbands = VolumesNBands(src);
  nrows  = VolumesNRows(src);
  ncols  = VolumesNColumns(src);
  dest   = VCreateVolumes(nbands,nrows,ncols);


  xmin = VRepnMaxValue(VDoubleRepn);
  xmax = VRepnMinValue(VDoubleRepn);

  sum1 = sum2 = 0;
  nsel = 0;
  nvol = 0;
  v0   = NULL;

  for (v = src->first; v != NULL; v = v->next) {

    switch (feature) {

    case SIZE:

      value = (double) VolumeSize(v);
      break;

    case CIRCULARITY:

      size = (double) VolumeSize(v);
      VolumeCentroid(v,&mean[0]);
      rmax = VolumeRadius(v,&mean[0]);
      rmax = rmax * rmax * rmax;
      value = (double) 3.0 * size / ((double) 4.0 * pi * rmax);
      break;

    case COMPACTNESS:
      
      size = (double) VolumeSize(v);
      bordersize = VolumeBorderSize(v);
      value = bordersize / size;
      break;

    default:
      VError("Illegal choice of feature");
    }


    if (type == 0 && value >= minval && value <= maxval) {
      u = VCopyVolume(v);
      VAddVolume(dest,u);
      nsel++;
    }

    if (value < xmin) {
      xmin = value;
      if (type == 1) {
	v0 = v;
	nsel = 1;
      }
    }

    if (value > xmax) {
      xmax = value;
      fprintf(stderr," xmax= %f\n",xmax);
      if (type == 2) {
	v0 = v;
	nsel = 1;
      }
    }

    sum1 += value * value;
    sum2 += value;

    nvol++;
  }

  if (nsel == 0) return NULL;

  if (v0 != NULL) {
    u = VCopyVolume(v0);
    VAddVolume(dest,u);
  }

  ave   = sum2 / (double) nvol;
  sigma = 0;
  if (nvol > 1)
    sigma = sqrt((sum1 - sum2 * sum2 / (double) nvol) / ((double) nvol - 1.0));
  
  fprintf(stderr,"  min: %12.4f,    max: %12.4f\n",xmin,xmax);
  fprintf(stderr," mean: %12.4f,  sigma: %12.4f\n",ave,sigma);
  fprintf(stderr,"%d out of %d volumes selected \n",nsel,nvol);
  return dest;
}
