/*
 *  $Id: VImageView.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file implements the VImageView widget, which displays a VImage.
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
 *  Authors: Arthur Pope, Daniel Ko, Dan Razzell
 *	     UBC Laboratory for Computational Intelligence
 */

/* From the Xt Intrinsics: */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/os.h"
#include "viaio/VImage.h"
#include "VImageVieP.h"
#include "viaio/mu.h"

/* From the standard C library: */
#include <math.h>

/* For portability: */
#include <X11/Xos.h>

/* File identification string: */
VRcsId ("$Id: VImageView.c 3177 2008-04-01 14:47:24Z karstenm $");

/* Later in this file: */
static void Initialize (Widget, Widget, ArgList, Cardinal *);
static void Realize (Widget, Mask *, XSetWindowAttributes *);
static void Destroy (Widget);
static void Redisplay (Widget, XEvent *, Region);
static Boolean SetValues (Widget, Widget, Widget, ArgList, Cardinal *);
static XtGeometryResult QueryGeometry (Widget,
				       XtWidgetGeometry *, XtWidgetGeometry *);
static void InputAction (Widget, XEvent *, String *, Cardinal *);
static void ZoomInAction (Widget, XEvent *, String *, Cardinal *);
static void ZoomOutAction (Widget, XEvent *, String *, Cardinal *);
static void MoveZoomCenterAction (Widget, XEvent *, String *, Cardinal *);
static void GetGC (Widget);
static Window CreateBusyWindow (Widget);
static void RecordImage (VImageViewWidget, VImage);
static void RenderImage (Widget);


/*
 *  Table of widget's resources.
 */

static XtResource resources[] = {
#define offset(field) XtOffset(VImageViewWidget, viv.field)
  /* {name, class, type, size, offset, default_type, default_addr}, */
  { VxNabsolute, VxCAbsolute, XtRBoolean, sizeof (Boolean),
    offset (absolute), XtRImmediate, (XtPointer) TRUE },
  { VxNband, VxCBand, XtRInt, sizeof (int),
    offset (band), XtRImmediate, (XtPointer) 0 },
  { VxNcolumnCenter, VxCColumnCenter, XtRInt, sizeof (int),
    offset (column_center), XtRImmediate, (XtPointer) 0},
  { VxNcursor, VxCCursor, XtRCursor, sizeof (Cursor),
    offset(cursor), XtRString, (XtPointer) "crosshair" },
  { VxNexposeCallback, VxCCallback, XtRCallback, sizeof (XtCallbackList),
    offset (expose_callback), XtRCallback, NULL },
  { VxNimage, VxCImage, XtRPointer, sizeof (XtPointer),
    offset (image), XtRImmediate, NULL },
  { VxNinputCallback, VxCCallback, XtRCallback, sizeof (XtCallbackList),
    offset (input_callback), XtRCallback, NULL },
  { VxNmoveZoomCenterCallback, VxCCallback, XtRCallback, 
    sizeof (XtCallbackList), 
    offset (move_zoom_center_callback), XtRCallback, NULL },
  { VxNproportion, VxCProportion, XtRBoolean, sizeof (Boolean),
    offset (proportion), XtRImmediate, (XtPointer) TRUE },
  { VxNresize, VxCResize, XtRBoolean, sizeof (Boolean),
    offset (resize), XtRImmediate, (XtPointer) FALSE },
  { VxNrowCenter, VxCRowCenter, XtRInt, sizeof (int),
    offset (row_center), XtRImmediate, (XtPointer) 0},
  { VxNusePixmap, VxCUsePixmap, XtRBoolean, sizeof (Boolean),
    offset (use_pixmap), XtRImmediate, (XtPointer) TRUE },
  { VxNvColormap, VxCVColormap, XtRPointer, sizeof (XtPointer),
    offset (v_colormap), XtRImmediate, NULL },
  { VxNzoomInCallback, VxCCallback, XtRCallback, sizeof (XtCallbackList),
    offset (zoom_in_callback), XtRCallback, NULL },
  { VxNzoomLevel, VxCZoomLevel, XtRInt, sizeof (int),
    offset (zoom_level), XtRImmediate, (XtPointer) 100 },
  { VxNzoomOutCallback, VxCCallback, XtRCallback, sizeof (XtCallbackList),
    offset (zoom_out_callback), XtRCallback, NULL }
#undef offset
};


/*
 *  Table binding action names to procedures.
 */

static XtActionsRec actions[] =
{
  /* { name, procedure }, */
  { "Input", InputAction },
  { "ZoomIn", ZoomInAction },
  { "ZoomOut", ZoomOutAction },
  { "MoveZoomCenter", MoveZoomCenterAction },
};


