/*
 *  $Id: VXInit.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains VX initialization routines.
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
#include "viaio/colormap.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"
#include "viaio/VImageView.h"
#include "viaio/VX.h"
#include "viaio/VXPrivate.h"

/* From X11R5 Xt and Motif: */
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
#include <X11/Xmu/Editres.h>
#endif
#include <X11/cursorfont.h>
#include <X11/ShellP.h>
#include <X11/Xatom.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/MainW.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/Form.h>

/* File identification string: */
VRcsId ("$Id: VXInit.c 3177 2008-04-01 14:47:24Z karstenm $");

/* Later in this file: */
static void RealizeWrapper (Widget, Mask *, XSetWindowAttributes *);
static void HandleUserResize (Widget, XtPointer, XConfigureEvent *, Boolean *);

/* Record for storing application resource: */
typedef struct {
    int msg_area_nlines;	/* number of lines visible in message area */
} VXAppResource;

/* Table of application resources: */
static XtResource resources[] = {
    { VXNmessageAreaNLines, VXCMessageAreaNLines, XmRInt, sizeof (int),
	  XtOffsetOf (VXAppResource, msg_area_nlines), XmRImmediate, 0 }
};

/* Dictionary of palette type keywords: */
static VDictEntry palette_dict[] = {
    { "any", None },
    { "default", XA_RGB_DEFAULT_MAP }, { "best", XA_RGB_BEST_MAP },
    { "gray", XA_RGB_GRAY_MAP }, { "grey", XA_RGB_GRAY_MAP },
    NULL
};

/* Program options: */
static VLong palette = None;
static VOptionDescRec options[] = {
    { "palette", VLongRepn, 1, & palette, VOptionalOpt,
	  palette_dict, "Color palette to use" }
};

/* Original Shell realize method, for which we substite a wrapper: */
static XtRealizeProc orig_shell_realize;

/* Global state information: */
AppRec VX_App;			/* application data */


/*
 * VXInit
 *
 * Initialize the VX library.
 * Global variable "VX_App.initialized" is set.
 */

