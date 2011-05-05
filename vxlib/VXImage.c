/*
 *  $Id: VXImage.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains VX image display routines.
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
 *  Author: Daniel Ko, UBC Laboratory for Computational Intelligence
 */

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "viaio/os.h"
#include "viaio/VImage.h"
#include "viaio/VImageView.h"
#include "viaio/VX.h"
#include "viaio/VXPrivate.h"

/* From X11R5 Xt and Motif: */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>

/* File identification string: */
VRcsId ("$Id: VXImage.c 3177 2008-04-01 14:47:24Z karstenm $");


/*
 * Extern: VX_Zoomed
 *
 * Handle the ZoomIn, ZoomOut, and MoveZoomCenter actions of
 * VX_App.x.imageView.
 */

extern void VX_Zoomed (Widget w, XtPointer client_data, XtPointer call_data)
{
    int zoom_level;

    if (VX_App.v.image == NULL)
	return;

    /* Get the new zoom level: */
    XtVaGetValues (w, VxNzoomLevel, & zoom_level, (char *) NULL);
    VX_App.v.zoom_level = zoom_level / 100.0f;

    /* Overlays pixmap is no longer consistent: */
    VX_App.o.pixmap_consistent = FALSE;

    /* Calculate row scale: */
    VX_App.v.row_scale = (float) VX_App.x.cur_height /
	VImageNRows (VX_App.v.image) * VX_App.v.zoom_level;
}


/*
 * VXSetImage
 *
 * Set the region of the image to be displayed.
 */

VBoolean VXSetImage (VImage image, VBand band, double zoom_level,
		     int row_center, int column_center)
{
    int width, height;

    if (! VX_App.initialized)
	VError ("VXSetImage: VX not initialized");

    /* Check for NULL image: */
    if (image == NULL) {
	VWarning ("VXSetImage: Cannot set NULL image");
	return FALSE;
    }

    /* Validate zoom_level: */
    if (zoom_level < 1.0) {
	VWarning ("VXSetImage: Zoom level less than 1");
	return FALSE;
    }

    /* Display busy cursor: */
    if (XtIsRealized (VX_App.x.imageView) == TRUE)
	XMapWindow (XtDisplay(VX_App.x.topLevel), VX_App.x.busyWindow);
    XSync (XtDisplay(VX_App.x.topLevel), FALSE);

    /* Overlays pixmap is no longer consistent: */
    VX_App.o.pixmap_consistent = FALSE;

    /* Store the image and band number: */
    VX_App.v.image = image;
    VX_App.v.band = band;
    VX_App.v.zoom_level = zoom_level;

    /* Determine new size of imageView widget: */

    /* New bounding box cannot be smaller than initial size
       of the imageView widget: */
    width = VMax (VX_App.x.init_width, VX_App.x.cur_width);
    height = VMax (VX_App.x.init_height, VX_App.x.cur_height);

    /* Determine new size of the imageViewWidget: */
    VImageWindowSize (VX_App.v.image, width, height,
		      &VX_App.x.cur_width, &VX_App.x.cur_height);

    /* Calculate row scale: */
    VX_App.v.row_scale = (float) VX_App.x.cur_height /
	VImageNRows (VX_App.v.image) * VX_App.v.zoom_level;

    /* Set the new image and current dimensions: */
    XtVSV (VX_App.x.imageView,
	   VxNimage, VX_App.v.image,
	   VxNband, band,
	   VxNrowCenter, row_center,
	   VxNcolumnCenter, column_center,
	   VxNzoomLevel, (int) (zoom_level * 100.0),
	   XmNwidth, (Dimension) VX_App.x.cur_width,
	   XmNheight, (Dimension) VX_App.x.cur_height, (char *) NULL);

    /* Assumed that the width and height are honored. */

    /* Hide busy cursor: */
    if (XtIsRealized (VX_App.x.imageView) == TRUE)
	XUnmapWindow (XtDisplay(VX_App.x.topLevel), VX_App.x.busyWindow);

    return TRUE;
}