/* 
 *  Default binding of events to actions.
 */

static char translations[] =
"!Ctrl<Btn2Down>: MoveZoomCenter()\n"
"None<Btn2Down>: ZoomIn()\n"
"None<Btn3Down>: ZoomOut()\n"
"<Key>: Input()\n"
"<BtnDown>: Input()";


/*
 *  Class record.
 */

VImageViewClassRec vImageViewClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) & coreClassRec,
					/* class_name		*/	"VImageView",
					/* widget_size		*/	sizeof (VImageViewRec),
					/* class_initialize		*/	NULL,
					/* class_part_initialize	*/	NULL,
					/* class_inited		*/	FALSE,
					/* initialize		*/	Initialize,
					/* initialize_hook		*/	NULL,
					/* realize			*/	Realize,
					/* actions			*/	actions,
					/* num_actions		*/	XtNumber (actions),
					/* resources		*/	resources,
					/* num_resources		*/	XtNumber (resources),
					/* xrm_class		*/	NULLQUARK,
					/* compress_motion		*/	TRUE,
					/* compress_exposure	*/	TRUE,
					/* compress_enterleave	*/	TRUE,
					/* visible_interest		*/	FALSE,
					/* destroy			*/	Destroy,
					/* resize			*/	NULL,
					/* expose			*/	Redisplay,
					/* set_values		*/	SetValues,
					/* set_values_hook		*/	NULL,
					/* set_values_almost	*/	XtInheritSetValuesAlmost,
					/* get_values_hook		*/	NULL,
					/* accept_focus		*/	NULL,
					/* version			*/	XtVersion,
					/* callback_private		*/	NULL,
					/* tm_table			*/	translations,
					/* query_geometry		*/	QueryGeometry,
					/* display_accelerator	*/	XtInheritDisplayAccelerator,
					/* extension		*/	NULL
  },
  { /* VImageView fields */
    /* empty			*/	0
  }
};

WidgetClass vImageViewWidgetClass = (WidgetClass) & vImageViewClassRec;


/*
 *  Initialize
 *
 *  Method for initializing a widget instance.
 */

/* ARGSUSED */
static void Initialize (Widget request, Widget new,
			ArgList args, Cardinal *num_args)
{
#define VNEW ((VImageViewWidget) new)

  VNEW->viv.ximage = NULL;
  VNEW->viv.pixmap = None;
  VNEW->viv.port.height = VNEW->viv.port.width = 0;
  VNEW->viv.free_vcolormap = FALSE;
    
  /* Allocate a GC for drawing to pixmap and window: */
  GetGC (new);

  /* Ensure that any image and band number supplied are consistent: */
  if (VNEW->viv.band < 0 ||
      (VNEW->viv.image &&
       VNEW->viv.band >= VImageNBands (VNEW->viv.image))) {
    VWarning ("VImageView: Band number %d is out of range");
    VNEW->viv.band = 0;
  }

  /* Make a copy of any image that has been provided: */
  if (VNEW->viv.image)
    RecordImage (VNEW, VNEW->viv.image);

  /* If no width and height have been specified, choose suitable defaults: */
  if (VNEW->core.width == 0)
    VNEW->core.width = 256;
  if (VNEW->core.height == 0)
    VNEW->core.height = 256;

  /* Ensure that the zooming parameters are suitable: */
  if (VNEW->viv.image) {
    if (VNEW->viv.row_center < 0 ||
	VNEW->viv.row_center >= VImageNRows (VNEW->viv.image)) {
      VWarning ("VImageView: rowCenter (%d) is out of image",
		VNEW->viv.row_center);
      VNEW->viv.row_center = 0;
    }
    if (VNEW->viv.column_center < 0 ||
	VNEW->viv.column_center >= VImageNColumns (VNEW->viv.image)) {
      VWarning ("VImageView: columnCenter (%d) is out of image",
		VNEW->viv.column_center);
      VNEW->viv.column_center = 0;
    }
  }
  if (VNEW->viv.zoom_level < 100) {
    VWarning ("VImageView: zoomLevel %d is less than 100",
	      VNEW->viv.zoom_level);
    VNEW->viv.zoom_level = 100;
  }

  /* Note the need to render the image: */
  VNEW->viv.render_needed = TRUE;

#undef VNEW
}


/*
 *  Realize
 *
 *  Method for creating widget's window.
 */

