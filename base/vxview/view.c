/*
 *  $Id: view.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *  This file implements views.
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
 *  Author: Arthur Pope, UBC Laboratory for Computational Intelligence
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* From the local directory: */
#include "vxview.h"

/* From the Vista library: */
#include <viaio/mu.h>
#include <viaio/VEdges.h>
#include <viaio/VImage.h>
#include <viaio/VImageView.h>

/* From X Windows and the Motif toolkit: */
#include <X11/IntrinsicP.h>
#include <X11/keysym.h>
#include <X11/CoreP.h>
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
#include <X11/Xmu/Editres.h>
#endif
#include <Xm/CascadeBG.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/List.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/MainW.h>
#include <Xm/PushBG.h>
#include <Xm/Scale.h>

/* From the standard C library: */
#include <math.h>

/* File identification string: */
VRcsId ("$Id: view.c 3629 2009-08-20 17:04:30Z proeger $");

/* Later in this file: */
static void FileMenuCB (Widget, XtPointer, XtPointer);
static void ViewMenuCB (Widget, XtPointer, XtPointer);
static void HelpMenuCB (Widget, XtPointer, XtPointer);
static void SelectObjectCB (Widget, XtPointer, XtPointer);
static void SelectBandCB (Widget, XtPointer, XtPointer);
static void ImageExposeCB (Widget, XtPointer, XtPointer);
static void DrawEdge (View, VEdge);
static void EdgeToWindowCoords (View, VFloat [2], XPoint *);
static void LocateEdge (View, double, double);
static void UpdatePixelReport (View, int, int);


/*
 *  CreateView
 *
 *  Create a new view.
 */

