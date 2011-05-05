/****************************************************************
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
 * $Id: rawtov.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief rawtov - converts a raw data file to Vista format

\par Description
converts a raw data file to Vista format.
A 'raw' file is an unstructured file not containing any
header info.


\par Usage

        <code>rawtov</code>

       \param -in      input image
       \param -out     output image
       \param -repn    pixel representation: 
                          (bit | ubyte | sbyte | short | long | float | double)
       \param -nbands  number of slices
       \param -nrows   number of rows
       \param -ncols   number of columns

\par Examples
<br>
\par Known bugs
none.

\file rawtov.c
\author G.Lohmann, MPI-CBS
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>

#include <stdio.h>
#include <stdlib.h>


VImage 
RawImage (FILE *fp,VLong header,VShort nbands,VShort nrows,VShort ncols,VShort repn)
{
  VImage dest;
  int npixels;
  VPointer *dest_pp;
  long offset=0;

  offset = header;
  npixels = nbands * nrows * ncols;

  dest = VCreateImage((int)nbands,(int)nrows,(int)ncols,(VRepnKind)repn);
  VFillImage(dest,VAllBands,0);

  dest_pp = VImageData(dest);
  fseek(fp, offset, 0);
  fread(dest_pp, npixels, VRepnSize(repn), fp);
  fclose(fp);

  return dest;
}



VDictEntry TYPEDict[] = {
  { "bit",   1 },
  { "ubyte", 2 },
  { "sbyte", 3 },
  { "short", 4 },
  { "long",  5 },
  { "float", 6 },
  { "double",7 },
  { NULL }
};


int main (int argc, char *argv[])
{
  static VLong header  = 0;
  static VShort nrows  = 256;
  static VShort ncols  = 256;
  static VShort nbands = 1;
  static VShort repn   = VUByteRepn;
  
  static VOptionDescRec  options[] = {
    {"header",VLongRepn,1,(VPointer) &header,VOptionalOpt,NULL,"number of bytes in header"},
    {"nbands",VShortRepn,1,(VPointer) &nbands,VOptionalOpt,NULL,"number of bands"},
    {"nrows",VShortRepn,1,(VPointer) &nrows,VOptionalOpt,NULL,"number of rows"},
    {"ncols",VShortRepn,1,(VPointer) &ncols,VOptionalOpt,NULL,"number of columns"},
    {"repn",VShortRepn,1,(VPointer) &repn,VOptionalOpt,TYPEDict,"pixel repn"}
  };
  FILE *fp,*fopen();
  VAttrList list;
  FILE *in_file,*out_file;
  VImage dest=NULL;
  char prg[50];	
  sprintf(prg,"rawtov V%s", getVersion());
  fprintf (stderr, "%s\n", prg);


  /* Parse command line arguments and identify files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file,&out_file);

  dest = RawImage (in_file,header,nbands,nrows,ncols,repn);
  if (!dest) VError(" failure ");

  /* Write the results to the output file: */
  if (! VWriteImages (out_file,NULL,1,&dest)) exit(1);
  fprintf (stderr, "Vista image of size %d x %d x %d created\n",nbands,nrows,ncols);
  return 0;
}



