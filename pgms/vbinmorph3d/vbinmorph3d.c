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
 * $Id: vbinmorph3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vbinmorph3d -- 3D binary morphology

\par Description
3D binary morphology.


\par Reference:
  P. Maragos, R.W. Schafer (1990):
  "Morphological Systems for multidimensional signal processing",
  Proc. of the IEEE, Vol. 78, No. 4, pp. 690--709.

\par Usage

	<code>vbinmorph3d

	\param -in      input image
	\param -out     output image
	\param -op      type of operation (dilate | erode | open | close). Default: dilate
 	\param -radius  radius of structuring element. Default: 2
	\param -file    File containing structuring element.

\par Examples

\par Known bugs
none.

\file vbinmorph3d.c
\author G.Lohmann, MPI-CBS, 2004
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <via.h>


VDictEntry OPDict[] = {
  { "dilate", 0 },
  { "erode", 1 },
  { "open", 2 },
  { "close", 3 },
  { NULL }
};


int main (int argc, char **argv)
{
  static VLong op = 0;
  static VShort radius=2;
  static VString se_filename = "";
  static VOptionDescRec options[] = {
    { "op", VLongRepn, 1, &op, VOptionalOpt, OPDict,"type of operation" },
    { "radius", VShortRepn, 1,(VPointer) & radius, 
	VOptionalOpt, NULL,"radius of structuring element" },
    { "file", VStringRepn, 1, & se_filename, VOptionalOpt, NULL,
      "File containing structuring element" }
  };

  FILE *in_file, *out_file, *se_file;
  VAttrList list,se_list;
  VImage src=NULL,dest1=NULL,dest2=NULL,se=NULL;
  VAttrListPosn posn;
  VoxelList sel=NULL;
  int nse = 0;
  char prg[50];	
  sprintf(prg,"vbinmorph3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);
    
  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file, &out_file);
  
  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  if (strlen(se_filename) < 2) {
    se = VGenSphere3d(radius);
  }
  else {
    se_file = VOpenInputFile (se_filename, TRUE);
    se_list = VReadFile (se_file, NULL);
    fclose(se_file);
    if (! se_list)
      VError("Error reading structuring element");
    fclose(se_file);

    for (VFirstAttr (se_list,&posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & se);
      if (VPixelRepn(se) != VBitRepn) 
	VError("Structuring element must be of type VBit");
    }
    if (se == NULL)
      VError("Structuring element not found in attr list");
  }

  
  /* convert SE to more efficient data type */
  sel = VConvertSE3d(se,&nse);
  if (sel == NULL) VError("error converting SE.");


  /* Performs morphological filtering on each image: */

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);
    
    switch (op) {
    case 0:
      dest2 = VDilateImage3d (src,NULL,sel,nse);
      break;

    case 1:
      dest2 = VErodeImage3d (src,NULL,sel,nse);
      break;

    case 2:
      dest1 = VErodeImage3d (src,NULL,sel,nse);
      dest2 = VDilateImage3d (dest1,src,sel,nse);
      VDestroyImage(dest1);
      break;

    case 3:
      dest1 = VDilateImage3d (src,NULL,sel,nse);
      dest2 = VErodeImage3d (dest1,src,sel,nse);
      VDestroyImage(dest1);
      break;
    }
    
    if (dest2 == NULL) exit (1);
    VCopyImageAttrs (src, dest2);
    VSetAttrValue (& posn, NULL, VImageRepn, dest2);
  }

  VFree(sel);
  VDestroyImage(se);


  /* Write the results to the output file: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (VWriteFile (out_file, list))
    fprintf (stderr, "%s: done.\n",argv[0]);
  return 0;
}