View CreateView (VBoolean propagate_WM_hints)
{
    View view;
    Widget main_window, work, form, w;
    Pixel pixel;
    XmString str;
    static Arg list_args[] = { { XmNlistSizePolicy, XmCONSTANT },
			       { XmNselectionPolicy, XmMULTIPLE_SELECT } };

    /* Allocate a record for keeping track of the view: */
    view = VNew (ViewRec);
    view->next = views;
    views = view;
    view->filename = view->short_name = NULL;
    view->attributes = NULL;
    view->nobjects = 0;
    view->objects = view->image_object = NULL;
    view->image = view->tmp_image = NULL;
    view->prefs = defaultPrefs;
    view->band = 0;
    view->act_color = view->show_color = view->edge_selected = FALSE;
    view->gc = 0;
    view->attr_shell = view->file_shell = view->pref_shell = NULL;

    /* Create the view's top-level shell: */
    view->view_shell = XVCPS ("view", topLevelShellWidgetClass, topLevelShell,
			      XmNcolormap, VColormapColormap (vcolormap),
			      XmNkeyboardFocusPolicy, XmPOINTER,
			      XmNtitle, defaultTitle,
			      XmNvisual, VColormapVisual (vcolormap),
			      (char *) NULL);
    if (iconPixmap != XmUNSPECIFIED_PIXMAP)
	XtVaSetValues (view->view_shell, XmNiconPixmap, iconPixmap,
		       (char *) NULL);
    XtAddCallback (view->view_shell, XmNdestroyCallback,
		   DestroyView, (XtPointer) view);

    /* If this window is being created at program startup, pass along
       the geometry and iconic resources which supply hints for the
       window manager: */
    if (propagate_WM_hints) {
	String geometry;
	Boolean iconic;

	XtVaGetValues (topLevelShell,
		       XtNgeometry, & geometry, XtNiconic, & iconic,
		       (char *) NULL);
	XtVaSetValues (view->view_shell,
		       XtNgeometry, geometry, XtNiconic, iconic,
		       (char *) NULL);
    }

    /* Participate in the editres protocol: */
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
    XtAddEventHandler (view->view_shell, (EventMask) 0, TRUE,
		       _XEditResCheckMessages, NULL);
#endif

    /* Create the main window widgets: */
    main_window = XmCreateMainWindow (view->view_shell, "main", NULL, 0);
    XtAddCallback (main_window, XmNhelpCallback, ShowHelpDialog, NULL);
    view->menu_bar =
	XmVaCreateSimpleMenuBar (main_window, "menubar",
				 XmVaCASCADEBUTTON, NULL, 0,	/* File */
				 XmVaCASCADEBUTTON, NULL, 0,	/* View */
				 XmVaCASCADEBUTTON, NULL, 0,	/* Help */
				 (char *) NULL);
    XtVaSetValues (view->menu_bar, XmNmenuHelpWidget,
		   XtNameToWidget (view->menu_bar, "button_2"),
		   (char *) NULL);
    work = XVCW ("work", xmFormWidgetClass, main_window, (char *) NULL);
    XmMainWindowSetAreas (main_window, view->menu_bar, NULL, NULL, NULL, work);

    /* Create the view's menus: */
    view->menus[fileMenu] =
	XmVaCreateSimplePulldownMenu (
	    view->menu_bar, "file", fileMenu, FileMenuCB,
	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,		/* New */
	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,		/* Remove */
	    XmVaSEPARATOR,					/* ---- */
	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,		/* Open */
	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,		/* Reread */
	    XmVaSEPARATOR,					/* ---- */
	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,		/* Quit */
	    (char *) NULL);

    view->menus[viewMenu] =
	XmVaCreateSimplePulldownMenu (
	    view->menu_bar, "view", viewMenu, ViewMenuCB,
	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,		/* Attrs */
	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,		/* Display */
	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,		/* Edge Data */
	    (char *) NULL);

    view->menus[helpMenu] =
	XmVaCreateSimplePulldownMenu (
	    view->menu_bar, "help", helpMenu, HelpMenuCB,
	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,		/* Index */
	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,		/* Version */
	    (char *) NULL);

    /* The main window contains a form on the left, an image on the right: */
    form = XVCW ("form", xmFormWidgetClass, work,
		 XmNtopAttachment, XmATTACH_FORM,
		 XmNleftAttachment, XmATTACH_FORM,
		 XmNresizePolicy, XmRESIZE_NONE, (char *) NULL);

    w = XVCW ("frame", xmFrameWidgetClass, work,
	      XmNtopAttachment, XmATTACH_FORM,
	      XmNbottomAttachment, XmATTACH_FORM,
	      XmNleftAttachment, XmATTACH_WIDGET,
	      XmNleftWidget, form,
	      XmNrightAttachment, XmATTACH_FORM, (char *) NULL);
    XtVaGetValues (form, XmNbackground, & pixel, NULL);
    view->image_view = XVCMW ("image", vImageViewWidgetClass, w,
			      XmNbackground, pixel,
			      XmNheight, appData.size,
			      XmNwidth, appData.size,
			      VxNproportion, TRUE,
			      VxNresize, FALSE,
			      VxNvColormap, vcolormap,
			      (char *) NULL);
    XtManageChild (w);
    XtAddCallback (view->image_view, VxNexposeCallback,
		   ImageExposeCB, (XtPointer) view);

    /* Create control widgets on the left: */
    XVCMW ("object_label", xmLabelGadgetClass, form,
	   XmNtopAttachment, XmATTACH_FORM,
	   XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    view->object_list = XmCreateScrolledList (form, "object_list",
					      list_args, XtNumber (list_args));
    XtVaSetValues (XtParent (view->object_list),
		   XmNtopAttachment, XmATTACH_FORM,
		   XmNleftAttachment, XmATTACH_FORM,
		   XmNrightAttachment, XmATTACH_FORM, (char *) NULL);
    XtAddCallback (view->object_list, XmNmultipleSelectionCallback,
		   SelectObjectCB, (XtPointer) view);
    XtManageChild (view->object_list);

    XVCMW ("image_label", xmLabelGadgetClass, form,
	   XmNtopAttachment, XmATTACH_WIDGET,
	   XmNtopWidget, XtParent (view->object_list),
	   XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    view->image_info = XVCMW ("image_info", xmLabelGadgetClass, form,
			      XmNtopAttachment, XmATTACH_WIDGET,
			      XmNtopWidget, XtParent (view->object_list),
			      XmNleftAttachment, XmATTACH_FORM,
			      XmNrightAttachment, XmATTACH_FORM,
			      XmNalignment, XmALIGNMENT_BEGINNING,
			      XmNrecomputeSize, FALSE, (char *) NULL);

    w = XVCMW ("band_label", xmLabelGadgetClass, form,
	       XmNtopAttachment, XmATTACH_WIDGET,
	       XmNtopWidget, view->image_info,
	       XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    view->band_0 = XVCMW ("band_0", xmLabelWidgetClass, form,
			  XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
			  XmNtopOffset, 0,
			  XmNtopWidget, w,
			  XmNleftAttachment, XmATTACH_FORM,
			  XmNrightAttachment, XmATTACH_FORM,
			  XmNalignment, XmALIGNMENT_BEGINNING,
			  XmNmappedWhenManaged, FALSE, (char *) NULL);

    view->band_scale = XVCMW ("band_scale", xmScaleWidgetClass, form,
			      XmNtopAttachment, XmATTACH_WIDGET,
			      XmNtopWidget, view->image_info,
			      XmNleftAttachment, XmATTACH_FORM,
			      XmNmappedWhenManaged, FALSE,
			      XmNminimum, 0,
			      XmNorientation, XmHORIZONTAL,
			      XmNshowValue, TRUE, (char *) NULL);
    XtAddCallback (view->band_scale, XmNvalueChangedCallback,
		   SelectBandCB, (XtPointer) view);

    view->band_n = XVCMW ("band_n", xmLabelWidgetClass, form,
			  XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
			  XmNtopOffset, 0,
			  XmNtopWidget, w,
			  XmNleftAttachment, XmATTACH_WIDGET,
			  XmNleftWidget, view->band_scale,
			  XmNrightAttachment, XmATTACH_FORM,
			  XmNmappedWhenManaged, FALSE, (char *) NULL);

    XVCMW ("pixel_label", xmLabelGadgetClass, form,
	   XmNtopAttachment, XmATTACH_WIDGET,
	   XmNtopWidget, view->band_scale,
	   XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    str = XmStringCreateSimple (" ");
    view->pixel_info = XVCMW ("pixel_info", xmLabelGadgetClass, form,
			      XmNtopAttachment, XmATTACH_WIDGET,
			      XmNtopWidget, view->band_scale,
			      XmNleftAttachment, XmATTACH_FORM,
			      XmNrightAttachment, XmATTACH_FORM,
			      XmNalignment, XmALIGNMENT_BEGINNING,
			      XmNlabelString, str,
			      XmNrecomputeSize, FALSE, (char *) NULL);
    XmStringFree (str);

    view->edge_label = XVCMW ("edge_label", xmLabelWidgetClass, form,
			      XmNtopAttachment, XmATTACH_WIDGET,
			      XmNtopWidget, view->pixel_info,
			      XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    view->edge_info = XVCMW ("edge_info", xmLabelWidgetClass, form,
			     XmNtopAttachment, XmATTACH_WIDGET,
			     XmNtopWidget, view->pixel_info,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNalignment, XmALIGNMENT_BEGINNING,
			     XmNrecomputeSize, FALSE, (char *) NULL);

    view->point_label = XVCMW ("point_label", xmLabelWidgetClass, form,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, view->edge_info,
			       XmNleftAttachment, XmATTACH_FORM,
			       (char *) NULL);

    view->point_info = XVCMW ("point_info", xmLabelWidgetClass, form,
			      XmNtopAttachment, XmATTACH_WIDGET,
			      XmNtopWidget, view->edge_info,
			      XmNleftAttachment, XmATTACH_FORM,
			      XmNrightAttachment, XmATTACH_FORM,
			      XmNalignment, XmALIGNMENT_BEGINNING,
			      XmNrecomputeSize, FALSE, (char *) NULL);

    XtManageChild (form);
    XtManageChild (work);
    XtManageChild (view->menu_bar);
    XtManageChild (main_window);

    nviews++;

    return view;
}


/*
 *  DestroyView
 *
 *  Remove a view.
 *  Arguments are defined for use as a callback.
 */

/* ARGSUSED */
void DestroyView (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data, prev;

    /* Delete it from the list of views: */
    if (view == views)
	views = views->next;
    else {
	for (prev = views; prev && prev->next != view; prev = prev->next) ;
	if (! prev || prev->next != view)
	    VError ("Internal error in DestroyView");
	prev->next = view->next;
    }

    /* Close any file open in the view: */
    CloseFile (view);

    /* Destroy widgets used by the view: */
    XtDestroyWidget (view->view_shell);

    VFree (view);
    nviews--;
}


/*
 *  Menu item callbacks.
 *
 *	FileMenuCB	File menu item selected
 *	ViewMenuCB	View menu item selected
 *	HelpMenuCB	Help menu item selected
 */

/* ARGSUSED */
static void FileMenuCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = MapWidgetToView (w);
    VStringConst filename;
    VAttrList attributes;

    if (! view)
	return;

    switch ((int) cl_data) {

    case newButton:
	ShowFileSelectDialog (NULL);
	break;

    case removeButton:
	XtDestroyWidget (view->view_shell);
	break;

    case openButton:
	ShowFileSelectDialog (view);
	break;

    case rereadButton:
	if (ReadFile (view->filename, & attributes)) {
	    filename = VNewString (view->filename);
	    CloseFile (view);
	    view->filename = filename;
	    view->attributes = attributes;
	    ShowFile (view, -1);
	}
	break;

    case quitButton:
	exit (EXIT_SUCCESS);
    }
}

/* ARGSUSED */
static void ViewMenuCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = MapWidgetToView (w);

    if (! view)
	return;

    switch ((int) cl_data) {

    case attrsButton:
	ShowAttrDialog (view);
	break;

    case displayButton:
	ShowPrefDialog (view);
	break;

    case edgeDataButton:
	ShowEdgeData (view);
    }
}

/* ARGSUSED */
static void HelpMenuCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = MapWidgetToView (w);

    if (! view)
	return;

    switch ((int) cl_data) {

    case helpIndexButton:
	ShowHelpDialog (NULL, NULL, NULL);
	break;

    case helpVersionButton:
	ShowVersionDialog (view);
    }
}


/*
 *  SelectObjectCB
 *
 *  Called when an object list entry is selected.
 */

/* ARGSUSED */
static void SelectObjectCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data;
    int object = ((XmListCallbackStruct *) ca_data)->item_position - 1;
    ViewObject *obj = & view->objects[object];

    obj->visible = ! obj->visible;
    switch (obj->type) {

    case VEdgesRepn:

	/* If deselecting an edge set, and an edge in that set was selected,
	   deselect the edge also: */
	if (! obj->visible && view->edge_selected &&
	    view->sel_edges_idx == object)
	    SelectEdge (view, FALSE, 0, 0, NULL, 0);
	UpdateView (view, edgeSetChange);
	break;

    case VImageRepn:

	/* If selecting a new image deselect any previous one: */
	if (obj->visible && view->image_object) {
	    view->image_object->visible = FALSE;
	    XmListDeselectPos (w, view->image_object - view->objects + 1);
	}
	view->band = 0;
	UpdateView (view, imageChange);
	break;

    default:
	break;
    }
}


/*
 *  SelectBandCB
 *
 *  Called when an image band is chosen.
 */

/* ARGSUSED */
static void SelectBandCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data;
    XmScaleCallbackStruct *cb = (XmScaleCallbackStruct *) ca_data;

    /* Make the slider jump to an integer position: */
    XmScaleSetValue (w, cb->value);
    /* ==> Redraw the widget because XmScale seems to fail to redraw
       the value label at the new slider position. */
    (w->core.widget_class->core_class.expose) (w, NULL, NULL);

    if (cb->value == view->band)
	return;

    view->band = cb->value;
    XtVaSetValues (view->image_view, VxNband, view->band, (char *) NULL);
}


