/*
 *  $Id: RenderVToX.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains functions for rendering a Vista image into an XImage
 *  with respect to a Vista colormap (VColormap).
 */

/*
 *  Copyright 1993, 1994 University of British Columbia
 *
 *  Permission to use, copy, modify, distribute, and sell this software and its
 *  documentation for any purpose is hereby granted without fee, provided that
 *  the above copyright notice appears in all copies and that both that
 *  copyright notice and this permission notice appear in supporting
 *  documentation. UBC makes no representations about the suitability of this
 *  software for any purpose. It is provided "as is" without express or
 *  implied warranty.
 *
 *  Authors: Dan Razzell, Daniel Ko, Art Pope
 *	     UBC Laboratory for Computational Intelligence
 */

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "colormap.h"
#include "viaio/os.h"
#include "viaio/VImage.h"
#include "VImageVieP.h"

/* From X Windows libraries: */
#include <X11/IntrinsicP.h>
#include <X11/Xatom.h>

/* File identification string: */
VRcsId ("$Id: RenderVToX.c 3177 2008-04-01 14:47:24Z karstenm $");


/*
 *  V_RenderVToX
 *
 *  Converts a Vista image into an XImage, dithered with respect to
 *  the supplied Vista colormap.
 *
 *  ==> Instead of a widget, take vx, port, image, band, whether to
 *	display color, whether to dither absolute
 */

extern XImage *V_RenderVToX (VImageViewWidget vw)
{
  VColormap vc = vw->viv.v_colormap;
  struct V_Port *port = & vw->viv.port;
  VBoolean color_image, color_display, use_stdcmap;
  VImage vimage, dimage = NULL;
  int band, nvalues[3], row, col;
  unsigned long i, t;
  VUByte *rp, *gp, *bp;
  XImage *ximage = vw->viv.ximage;
  char *xp_row, *xp;

  /* Determine whether we're displaying a color image or a single band,
     and whether it's on a color display or a grayscale one: */
  color_image =
    vw->viv.is_color && vw->viv.band + 2 < VImageNBands (vw->viv.image);
  color_display =
    vc->vinfo.class != StaticGray && vc->vinfo.class != GrayScale;
  use_stdcmap = color_image && color_display;

  /* If we're displaying a color image on a grayscale display, first
     convert it to gray: */
  if (color_image && ! color_display) {
    if (! (vimage = VRGBImageToGray (vw->viv.image, NULL, vw->viv.band)))
      return NULL;
    band = 0;
  } else {
    vimage = vw->viv.image;
    band = vw->viv.band;
  }

  /* Allocate an image to dither into: */
  if (! (dimage = VCreateImage (use_stdcmap ? 3 : 1,
				port->height, port->width, VUByteRepn)))
    goto Return;

  /* Dither into the appropriate range of shades: */
  if (use_stdcmap) {
    nvalues[0] = vc->stdcmap.red_max + 1;
    nvalues[1] = vc->stdcmap.green_max + 1;
    nvalues[2] = vc->stdcmap.blue_max + 1;
  } else nvalues[0] = vc->ngrays;
  if (! VDither (vimage, dimage, band,
		 port->first_row, port->first_column,
		 port->nrows, port->ncolumns, nvalues, vw->viv.absolute))
    goto Return;

  rp = VPixelPtr (dimage, 0, 0, 0);
  if (use_stdcmap) {
    gp = VPixelPtr (dimage, 1, 0, 0);
    bp = VPixelPtr (dimage, 2, 0, 0);
  }

#define ComputePixelValue(r,g,b)					      \
        if (use_stdcmap) {						      \
	    t = r * vc->stdcmap.red_mult + g * vc->stdcmap.green_mult +	      \
		b * vc->stdcmap.blue_mult;				      \
	    t = vc->indcmap ?						      \
		vc->indcmap[t] : (t + vc->stdcmap.base_pixel) & 0xFFFFFFFF;   \
	} else t = vc->invgmap[r];

  /* Look for common special cases: */
  if (ximage->bytes_per_line == port->width) {

    /* One-byte XImage pixels and no row padding: */
    xp = ximage->data;
    for (i = port->height * port->width; i > 0; i--) {
      ComputePixelValue (*rp++, *gp++, *bp++);
      *xp++ = t;
    }
	

  } else if (ximage->bits_per_pixel == CHAR_BIT) {

    /* One-byte XImage pixels: */
    for (row = 0, xp_row = ximage->data; row < port->height;
	 row++, xp_row += ximage->bytes_per_line) {
      xp = xp_row;
      for (col = 0; col < port->width; col++) {
	ComputePixelValue (*rp++, *gp++, *bp++);
	*xp++ = t;
      }
    }

  } else {

    /* General case: */
    for (row = 0; row < port->height; row++)
      for (col = 0; col < port->width; col++) {
	ComputePixelValue (*rp++, *gp++, *bp++);
	XPutPixel (ximage, col, row, t);
      }
  }

#undef ComputePixelValue

 Return:
  if (vimage != vw->viv.image)
    VDestroyImage (vimage);
  if (dimage)
    VDestroyImage (dimage);
  return ximage;
}
