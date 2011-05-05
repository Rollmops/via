/*
 * $Id: VXDialog.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 * This file contains VX dialog routines.
 *
 * [Thanks to Dan Heller (argv@sun.com)]
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
#include "viaio/os.h"
#include "viaio/VImage.h"
#include "viaio/mu.h"
#include "viaio/VX.h"
#include "viaio/VXPrivate.h"

/* From X11R5 Xt and Motif: */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
#include <X11/Xmu/Editres.h>
#endif
#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/FileSB.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <Xm/Text.h>

/* File identification string: */
VRcsId ("$Id: VXDialog.c 3177 2008-04-01 14:47:24Z karstenm $");


/* New Data type(s): */

/* Type: DialogRec
 *
 * Record for storing dialog related data.
 */
typedef struct {
    Widget fileBox;		/* file-selection dialog box */
    Widget warningBox;		/* warning dialog box */
} DialogRec;

/*
 * Type: VXInputResponse
 *
 * Record for storing user response to an input box.
 */
typedef struct {
    VBoolean responded;		/* TRUE iff user has responded */
    VString value;		/* user input */
} VXInputResponse;

/*
 * Type: VXYesNoResponse
 *
 * Record for storing user response to a yes-no box.
 */
typedef struct {
    VBoolean responded;		/* TRUE iff user has responded */
    VXAnswer value;		/* user response */
} VXYesNoResponse;

/*
 * Type: VXFileResponse
 *
 * Record for storing user response to a file box.
 */
typedef struct {
    VBoolean responded;		/* TRUE iff user has responded */
    VString value;		/* filename of the selected file */
} VXFileResponse;


/* Variable(s): */
static DialogRec l_dialog;


/* Function prototypes: */
static void MessageBoxCallback (Widget, XtPointer, XtPointer);
static void TextBoxCallback (Widget, XtPointer, XtPointer);
static void InputBoxCallback (Widget, XtPointer, XtPointer);
static void YesNoBoxCallback (Widget, XtPointer, XtPointer);
static void FileBoxCallback (Widget, XtPointer, XtPointer);
static void WarningBoxCallback (Widget, XtPointer, XtPointer);


/*
 * Extern: VX_InitDialog
 *
 * Initialize the dialog module.
 */

extern VBoolean VX_InitDialog (void)
{
    l_dialog.fileBox = NULL;
    return TRUE;
}


/*
 * Local: MessageBoxCallback
 *
 * Destroy the message box when the "Ok" button is pressed.
 */

static void MessageBoxCallback (Widget w, XtPointer client_data,
				XtPointer call_data)
{
    XtDestroyWidget (XtParent(w));
}


/*
 * Local: TextBoxCallback
 *
 * Destroy the text box when the "Close" button is pressed.
 */

static void TextBoxCallback (Widget w, XtPointer client_data,
			     XtPointer call_data)
{
    XtDestroyWidget (XtParent(XtParent(w)));
}


/*
 * Local: InputBoxCallback
 *
 * Store the user response when a button is pressed.
 */

static void InputBoxCallback (Widget w, XtPointer client_data,
			      XtPointer call_data)
{
    XmSelectionBoxCallbackStruct *callback_data;
    VXInputResponse *response;

    callback_data = (XmSelectionBoxCallbackStruct *) call_data;
    response = (VXInputResponse *) client_data;

    response->responded = TRUE;

    /* Check why this callback is invoked and act accordingly: */
    if (callback_data->reason == XmCR_OK) { /* "Ok" button is pressed. */

	/* Set the "value" field of "response" to the input string: */
	XmStringGetLtoR (callback_data->value, XmSTRING_DEFAULT_CHARSET,
			 (char **) &(response->value));

    } else { /* "Cancel" button is pressed. */

	/* Set the "value" field of "response" to "NULL"
	   to indicate "Cancel": */
	response->value = NULL;
    }
}


/*
 * Local: YesNoBoxCallback
 *
 * Store the user response when a button is pressed.
 */

