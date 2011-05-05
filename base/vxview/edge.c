/*
 *  $Id: edge.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *  These routines display information about edges.
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

/* From the local directory: */
#include "vxview.h"

/* From the Vista library: */
#include <viaio/VImageView.h>

/* From the Motif toolkit: */
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MwmUtil.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/Text.h>

/* File identification string: */
VRcsId ("$Id: edge.c 3629 2009-08-20 17:04:30Z proeger $");


/*
 *  SelectEdge
 *
 *  Select or deselect a particular edge point.
 */

void SelectEdge (View view, VBoolean select,
		 int edges_idx, int edge_idx, VEdge edge, int pt_idx)
{
    VFloat *pt;
    XmString str;
    char buf[100];

    if (view->edge_selected = select) {

	/* Record the edge selected: */
	view->sel_edges_idx = edges_idx;
	view->sel_edge_idx = edge_idx;
	view->sel_edge = edge;
	view->sel_pt_idx = pt_idx;

	/* Update the edge and point info strings: */
	sprintf (buf, "%d of %s", view->sel_edge_idx,
		 view->objects[view->sel_edges_idx].name);
	str = XmStringCreateSimple (buf);
	XtVaSetValues (view->edge_info, XmNlabelString, str, (char *) NULL);
	XmStringFree (str);
	pt = VEdgePointArray (view->sel_edge)[view->sel_pt_idx];
	sprintf (buf, "%d: [%g,%g]", view->sel_pt_idx, pt[0], pt[1]);
	str = XmStringCreateSimple (buf);
	XtVaSetValues (view->point_info, XmNlabelString, str, (char *) NULL);
	XmStringFree (str);

	/* Make them visible: */
	XtSetMappedWhenManaged (view->point_info, TRUE);
	XtSetMappedWhenManaged (view->edge_label, TRUE);
	XtSetMappedWhenManaged (view->edge_info, TRUE);
	XtSetMappedWhenManaged (view->point_label, TRUE);

    } else {

	/* Make the edge and point info strings invisible: */
	XtSetMappedWhenManaged (view->edge_label, FALSE);
	XtSetMappedWhenManaged (view->edge_info, FALSE);
	XtSetMappedWhenManaged (view->point_label, FALSE);
	XtSetMappedWhenManaged (view->point_info, FALSE);
    }

    /* Make the Edge Data menu item (in)sensitive: */
    SetMenuSensitivity (view->menus[viewMenu], edgeDataButton,
			view->edge_selected);
}


/*
 *  ShowEdgeData
 *
 *  Pop up a dialog showing more information about the selected edge.
 */

void ShowEdgeData (View view)
{
    Widget shell, pane, buttons, text, w;
    static Arg text_args[] = { { XmNcursorPositionVisible, FALSE },
			       { XmNeditable, FALSE },
			       { XmNeditMode, XmMULTI_LINE_EDIT },
			       { XmNtraversalOn, FALSE } };
    VEdges edges;
    int i;
    VFloat *pt0, *pt1;
    char *cp, buf[4000];

    /* Create the dialog: */
    shell = XVCPS ("edge", topLevelShellWidgetClass, topLevelShell,
		   XmNcolormap, VColormapColormap (vcolormap),
		   XmNdeleteResponse, XmDESTROY,
		   XmNiconPixmap, iconPixmap,
		   XmNmwmInputMode, MWM_INPUT_MODELESS,
		   XmNvisual, VColormapVisual (vcolormap),
		   (char *) NULL);

    pane = XVCW ("pane", xmPanedWindowWidgetClass, shell,
		 XmNsashWidth, 1,		/* to prevent moving sash */
		 XmNsashHeight, 1, (char *) NULL);
    XtAddCallback (pane, XmNhelpCallback, ShowHelpDialog, NULL);

    text = XmCreateScrolledText (pane, "text",
				 text_args, XtNumber (text_args));
    XtManageChild (text);

    buttons = XVCW ("buttons", xmFormWidgetClass, pane,
		    XmNfractionBase, 10, (char *) NULL);

    w = XVCMW ("close", xmPushButtonGadgetClass, buttons,
	       XmNtopAttachment, XmATTACH_FORM,
	       XmNbottomAttachment, XmATTACH_FORM,
	       XmNleftAttachment, XmATTACH_POSITION,
	       XmNleftPosition, 1,
	       XmNrightAttachment, XmATTACH_POSITION,
	       XmNrightPosition, 3,
	       XmNshowAsDefault, TRUE,
	       (char *) NULL);
    XtAddCallback (w, XmNactivateCallback,
		   DestroyCallback, (XtPointer) shell);
    XtVaSetValues (buttons, XmNcancelButton, w, (char *) NULL);
    XtVaSetValues (buttons, XmNdefaultButton, w, (char *) NULL);

    XtManageChild (buttons);
    FixButtonPane (w);
    XtManageChild (pane);

    /* Fill in information about the selected edge: */
    edges = view->objects[view->sel_edges_idx].edges;
    sprintf (buf,
	     "File:         %s\n"
	     "Edge Set:     %s\n\n"
	     "Edge:         %d of [0,%d]\n"
	     "Edge Fields: ",
	     view->filename, view->objects[view->sel_edges_idx].name,
	     view->sel_edge_idx, VNEdges (edges) - 1);
    cp = buf + strlen (buf);
    for (i = 0; i < VNEdgeFields (edges); i++) {
	sprintf (cp, " %g", (float) (VEdgeFields (view->sel_edge)[i]));
	cp += strlen (cp);
    }
    pt0 = VEdgePointArray (view->sel_edge)[0];
    pt1 = VEdgePointArray (view->sel_edge)[VEdgeNPoints (view->sel_edge) - 1];
    sprintf (cp, "\n"
	     "Edge Endpts:  [%g,%g] [%g,%g]\n\n"
	     "Point:        %d of [0,%d]\n"
	     "Point Fields:",
	     (float) pt0[0], (float) pt0[1], (float) pt1[0], (float) pt1[1],
	     view->sel_pt_idx, VEdgeNPoints (view->sel_edge) - 1);
    cp += strlen (cp);
    pt0 = VEdgePointArray (view->sel_edge)[view->sel_pt_idx];
    for (i = 0; i < VNPointFields (edges); i++) {
	sprintf (cp, " %g", (float) (pt0[i]));
	cp += strlen (cp);
    }
    sprintf (cp, "\n");
    XmTextSetString (text, buf);

    /* Make the dialog visible: */
    XtManageChild (shell);
}