void VXInit (VStringConst class, VStringConst *default_res,
	     int *argc, char **argv)
{
    VXAppResource app_resource;
    XVisualInfo vinfo_template;
    Dimension height, width;
    int depth;
    Pixmap icon;

    /* Do not initialize more than once: */
    if (VX_App.initialized)
	VError ("VXInit: VX already initialized");

    /* Initialize image related fields in app: */
    VX_App.v.image = NULL;
    VX_App.v.row_scale = 0;

    /* Initialize overlays related fields in app: */
    VX_App.o.pixmap_consistent = FALSE;
    VX_App.o.pixmap_allocated = FALSE;

    /* Initialize widgets related fields in app: */
    VX_App.x.cur_width = VX_App.x.cur_height = 0;
    VX_App.x.init_width = VX_App.x.init_height = 0;

    /* Motif creates shells for things like menus. To ensure they get the
       visual we're using for other application widgets, we wrap our
       own routine around the Realize method for Shell widgets: */
    orig_shell_realize = shellClassRec.core_class.realize;
    shellClassRec.core_class.realize = RealizeWrapper;

    /* Create top level widget: */
    VX_App.x.topLevel = XtVaAppInitialize (&(VX_App.x.appContext),
					   (String) class, NULL, 0,
#if ! defined(XtSpecificationRelease) || XtSpecificationRelease < 5
					   (Cardinal *)
#endif				      
					   argc, argv, (String *) default_res,
					   XmNallowShellResize, TRUE,
					   XmNkeyboardFocusPolicy, XmPOINTER,
					   (char *) NULL);

    /* Parse VX-specific command line options: */
    if (! VParseCommand (VNumber (options), options, argc, argv)) {

	/* On encountering -help or an error, pass -help back to the
	   caller to force usage to be printed: */
	*argc = 2;
	argv[1] = "-help";
	return;
    }

    /* Any visual we use has to have the same depth as the root window
       because Motif assumes that: */
    vinfo_template.depth = DefaultDepthOfScreen (XtScreen (VX_App.x.topLevel));
    VX_App.x.vcolormap = VCreateColormap (XtScreen (VX_App.x.topLevel),
					  (Atom) palette,
					  VisualDepthMask, & vinfo_template);
    if (VX_App.x.vcolormap)
	XtVaSetValues (VX_App.x.topLevel,
		       XtNcolormap, VColormapColormap (VX_App.x.vcolormap),
		       XtNvisual, VColormapVisual (VX_App.x.vcolormap),
		       (char *) NULL);
    else VWarning ("Failed to get %s palette",
		   XGetAtomName (XtDisplay (VX_App.x.topLevel),
				 (Atom) palette));

    /* Give the top level shell the Vista icon: */
    XtVGV (VX_App.x.topLevel, XmNdepth, & depth, (char *) NULL);
    icon = XmGetPixmap (XtScreen (VX_App.x.topLevel), "vista_icon",
			BlackPixelOfScreen (XtScreen (VX_App.x.topLevel)),
			WhitePixelOfScreen (XtScreen (VX_App.x.topLevel)));
    if (icon != XmUNSPECIFIED_PIXMAP)
	XtVSV (VX_App.x.topLevel, XmNiconPixmap, icon, (char *) NULL);

    /* Participate in the Editres Protocol: */
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
    XtAddEventHandler (VX_App.x.topLevel, (EventMask)0,
		       True, _XEditResCheckMessages, NULL);
#endif

    /* Get application resource (e.g., VXNmessageAreaNLines): */
    XtVaGetApplicationResources (VX_App.x.topLevel, &app_resource,
				 resources, XtNumber(resources),
				 (char *) NULL);
    VX_App.x.msg_area_nlines = app_resource.msg_area_nlines;

    /* Create main window, and menu bar: */
    VX_App.x.mainWindow = XtVCMW ("mainWindow", xmMainWindowWidgetClass,
				  VX_App.x.topLevel, (char *) NULL);
    VX_App.x.menuBar = XmCreateMenuBar (VX_App.x.mainWindow, "menuBar",
					NULL, 0);
    XtManageChild (VX_App.x.menuBar);

    /* Create imageView widget and its frame: */
    VX_App.x.imageViewFrame = XtVCMW ("imageViewFrame", xmFrameWidgetClass,
				      VX_App.x.mainWindow, (char *) NULL);
    VX_App.x.imageView = XtVCMW ("imageView", vImageViewWidgetClass,
				 VX_App.x.imageViewFrame,
				 VxNproportion, TRUE,
				 VxNresize, FALSE,
				 VxNvColormap, VX_App.x.vcolormap,
				 (char *) NULL);

    /* Get the initial width and height of the imageView widget: */
    XtVGV (VX_App.x.imageView,
	   XmNheight, &height,
	   XmNwidth, &width, (char *) NULL);
    VX_App.x.cur_width = VX_App.x.init_width = (int) width;
    VX_App.x.cur_height = VX_App.x.init_height = (int) height;

    /* Add event handler to handle user resize: */
    XtAddEventHandler (VX_App.x.imageView, StructureNotifyMask,
		       FALSE, (XtEventHandler) HandleUserResize, NULL);

    /* Add expose callback to redraw lines: */
    XtAddCallback (VX_App.x.imageView, VxNexposeCallback,
		   VX_RedrawOverlays, NULL);

    /* Add event handlers to handle the ZoomIn, ZoomOut and
       MoveZoomCenter actions: */
    XtAddCallback (VX_App.x.imageView, VxNzoomInCallback, VX_Zoomed, NULL);
    XtAddCallback (VX_App.x.imageView, VxNzoomOutCallback,
		   VX_Zoomed, NULL);
    XtAddCallback (VX_App.x.imageView, VxNmoveZoomCenterCallback,
		   VX_Zoomed, NULL);

    /* Inform main window of the roles of its children: */
    XmMainWindowSetAreas (VX_App.x.mainWindow, VX_App.x.menuBar, NULL, NULL,
			  NULL, VX_App.x.imageViewFrame);

    /* Create message area and its frame: */
    VX_App.x.msgAreaFrame = XtVCMW ("msgAreaFrame", xmFrameWidgetClass,
				    VX_App.x.mainWindow, (char *) NULL);
    VX_App.x.msgArea = XmCreateScrolledText (VX_App.x.msgAreaFrame, "msgArea",
					     NULL, 0);

    XtVSV (VX_App.x.msgArea,
	   XmNeditable, FALSE,
	   XmNmarginWidth, 1,
	   XmNmarginHeight, 1,
	   XmNautoShowCursorPosition, FALSE,
	   XmNcursorPositionVisible, FALSE,
	   XmNeditMode, XmMULTI_LINE_EDIT, (char *) NULL);
    XtManageChild (VX_App.x.msgArea);

    if (VX_App.x.msg_area_nlines > 0) {
	XtVSV (VX_App.x.mainWindow,
	       XmNmessageWindow, VX_App.x.msgAreaFrame, (char *) NULL);
	XtVSV (VX_App.x.msgArea,
	       XmNrows, VX_App.x.msg_area_nlines, (char *) NULL);
    } else XtUnmanageChild (VX_App.x.msgAreaFrame);

    /* VX is now initialized: */
    VX_App.initialized = TRUE;

    if (! (VX_InitMenu () && VX_InitInput () && VX_InitLine () &&
	   VX_InitText () && VX_InitDialog ()))
	VError ("VXInit: Initialization failed");
}