static void Realize (Widget w, Mask *value_mask,
		     XSetWindowAttributes *attributes)
{
#define VW ((VImageViewWidget) w)

  /* Force a background of None so that the window isn't cleared by the
     server following an Expose event, or by Xt following a SetValues: */
  attributes->background_pixmap = None;
  *value_mask |= CWBackPixmap;
  *value_mask &= ~ CWBackPixel;

  /* Set the window's cursor attribute: */
  attributes->cursor = VW->viv.cursor;
  if ((attributes->cursor = VW->viv.cursor) != None)
    *value_mask |= CWCursor;

  /* Create the window: */
  XtCreateWindow (w, (unsigned int) InputOutput, (Visual *) CopyFromParent,
		  *value_mask, attributes );

  /* If no vcolormap has been allocated, allocate one: */
  if (! VW->viv.v_colormap) {
    XWindowAttributes attrs;
    XVisualInfo template;

    if (! XGetWindowAttributes (XtDisplay (w), XtWindow (w), & attrs))
      VWarning ("VImageView: XGetWindowAttributes failed");
    template.visualid = XVisualIDFromVisual (attrs.visual);
    template.depth = attrs.depth;
    VW->viv.v_colormap = VCreateColormap (VW->core.screen, None,
					  VisualIDMask | VisualDepthMask,
					  & template);
    if (! VW->viv.v_colormap)
      VWarning ("VImageView: VCreateColormap failed");
    VW->viv.free_vcolormap = TRUE;
  }

  VW->viv.busy_window = CreateBusyWindow (w);

#undef VW
}


/*
 *  Destroy
 *
 *  Method for destroying a widget instance.
 */

static void Destroy (Widget w)
{
  Display *display = XtDisplay (w);

#define VW ((VImageViewWidget) w)

  if (VW->viv.ximage)
    XDestroyImage (VW->viv.ximage);
  if (VW->viv.pixmap != None)
    XFreePixmap (display, VW->viv.pixmap);
  XtReleaseGC (w, VW->viv.gc);
  if (VW->viv.free_vcolormap)
    VDestroyColormap (VW->viv.v_colormap);

#undef VW
}


/*
 *  Redisplay
 *
 *  Method for handling expose events in a widget's window.
 */

static void Redisplay (Widget w, XEvent *event, Region region)
{
  int x, y;
  unsigned int width, height;

#define VW ((VImageViewWidget) w)
#define EVENT ((XExposeEvent *) event)

  if (! XtIsRealized (w))
    return;

  RenderImage (w);

  /* If there is an event, it reports the area to be redrawn: */
  if (event) {
    x = EVENT->x;
    y = EVENT->y;
    width = EVENT->width;
    height = EVENT->height;
  } else {
    x = y = 0;
    width = VW->viv.port.width;
    height = VW->viv.port.height;
  }
    
  /* Draw the image, if there is one: */
  if (VW->viv.image) {
    if (VW->viv.pixmap != None)
	    
      /* Copy from the backing pixmap to the window: */
      XCopyArea (XtDisplay (w), VW->viv.pixmap, XtWindow (w),
		 VW->viv.gc, x, y, width, height, x, y);
	
    else if (VW->viv.ximage)
	    
      /* Copy from the host-resident XImage to the window: */
      XPutImage (XtDisplay (w), XtWindow (w), VW->viv.gc,
		 VW->viv.ximage, x, y, x, y, width, height);
  } 

  /* Clear any portions of the window to the right or bottom of the image: */
  if (VW->viv.port.width < x + width ||
      VW->viv.port.height < y + height) {

    /* Temporarily give the window the background pixel or pixmap
       specified by the widget resource: */
    if (VW->core.background_pixmap != XtUnspecifiedPixmap)
      XSetWindowBackgroundPixmap (XtDisplay (w), XtWindow (w),
				  VW->core.background_pixmap);
    else XSetWindowBackground (XtDisplay (w), XtWindow (w),
			       VW->core.background_pixel);

    /* Clear from port.width to x + width: */
    if (VW->viv.port.width < x + width)
      XClearArea (XtDisplay (w), XtWindow (w),
		  VW->viv.port.width, y,
		  x + width - VW->viv.port.width, height, FALSE);

    /* Clear from port.height to y + height: */
    if (VW->viv.port.height < y + height)
      XClearArea (XtDisplay (w), XtWindow (w),
		  x, VW->viv.port.height,
		  width, y + height - VW->viv.port.height, FALSE);

    /* Return to a background of None: */
    XSetWindowBackgroundPixmap (XtDisplay (w), XtWindow (w), None);
  }
	
  /* Call any expose callbacks to draw overtop image: */
  XtCallCallbacks (w, VxNexposeCallback, (caddr_t) region);

#undef VW
#undef EVENT
}