/*
 *  ImageExposeCB
 *
 *  Called when the image is redrawn.
 */

/* ARGSUSED */
static void ImageExposeCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data;
    ViewObject *obj;
    int i, j;
    Pixel color;
    VEdge e;

    /* First time through (after widgets have been realized) allocate a
       graphics context for drawing edges over the image: */
    if (! view->gc)
	view->gc = XCreateGC (myDisplay, XtWindow (view->image_view), 0, NULL);

    /* Draw each edge set chosen for display: */
    for (i = j = 0, obj = view->objects; i < view->nobjects; i++, obj++)
	if (obj->visible && obj->type == VEdgesRepn) {

	    /* Choose a color for the edge set: */
	    color = appData.edge_set_colors[j++ % numberEdgeSetColors];
	    XSetForeground (myDisplay, view->gc, color);

	    /* Draw each edge: */
	    for (e = VFirstEdge (obj->edges);
		 VEdgeExists (e); e = VNextEdge (e))
		DrawEdge (view, e);
	}

    /* If an edge is selected, redraw it in the hilite color: */
    if (view->edge_selected) {
	XSetForeground (myDisplay, view->gc, appData.hilite_color);
	if (view->prefs.edge.lines)
	    DrawEdge (view, view->sel_edge);
	else {
	    VFloat *ept = VEdgePointArray (view->sel_edge) [view->sel_pt_idx];
	    XPoint xpt;

	    EdgeToWindowCoords (view, ept, & xpt);
	    if (view->prefs.edge.endpoints &&
		(view->sel_pt_idx == 0 ||
		 view->sel_pt_idx == VEdgeNPoints (view->sel_edge) - 1))
		XDrawRectangle (myDisplay, XtWindow (view->image_view),
				view->gc, xpt.x - 1, xpt.y - 1, 2, 2);
	    else XDrawPoint (myDisplay, XtWindow (view->image_view),
			     view->gc, xpt.x, xpt.y);
	}
    }
}


