/*
 *  $Id: help.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *  This file implements help dialogs.
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
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/MwmUtil.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>

/* From the Vista library: */
#include <viaio/mu.h>

/* From the standard C library: */
#include <ctype.h>

/* File identification string: */
VRcsId ("$Id: help.c 3629 2009-08-20 17:04:30Z proeger $");

/* File containing help text: */
#ifndef helpFilename
#define helpFilename	"vxview.help"
#endif

/* List of topics and their blurbs: */
#define maxTopics	30
typedef struct {
    char *topic;			/* name of topic */
    char *text;				/* blurb on topic */
    int len;				/* length of blurb */
} Topic;
static int ntopics;
static Topic *topics;

/* Widgets used to display help info: */
static Widget helpDialog;		/* dialog */
static Widget helpTopics;		/* XmList of help topics */
static Widget helpText;			/* XmText displaying help text */

/* Later in this file: */
static void CreateHelpDialog (void);
static void LoadHelpFile (void);
static void HelpSelectCB (Widget, XtPointer, XtPointer);
static void HelpDestroyCB (Widget, XtPointer, XtPointer);


/*
 *  ShowVersionDialog
 *
 *  Handler for a Vista warning, which reports it with a pop-up dialog.
 */

void ShowVersionDialog (View view)
{
    Widget dialog;
    Arg al[] = { { XmNmessageString },
		 { XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL } };

    #define programVersion          \
        "vxview version " ## Version ## ", compiled " ## __DATE__ ## "\n\n" \
        "Laboratory for Computational Intelligence\n" \
	"University of British Columbia"

    /* Create a dialog displaying the message: */
    al[0].value = (XtArgVal) XmStringCreateLtoR ((char *) "bla",
						 XmSTRING_DEFAULT_CHARSET);
    dialog = XmCreateInformationDialog (view->view_shell, "version",
					al, XtNumber (al));
    XmStringFree ((XmString) al[0].value);
    XtVaSetValues (XtParent (dialog),
		   XmNdeleteResponse, XmDESTROY, (char *) NULL);
    XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_HELP_BUTTON));
    XtAddCallback (dialog, XmNokCallback,
		   (XtCallbackProc) XtDestroyWidget, NULL);
    XtManageChild (dialog);

    #undef programVersion
}


/*
 *  ShowHelpDialog
 *
 *  Pop-up dialog providing help information.
 *  Arguments are defined for use as a callback.
 */

void ShowHelpDialog (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    VStringConst topic = (VStringConst) cl_data;
    int i;

    /* Create the help dialog if it doesn't already exist: */
    if (! helpDialog) {
	CreateHelpDialog ();
	XmListSelectPos (helpTopics, 1, TRUE);
    }

    /* If a topic was specified, bring it into view: */
    if (topic) {
	for (i = 0; i < ntopics; i++)
	    if (strcmp (topic, topics[i].topic) == 0)
		break;
	if (i < ntopics)
	    XmListSelectPos (helpTopics, i + 1, TRUE);
    }

    /* Pop-up the help dialog: */
    XtManageChild (helpDialog);
    XRaiseWindow (myDisplay, XtWindow (helpDialog));
}


/*
 *  CreateHelpDialog
 *
 *  Create the help index and text dialogs for later use
 */