/*
 *  SetValues
 *
 *  Method for handling changes to a widget's public instance variables.
 */

static Boolean SetValues (Widget current, Widget request, Widget new,
			  ArgList args, Cardinal *num_args)
{
  int i;
  Boolean do_redisplay = FALSE, was_resized = FALSE;
  Boolean height_set = FALSE, width_set = FALSE;

#define VCUR ((VImageViewWidget) current)
#define VREQ ((VImageViewWidget) request)
#define VNEW ((VImageViewWidget) new)

  /* Ensure that the new image and band number are consistent: */
  if (VNEW->viv.band < 0 ||
      (VNEW->viv.image &&
       VNEW->viv.band >= VImageNBands (VNEW->viv.image))) {
    VWarning ("VImageView: Band number %d is out of range");
    VNEW->viv.band = 0;
  }

  /* Check for a change in band number: */
  if (VCUR->viv.band != VNEW->viv.band)
    do_redisplay = TRUE;

  /* Check for a change in image: */
  if (VCUR->viv.image != VNEW->viv.image) {

    /* Discard any previous image: */
    if (VCUR->viv.image)
      VDestroyImage (VCUR->viv.image);

    /* Make a copy of any new image that has been provided: */
    if (VNEW->viv.image)
      RecordImage (VNEW, VNEW->viv.image);

    do_redisplay = TRUE;
    was_resized = VNEW->viv.image != NULL;
  }

  /* Recalculate the window size if something affecting it has changed,
     and if the resize resource is TRUE: */
  if (was_resized && VNEW->viv.resize) {

    /* Note whether width or height was explicitly set: */
    for (i = 0; i < *num_args; i++) {
      if (strcmp (args[i].name, XtNheight) == 0)
	height_set = TRUE;
      if (strcmp (args[i].name, XtNwidth) == 0)
	width_set = TRUE;
    }

    /* Compute a new height if none was explicitly set by the caller
       or by the superclass: */
    if (! height_set && VCUR->core.height == VREQ->core.height)
      VNEW->core.height = VImageNRows (VNEW->viv.image);
	
    /* Compute a new width if none was explicitly set by the caller
       or by the superclass: */
    if (! width_set && VCUR->core.width == VREQ->core.width)
      VNEW->core.width = VImageNColumns (VNEW->viv.image);
  }

  /* Check for a change in proportion: */
  if (VCUR->viv.proportion != VNEW->viv.proportion)
    do_redisplay = TRUE;

  /* Check for a change in absolute: */
  if (VCUR->viv.absolute != VNEW->viv.absolute)
    do_redisplay = TRUE;

  /* Ensure that the zooming parameters are suitable: */
  if (VNEW->viv.image) {
    if (VNEW->viv.row_center < 0 ||
	VNEW->viv.row_center >= VImageNRows (VNEW->viv.image)) {
      if (VCUR->viv.row_center != VNEW->viv.row_center) {
	VWarning ("VImageView: rowCenter %d is out of image",
		  VNEW->viv.row_center);
      }
      VNEW->viv.row_center = 0;
    }
    if (VNEW->viv.column_center < 0 ||
	VNEW->viv.column_center >= VImageNColumns (VNEW->viv.image)) {
      if (VCUR->viv.column_center != VNEW->viv.column_center) {
	VWarning ("VImageView: columnCenter %d is out of image",
		  VNEW->viv.column_center);
      }
      VNEW->viv.column_center = 0;
    }
  }
  if (VNEW->viv.zoom_level < 100) {
    VWarning ("VImageView: zoomLevel %d is less than 100",
	      VNEW->viv.zoom_level);
    VNEW->viv.zoom_level = 100;
  }
    
  /* Check for a change in zooming parameters: */
  if (VCUR->viv.row_center != VNEW->viv.row_center ||
      VCUR->viv.column_center != VNEW->viv.column_center ||
      VCUR->viv.zoom_level != VNEW->viv.zoom_level)
    do_redisplay = TRUE;

  /* Check for a change in vcolormap parameters: */
  if (VCUR->viv.v_colormap != VNEW->viv.v_colormap) {
    if (VCUR->viv.free_vcolormap) {
      VDestroyColormap (VCUR->viv.v_colormap);
      VNEW->viv.free_vcolormap = FALSE;
    }
    do_redisplay = TRUE;
  }

  /* Rerender the image into a pixmap, if necessary: */
  VNEW->viv.render_needed |= do_redisplay;

  /* Check for a change in cursor: */
  if (VCUR->viv.cursor != VNEW->viv.cursor &&
      XtIsRealized (new))
    XDefineCursor (XtDisplay(new), XtWindow (new),
		   VNEW->viv.cursor);

  return do_redisplay;

#undef VCUR
#undef VREQ
#undef VNEW
}