/*
 *  DrawEdge
 *
 *  Draw an edge overtop the image using the current graphic context.
 */

static void DrawEdge (View view, VEdge e)
{
    int i;
    static int npoints = 0;
    static XPoint *points = NULL;
    static XRectangle rectangles[2] = { { 0, 0, 2, 2, },
					{ 0, 0, 2, 2, } };

    /* Ensure that the points vector is large enough to hold all of the
       edges points: */
    if (npoints < VEdgeNPoints (e)) {

	VFree (points);
	npoints = VMax (VEdgeNPoints (e) + 100, 1000);
	points = VMalloc (sizeof (XPoint) * npoints);
    }

    /* Convert each edge point to window coordinates: */
    for (i = 0; i < VEdgeNPoints (e); i++)
	EdgeToWindowCoords (view, VEdgePointArray (e)[i], & points[i]);

    /* Draw the edge: */
    if (view->prefs.edge.points)
	XDrawPoints (myDisplay, XtWindow (view->image_view), view->gc,
		     points, VEdgeNPoints (e), CoordModeOrigin);
    if (view->prefs.edge.lines)
	XDrawLines (myDisplay, XtWindow (view->image_view), view->gc,
		    points, VEdgeNPoints (e), CoordModeOrigin);
    if (view->prefs.edge.endpoints) {
	rectangles[0].x = points[0].x - 1;
	rectangles[0].y = points[0].y - 1;
	rectangles[1].x = points[VEdgeNPoints (e) - 1].x - 1;
	rectangles[1].y = points[VEdgeNPoints (e) - 1].y - 1;
	XDrawRectangles (myDisplay, XtWindow (view->image_view),
			 view->gc, rectangles, 2);
    }
}

