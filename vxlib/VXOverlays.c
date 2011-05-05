/*
 *  $Id: VXOverlays.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains VX overlays routines.
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

/* From the standard C libaray: */
#include <math.h>

/* From X11R5 Xt and Motif: */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>

/* File identification string: */
VRcsId ("$Id: VXOverlays.c 3177 2008-04-01 14:47:24Z karstenm $");


/*
 * Extern: VX_RedrawOverlays
 *
 * Redraw overlays on exposure.
 */

extern void VX_RedrawOverlays (Widget w, XtPointer client_data,
			       XtPointer call_data)
{
    if (!XtIsRealized (VX_App.x.imageView))
	return;

    if (VX_App.o.pixmap_consistent) {

	/* Copy pixmap to screen: */
	XCopyArea (XtDisplay(VX_App.x.imageView), VX_App.o.pixmap,
		   XtWindow(VX_App.x.imageView), VX_App.o.gc, 0, 0,
		   (unsigned int) VX_App.x.cur_width,
		   (unsigned int) VX_App.x.cur_height, 0, 0);
    }

    /* Redraw lines and texts: */
    VX_RedrawLines ();
    VX_RedrawTexts ();
}


/*
 * VXStoreOverlays
 *
 * Store the states of existing overlays.
 */

void VXStoreOverlays (void)
{
    if (! VX_App.initialized)
	VError ("VXStoreOverlays: VX not initialized");

    if (! VX_App.in_main_loop)
	VError ("VXStoreOverlays: Main event loop not started");

    /* Store lines and texts: */
    VX_StoreLines ();
    VX_StoreTexts ();

    /* Free pixmap: */
    if (VX_App.o.pixmap_allocated) {
	XFreePixmap (XtDisplay(VX_App.x.imageView), VX_App.o.pixmap);
	VX_App.o.pixmap_allocated = FALSE;
    }

    /* Create new pixmap: */
    VX_App.o.pixmap =
	XCreatePixmap (XtDisplay(VX_App.x.imageView),
		       XtWindow(VX_App.x.imageView),
		       (unsigned int) VX_App.x.cur_width,
		       (unsigned int) VX_App.x.cur_height,
		       (unsigned int) DisplayPlanes (
					       XtDisplay(VX_App.x.imageView),
		       DefaultScreen(XtDisplay(VX_App.x.imageView))));

    /* Copy screen to pixmap: */
    XSync (XtDisplay(VX_App.x.imageView), False);
    XCopyArea (XtDisplay(VX_App.x.imageView), XtWindow(VX_App.x.imageView),
	       VX_App.o.pixmap, VX_App.o.gc, 0, 0,
	       (unsigned int) VX_App.x.cur_width,
	       (unsigned int) VX_App.x.cur_height, 0, 0);

    VX_App.o.pixmap_allocated = TRUE;
    VX_App.o.pixmap_consistent = TRUE;
}


/*
 * VXRestoreOverlays
 *
 * Restore states of previously stored overlays.
 */

void VXRestoreOverlays (void)
{
    if (! VX_App.initialized)
	VError ("VXRestoreOverlays: VX not initialized");

    if (! VX_App.in_main_loop)
	VError ("VXRestoreOverlays: Main event loop not started");

    /* Restore lines and texts: */
    VX_RestoreLines ();
    VX_RestoreTexts ();

    if (!VX_App.o.pixmap_consistent) {

	/* Redraw everything: */
	VImageViewRedraw (VX_App.x.imageView);

    } else {

	/* Copy pixmap to screen: */
	XCopyArea (XtDisplay(VX_App.x.imageView), VX_App.o.pixmap,
		   XtWindow(VX_App.x.imageView), VX_App.o.gc, 0, 0,
		   (unsigned int) VX_App.x.cur_width,
		   (unsigned int) VX_App.x.cur_height, 0, 0);
    }
}