/*
 *  QueryGeometry
 *
 *  Method for validating proposed geometry changes.
 */

static XtGeometryResult QueryGeometry (Widget w,
				       XtWidgetGeometry *proposed,
				       XtWidgetGeometry *answer)
{
#define VW ((VImageViewWidget) w)


  if (!VW->viv.resize) {
    answer->width = VW->core.width;
    answer->height = VW->core.height;
  } else {	
    /* Choose a preferred size based on image size, if we have an image, or
       a default size, if we have no image: */
    if (VW->viv.image) {
      answer->width = VImageNColumns (VW->viv.image);
      answer->height = VImageNRows (VW->viv.image);
    } else answer->width = answer->height = 256;
  }

  answer->request_mode = CWWidth | CWHeight;

  /* Return YES if this happens to be the same dimensions proposed: */
  if ((proposed->request_mode & CWWidth) &&
      (proposed->request_mode & CWHeight) &&
      (proposed->width == answer->width) &&
      (proposed->height == answer->height))
    return XtGeometryYes;

  /* Return NO if this represents no change from the current dimensions: */
  if (answer->width == VW->core.width && answer->height == VW->core.height)
    return XtGeometryNo;

  /* Otherwise return ALMOST: */
  return XtGeometryAlmost;

#undef VW
}


/*
 *  InputAction
 *
 *  Input action procedure.
 */

/* ARGSUSED */
static void InputAction (Widget w, XEvent *event,
			 String *params, Cardinal *num_params)
{
  XtCallCallbacks (w, VxNinputCallback, (caddr_t) event);
}


/*
 * ZoomInAction
 *
 * ZoomIn action procedure
 */

/* ARGSUSED */
static void ZoomInAction (Widget w, XEvent *event,
			  String *params, Cardinal *num_params)
{
  int zoom_level, x, y;
  double row, column;

  /* Get cursor position: */
  if (event->type == KeyPress || event->type == KeyRelease) {
    x = ((XKeyEvent *) event)->x;
    y = ((XKeyEvent *) event)->y;
  } else if (event->type == ButtonPress || event->type == ButtonRelease) {
    x = ((XButtonEvent *) event)->x;
    y = ((XButtonEvent *) event)->y;
  } else {
    XtCallCallbacks (w, VxNzoomInCallback, (caddr_t) event);
    return;
  }
    
  /* Set new zooming parameters: */
  if (VImageViewWindowToImage (w, x, y, & row, & column)) {
    XtVaGetValues (w, VxNzoomLevel, & zoom_level, (char *) NULL);
    XtVaSetValues (w, VxNrowCenter, (int) row,
		   VxNcolumnCenter, (int) column,
		   VxNzoomLevel, zoom_level * 2, (char *) NULL);
  }
    
  XtCallCallbacks (w, VxNzoomInCallback, (caddr_t) event);
}


/*
 * ZoomOutAction
 *
 * ZoomOut action procedure
 */

/* ARGSUSED */
static void ZoomOutAction (Widget w, XEvent *event,
			   String *params, Cardinal *num_params)
{
  int zoom_level, x, y;
  double row, column;

  /* Get cursor position: */
  if (event->type == KeyPress || event->type == KeyRelease) {
    x = ((XKeyEvent *) event)->x;
    y = ((XKeyEvent *) event)->y;
  } else if (event->type == ButtonPress || event->type == ButtonRelease) {
    x = ((XButtonEvent *) event)->x;
    y = ((XButtonEvent *) event)->y;
  } else {
    XtCallCallbacks (w, VxNzoomOutCallback, (caddr_t) event);
    return;
  }
    
  /* Set new zooming parameters: */
  if (VImageViewWindowToImage (w, x, y, & row, & column)) {
    XtVaGetValues (w, VxNzoomLevel, & zoom_level, (char *) NULL);
    if (zoom_level > 100)
      XtVaSetValues (w, VxNrowCenter, (int) row,
		     VxNcolumnCenter, (int) column,
		     VxNzoomLevel, VMax (100, zoom_level / 2),
		     (char *) NULL);
  }
    
  XtCallCallbacks (w, VxNzoomOutCallback, (caddr_t) event);
}


/*
 * MoveZoomCenterAction
 *
 * MoveZoomCenter action procedure
 */

