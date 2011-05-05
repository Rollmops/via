/*
 *  $Id: VXLine.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains VX line drawing routines.
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
#include "viaio/VList.h"
#include "viaio/VX.h"
#include "viaio/VXPrivate.h"

/* From the standard C libaray: */
#include <math.h>

/* From X11R5 Xt and Motif: */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>

/* File identification string: */
VRcsId ("$Id: VXLine.c 3177 2008-04-01 14:47:24Z karstenm $");

/* Later in this file: */
static void DrawLineInMemory (float, float, float, float);
static void DrawLineOnScreen (float, float, float, float);
static void DrawLine (float, float, float, float);
static void DrawLinesOnScreen (VList);
static void FreeLine (VPointer);


/*
 * Record for storing line related data.
 */

typedef struct {
    Pixel color;		/* current line color. */
    float width;		/* current line width. */
    VList ns_lines;		/* list of lines not saved by
				   VXStoreOverlays. */
    VList s_lines;		/* list of saved lines. */
    VBoolean saved_consistent;	/* TRUE iff s_lines should be displayed */
    GC gc;			/* Graphic context ID. */
} LineRec;
static LineRec l_line;

/*
 * Record for storing data of a line segment.
 */

typedef struct {
    Pixel color;		/* color */
    float width;		/* width */
    float r1, c1, r2, c2;	/* two end-points */
} LineStatesRec, *LineStates;


/*
 * Extern: VX_InitLine
 *
 * Initialize the line module.
 */

extern VBoolean VX_InitLine (void)
{
    /* Allocate storage for line lists: */
    l_line.ns_lines = VListCreate();
    l_line.s_lines = VListCreate();

    l_line.saved_consistent = FALSE;

    /* Set default line color and width: */
    VXSetLineColor (VXIsColorDisplay () ? "yellow" : "white");
    VXSetLineWidth (0.0);

    return TRUE;
}


/*
 * Extern: VX_GetLineGC
 *
 * Get a GC for drawing lines.
 */

extern void VX_GetLineGC (void)
{
    XGCValues gcv;

    /* Create a graphic context for drawing lines: */
    l_line.gc = XCreateGC (XtDisplay(VX_App.x.imageView),
			   XtWindow(VX_App.x.imageView),
			   0,
			   &gcv);
}


/*
 * Extern: VX_RedrawLines
 *
 * Redraw lines on exposure.
 */

extern void VX_RedrawLines (void)
{
    if (l_line.saved_consistent && !VX_App.o.pixmap_consistent)
	DrawLinesOnScreen (l_line.s_lines);

    DrawLinesOnScreen (l_line.ns_lines);
}


/*
 * Extern: VX_StoreLines
 *
 * Save the states of existing lines.
 */

extern void VX_StoreLines (void)
{
    if (l_line.saved_consistent) {
	/* combine s_lines and ns_lines: */
	VListConcat (l_line.s_lines, l_line.ns_lines);
    } else {
	/* Free s_lines: */
	VListDestroy (l_line.s_lines, FreeLine);
	/* Remember currently visible lines: */
	l_line.s_lines = l_line.ns_lines;
	l_line.saved_consistent = TRUE;
    }

    /* Reset ns_lines to empty list: */
    l_line.ns_lines = VListCreate ();
}

/*
 * Extern: VX_RestoreLines
 *
 * Revert to previously stored state.
 */

extern void VX_RestoreLines (void)
{
    /* Free ns_lines: */
    VListDestroy (l_line.ns_lines, FreeLine);
    l_line.ns_lines = VListCreate ();

    /* Display saved lines: */
    l_line.saved_consistent = TRUE;
}


/*
 * Local: DrawLineInMemory
 *
 * "Draw" a line whose end-points are represented in floating point
 * row-column coordinate with upper-left-hand corner as origin.
 * The line is only stored internally for later redraw on exposure.
 */

static void DrawLineInMemory (float r1, float c1,
				  float r2, float c2)
{
    LineStates line;

    /* Add line to line list: */
    line = VMalloc (sizeof (LineStatesRec));
    line->color = l_line.color;
    line->width = l_line.width;
    line->r1 = r1;
    line->c1 = c1;
    line->r2 = r2;
    line->c2 = c2;
    VListAppend (l_line.ns_lines, (VPointer) line);
}


/*
 * Local: DrawLineOnScreen
 *
 * Draw a line whose end-points are represented in floating point
 * row-column coordinate with upper-left-hand corner as origin.
 */

static void DrawLineOnScreen (float r1, float c1, float r2, float c2)
{
    int win_x1, win_y1, win_x2, win_y2;

    /* Convert image coordinates to window coordinates: */
    VImageViewImageToWindow (VX_App.x.imageView, r1, c1, & win_x1, & win_y1);
    VImageViewImageToWindow (VX_App.x.imageView, r2, c2, & win_x2, & win_y2);

    XDrawLine (XtDisplay (VX_App.x.imageView), XtWindow (VX_App.x.imageView),
	       l_line.gc, win_x1, win_y1, win_x2, win_y2);
}


/*
 * Local: DrawLine
 *
 * DrawLineInMemory + DrawLineOnScreen
 */

static void DrawLine (float r1, float c1, float r2, float c2)
{
    DrawLineInMemory (r1, c1, r2, c2);
    if (XtIsRealized (VX_App.x.imageView)) {
	XSetForeground (XtDisplay(VX_App.x.imageView),
			l_line.gc,
			l_line.color);
	XSetLineAttributes (XtDisplay(VX_App.x.imageView),
			    l_line.gc,
			    (unsigned int) (VX_App.v.row_scale * l_line.width),
			    LineSolid,
			    CapButt,
			    JoinMiter);
	DrawLineOnScreen (r1, c1, r2, c2);
    }
}


