/*
 *  $Id: pref.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *  This file implements the dialog for establishing display preferences.
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

/* From the Motif toolkit: */
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/MwmUtil.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/Scale.h>
#include <Xm/ToggleBG.h>

/* From the standard C library: */
#include <math.h>

/* File identification string: */
VRcsId ("$Id: pref.c 3629 2009-08-20 17:04:30Z proeger $");

/* Default display preferences: */
Preferences defaultPrefs = {
    { 0.0, 0.0, FALSE, IntensityAbsolute },
    { FALSE, TRUE, FALSE }
};


/* Later in this file: */
static void CreatePrefDialog (View);
static void UpdatePrefDialog (View);
static void PrefResetCB (Widget, XtPointer, XtPointer);
static void PrefUpdateCB (Widget, XtPointer, XtPointer);
static void PrefIntensityCB (Widget, XtPointer, XtPointer);
static void PrefDestroyCB (Widget, XtPointer, XtPointer);
static float ScaleGetFloatValue (Widget);
static void ScaleSetFloatValue (Widget, float);

/*
 *  ShowPrefDialog
 *
 *  Pop-up dialog for setting display preferences.
 */

void ShowPrefDialog (View view)
{
    if (view->pref_shell) {
	UpdatePrefDialog (view);
 	XtManageChild (view->pref_shell);
	XRaiseWindow (myDisplay, XtWindow (view->pref_shell));
    } else {
	CreatePrefDialog (view);
	UpdatePrefDialog (view);
	XtManageChild (view->pref_shell);
    }
}


/*
 *  CreatePrefDialog
 *
 *  Create dialog for setting display preferences.
 */