/* ARGSUSED */
static void MoveZoomCenterAction (Widget w, XEvent *event,
				  String *params, Cardinal *num_params)
{
  int x, y;
  double row, column;

  if (event->type == KeyPress || event->type == KeyRelease) {
    x = ((XKeyEvent *) event)->x;
    y = ((XKeyEvent *) event)->y;
  } else if (event->type == ButtonPress || event->type == ButtonRelease) {
    x = ((XButtonEvent *) event)->x;
    y = ((XButtonEvent *) event)->y;
  } else {
    XtCallCallbacks (w, VxNmoveZoomCenterCallback, (caddr_t) event);
    return;
  }
    
  if (VImageViewWindowToImage (w, x, y, & row, & column))
    XtVaSetValues (w, VxNrowCenter, (int) row,
		   VxNcolumnCenter, (int) column, (char *) NULL);
    
  XtCallCallbacks (w, VxNmoveZoomCenterCallback, (caddr_t) event);
}


/*
 *  GetGC
 *
 *  Allocate a GC for drawing into the pixmap or window.
 */

static void GetGC (Widget w)
{
  XGCValues values;
  XtGCMask mask = GCForeground | GCBackground | GCGraphicsExposures;

#define VW ((VImageViewWidget) w)

  values.background = BlackPixelOfScreen (XtScreen (w));
  values.foreground = WhitePixelOfScreen (XtScreen (w));
  values.graphics_exposures = FALSE;
  VW->viv.gc = XtGetGC (w, mask, & values);

#undef VW
}


/*
 * CreateBusyWindow
 *
 * Create a window for displaying a busy cursor.
 */

static Window CreateBusyWindow (Widget w)
{
  unsigned long valuemask;
  XSetWindowAttributes attributes;
  Screen *screen = XtScreen (w);

  /* Create a window that will display a busy cursor when mapped: 
     [thanks to Andrew Wason (aw@cellar.bae.bellcore.com), Dan Heller 
     (argv@sun.com), and mouse@larry.mcrcim.mcgill.edu] */
    
  /* Ignore device events while the busy cursor is displayed. */
  valuemask = CWDontPropagate | CWCursor;
  attributes.do_not_propagate_mask = (KeyPressMask | KeyReleaseMask |
				      ButtonPressMask | ButtonReleaseMask |
				      PointerMotionMask);
  attributes.cursor = XCreateFontCursor (XtDisplay (w), XC_watch);
    
  /* The window will be as big as the display screen, and clipped by
     its own parent window, so we never have to worry about resizing */
  return XCreateWindow (XtDisplay (w), XtWindow (w), 0, 0,
			WidthOfScreen (screen), HeightOfScreen (screen),
			(unsigned int) 0, CopyFromParent, InputOnly,
			CopyFromParent, valuemask, & attributes);
}


/*
 *  RecordImage
 *
 *  Make a copy of an image supplied for display, and also note (a) its
 *  pixel aspect ratio, and (b) whether its an RGB color image.
 */

static void RecordImage (VImageViewWidget w, VImage image)
{
  /* Take a copy of the new image's pixels: */
  w->viv.image = VCopyImagePixels (image, NULL, VAllBands);

    /* Note its pixel aspect ratio and whether or not it's color: */
  w->viv.pixel_aspect_ratio = 1.0;
  if (VGetAttr (VImageAttrList (image), VPixelAspectRatioAttr, NULL,
		VFloatRepn, & w->viv.pixel_aspect_ratio) == VAttrBadValue)
    VWarning ("VImageView: Image has bad %s attribute",
	      VPixelAspectRatioAttr);

  w->viv.is_color =
    (VImageColorInterp (image) == VBandInterpRGB) &&
    (VImageNComponents (image) == 1);
  if (w->viv.is_color) {
    VSetAttr (VImageAttrList (w->viv.image), VColorInterpAttr,
	      VBandInterpDict, VLongRepn, VBandInterpRGB);
    VImageNComponents (w->viv.image) = 1;
    VImageNColors (w->viv.image) = 3;
    VImageNViewpoints (w->viv.image) = VImageNViewpoints (image);
    VImageNFrames (w->viv.image) = VImageNFrames (image);
  }
}


/*
 *  RenderImage
 *
 *  Render the image into a pixmap cached at the server.
 */