static void EdgeToWindowCoords (View view, VFloat ept[2], XPoint *xpt)
{
    int nrows = VImageNRows (view->tmp_image ? view->tmp_image : view->image);
    int ipt[2];

    VImageViewImageToWindow (view->image_view, nrows - ept[1], ept[0],
			     & ipt[0], & ipt[1]);
    xpt->x = ipt[0];
    xpt->y = ipt[1];
}


/*
 *  LocateEdge
 *
 *  Select an edge in response to a button click at (row,column) on the image.
 */

static void LocateEdge (View view, double row, double column)
{
    double width, height, x, y, t;
    int edges_idx, edge_idx, pt_idx;
    enum { first_search, resume_search, cont_search } state;
    VBoolean lap = FALSE;
    VEdge edge;
    VFloat *pt;

    x = column;
    y = VImageNRows (view->tmp_image ? view->tmp_image : view->image) - row;

    /* Compute a range threshold, in image pixels, based on the
       current zoom level: */
    VImageViewPixelSize (view->image_view, & width, & height);
    t = VMax (appData.select_range / width,
	      appData.select_range / height);
    t = VMax (t, 1.0);

    /* If the new click was within range of the last one, resume the
       search for an edge point with the last edge point selected, otherwise
       start from the first point, edge, and edge set: */
    state = (view->edge_selected && row >= view->sel_row - t &&
	     row <= view->sel_row + t && column >= view->sel_column - t  &&
	     column <= view->sel_column + t) ? resume_search : first_search;
    while (1) {
	for (edges_idx = 0; edges_idx < view->nobjects; edges_idx++) {
	    if (view->objects[edges_idx].type != VEdgesRepn ||
		! view->objects[edges_idx].visible)
		continue;
	    for (edge = VFirstEdge (view->objects[edges_idx].edges),
		 edge_idx = 0; VEdgeExists (edge);
		 edge = VNextEdge (edge), edge_idx++) {
		for (pt_idx = 0; pt_idx < VEdgeNPoints (edge); pt_idx++) {
		    if (state == resume_search || state == cont_search)
			lap = (edges_idx == view->sel_edges_idx &&
			       edge_idx == view->sel_edge_idx &&
			       pt_idx == view->sel_pt_idx);
		    if (state == resume_search) {
			if (lap)
			    state = cont_search;
		    } else {
			pt = VEdgePointArray (edge)[pt_idx];
			if (pt[0] >= x - t && pt[0] <= x + t &&
			    pt[1] >= y - t && pt[1] <= y + t) {
			    view->sel_row = row;
			    view->sel_column = column;
			    SelectEdge (view, TRUE, edges_idx, edge_idx,
					edge, pt_idx);
			    return;
			}
			if (lap && state == cont_search) {
			    SelectEdge (view, FALSE, 0, 0, NULL, 0);
			    return;
			}
		    }
		}
	    }
	}
	if (state == first_search) {
	    SelectEdge (view, FALSE, 0, 0, NULL, 0);
	    return;
	}
    }
}