/*
 * Local: DrawLinesOnScreen
 *
 * Draw all lines in a line list.
 */

static void DrawLinesOnScreen (VList line_list)
{
    LineStates line;
    Pixel prev_color = 0;
    float prev_width = 0;

    if (! XtIsRealized (VX_App.x.imageView))
	return;

    line = (LineStates) VListFirst (line_list);

    /* Just to make prev color and width different from current ones: */
    if (line) {
	prev_color = line->color + 1;
	prev_width = line->width + 1;
    }

    while (line) {
	/* If color has changed then reset foreground color: */
	if (line->color != prev_color) {
	    XSetForeground (XtDisplay(VX_App.x.imageView),
			    l_line.gc,
			    line->color);
	    prev_color = line->color;
	}
	/* If width has changed then reset width: */
	if (line->width != prev_width) {
	    XSetLineAttributes (XtDisplay(VX_App.x.imageView),
				l_line.gc,
				(unsigned int) (VX_App.v.row_scale * line->width),
				LineSolid,
				CapButt,
				JoinMiter);
	    prev_width = line->width;
	}

	/* Draw a line: */
	DrawLineOnScreen (line->r1, line->c1, line->r2, line->c2);
	line = (LineStates) VListNext (line_list);
    }
}


/*
 * Local: FreeLine
 *
 * Free an instance of LineStates
 */

static void FreeLine (VPointer data)
{
    VFree ((LineStates) data);
}


/*
 * VXSetLineColor
 *
 * Set line color.
 */

VBoolean VXSetLineColor (VStringConst color_name)
{
    XColor color;
    Display *display;
    int screen;
    Colormap cmap;

    if (! VX_App.initialized)
	VError ("VXSetLineColor: VX not initialized");

    display = XtDisplay (VX_App.x.imageView);
    screen = DefaultScreen (display);
    cmap = DefaultColormap (display, screen);

    if (! XParseColor (display, cmap, color_name, & color)) {
	VWarning ("VXSetLineColor: Color %s not recognized", color_name);
	return FALSE;
    }

    if (VX_App.x.vcolormap) {
	VColormapRGBPixel (VX_App.x.vcolormap, & color);
	l_line.color = color.pixel;
    } else if (XAllocColor (display, cmap, & color)) {
	l_line.color = color.pixel;
    } else {
	VWarning ("VXSetLineColor: Cannot allocate color %s", color_name);
	return FALSE;
    }

    return TRUE;
}


/*
 * VXSetLineWidth
 *
 * Set line width.
 */

void VXSetLineWidth (double width)
{
    if (! VX_App.initialized)
	VError ("VXSetLineWidth: VX not initialized");

    /* Store line width: */
    l_line.width = width;
}


/*
 * VXDrawLine
 *
 * Draw a line and add it to the line list.
 */

VBoolean VXDrawLine (double r1, double c1, double r2, double c2)
{
    if (! VX_App.initialized)
	VError ("VXDrawLine: VX not initialized");

    if (VX_App.v.image == NULL) {
	VWarning ("VXDrawLine: Cannot draw on NULL image");
	return FALSE;
    }

    DrawLine ((float) r1, (float) c1, (float) r2, (float) c2);
    return TRUE;
}


/*
 * VXDrawEdges
 *
 * Draw edges.
 * [Thanks to David Lowe (lowe@cs.ubc.ca)]
 */

VBoolean VXDrawEdges(VEdges edges)
{
    VEdge e;
    int j, np, nrows;
    VFloat **points;

    if (! VX_App.initialized)
	VError ("VXDrawEdges: VX not initialized");

    if (VX_App.v.image == NULL) {
	VWarning ("VXDrawEdges: Cannot draw on NULL image");
	return FALSE;
    }

    nrows = VEdgesNRows(edges);

    /* For each edge in the edges set: */
    for (e = VFirstEdge(edges); VEdgeExists(e); e = VNextEdge(e)) {
	np = VEdgeNPoints(e);
	points = VEdgePointArray(e);

	/* If there is exactly 1 point, then display it as a point. */
	if (VEdgeNPoints(e) == 1)
	    DrawLine ((float) nrows - points[0][1],
		      (float) points[0][0],
		      (float) nrows - points[0][1],
		      (float) points[0][0]);
	else {
	    for (j = 0; j < np - 1; j++) {
		DrawLine ((float) nrows - points[j][1],
			  (float) points[j][0],
			  (float) nrows - points[j+1][1],
			  (float) points[j+1][0]);
	    }
	    /* If this is a closed edge, then join first point to last. */
	    if (e->closed)
		DrawLine ((float) nrows - points[0][1],
			  (float) points[0][0],
			  (float) nrows - points[np - 1][1],
			  (float) points[np - 1][0]);
	}
    }

    return TRUE;
}


/*
 * VXClearLines
 *
 * Clear line from screen and line list.
 */

void VXClearLines(void)
{
    if (! VX_App.initialized)
	VError ("VXClearLines: VX not initialized");

    /* Free ns_lines: */
    VListDestroy (l_line.ns_lines, FreeLine);
    l_line.ns_lines = VListCreate ();

    /* Saved lines should no longer be displayed: */
    l_line.saved_consistent = FALSE;

    /* Overlays pixmap is no longer consistent: */
    VX_App.o.pixmap_consistent = FALSE;

    /* Redraw image and overlays: */
    if (XtIsRealized (VX_App.x.imageView))
	VImageViewRedraw (VX_App.x.imageView);
}