static void CreateHelpDialog (void)
{
    Widget pane, form, buttons, w;
    int i;
    XmString str;
    static Arg list_args[] = { { XmNlistSizePolicy, XmCONSTANT },
			       { XmNselectionPolicy, XmSINGLE_SELECT } };
    static Arg text_args[] = { { XmNcursorPositionVisible, FALSE },
			       { XmNeditable, FALSE },
			       { XmNeditMode, XmMULTI_LINE_EDIT } };

    /* Read text about various topics from the help file: */
    if (ntopics == 0)
	LoadHelpFile ();

    /* Create the help dialog: */
    helpDialog = XVCPS ("help", topLevelShellWidgetClass, topLevelShell,
			XmNcolormap, VColormapColormap (vcolormap),
			XmNiconPixmap, iconPixmap,
			XmNmwmInputMode, MWM_INPUT_MODELESS,
			XmNvisual, VColormapVisual (vcolormap),
			(char *) NULL);
    XtAddCallback (helpDialog, XmNdestroyCallback, HelpDestroyCB, NULL);

    /* Use a pane widget to separate the dialog into a top and bottom areas: */
    pane = XVCW ("pane", xmPanedWindowWidgetClass, helpDialog,
		 XmNsashWidth, 1,		/* to prevent moving sash */
		 XmNsashHeight, 1, (char *) NULL);

    /* Create a form containing the topic list and text at the top of
       the dialog: */
    form = XVCW ("form1", xmFormWidgetClass, pane, (char *) NULL);

    w = XVCMW ("topics_label", xmLabelGadgetClass, form,
	       XmNtopAttachment, XmATTACH_FORM,
	       XmNleftAttachment, XmATTACH_FORM, (char *) NULL);

    helpTopics = XmCreateScrolledList (form, "topic_list",
				       list_args, XtNumber (list_args));
    XtVaSetValues (XtParent (helpTopics),
		   XmNtopAttachment, XmATTACH_WIDGET,
		   XmNtopWidget, w,
		   XmNleftAttachment, XmATTACH_FORM,
		   XmNbottomAttachment, XmATTACH_FORM, (char *) NULL);
    XtAddCallback (helpTopics, XmNsingleSelectionCallback, HelpSelectCB, NULL);
    for (i = 0; i < ntopics; i++) {
	str = XmStringCreateSimple (topics[i].topic);
	XmListAddItemUnselected (helpTopics, str, 0);
    }

    XtManageChild (helpTopics);

    w = XVCMW ("text_label", xmLabelGadgetClass, form,
	       XmNtopAttachment, XmATTACH_FORM,
	       XmNleftAttachment, XmATTACH_WIDGET,
	       XmNleftWidget, helpTopics, (char *) NULL);

    helpText = XmCreateScrolledText (form, "text",
				     text_args, XtNumber (text_args));
    XtVaSetValues (XtParent (helpText),
		   XmNtopAttachment, XmATTACH_WIDGET,
		   XmNtopWidget, w,
		   XmNbottomAttachment, XmATTACH_FORM,
		   XmNleftAttachment, XmATTACH_WIDGET,
		   XmNleftWidget, helpTopics,
		   XmNrightAttachment, XmATTACH_FORM, (char *) NULL);
    XtManageChild (helpText);

    /* Create a form containing the Close button at the bottom of the
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
		   UnmanageCallback, (XtPointer) helpDialog);
    XtVaSetValues (form, XmNcancelButton, w, (char *) NULL);
    XtVaSetValues (form, XmNdefaultButton, w, (char *) NULL);
    XtVaSetValues (buttons, XmNcancelButton, w, (char *) NULL);
    XtVaSetValues (buttons, XmNdefaultButton, w, (char *) NULL);

    XtManageChild (buttons);
    FixButtonPane (w);
    XtManageChild (form);
    XtManageChild (pane);
}


/*
 *  LoadHelpFile
 *
 *  Load the file containing help topics and their blurbs.
 */

static void LoadHelpFile (void)
{
    FILE *f;
    size_t len, incr;
    Topic *topic;
    char buf[100];

    ntopics = 0;
    topics = topic = VMalloc (sizeof (Topic) * maxTopics);
    if (! (f = fopen (helpFilename, "r"))) {
	VWarning ("Unable to open help database %s", helpFilename);
	ntopics = 1;
	topic->topic = "(No help topics)";
	topic->text = "(No help text)";
	topic->len = strlen (topic->text);
	return;
    }

    do {
	fgets (buf, sizeof (buf), f);
    } while (! feof (f) && buf[0] != '@');

    while (! feof (f) && ntopics < maxTopics) {
	len = strlen (buf);
	if (buf[len - 1] == '\n')
	    buf[len - 1] = 0;
	topic->topic = VNewString (buf + 1);
	topic->text = NULL;
	len = 0;
	while (1) {
	    fgets (buf, sizeof (buf), f);
	    if (feof (f) || buf[0] == '@')
		break;
	    incr = strlen (buf);
	    topic->text = VRealloc ((XtPointer) topic->text, len + incr + 1);
	    strcpy (topic->text + len, buf);
	    len += incr;
	}
	while (len > 0 && isspace (topic->text[len - 1]))
	    len--;
	topic->text[len] = 0;
	topic->len = len;
	ntopics++;
	topic++;
    }

    fclose (f);
}


/*
 *  Help dialog callbacks.
 *
 *  HelpSelectCB	help topic selected
 *  HelpDestroyCB	help dialog destroyed via window manager
 */

/* ARGSUSED */
static void HelpSelectCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    XmListCallbackStruct *cb = (XmListCallbackStruct *) ca_data;
    Topic *topic = & topics[cb->item_position - 1];

    XmTextSetString (helpText, (char *) topic->text);
}

/* ARGSUSED */
static void HelpDestroyCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    helpDialog = NULL;
}