static void YesNoBoxCallback (Widget w, XtPointer client_data,
			      XtPointer call_data)
{
    XmAnyCallbackStruct *callback_data;
    VXYesNoResponse *response;

    callback_data = (XmAnyCallbackStruct *) call_data;
    response = (VXYesNoResponse *) client_data;

    response->responded = TRUE;

    /* Check why this callback is invoked and act accordingly: */
    if (callback_data->reason == XmCR_OK)  {

	/* "Yes" button is pressed. */
	response->value = VXAyes;

    } else if (callback_data->reason == XmCR_CANCEL) {

	/* "No" button is pressed. */
	response->value = VXAno;

    } else {

	/* "Cancel" button is pressed: */
	response->value = VXAcancel;
    }
}

/*
 * Local: FileBoxCallback
 *
 * Store the user response when a button is pressed.
 */

static void FileBoxCallback (Widget w, XtPointer client_data,
			     XtPointer call_data)
{
    XmFileSelectionBoxCallbackStruct *callback_data;
    VXFileResponse *response;

    callback_data = (XmFileSelectionBoxCallbackStruct *) call_data;
    response = (VXFileResponse *) client_data;

    response->responded = TRUE;

    /* Check why this callback is invoked and act accordingly: */
    if (callback_data->reason == XmCR_OK) { /* "Ok" button is pressed. */

	/* Set the "value" field of "response" to the full path name
	   of the selected file: */
	XmStringGetLtoR (callback_data->value, XmSTRING_DEFAULT_CHARSET,
			 (char **) &(response->value));

    } else { /* "Cancel" button is pressed. */

	/* Set the "value" field of "response" to "NULL"
	   to indicate "Cancel": */
	response->value = NULL;
    }
}


/*
 * Local: WarningBoxCallback
 *
 * Notify that the user has responded.
 */

static void WarningBoxCallback (Widget w, XtPointer client_data,
				XtPointer call_data)
{
    VBoolean *responded;

    responded = (VBoolean *) client_data;
    *responded = TRUE;
}


/*
 * VXPopupMessageBox
 *
 * Popup a modeless message box to display a message.
 */

