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
 * $Id: pngtov.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief pngtov - convert PNG image to Vista data file

\par Description
pngtov converts an image from a PNG-format file to a Vista data file.

\par Usage

        <code>pnmtov</code>

        \param -in      input image
        \param -out     output image
        \param -name    Name to be given created image. Default: image

\par Examples
<br>
\par Known bugs
none.

\file pngtov.c
\author G.Lohmann, MPI-CBS
*/

/* From the Vista library: */
#include <stdio.h>
#include <stdlib.h>

#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/os.h>
#include <viaio/VImage.h>
#include <png.h>


VImage VReadPng(FILE *fp)
{
  VImage dest=NULL;
  void *header;
  int i,j,k,u,number=8;
  int nbands,nframes;
  png_structp png_ptr;
  png_infop info_ptr;
  png_infop end_info;
  png_uint_32 width=0,height=0;
  int bit_depth,color_type,interlace_type;
  int compression_type,filter_method;
  int is_png;
  png_voidp error_ptr;
  png_error_ptr user_error_ptr;
  png_error_ptr user_warning_ptr;
  png_bytepp row_pointers;
  png_colorp palette;
  int num_palette;


  header = (char *) malloc(64);

  fread(header, 1, number, fp);
  is_png = !png_sig_cmp(header, 0, number);
  if (!is_png)  VError(" file is not PNG");


  png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, (png_voidp)error_ptr,
     user_error_ptr, user_warning_ptr);
  if (!png_ptr) VError(" error 0");

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
    VError(" error 1");
  }

  end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr,(png_infopp)NULL);
    VError("error 2");
  }

  if (setjmp(png_jmpbuf(png_ptr))){
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    VError("error 3");
  }

  png_set_sig_bytes(png_ptr, number);
  png_init_io(png_ptr,fp);


  png_read_png(png_ptr,info_ptr,PNG_TRANSFORM_IDENTITY,NULL);

  row_pointers = png_get_rows(png_ptr,info_ptr);

  png_get_IHDR(png_ptr, info_ptr, &width, &height,
	       &bit_depth, &color_type, &interlace_type,
	       &compression_type, &filter_method);
  
  fprintf(stderr," width= %ld, height= %ld\n",width,height);
  if (bit_depth != 8) VError(" only bit_depth=8 supported, (%ld)",bit_depth);


  fclose(fp);

  if (color_type == PNG_COLOR_TYPE_GRAY) {
    dest = VCreateImage(1,(int)height,(int)width,VUByteRepn);
    VFillImage(dest,VAllBands,0);

    for (i=0; i<height; i++) {
      for (j=0; j<width; j++) {
	u = row_pointers[i][j];
	VPixel(dest,0,i,j,VUByte) = u;
      }
    }
  }

  else if (color_type == PNG_COLOR_TYPE_RGB) {
    nbands  = 3;
    nframes = 1;
    dest = VCreateImage(nbands,(int)height,(int)width,VUByteRepn);

    VSetBandInterp (dest,VBandInterpNone,nframes,VBandInterpNone,nframes,
		    VBandInterpRGB,3,VBandInterpNone,nframes);
    VFillImage(dest,VAllBands,0);

    for (i=0; i<height; i++) {
      k = 0;
      for (j=0; j<width*3; j++) {
	u = row_pointers[i][j];
	VPixel(dest,0,i,k,VUByte) = u;
	u = row_pointers[i][j+1];
	VPixel(dest,1,i,k,VUByte) = u;
	u = row_pointers[i][j+2];
	VPixel(dest,2,i,k,VUByte) = u;
	k++;
      }
    }
  }

  
  else if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_get_PLTE(png_ptr,info_ptr,&palette,&num_palette);

    nbands  = 3;
    nframes = 1;
    dest = VCreateImage(nbands,(int)height,(int)width,VUByteRepn);
    
    VSetBandInterp (dest,VBandInterpNone,nframes,VBandInterpNone,nframes,
		    VBandInterpRGB,3,VBandInterpNone,nframes);
    
    VFillImage(dest,VAllBands,0);
    png_get_PLTE(png_ptr,info_ptr,&palette,&num_palette);

    for (i=0; i<height; i++) {
      for (j=0; j<width; j++) {
	u = row_pointers[i][j];
	if (u >= num_palette || u < 0) VError(" err in color palette dim");

	k = palette[u].red;
	VPixel(dest,0,i,j,VUByte) = k;
	k = palette[u].green;
	VPixel(dest,1,i,j,VUByte) = k;
	k = palette[u].blue;
	VPixel(dest,2,i,j,VUByte) = k;
      }
    }
  }

  else {
    VError(" unknown color type");
  }
  
  return dest;
}


/* Command line options: */
static VStringConst object_name = "image";
static VOptionDescRec options[] = {
    { "name", VStringRepn, 1, & object_name, VOptionalOpt, NULL,
	  "Name to be given created image" }
};


int main (int argc, char **argv)
{
  FILE *in_file, *out_file;
  VImage image=NULL;
  VAttrList list = VCreateAttrList ();
  char prg[50];	
  sprintf(prg,"pngtov V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments: */
  VParseFilterCmd(VNumber(options),options,argc,argv,&in_file,&out_file);
    
  if (! (image = VReadPng (in_file))) exit (EXIT_FAILURE);

  VHistory(VNumber(options),options,prg,&list,&list);
  VAppendAttr (list, object_name, NULL, VImageRepn, image);
  if (! VWriteFile (out_file, list))
    exit (EXIT_FAILURE);

  fprintf (stderr, "%s: done.\n",argv[0]);
  return EXIT_SUCCESS;
}