/*
 *  Action procedures.
 *
 *  These procedures are tied, via a translation table, to certain events
 *  input via the VImageView widget.
 *
 *	ImageMoveAction		moves the pointer up, left, down, or right
 *				under keyboard control
 *
 *	ImageReportAction	updates report about the pointed-to pixel
 *
 *	ImageSelectAction	(un)selects an edge near the pointer
 */

/* ARGSUSED */
void ImageMoveAction (Widget w, XEvent *event,
		      String *params, Cardinal *nparams)
{
    View view = MapWidgetToView (w);
    int row, column;
    double width, height;

    if (*nparams != 1) {
	VWarning ("ImageMove() action must have exactly one parameter");
	return;
    }

    if (VImageViewPixelSize (view->image_view, & width, & height)) {
	row = (int) VMax (height, 1.0);
	column = (int) VMax (width, 1.0);
    } else row = column = 1;

    if (strcmp (params[0], "left") == 0)
	XWarpPointer (myDisplay, XtWindow (view->image_view), None,
		      0, 0, 0, 0, -column, 0);
    else if (strcmp (params[0], "up") == 0)
	XWarpPointer (myDisplay, XtWindow (view->image_view), None,
		      0, 0, 0, 0, 0, -row);
    else if (strcmp (params[0], "right") == 0)
	XWarpPointer (myDisplay, XtWindow (view->image_view), None,
		      0, 0, 0, 0, column, 0);
    else if (strcmp (params[0], "down") == 0)
	XWarpPointer (myDisplay, XtWindow (view->image_view), None,
		      0, 0, 0, 0, 0, row);
    else VWarning ("ImageMove() action's parameter must be up, left, "
		   "down, or right");
}

/* ARGSUSED */
void ImageReportAction (Widget w, XEvent *event,
			String *params, Cardinal *nparams)
{
    View view = MapWidgetToView (w);
    XmString str;
    char *text;

    XtVaGetValues (view->pixel_info, XmNlabelString, & str, (char *) NULL);
    XmStringGetLtoR (str, XmSTRING_DEFAULT_CHARSET, & text);
    XmStringFree (str);
    printf ("%s\n", text);
    XtFree ((XtPointer) text);
}

/* ARGSUSED */
void ImageSelectAction (Widget w, XEvent *event,
			String *params, Cardinal *nparams)
{
    View view = MapWidgetToView (w);
    int win_x, win_y, root_x, root_y;
    double row, column;
    Window root, child;
    unsigned int keys_buttons;

    switch (event->type) {

    case KeyPress:
    case KeyRelease:
	win_x = event->xkey.x;
	win_y = event->xkey.y;
	break;

    case ButtonPress:
    case ButtonRelease:
	win_x = event->xbutton.x;
	win_y = event->xbutton.y;
	break;

    default:
	if (! XQueryPointer (myDisplay, XtWindow (view->image_view),
			     & root, & child, & root_x, & root_y,
			     & win_x, & win_y, & keys_buttons))
	    return;
    }
    if (VImageViewWindowToImage (view->image_view,
				 win_x, win_y, & row, & column)) {
	LocateEdge (view, row, column);
	UpdateView (view, edgeSelChange);
    }
}

/* ARGSUSED */
void ImageTrackAction (Widget w, XEvent *event,
		       String *params, Cardinal *nparams)
{
    View view = MapWidgetToView (w);
    int root_x, root_y, win_x, win_y;
    Window root, child;
    unsigned int keys_buttons;

    switch (event->type) {

    case LeaveNotify:
	win_x = win_y = -1;
	break;

    case MotionNotify:
	win_x = event->xmotion.x;
	win_y = event->xmotion.y;
	break;

    default:
	if (! XQueryPointer (myDisplay, XtWindow (view->image_view),
			     & root, & child, & root_x, & root_y,
			     & win_x, & win_y, & keys_buttons))
	    win_x = win_y = -1;
    }
    UpdatePixelReport (view, win_x, win_y);
}