void VXPopupMessageBox (VStringConst title, VStringConst message)
{
    Widget msgBox, button;
    XmString title_str, message_str;

    if (! VX_App.initialized)
	VError ("VXPopupMessageBox: VX not initialized");

    if (!VX_App.in_main_loop)
	VError ("VXPopupMessageBox: Main event loop not started");

    /* Create a message dialog box: */
    msgBox = XmCreateMessageDialog (VX_App.x.topLevel, "messageBox",
				    NULL, 0);

    /* Participate in the Editres Protocol: */
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
    XtAddEventHandler (XtParent (msgBox), (EventMask)0,
		       True, _XEditResCheckMessages, NULL);
#endif

    /* Set the title and message string: */
    title_str = XmStringCreateLtoR ((char *) title, XmSTRING_DEFAULT_CHARSET);
    message_str =
	XmStringCreateLtoR ((char *) message, XmSTRING_DEFAULT_CHARSET);
    XtVaSetValues (msgBox,
		   XmNautoUnmanage, FALSE,
		   XmNmessageString, message_str,
		   XmNdialogTitle, title_str,
		   (char *) NULL);
    XmStringFree (title_str);
    XmStringFree (message_str);

    /* Add a callback to be invoked when the "Ok" button is pressed: */
    XtAddCallback (msgBox, XmNokCallback, MessageBoxCallback, NULL);

    /* Do not display the "Help" and "Cancel" buttons: */
    button = XmMessageBoxGetChild (msgBox, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild (button);
    button = XmMessageBoxGetChild (msgBox, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild (button);

    /* Popup the message box: */
    XtManageChild (msgBox);
}


/*
 * VXPopupTextBox
 *
 * Popup a modeless text box to display a piece of text.
 */

void VXPopupTextBox (int nrows, int ncolumns, VStringConst title,
		     VStringConst text)
{
    Widget textArea, textBox, button, form;
    XmString title_str, close_str;

    if (! VX_App.initialized)
	VError ("VXPopupTextBox: VX not initialized");

    if (!VX_App.in_main_loop)
	VError ("VXPopupTextBox: Main event loop not started");

    /* Create a dialog shell: */
    textBox = XmCreateDialogShell (VX_App.x.topLevel, "textBox", NULL, 0);

    /* Participate in the Editres Protocol: */
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
    XtAddEventHandler (textBox, (EventMask)0,
		       True, _XEditResCheckMessages, NULL);
#endif

    /* Create a form to hold the text widget and the "Close" button widget: */
    title_str = XmStringCreateLtoR ((char *)title, XmSTRING_DEFAULT_CHARSET);
    form = XtVCW ("form", xmFormWidgetClass, textBox,
		  XmNdialogTitle, title_str,
		  XmNfractionBase, 100, (char *) NULL);
    XmStringFree (title_str);

    /* Create a text widget as a child of a scrolled window: */
    textArea = XmCreateScrolledText (form, "textArea", NULL, 0);

    /* Set the text string and display mode of the text widget: */
    XtVSV (textArea,
	   XmNrows, (short) nrows,
	   XmNcolumns, (short) ncolumns,
	   XmNvalue, text,
	   XmNeditable, FALSE,
	   XmNeditMode, XmMULTI_LINE_EDIT,
	   XmNcursorPositionVisible, FALSE, (char *) NULL);

    /* Set the layout of the scrolled window: */
    XtVSV (XtParent(textArea),
	   XmNtopAttachment, XmATTACH_FORM,
	   XmNtopOffset, 2,
	   XmNleftAttachment, XmATTACH_FORM,
	   XmNleftOffset, 2,
	   XmNrightAttachment, XmATTACH_FORM,
	   XmNrightOffset, 2,
	   XmNbottomAttachment, XmATTACH_FORM,
	   XmNbottomOffset, 50, (char *) NULL);

    /* Create the "Close" button: */
    close_str = XmStringCreateLtoR ("Close", XmSTRING_DEFAULT_CHARSET);
    button = XtVCMW ("closeButton",
		     xmPushButtonWidgetClass,
		     form,
		     XmNlabelString, close_str,
		     XmNleftAttachment, XmATTACH_POSITION,
		     XmNleftPosition, 45,
		     XmNtopAttachment, XmATTACH_WIDGET,
		     XmNtopWidget, XtParent(textArea),
		     XmNtopOffset, 10, (char *) NULL);
    XmStringFree (close_str);

    /* Add a callback to be invoked when the "Close" button is pressed: */
    XtAddCallback (button, XmNactivateCallback, TextBoxCallback, NULL);

    /* Display the text dialog box: */
    XtManageChild (textArea);
    XtManageChild (form);
}


/*
 * VXPopupInputBox
 *
 * Popup a modal input box for inputting a string.
 */

VString VXPopupInputBox (VStringConst title, VStringConst prompt,
			 VStringConst text)
{
    Widget inputBox, button;
    XmString title_str, prompt_str, text_str;
    static VXInputResponse response;

    if (! VX_App.initialized)
	VError ("VXPopupInputBox: VX not initialized");

    if (!VX_App.in_main_loop)
	VError ("VXPopupInputBox: Main event loop not started");

    /* Use has not responded yet: */
    response.responded = FALSE;

    /* Create the input box: */
    inputBox = XmCreatePromptDialog (VX_App.x.topLevel, "inputBox", NULL, 0);

    /* Participate in the Editres Protocol: */
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
    XtAddEventHandler (XtParent (inputBox), (EventMask)0,
		       True, _XEditResCheckMessages, NULL);
#endif

    /* Add a callback to be invoked when the "Cancel" button is pressed: */
    XtAddCallback (inputBox, XmNcancelCallback, InputBoxCallback, & response);

    /* Add a callback to be invoked when the "Ok" button is pressed: */
    XtAddCallback (inputBox, XmNokCallback, InputBoxCallback, & response);

    /* Do not display the "Help" button: */
    button = XmSelectionBoxGetChild (inputBox, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild (button);

    /* Set the title, prompt, and default input: */
    title_str = XmStringCreateLtoR ((char *)title, XmSTRING_DEFAULT_CHARSET);
    prompt_str = XmStringCreateLtoR ((char *)prompt, XmSTRING_DEFAULT_CHARSET);
    text_str = XmStringCreateLtoR ((char *)text, XmSTRING_DEFAULT_CHARSET);
    XtVSV (inputBox,
	   XmNautoUnmanage, FALSE,
	   XmNdialogStyle, XmDIALOG_APPLICATION_MODAL,
	   XmNdialogTitle, title_str,
	   XmNselectionLabelString, prompt_str,
	   XmNtextString, text_str, (char *) NULL);
    XmStringFree (title_str);
    XmStringFree (prompt_str);
    XmStringFree (text_str);

    /* Display the inputBox: */
    XtManageChild (inputBox);

    /* Wait until the user had responed: */
    while (response.responded != TRUE || XtAppPending (VX_App.x.appContext))
	XtAppProcessEvent (VX_App.x.appContext, XtIMAll);

    /* Destroy the inputBox: */
    XtDestroyWidget (XtParent (inputBox));

    return response.value;
}


/*
 * VXPopupYesNoBox
 *
 * Popup a modal yes-no box for inputting a yes/no answer.
 */

VXAnswer VXPopupYesNoBox (VStringConst title, VStringConst question)
{
    Widget yesNoBox;
    XmString title_str, question_str, yes_str, no_str, cancel_str;
    static VXYesNoResponse response;

    if (! VX_App.initialized)
	VError ("VXPopupYesNoBox: VX not initialized");

    if (!VX_App.in_main_loop)
	VError ("VXPopupYesNoBox: Main event loop not started");

    /* User has not responded yet: */
    response.responded = FALSE;

    /* Create the yes-no box: */
    yesNoBox = XmCreateQuestionDialog (VX_App.x.topLevel, "yesNoBox", NULL, 0);

    /* Participate in the Editres Protocol: */
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
    XtAddEventHandler (XtParent (yesNoBox), (EventMask)0,
		       True, _XEditResCheckMessages, NULL);
#endif

    /* Add a callback to be invoked when any button is pressed: */
    XtAddCallback (yesNoBox, XmNokCallback, YesNoBoxCallback, & response);
    XtAddCallback (yesNoBox, XmNcancelCallback, YesNoBoxCallback, & response);
    XtAddCallback (yesNoBox, XmNhelpCallback, YesNoBoxCallback, & response);

    /* Set the title, question, and button labels: */
    title_str = XmStringCreateLtoR ((char *)title, XmSTRING_DEFAULT_CHARSET);
    question_str = XmStringCreateLtoR ((char *)question,
				       XmSTRING_DEFAULT_CHARSET);
    yes_str = XmStringCreateLtoR ("Yes", XmSTRING_DEFAULT_CHARSET);
    no_str = XmStringCreateLtoR ("No", XmSTRING_DEFAULT_CHARSET);
    cancel_str = XmStringCreateLtoR ("Cancel", XmSTRING_DEFAULT_CHARSET);
    XtVSV (yesNoBox,
	   XmNautoUnmanage, FALSE,
	   XmNdialogStyle, XmDIALOG_APPLICATION_MODAL,
	   XmNdialogTitle, title_str,
	   XmNmessageString, question_str,
	   XmNokLabelString, yes_str,
	   XmNcancelLabelString, no_str,
	   XmNhelpLabelString, cancel_str, (char *) NULL);
    XmStringFree (title_str);
    XmStringFree (question_str);
    XmStringFree (yes_str);
    XmStringFree (no_str);
    XmStringFree (cancel_str);

    /* Display the yes-no box: */
    XtManageChild (yesNoBox);

    /* Wait until the user has responded: */
    while (response.responded != TRUE || XtAppPending (VX_App.x.appContext))
	XtAppProcessEvent (VX_App.x.appContext, XtIMAll);

    /* Destroy the yes-no box: */
    XtDestroyWidget (XtParent (yesNoBox));

    return response.value;
}


/*
 * VXPopupFileBox
 *
 * Popup a modal file selection box.
 */

VString VXPopupFileBox (VStringConst title)
{
    Widget button;
    XmString title_str;
    static VXFileResponse response;

    if (! VX_App.initialized)
	VError ("VXPopupFileBox: VX not initialized");

    if (!VX_App.in_main_loop)
	VError ("VXPopupFileBox: Main event loop not started");

    /* User has not responded yet: */
    response.responded = FALSE;

    /* Create the file selection box if not yet created: */
    if (l_dialog.fileBox == NULL) {

	/* Display busy cursor: */
	if (XtIsRealized (VX_App.x.topLevel))
	    XMapWindow (XtDisplay(VX_App.x.topLevel), VX_App.x.busyWindow);
	XSync (XtDisplay(VX_App.x.topLevel), FALSE);

	/* Create dialog shell and file-selection box: */
	l_dialog.fileBox =
	    XmCreateFileSelectionDialog (VX_App.x.topLevel, "fileBox",
					 NULL, 0);

	/* Participate in the Editres Protocol: */
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
	XtAddEventHandler (XtParent (l_dialog.fileBox), (EventMask)0,
			   True, _XEditResCheckMessages, NULL);
#endif

	/* Add a callback to be invoked when any button is pressed: */
	XtAddCallback (l_dialog.fileBox, XmNcancelCallback, FileBoxCallback,
		       & response);
	XtAddCallback (l_dialog.fileBox, XmNokCallback, FileBoxCallback,
		       & response);

	/* Do not display the "Help" button: */
	button = XmFileSelectionBoxGetChild (l_dialog.fileBox,
					     XmDIALOG_HELP_BUTTON);
	XtUnmanageChild (button);

	/* Hide busy cursor: */
	if (XtIsRealized (VX_App.x.topLevel))
	    XUnmapWindow (XtDisplay(VX_App.x.topLevel), VX_App.x.busyWindow);
    }

    /* Set the title and other resources: */
    title_str = XmStringCreateLtoR ((char *)title, XmSTRING_DEFAULT_CHARSET);
    XtVSV (l_dialog.fileBox,
	   XmNautoUnmanage, FALSE,
	   XmNdialogStyle, XmDIALOG_APPLICATION_MODAL,
	   XmNdialogTitle, title_str, (char *) NULL);
    XmStringFree (title_str);

    /* Display the file selection box: */
    XtManageChild (l_dialog.fileBox);

    /* Wait until the user has responded: */
    while (response.responded != TRUE || XtAppPending (VX_App.x.appContext))
	XtAppProcessEvent (VX_App.x.appContext, XtIMAll);

    /* Hide the file selection box: */
    XtUnmanageChild (l_dialog.fileBox);

    return response.value;
}


/*
 * VX_Warning
 *
 * Internal routine to popup a modal dialog box displaying a warning message.
 */

void VX_Warning (VStringConst message)
{
    Widget button;
    XmString str;
    static VBoolean responded;

    /* User has not responded yet: */
    responded = FALSE;

    /* Create the warning box if not yet created: */
    if (! l_dialog.warningBox) {

	l_dialog.warningBox =
	    XmCreateWarningDialog (VX_App.x.topLevel, "warningBox", NULL, 0);

	/* Participate in the Editres Protocol: */
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
	XtAddEventHandler (XtParent (l_dialog.warningBox), (EventMask)0,
			   True, _XEditResCheckMessages, NULL);
#endif

	/* Add a callback to be invoked when the "Ok" button is pressed: */
	XtAddCallback (l_dialog.warningBox, XmNokCallback,
		       WarningBoxCallback, (XtPointer) &responded);

	/* Do not display the "Help" and "Cancel" buttons: */
	button = XmMessageBoxGetChild (l_dialog.warningBox,
				       XmDIALOG_CANCEL_BUTTON);
	XtUnmanageChild (button);
	button = XmMessageBoxGetChild (l_dialog.warningBox,
				       XmDIALOG_HELP_BUTTON);
	XtUnmanageChild (button);

	/* Set the title string: */
	str = XmStringCreateLtoR ("Warning", XmSTRING_DEFAULT_CHARSET);
	XtVSV (l_dialog.warningBox,
	       XmNdialogStyle, XmDIALOG_APPLICATION_MODAL,
	       XmNautoUnmanage, FALSE,
	       XmNdialogTitle, str, (char *) NULL);
	XmStringFree (str);
    }


    /* Set the message string: */
    str = XmStringCreateLtoR ((char *) message, XmSTRING_DEFAULT_CHARSET);
    XtVSV (l_dialog.warningBox, XmNmessageString, str, (char *) NULL);
    XmStringFree (str);

    /* Display the warning box: */
    XtManageChild (l_dialog.warningBox);

    /* Wait until the user has responded: */
    while (responded != TRUE || XtAppPending (VX_App.x.appContext))
	XtAppProcessEvent (VX_App.x.appContext, XtIMAll);

    /* Hide the warning box: */
    XtUnmanageChild (l_dialog.warningBox);
}