static void RenderImage (Widget w)
{
#define VW ((VImageViewWidget) w)

  float row_scale, column_scale, scale;
  int save_height, save_width;
  VBoolean size_change;

  /* If the widget isn't realized, postpone rendering until it is: */
  if (! XtIsRealized (w) || ! VW->viv.v_colormap)
    return;

  /* If no image is set, we're done already: */
  if (! VW->viv.image) {
    VW->viv.render_needed = FALSE;
    VW->viv.port.width = VW->viv.port.height = 0.0;
    return;
  }

  /* Determine the portion of the image displayed at the
     current zoom level: */
  save_height = VW->viv.port.height;
  save_width = VW->viv.port.width;
  if (VW->viv.proportion) {
    row_scale = (float) VW->core.height / VImageNRows (VW->viv.image);
    column_scale = (float) VW->core.width /
      (VImageNColumns (VW->viv.image) * VW->viv.pixel_aspect_ratio);
    scale = VMin (row_scale, column_scale) * VW->viv.zoom_level / 100.0f;
    VW->viv.port.nrows = (float) VW->core.height / scale + 0.5f;
    VW->viv.port.nrows = VMax (VW->viv.port.nrows, 1);
    VW->viv.port.nrows =
      VMin (VW->viv.port.nrows, VImageNRows (VW->viv.image));
    VW->viv.port.ncolumns = (float) VW->core.width /
      (scale * VW->viv.pixel_aspect_ratio) + 0.5f;
    VW->viv.port.ncolumns = VMax (VW->viv.port.ncolumns, 1);
    VW->viv.port.ncolumns =
      VMin (VW->viv.port.ncolumns, VImageNColumns (VW->viv.image));
    VW->viv.port.height = VW->viv.port.nrows * scale;
    VW->viv.port.height = VMin (VW->viv.port.height, VW->core.height);
    VW->viv.port.width = VW->viv.port.ncolumns * scale *
      VW->viv.pixel_aspect_ratio;
    VW->viv.port.width = VMin (VW->viv.port.width, VW->core.width);
  } else {
    scale = (float) VW->viv.zoom_level / 100.0;
    VW->viv.port.nrows = VImageNRows (VW->viv.image) / scale + 0.5f;
    VW->viv.port.nrows = VMax (VW->viv.port.nrows, 1);
    VW->viv.port.ncolumns = VImageNColumns (VW->viv.image) / scale + 0.5f;
    VW->viv.port.ncolumns = VMax (VW->viv.port.ncolumns, 1);
    VW->viv.port.height = VW->core.height;
    VW->viv.port.width = VW->core.width;
  }
  VW->viv.port.first_row =
    VW->viv.row_center - VW->viv.port.nrows / 2;
  VW->viv.port.first_row = VMax (VW->viv.port.first_row, 0);
  VW->viv.port.first_row =
    VMin (VW->viv.port.first_row,
	  VImageNRows (VW->viv.image) - VW->viv.port.nrows);
  VW->viv.port.first_column =
    VW->viv.column_center - VW->viv.port.ncolumns / 2;
  VW->viv.port.first_column = VMax (VW->viv.port.first_column, 0);
  VW->viv.port.first_column =
    VMin (VW->viv.port.first_column,
	  VImageNColumns (VW->viv.image) - VW->viv.port.ncolumns);

  /* ==> purify reports an uninitialized memory read here. */
  size_change = (VW->viv.port.height != VW->viv.alloced_height ||
		 VW->viv.port.width != VW->viv.alloced_width);

  if (! VW->viv.render_needed && ! size_change &&
      save_height == VW->viv.port.height &&
      save_width == VW->viv.port.width)
    return;
  VW->viv.render_needed = FALSE;

  /* If an XImage is allocated but not of the right size, free it: */
  if (VW->viv.ximage && size_change) {
    XDestroyImage (VW->viv.ximage);
    VW->viv.ximage = NULL;
  }

  /* If no XImage is allocated, allocate one: */
  if (! VW->viv.ximage) {
    VW->viv.ximage =
      XCreateImage (XtDisplay (w),
		    VW->viv.v_colormap->vinfo.visual,
		    VW->viv.v_colormap->vinfo.depth,
		    ZPixmap, 0, NULL,
		    VW->viv.port.width, VW->viv.port.height,
		    BitmapPad (XtDisplay (w)), 0);
    if (! VW->viv.ximage) {
      VWarning ("VImageView: Unable to allocate XImage");
      return;
    }
    VW->viv.ximage->data =
      VMalloc (VW->viv.ximage->bytes_per_line * VW->viv.port.height);
  }

  /* If a pixmap is allocated but not of the right size, free it: */
  if (VW->viv.pixmap != None && size_change) {
    XFreePixmap (XtDisplay (w), VW->viv.pixmap);
    VW->viv.pixmap = None;
  }
    
  /* If no pixmap is allocated, allocate one: */
  if (VW->viv.pixmap == None && VW->viv.use_pixmap)
    VW->viv.pixmap =
      XCreatePixmap (XtDisplay (w), 
		     RootWindow (XtDisplay (w),
				 DefaultScreen (XtDisplay (w))),
		     VW->viv.port.width, VW->viv.port.height,
		     VW->core.depth);

  VW->viv.alloced_height = VW->viv.port.height;
  VW->viv.alloced_width = VW->viv.port.width;

  XMapWindow (XtDisplay (w), VW->viv.busy_window);
  XSync (XtDisplay (w), FALSE);

  V_RenderVToX (VW);

  XUnmapWindow (XtDisplay (w), VW->viv.busy_window);

  /* Copy the bitmap or pixmap to the backing pixmap: */
  if (VW->viv.pixmap != None)
    XPutImage (XtDisplay (w), VW->viv.pixmap, VW->viv.gc, VW->viv.ximage,
	       0, 0, 0, 0, VW->viv.port.width, VW->viv.port.height);
    
#undef VW
}


