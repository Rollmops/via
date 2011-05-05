/*
 *  $Id: attr.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *  This file implements the dialog for displaying attribute lists.
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
#include <viaio/file.h>

/* From the Motif toolkit: */
#include <Xm/Form.h>
#include <Xm/MwmUtil.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/Text.h>

/* File identification string: */
VRcsId ("$Id: attr.c 3629 2009-08-20 17:04:30Z proeger $");

/* Later in this file: */
static void CreateAttrDialog (View);
static void UpdateAttrDialog (View);
static void AttrDestroyCB (Widget, XtPointer, XtPointer);


/*
 *  ShowAttrDialog
 *
 *  Pop-up dialog for displaying a file's attributes.
 */

void ShowAttrDialog (View view)
{
    if (view->attr_shell) {
	UpdateAttrDialog (view);
	XtManageChild (view->attr_shell);
	XRaiseWindow (myDisplay, XtWindow (view->attr_shell));
    } else {
	CreateAttrDialog (view);
	UpdateAttrDialog (view);
	XtManageChild (view->attr_shell);
    }
}


/*
 *  CreateAttrDialog
 *
 *  Create a dialog for displaying file attributes.
 */

static void CreateAttrDialog (View view)
{
    Widget pane, buttons, w;
    static Arg text_args[] = { { XmNcursorPositionVisible, FALSE },
			       { XmNeditable, FALSE },
			       { XmNeditMode, XmMULTI_LINE_EDIT },
			       { XmNscrollHorizontal, TRUE },
			       { XmNscrollVertical, TRUE } };

    /* Create the dialog's main components: */
    view->attr_shell = XVCPS ("attr", topLevelShellWidgetClass,
			      view->view_shell,
			      XmNcolormap, VColormapColormap (vcolormap),
 			      XmNiconPixmap, iconPixmap,
			      XmNmwmInputMode, MWM_INPUT_MODELESS,
			      XmNvisual, VColormapVisual (vcolormap),
			      (char *) NULL);
    XtAddCallback (view->attr_shell, XmNdestroyCallback,
		   AttrDestroyCB, (XtPointer) view);

    /* Use a pane widget to separate the dialog into a top and bottom areas: */
    pane = XVCW ("pane", xmPanedWindowWidgetClass, view->attr_shell,
		 XmNsashWidth, 1,		/* to prevent moving sash */
		 XmNsashHeight, 1, (char *) NULL);
    XtAddCallback (pane, XmNhelpCallback, ShowHelpDialog, NULL);

    /* Create a scrolled text widget for displaying the file name and
       attributes: */
    view->attr_text =
	XmCreateScrolledText (pane, "text", text_args, XtNumber (text_args));
    XtManageChild (view->attr_text);

    /* Create a form containing the Close and Help buttons at the bottom of the
       dialog: */
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
		   UnmanageCallback, (XtPointer) view->attr_shell);
    XtVaSetValues (buttons, XmNcancelButton, w, (char *) NULL);
    XtVaSetValues (buttons, XmNdefaultButton, w, (char *) NULL);

    w = XVCMW ("help", xmPushButtonGadgetClass, buttons,
	       XmNtopAttachment, XmATTACH_FORM,
	       XmNbottomAttachment, XmATTACH_FORM,
	       XmNleftAttachment, XmATTACH_POSITION,
	       XmNleftPosition, 7,
	       XmNrightAttachment, XmATTACH_POSITION,
	       XmNrightPosition, 9, (char *) NULL);
    XtAddCallback (w, XmNactivateCallback, ShowHelpDialog, NULL);

    XtManageChild (buttons);
    FixButtonPane (w);
    XtManageChild (pane);
}


/*
 *  UpdateAttrDialog
 *
 *  Update an attributes dialog to reflect a particular view's file attributes.
 */

static void UpdateAttrDialog (View view)
{
    FILE *f;
    size_t len;
    char *cp;

    /* For now the attribute list is rendered as text by encoding
       it to a temporary file: */
    f = tmpfile ();
    if (! f) {
	VWarning ("Unable to open temporary file using tmpfile(3)");
	return;
    }
    fprintf (f, "File: %s\n", view->filename);
    VWriteFile (f, view->attributes);
    rewind (f);
    while (! (fgetc (f) == VFileDelimiter[0] &&
	      fgetc (f) == VFileDelimiter[1])) ;
    len = ftell (f);
    cp = VMalloc (len + 1);
    rewind (f);
    if (fread (cp, sizeof (char), len, f) != len)
	VWarning ("Internal error in UpdateAttrDialog");
    fclose (f);
    cp[len] = 0;

    /* Paste the text into the dialog: */
    XmTextSetString (view->attr_text, cp);
    VFree (cp);
}


/* ARGSUSED */
static void AttrDestroyCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data;

    view->attr_shell = NULL;
}
