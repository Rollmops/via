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
 * $Id: vtopnm.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/
 
/*! \brief vtopnm -  convert image from Vista data file to portable any map

\par Description
vtopnm  converts one band of an image to a portable any map (PNM) file. A wide range
of filters are available to convert PNM files to other formats.

This program only produces portable graymaps whose pixel values are in the  range  0
to  255.  Any  type  of Vista image is accepted, but the image is first converted to
have ubyte  pixels  if  necessary.  This  conversion  is  performed  by  
VConvertImageRange(3Vi).

\par Usage

        <code>vtoppm</code>

        \param -in      input image
        \param -out     output image
        \param -band    Specifies the image band to be converted. Default: 0.

\par Examples
<br>
\par Known bugs
none.

\file vtopnm.c
\author David Lowe, UBC Laboratory for Computational Intelligence,
        G.Lohmann (extension to pnm format)
*/



/*
 *
 *  The vtopnm program, which converts Vista image files to portable any map
 *  (PNM) files.
 */


/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"



/* Later in this file: */
void WritePnmFile (FILE *, VImage, VBand);

int main (int argc, char *argv[])
{
    static VShort band;
    static VOptionDescRec options[] = {
	{ "band", VShortRepn, 1, & band, VOptionalOpt, NULL,"Image band to convert" }
    };
    FILE *infile, *outfile;
    int nimages;
    VAttrList attributes;
    VImage *images, image;
    char prg[50];	
    sprintf(prg,"vtopnm V%s", getVersion());
    fprintf (stderr, "%s\n", prg);


    /* Parse command line arguments and identify the input and output files */
    VParseFilterCmd (VNumber (options), options, argc, argv,
		     & infile, & outfile);

    /* Read all images from the input file */
    if ((nimages = VReadImages (infile, & attributes, & images)) == 0)
	exit (EXIT_FAILURE);	/* no images found */
    fclose (infile);

    /* Ensure that the specified band exists: */
    if (nimages != 1)
	VWarning ("Only the first of %d images will be converted", nimages);
    if (band < 0 || band >= VImageNBands (images[0]))
	VError ("Image of %d band(s) has no band %d",
		VImageNBands (images[0]), band);

    /* Convert the image to UByte: */
    if (VPixelRepn(images[0]) != VUByteRepn) {
	image = VConvertImageRange (images[0], NULL, (VBand) band, VUByteRepn);
	if (! image) exit (EXIT_FAILURE);
	band = 0;
    } else image = images[0];

    /* Output the first image as a ppm file. */
    WritePnmFile (outfile, image, (VBand) band);

    return EXIT_SUCCESS;
}


/* Output an image in the ppm format (see ppm man page for details).
 */
void WritePnmFile (FILE *outfile, VImage image, VBand band)
{
  VUByte *src_pp;
  int i,u,r,c;
  int nrows = VImageNRows (image),
    ncols = VImageNColumns (image);

  if (VImageColorInterp(image) == VBandInterpRGB) {    /* color map */
    fprintf (outfile, "P6\n%d %d\n255\n",ncols,nrows);
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	u = (int)VPixel(image,0,r,c,VUByte);
	putc(u,outfile);
	u = (int)VPixel(image,1,r,c,VUByte);
	putc(u,outfile);
	u = (int)VPixel(image,2,r,c,VUByte);
	putc(u,outfile);
      }
    }
  }


  else {     /* gray map */
    fprintf (outfile, "P5\n%d %d\n255\n",ncols,nrows);
    src_pp = (VUByte *) VPixelPtr (image, band, 0, 0);
    for (i=0; i<nrows*ncols; i++) {
      u = *src_pp++;
      putc(u,outfile);
    }
  }
  fclose(outfile);
}