/*
 * VXAppMainLoop
 *
 * Display the interface and start the main event loop.
 * Global variable VX_InMainLoop is modified.
 */

void VXAppMainLoop (void)
{
    Dimension width, height;
    XGCValues gcv;
    unsigned long valuemask;
    XSetWindowAttributes attributes;
    XEvent event;

    if (!VX_App.initialized)
	VError ("VXAppMainLoop: VX not initialized");

    /* Display the widgets: */
    XtRealizeWidget (VX_App.x.topLevel);

    /* Create a window that will display a busy cursor when mapped:
       [thanks to Andrew Wason (aw@cellar.bae.bellcore.com), Dan Heller
       (argv@sun.com), and mouse@larry.mcrcim.mcgill.edu] */

    /* Ignore device events while the busy cursor is displayed. */
    valuemask = CWDontPropagate | CWCursor;
    attributes.do_not_propagate_mask =  (KeyPressMask | KeyReleaseMask |
					 ButtonPressMask | ButtonReleaseMask |
					 PointerMotionMask);
    attributes.cursor =
	XCreateFontCursor(XtDisplay(VX_App.x.topLevel), XC_watch);

    /* The window will be as big as the display screen, and clipped by
       its own parent window, so we never have to worry about resizing */
    VX_App.x.busyWindow =
	XCreateWindow(XtDisplay(VX_App.x.topLevel),
		      XtWindow(VX_App.x.topLevel), 0, 0, 65535, 65535,
		      (unsigned int) 0, CopyFromParent, InputOnly,
		      CopyFromParent, valuemask, & attributes);
    VX_GetLineGC ();
    VX_GetTextGC ();

    /* Create a graphic context for copying pixmap: */
    VX_App.o.gc = XCreateGC (XtDisplay(VX_App.x.imageView),
			     XtWindow(VX_App.x.imageView), 0, & gcv);

    /* Get the initial width and height of the imageView widget: */
    XtVGV (VX_App.x.imageView, XmNheight, &height, XmNwidth, &width,
	   (char *) NULL);

    /* If there is already an image to be displayed: */
    if (VX_App.v.image != NULL) {

	/* Determine desired size of the VImageView widget: */
	VImageWindowSize (VX_App.v.image, width, height,
			  &VX_App.x.cur_width, &VX_App.x.cur_height);

	/* Calculate row scale and column scale: */
	VX_App.v.row_scale = (float) VX_App.x.cur_height /
	    VImageNRows (VX_App.v.image) * VX_App.v.zoom_level;

	/* Set the XmNwidth and XmNheight resources of the VImageView widget
	   according to the new size: */
	XtVSV (VX_App.x.imageView,
	       XmNwidth, (Dimension) VX_App.x.cur_width,
	       XmNheight, (Dimension) VX_App.x.cur_height, (char *) NULL);
    }

    /* Due to a bug (?) in XmText widget, we have to simulate
       a button press to make the widget works properly: */
    event.type = FocusIn;
    XtCallActionProc (VX_App.x.msgArea, "grab-focus", & event, NULL, 0);
    XmTextSetInsertionPosition (VX_App.x.msgArea, 0);

    /* Set VX_Warning as Vista's warning handler. It pops up a dialog: */
    VSetWarningHandler (VX_Warning);

    VX_App.in_main_loop = TRUE;

    /* Start the main event loop: */
    XtAppMainLoop (VX_App.x.appContext);
}