/*
 *  VImageViewWindowToImage
 *
 *  Convenience routine mapping window coordinates to image coordinates.
 */

VBoolean VImageViewWindowToImage (Widget w, int x, int y,
				  double *rowp, double *columnp)
{
#define VW ((VImageViewWidget) w)

  if (! VW->viv.image) {
    VWarning ("VImageViewWindowToImage: Widget has NULL image resource");
    return FALSE;
  }

  /* Check if (x,y) lies outside of displayed image: */
  if (x < 0 || x >= VW->viv.port.width ||
      y < 0 || y >= VW->viv.port.height)
    return FALSE;

  /* Convert window coordinates to image coordinates: */
  *rowp = (double) y * VW->viv.port.nrows / VW->viv.port.height +
    VW->viv.port.first_row;
  *columnp = (double) x * VW->viv.port.ncolumns / VW->viv.port.width +
    VW->viv.port.first_column;

  return TRUE;

#undef VW
}


/*
 *  VImageViewClipToImage
 *
 *  Convenience routine mapping window coordinates (clipped against the
 *  boundary of the displayed image) to image coordinates.
 */

VBoolean VImageViewClipToImage (Widget w, int x, int y,
				double *rowp, double *columnp)
{
#define VW ((VImageViewWidget) w)

  if (! VW->viv.image) {
    VWarning ("VImageViewClipToImage: Widget has NULL image resource");
    return FALSE;
  }

  /* Clip x and y coordinates to the portion of the widget
     containing the displayed image: */
  x = VMax (0, x);
  x = VMin (VW->viv.port.width - 1, x);
  y = VMax (0, y);
  y = VMin (VW->viv.port.height - 1, y);
    
  return VImageViewWindowToImage (w, x, y, rowp, columnp);

#undef VW
}


/*
 *  VImageViewImageToWindow
 *
 *  Convenience routine mapping image coordinates to window coordinates.
 */

VBoolean VImageViewImageToWindow (Widget w, double row, double column,
				  int *xp, int *yp)
{
#define VW ((VImageViewWidget) w)

  if (! VW->viv.image) {
    VWarning ("VImageViewImageToWindow: Widget has NULL image resource");
    return FALSE;
  }

  row -= VW->viv.port.first_row;
  column -= VW->viv.port.first_column;

  *xp = ceil (column * VW->viv.port.width / VW->viv.port.ncolumns);
  *yp = ceil (row * VW->viv.port.height / VW->viv.port.nrows);

  return TRUE;

#undef VW
}


/*
 *  VImageViewPixelSize
 *
 *  Convenience routine for finding the width and height (in window
 *  pixels) of an image pixel.
 */

VBoolean VImageViewPixelSize (Widget w, double *width, double *height)
{
#define VW ((VImageViewWidget) w)
    
  if (! VW->viv.image) {
    VWarning ("VImageViewImagePixelSize: Widget has NULL image resource");
    return FALSE;
  }

  *width = (double) VW->viv.port.width / VW->viv.port.ncolumns;
  *height = (double) VW->viv.port.height / VW->viv.port.nrows;

  return TRUE;

#undef VW
}


/*
 *  VImageViewRedraw.
 *
 *  Routine for redrawing the image and calling the expose callbacks.
 */

void VImageViewRedraw (Widget w)
{
#define VW ((VImageViewWidget) w)

  if (! XtIsRealized (w))
    return;

  Redisplay (w, NULL, NULL);
    
#undef VW
}
