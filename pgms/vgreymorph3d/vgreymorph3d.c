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
 * $Id: vgreymorph3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vgreymorph3d -- 3D greylevel morphology

\par Description
3D greylevel morphology.

\par References
  P. Maragos, R.W. Schafer (1990):
  "Morphological Systems for multidimensional signal processing",
  Proc. of the IEEE, Vol. 78, No. 4, pp. 690--709.

\par Usage

        <code>vgreymorph3d</code>

        \param -in    input image
        \param -out   output image
        \param -op    type of operation (dilate | erode | open | close). Default: dilate
        \param -dim   dim of structuring element (2D | 3D). Default: 3D
        \param -radius radius of structuring element. Default: 2
        \param -file   file containing structuring element

\par Examples
<br>
\par Known bugs
none.

\file vgreymorph3d.c
\author G.Lohmann, MPI-CBS, 2004
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <stdlib.h>
#include <via.h>



VDictEntry OPDict[] = {
  { "dilate", 0 },
  { "erode", 1 },
  { "open", 2 },
  { "close", 3 },
  { NULL }
};

VDictEntry DIMDict[] = {
  { "2D", 0 },
  { "3D", 1 },
  { NULL }
};


int main(int argc, char *argv[])
{
  static VLong op = 0;
  static VLong dim = 1;
  static VShort radius = 2;
  static VString mask_filename=" ";
  static VOptionDescRec options[] = {
    { "op", VLongRepn, 1, &op, VOptionalOpt, OPDict,
        "type of operation" },
    { "dim", VLongRepn, 1, &dim, VOptionalOpt, DIMDict,
        "dim of structuring element (2D or 3D)" },
    { "radius", VShortRepn, 1,(VPointer) & radius, 
	VOptionalOpt, NULL,"radius of structuring element" },
    { "file", VStringRepn, 1, & mask_filename, VOptionalOpt, NULL,
      "File containing structuring element" }
  };


  FILE *in_file, *out_file, *mask_file;
  VAttrList list, list2;
  VAttrListPosn posn;
  VImage src=NULL, se=NULL, result, tmp;
  char prg[50];	
  sprintf(prg,"vgreymorph3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments and identify files: */
  VParseFilterCmd(VNumber(options),options,argc,argv,&in_file,&out_file);

  if (strlen(mask_filename) < 2) {
    if (dim == 0)
      se = VGenSphere2d(radius);
    else if (dim == 1)
      se = VGenSphere3d(radius);
    else
      VError("illegal choice of parameter 'dim' (%d), must be 2 or 3",dim);
  }
  else {
    mask_file = VOpenInputFile (mask_filename, TRUE);
    list2 = VReadFile (mask_file, NULL);
    if (! list2)
      VError("Error reading structuring element");
    fclose(mask_file);
    for (VFirstAttr (list2,&posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & se);
      if (VPixelRepn(se) != VBitRepn) 
	VError("Structuring element must be of type VBit"); 
    }
  }


  /* Read the input file: */
  list = VReadFile (in_file, 0); if (! list) exit(1);
  fclose(in_file);

  /* For each attribute read... */
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr(& posn)) {

    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);
    
    switch (op) {
    case 0:
      result = VGreyDilation3d(src,se,NULL);
      break;
    case 1:
      result = VGreyErosion3d(src,se,NULL);
      break;
    case 2:
      tmp = VGreyErosion3d(src,se,NULL);
      result = VGreyDilation3d(tmp,se,NULL);
      VDestroyImage(tmp);
      break;
    case 3:
      tmp = VGreyDilation3d(src,se,NULL);
      result = VGreyErosion3d(tmp,se,NULL);
      VDestroyImage(tmp);
      break;
    }
      
    if (! result) exit(1);
    VSetAttrValue(&posn, 0, VImageRepn, result);
    VDestroyImage (src);
  } 
  
  /* Write out the results: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit(1);
  fprintf(stderr, "%s: done.\n", argv[0]);
  return 0;
}

