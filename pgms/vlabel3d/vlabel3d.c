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
 * $Id: vlabel3d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vlabel3d  - 3D connected component labelling

\par Description
3D connected component labelling

\par Reference
The algorithm is described in :
G. Lohmann (1998). "Volumetric Image Analysis",
John Wiley & Sons, Chichester, England.

\par Usage

        <code>vlabel3d</code>

        \param -in      input image
        \param -out     output image
        \param -n       neighbourhood type (6 | 26). Default: 26
        \param -repn    output representation type (ubyte | short). Default: short

\par Examples
<br>
\par Known bugs
none.

\file vlabel3d.c
\author G.Lohmann, MPI-CBS, 2004
*/


#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <via.h>

VDictEntry TYPEDict[] = {
  { "ubyte", 0 },
  { "short", 1 },
  { NULL }
};


VDictEntry ADJDict[] = {
  { "6", 0 },
  { "26", 1 },
  { NULL }
};

int 
main (int argc,char *argv[])
{ 
  static VShort aneighb = 1;
  static VShort arepn = 1;
  static VOptionDescRec  options[] = {
    {"n",VShortRepn,1,(VPointer) &aneighb,
       VOptionalOpt,ADJDict,"neighbourhood type"},
    {"repn",VShortRepn,1,(VPointer) &arepn,
       VOptionalOpt,TYPEDict,"output representation type (ubyte or short)"}
  };
  FILE *in_file,*out_file;
  VAttrList list=NULL;
  VAttrListPosn posn;
  VImage src=NULL,dest=NULL;
  int nl=0,neighb;
  VRepnKind repn;
  char prg[50];	
  sprintf(prg,"vlabel3d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,&out_file);

  if (arepn == 0) repn = VUByteRepn;
  else repn = VShortRepn;

  if (aneighb == 0) neighb = 6;
  else neighb = 26;

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    dest = VLabelImage3d(src,NULL, (int) neighb,repn,&nl);
    fprintf(stderr," numlabels= %d\n",nl);
    VSetAttrValue (& posn, NULL,VImageRepn,dest);
  }
  if (src == NULL) VError(" no input image found");


  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
