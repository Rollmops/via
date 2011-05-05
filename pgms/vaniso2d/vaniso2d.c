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
 * $Id: vaniso2d.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vaniso2d -- 2D anisotropic diffusion

\par Description
Image denoising using 2D anisotropic diffusion.

\par Reference:
  G.Gerig, O. Kuebler, R. Kikinis, F.Jolesz:
  Nonlinear anisotropic filtering of MRI data.
  IEEE Trans on Medical Imaging, Vol.11, No.2, June 1992.

\par Usage

	<code>vaniso2d

	\param -in     input image
	\param -out    output image
	\param -iter   number of iterations
	\param -type   type of diffusion function (0 or 1)
	\param -kappa  diffusion parameter. Default: 4
	\param -alpha  diffusion parameter. Default: 0.3

\par Examples

\par Known bugs
none.

\file vaniso2d.c
\author G.Lohmann, MPI-CBS, 2004
*/


#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>

extern VImage VAniso2d(VImage,VImage,VShort,VShort,VFloat,VFloat);


int 
main (int argc,char *argv[])
{
  static VShort numiter = 10;
  static VShort type    = 1;
  static VFloat kappa   = 4.0;
  static VFloat alpha   = 0.3;
  static VOptionDescRec  options[] = {
    {"iter",VShortRepn,1,(VPointer) &numiter,
       VOptionalOpt,NULL,"number of iterations"},
    {"type",VShortRepn,1,(VPointer) &type,
       VOptionalOpt,NULL,"type of diffusion function (0 or 1)"},
    {"kappa",VFloatRepn,1,(VPointer) &kappa,
       VOptionalOpt,NULL,"diffusion parameter"},
    {"alpha",VFloatRepn,1,(VPointer) &alpha,
       VOptionalOpt,NULL,"diffusion parameter"},
  };
  FILE *in_file,*out_file;
  VAttrList list=NULL;
  VAttrListPosn posn;
  VImage src=NULL,dest=NULL;
  char prg[50];	
  sprintf(prg,"vaniso2d V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,&out_file);

  if (alpha <= 0) VError(" alpha must be positive");

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    dest = VAniso2d(src,dest,numiter,type,kappa,alpha);
    VSetAttrValue (& posn, NULL,VImageRepn,dest);
  }

  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