static void CreatePrefDialog (View view)
{
    Widget pane, form, image_label, image_box, edge_label, edge_box;
    Widget buttons, w;

    /* Create the dialog's main components: */
    view->pref_shell = XVCPS ("pref", topLevelShellWidgetClass,
			      view->view_shell,
			      XmNcolormap, VColormapColormap (vcolormap),
 			      XmNiconPixmap, iconPixmap,
			      XmNmwmDecorations, MWM_DECOR_BORDER |
			          MWM_DECOR_TITLE | MWM_DECOR_MENU |
			          MWM_DECOR_MINIMIZE,
			      XmNmwmFunctions, MWM_FUNC_MOVE |
			          MWM_FUNC_MINIMIZE | MWM_FUNC_CLOSE,
			      XmNmwmInputMode, MWM_INPUT_MODELESS,
			      XmNvisual, VColormapVisual (vcolormap),
			      (char *) NULL);
    XtAddCallback (view->pref_shell, XmNdestroyCallback,
		   PrefDestroyCB, (XtPointer) view);

    /* Use a pane widget to separate the dialog into a top and bottom areas: */
    pane = XVCW ("pane", xmPanedWindowWidgetClass, view->pref_shell,
		 XmNsashWidth, 1,		/* to prevent moving sash */
		 XmNsashHeight, 1, (char *) NULL);
    XtAddCallback (pane, XmNhelpCallback, ShowHelpDialog,
		   "Display Preferences");

    /* Create a form containing the controls at the top of the dialog: */
    form = XVCW ("form1", xmFormWidgetClass, pane, (char *) NULL);

    image_label = XVCMW ("image_label", xmLabelGadgetClass, form,
			 XmNtopAttachment, XmATTACH_FORM,
			 XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    image_box = XVCW ("image_box", xmFormWidgetClass, form,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopOffset, 0,
		      XmNtopWidget, image_label,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM, (char *) NULL);

    edge_label = XVCMW ("edge_label", xmLabelGadgetClass, form,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, image_box,
			XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    edge_box = XVCW ("edge_box", xmFormWidgetClass, form,
		     XmNtopAttachment, XmATTACH_WIDGET,
		     XmNtopOffset, 0,
		     XmNtopWidget, edge_label,
		     XmNleftAttachment, XmATTACH_FORM,
		     XmNrightAttachment, XmATTACH_FORM, (char *) NULL);

    /* Create image display preference controls: */
    XVCMW ("brightness_label", xmLabelGadgetClass, image_box,
	  XmNtopAttachment, XmATTACH_FORM,
	  XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    view->brightness_scale =
	XVCMW ("brightness_scale", xmScaleWidgetClass, image_box,
	       XmNtopAttachment, XmATTACH_FORM,
	       XmNleftAttachment, XmATTACH_FORM,
	       XmNrightAttachment, XmATTACH_FORM,
	       XmNorientation, XmHORIZONTAL,
	       XmNshowValue, TRUE, (char *) NULL);
    XtAddCallback (view->brightness_scale, XmNvalueChangedCallback,
		   PrefUpdateCB, (XtPointer) view);

    XVCMW ("contrast_label", xmLabelGadgetClass, image_box,
	   XmNtopAttachment, XmATTACH_WIDGET,
	   XmNtopWidget, view->brightness_scale,
	   XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    view->contrast_scale =
	XVCMW ("contrast_scale", xmScaleWidgetClass, image_box,
	       XmNtopAttachment, XmATTACH_WIDGET,
	       XmNtopWidget, view->brightness_scale,
	       XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
	       XmNleftWidget, view->brightness_scale,
	       XmNrightAttachment, XmATTACH_FORM,
	       XmNorientation, XmHORIZONTAL,
	       XmNshowValue, TRUE, (char *) NULL);
    XtAddCallback (view->contrast_scale, XmNvalueChangedCallback,
		   PrefUpdateCB, (XtPointer) view);

    XVCMW ("options_label", xmLabelGadgetClass, image_box,
	   XmNtopAttachment, XmATTACH_WIDGET,
	   XmNtopWidget, view->contrast_scale,
	   XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    view->bi_toggle = XVCMW ("ignore_bi", xmToggleButtonGadgetClass, image_box,
			     XmNtopAttachment, XmATTACH_WIDGET,
			     XmNtopWidget, view->contrast_scale,
			     XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
			     XmNleftWidget, view->contrast_scale,
			     XmNnavigationType, XmTAB_GROUP, (char *) NULL);
    XtAddCallback (view->bi_toggle, XmNvalueChangedCallback,
		   PrefUpdateCB, (XtPointer) view);

    view->intensity_menu =
	XmVaCreateSimpleOptionMenu (image_box, "intensity_menu",
				    NULL, 0, 0, PrefIntensityCB,
		/* Absolute */	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,
		/* Signed */	    XmVaPUSHBUTTON, NULL, 0, NULL, NULL,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget, view->bi_toggle,
				    XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
				    XmNleftWidget, view->bi_toggle,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNnavigationType, XmTAB_GROUP,
				    (char *) NULL);
    XtManageChild (view->intensity_menu);

    XtManageChild (image_box);

    /* Add edge display preference controls: */
    XVCMW ("show_label", xmLabelGadgetClass, edge_box,
	   XmNtopAttachment, XmATTACH_FORM,
	   XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    view->points_toggle = XVCMW ("points", xmToggleButtonGadgetClass, edge_box,
				 XmNtopAttachment, XmATTACH_FORM,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNnavigationType, XmTAB_GROUP,
				 (char *) NULL);
    XtAddCallback (view->points_toggle, XmNvalueChangedCallback,
		   PrefUpdateCB, (XtPointer) view);

    view->lines_toggle = XVCMW ("lines", xmToggleButtonGadgetClass, edge_box,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, view->points_toggle,
				XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
				XmNleftWidget, view->points_toggle,
				XmNnavigationType, XmTAB_GROUP,
				(char *) NULL);
    XtAddCallback (view->lines_toggle, XmNvalueChangedCallback,
		   PrefUpdateCB, (XtPointer) view);

    view->endpoints_toggle =
	XVCMW ("endpoints", xmToggleButtonGadgetClass, edge_box,
	       XmNtopAttachment, XmATTACH_WIDGET,
	       XmNtopWidget, view->lines_toggle,
	       XmNbottomAttachment, XmATTACH_FORM,
	       XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
	       XmNleftWidget, view->lines_toggle,
	       XmNnavigationType, XmTAB_GROUP, (char *) NULL);
    XtAddCallback (view->endpoints_toggle, XmNvalueChangedCallback,
		   PrefUpdateCB, (XtPointer) view);

    XtManageChild (edge_box);

    /* Create a form containing the Close, Reset, and Help buttons at
       the bottom of the dialog: */
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
		   UnmanageCallback, (XtPointer) view->pref_shell);
    XtVaSetValues (form, XmNcancelButton, w, (char *) NULL);
    XtVaSetValues (form, XmNdefaultButton, w, (char *) NULL);
    XtVaSetValues (buttons, XmNcancelButton, w, (char *) NULL);
    XtVaSetValues (buttons, XmNdefaultButton, w, (char *) NULL);

    w = XVCMW ("reset", xmPushButtonGadgetClass, buttons,
	       XmNtopAttachment, XmATTACH_FORM,
	       XmNbottomAttachment, XmATTACH_FORM,
	       XmNleftAttachment, XmATTACH_POSITION,
	       XmNleftPosition, 4,
	       XmNrightAttachment, XmATTACH_POSITION,
	       XmNrightPosition, 6, (char *) NULL);
    XtAddCallback (w, XmNactivateCallback, PrefResetCB, (XtPointer) view);

    w = XVCMW ("help", xmPushButtonGadgetClass, buttons,
	       XmNtopAttachment, XmATTACH_FORM,
	       XmNbottomAttachment, XmATTACH_FORM,
	       XmNleftAttachment, XmATTACH_POSITION,
	       XmNleftPosition, 7,
	       XmNrightAttachment, XmATTACH_POSITION,
	       XmNrightPosition, 9, (char *) NULL);
    XtAddCallback (w, XmNactivateCallback, ShowHelpDialog,
		   "Display Preferences");

    XtManageChild (buttons);
    FixButtonPane (w);
    XtManageChild (form);
    XtManageChild (pane);
}


/*
 *  UpdatePrefDialog
 *
 *  Update a preferences dialog to reflect the preferences associated with
 *  a particular view.
 */

static void UpdatePrefDialog (View view)
{
    VStringConst menu_item;
    Widget w;

    /* Load the dialog with the current preferences: */
    ScaleSetFloatValue (view->brightness_scale, view->prefs.image.brightness);
    ScaleSetFloatValue (view->contrast_scale, view->prefs.image.contrast);
    XmToggleButtonGadgetSetState (view->bi_toggle,
				  view->prefs.image.ignore_BI, FALSE);

    menu_item = (view->prefs.image.intensity == IntensityAbsolute ?
		 "button_0" : "button_1");
    XtVaGetValues (view->intensity_menu, XmNsubMenuId, & w, (char *) NULL);
    XtVaSetValues (view->intensity_menu,
		   XmNmenuHistory, XtNameToWidget (w, menu_item),
		   (char *) NULL);

    XmToggleButtonGadgetSetState (view->points_toggle,
				  view->prefs.edge.points, FALSE);
    XmToggleButtonGadgetSetState (view->lines_toggle,
				  view->prefs.edge.lines, FALSE);
    XmToggleButtonGadgetSetState (view->endpoints_toggle,
				  view->prefs.edge.endpoints, FALSE);
}


/*
 *  Callbacks tied to controls in the display preferences dialog.
 *
 *    PrefResetCB	Reset button
 *    PrefUpdateCB	any change in preferences
 *    PrefIntensityCB	selection from intensity option menu
 *    PrefDestroyCB	dialog closed by window manager
 */

/* ARGSUSED */
static void PrefResetCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data;

    view->prefs = defaultPrefs;
    UpdatePrefDialog (view);
    UpdateView (view, intensityChange | contrastChange | edgeChange);
}

/* ARGSUSED */
static void PrefUpdateCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data;
    Preferences p;
    int changes = 0;

    /* Fetch the values entered: */
    p.image.brightness = ScaleGetFloatValue (view->brightness_scale);
    p.image.contrast = ScaleGetFloatValue (view->contrast_scale);
    if (p.image.brightness != view->prefs.image.brightness ||
	p.image.contrast != view->prefs.image.contrast)
	changes |= contrastChange;
    p.image.ignore_BI = XmToggleButtonGadgetGetState (view->bi_toggle);
    if (p.image.ignore_BI != view->prefs.image.ignore_BI)
	changes |= ignoreBIChange;

    p.edge.points = XmToggleButtonGadgetGetState (view->points_toggle);
    p.edge.lines = XmToggleButtonGadgetGetState (view->lines_toggle);
    p.edge.endpoints = XmToggleButtonGadgetGetState (view->endpoints_toggle);
    if (p.edge.points != view->prefs.edge.points ||
	p.edge.lines != view->prefs.edge.lines ||
	p.edge.endpoints != view->prefs.edge.endpoints)
	changes |= edgeChange;

    /* Update the view if those values have changed: */
    if (changes) {
	p.image.intensity = view->prefs.image.intensity;
	view->prefs = p;
	UpdateView (view, changes);
    }
}

/* ARGSUSED */
static void PrefIntensityCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) MapWidgetToView (w);
    int item_no = (int) cl_data;

    if (view->prefs.image.intensity != item_no) {
	view->prefs.image.intensity = item_no;
	UpdateView (view, intensityChange);
    }
}

/* ARGSUSED */
static void PrefDestroyCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data;

    view->pref_shell = NULL;
}


/*
 *  ScaleGetFloatValue, ScaleSetFloatValue
 *
 *  Get and set a floating-point scale value.
 */

static float ScaleGetFloatValue (Widget w)
{
    short pts;
    int i;

    XtVaGetValues (w, XmNdecimalPoints, & pts, XmNvalue, & i, (char *) NULL);
    return i / pow (10.0, (double) pts);
}

static void ScaleSetFloatValue (Widget w, float f)
{
    short pts;

    XtVaGetValues (w, XmNdecimalPoints, & pts, (char *) NULL);
    XmScaleSetValue (w, (int) (f * pow (10.0, (double) pts)));
}