/*
 *  VXReportValidOptions
 *
 *  Print, to stderr, a summary of VX-related program options.
 */

void VXReportValidOptions (void)
{
    VReportValidOptions (VNumber (options), options);
}


/*
 *  Local: RealizeWrapper
 *
 *  Ensure that a Shell widget about to be realized is given the appropriate
 *  visual. If this isn't done, shell-derived widgets created by the Motif
 *  library for things like menus inherit the root window's visual because
 *  they are descended directly from the root window.
 */

static void RealizeWrapper (Widget wid, Mask *vmask,
			    XSetWindowAttributes *attr)
{
    ShellWidget w = (ShellWidget) wid;

    if (VX_App.x.vcolormap)
	w->shell.visual = VColormapVisual (VX_App.x.vcolormap);
    orig_shell_realize (wid, vmask, attr);
}


/*
 * Local: HandleUserResize
 *
 * Handle resize of the VImageView widget to maintain its width-height
 * proportion.
 */

static void HandleUserResize (Widget w, XtPointer unused,
			      XConfigureEvent *event,
			      Boolean *continue_to_dispatch)
{
    int width, height;

    /* We are only interested in resize events:  */
    if (event->type != ConfigureNotify ||
	(VX_App.x.cur_width == event->width &&
	 VX_App.x.cur_height == event->height)) {
	return;
    }

    /* Screen has changed and pixmap is no longer consistent: */
    VX_App.o.pixmap_consistent = FALSE;

    /* Respect new size if no image is currently managed by VX: */
    if (VX_App.v.image == NULL) {

	/* Update current width and height: */
	VX_App.x.cur_width = (int) event->width;
	VX_App.x.cur_height = (int) event->height;
	return;
    }

    if (XtIsRealized (VX_App.x.imageView) == TRUE)
	XMapWindow (XtDisplay(VX_App.x.topLevel), VX_App.x.busyWindow);

    XSync (XtDisplay(VX_App.x.topLevel), FALSE);

    /* Determine desired size: */

    /* New bounding box cannot be smaller than initial size
       of the VImageView widget: */
    width = VMax (VX_App.x.init_width, event->width);
    height = VMax (VX_App.x.init_height, event->height);

    /* Determine desired size of the VImageView widget: */
    VImageWindowSize (VX_App.v.image, width, height,
		      &VX_App.x.cur_width, &VX_App.x.cur_height);

    /* Calculate row scale */
    VX_App.v.row_scale = (float) VX_App.x.cur_height /
	VImageNRows (VX_App.v.image) * VX_App.v.zoom_level;

    /* Set the XmNwidth and XmNheight resources of the VImageView widget
       according to the new size: */
    XtVSV (VX_App.x.imageView,
	   XmNwidth, (Dimension) VX_App.x.cur_width,
	   XmNheight, (Dimension) VX_App.x.cur_height, (char *) NULL);

    if (XtIsRealized (VX_App.x.imageView) == TRUE)
	XUnmapWindow (XtDisplay(VX_App.x.topLevel), VX_App.x.busyWindow);
}