/*
 *  UpdateView
 *
 *  Update the depiction of an image and/or some edge sets as a result of
 *  some user-directed change. The nature of the change is indicated by the
 *  changes parameter:
 *
 *	imageChange	different image selected for viewing
 *	edgeSetChange	different edge sets selected for viewing
 *	ignoreBIChange	change in whether band_interp ignored
 *	intensityChange	change in absolute/signed intensity interpretation
 *	contrastChange	change in brightness or contrast adjustment
 *	edgeChange	change in edge drawing options
 *	edgeSelChange	different edge selected
 */

void UpdateView (View view, int changes)
{
    int nbands, nrows, ncolumns, i, n;
    VRepnKind repn;
    Arg args[20];
    XmString str;
    char buf[100];

    if (changes & (imageChange | ignoreBIChange)) {

	/* Note the image object now chosen for display: */
	for (i = 0; i < view->nobjects; i++)
	    if (view->objects[i].type == VImageRepn &&
		view->objects[i].visible)
		break;
	if (i < view->nobjects) {
	    view->image_object = & view->objects[i];
	    view->image = view->image_object->image;
	    view->act_color =
		(VImageNBands (view->image) == 3 &&
		 VImageColorInterp (view->image) == VBandInterpRGB);
	    view->show_color =
		view->act_color && ! view->prefs.image.ignore_BI;
	} else {
	    view->image_object = NULL;
	    view->image = NULL;
	    view->act_color = view->show_color = FALSE;
	}

	/* Format a string describing the image: */
	if (view->image)
	    sprintf (buf, "%d rows, %d cols, %s",
		     VImageNRows (view->image), VImageNColumns (view->image),
		     VPixelRepnName (view->image));
	else buf[0] = 0;
	str = XmStringCreateSimple (buf);
	XtVaSetValues (view->image_info, XmNlabelString, str, (char *) NULL);
	XmStringFree (str);

	/* Tailor the band selection widgets for this image: */
	if (! view->image || VImageNBands (view->image) == 1 ||
	    view->show_color) {
	    str = XmStringCreateSimple (view->show_color ?
					"(3-Band Color)" :
					"(Single Band)");
	    XtVaSetValues (view->band_0, XmNlabelString, str, (char *) NULL);
	    XmStringFree (str);
	    XtSetMappedWhenManaged (view->band_0, TRUE);
	    XtSetMappedWhenManaged (view->band_n, FALSE);
	    XtSetMappedWhenManaged (view->band_scale, FALSE);
	} else {
	    XtVaSetValues (view->band_scale,
			   XmNmaximum, VImageNBands (view->image) - 1,
			   XmNvalue, view->band, (char *) NULL);
	    sprintf (buf, "of [0,%d]", VImageNBands (view->image) - 1);
	    str = XmStringCreateSimple (buf);
	    XtVaSetValues (view->band_n, XmNlabelString, str, (char *) NULL);
	    XmStringFree (str);
	    XtSetMappedWhenManaged (view->band_0, FALSE);
	    XtSetMappedWhenManaged (view->band_n, TRUE);
	    XtSetMappedWhenManaged (view->band_scale, TRUE);
	}
    }

    if ((changes & (imageChange | ignoreBIChange)) ||
	((changes & edgeSetChange) && ! view->image)) {

	/* Determine new dimensions for the image display area: */
	if (view->image) {
	    nbands = VImageNBands (view->image);
	    nrows = VImageNRows (view->image);
	    ncolumns = VImageNColumns (view->image);
	    repn = VPixelRepn (view->image);
	} else {

	    /* If no image is being displayed, use the dimensions of the first
	       edge set that is being displayed: */
	    nbands = nrows = ncolumns = 1;
	    repn = VBitRepn;
	    for (i = 0; i < view->nobjects; i++)
		if (view->objects[i].type == VEdgesRepn &&
		    view->objects[i].visible) {
		    nrows = VEdgesNRows (view->objects[i].edges);
		    ncolumns = VEdgesNColumns (view->objects[i].edges);
		    break;
		}
	}

	/* Discard any temporary image if it doesn't have the same
	   properties as the image to be displayed: */
	if (view->tmp_image &&
	    (VImageNBands (view->tmp_image) != nbands ||
	     VImageNRows (view->tmp_image) != nrows ||
	     VImageNColumns (view->tmp_image) != ncolumns ||
	     VPixelRepn (view->tmp_image) != repn)) {
	    VDestroyImage (view->tmp_image);
	    view->tmp_image = NULL;
	    changes |= imageChange;
	}

	/* Create a blank background image if necessary: */
	if (! view->image) {
	    if (! view->tmp_image)
		view->tmp_image = VCreateImage (nbands, nrows, ncolumns, repn);
	    VFillImage (view->tmp_image, VAllBands, 0.0);
	}
    }

    /* Perform any required contrast adjustment or stripping of the band
       interpretation attribute (when "Ignore Band Interpretation" mode is
       selected): */
    if (view->image &&
	(changes & (imageChange | ignoreBIChange | contrastChange))) {
	VImage tmp_image = view->tmp_image;

	view->tmp_image = NULL;

	/* Create a contrast-adjusted version of the image: */
	if (view->prefs.image.brightness != 0 ||
	    view->prefs.image.contrast != 0)
	    view->tmp_image =
		VAdjustImage (view->image, tmp_image, VAllBands,
			      (double) view->prefs.image.brightness,
			      (double) view->prefs.image.contrast);

	/* If the image has an RGB band interpretation, but that attribute
	   is to be ignored, then ensure we're displaying a copy of the image
	   with ncolors set to 1: */
	if (view->act_color && view->prefs.image.ignore_BI &&
	    ! view->tmp_image) {
	    view->tmp_image = VCopyImage (view->image, tmp_image, VAllBands);
	    VExtractAttr (VImageAttrList (view->tmp_image),
			  VColorInterpAttr, NULL, VLongRepn, NULL, FALSE);
	}

	/* Discard any temporary image we didn't end up needing: */
	if (tmp_image && tmp_image != view->tmp_image) {
	    VDestroyImage (tmp_image);
	    view->tmp_image = NULL;
	}
    }

    /* Pass new resource values to the image display widget: */
    n = 0;
    if (changes & (imageChange | ignoreBIChange | contrastChange)) {
	XtSetArg (args[n], VxNimage,
		  view->tmp_image ? view->tmp_image : view->image); n++;
	XtSetArg (args[n], VxNband, view->band); n++;
	XtSetArg (args[n], VxNabsolute,
		  view->prefs.image.intensity == IntensityAbsolute); n++;
    } else if (changes & intensityChange) {
	XtSetArg (args[n], VxNabsolute,
		  view->prefs.image.intensity == IntensityAbsolute); n++;
    }
    if (n > 0)
	XtSetValues (view->image_view, args, (unsigned int) n);
    else VImageViewRedraw (view->image_view);

    /* Update the report of pointer location and pixel value if the
       underlying image may have changed: */
    if ((changes & (imageChange | ignoreBIChange)) &&
	XtIsRealized (view->view_shell)) {
	Window root, child;
	int root_x, root_y, win_x, win_y;
	unsigned int keys_buttons;

	if (! XQueryPointer (XtDisplay (view->image_view),
			     XtWindow (view->image_view),
			     & root, & child, & root_x, & root_y,
			     & win_x, & win_y, & keys_buttons))
	    win_x = win_y = -1;
	UpdatePixelReport (view, win_x, win_y);
    }
}


/*
 *  UpdatePixelReport
 *
 *  Update the display of the current pixel coordinates and value.
 */

static void UpdatePixelReport (View view, int x, int y)
{
    double row, column;
    int r, c;
    char buf[100];
    XmString str;

    if (VImageViewWindowToImage (view->image_view, x, y, & row, & column)) {
	r = row;
	c = column;
	if (view->image) {
	    if (view->show_color)
		sprintf (buf, "[%d,%d]:  %g %g %g", r, c,
			 VGetPixel (view->image, 0, r, c),
			 VGetPixel (view->image, 1, r, c),
			 VGetPixel (view->image, 2, r, c));
	    else
		sprintf (buf, "[%d,%d]:  %g", r, c,
			 VGetPixel (view->image, view->band, r, c));
	} else sprintf (buf, "[%d, %d]", r, c);
    } else buf[0] = 0;
    str = XmStringCreateSimple (buf);
    XtVaSetValues (view->pixel_info, XmNlabelString, str, (char *) NULL);
    XmStringFree (str);
}
