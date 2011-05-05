/*
 *  $Id: VXInput.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains VX input-callback registration routines.
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

/* From X11R5 Xt and Motif: */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/keysym.h>
#include <Xm/Xm.h>

/* File identification string: */
VRcsId ("$Id: VXInput.c 3177 2008-04-01 14:47:24Z karstenm $");

/*
 * Record for storing input related data.
 */

typedef struct {
    VList input_cb_list[VXnInputTypes];	/* array of input callback lists */
} InputRec;
static InputRec l_input;

/*
 * Type: VXInputCb
 *
 * Record for storing an input-callback and its associated data.
 */
typedef struct {
    VXInputCallback callback;	/* callback function */
    VPointer client_data;	/* client data */
} VXInputCb;


/* Later in this file: */
static void DispatchKeyPress (Widget, VList, XKeyPressedEvent *,
			      Boolean *);
static void DispatchButtonPress (Widget, VList, XButtonPressedEvent *,
				 Boolean *);
static void DispatchButtonRelease (Widget, VList, XButtonReleasedEvent *,
				   Boolean *);
static void DispatchPointerMotion (Widget, VList, XMotionEvent *,
				   Boolean *);


/*
 * Extern: VX_InitInput
 *
 * Initialize the menu module.
 */

extern VBoolean VX_InitInput (void)
{
    int i;

    for (i = 0; i < VXnInputTypes; i++)
	l_input.input_cb_list[i] = NULL;
    return TRUE;
}


/*
 * Local: DispatchKeyPress
 *
 * Call the input callbacks associated with the VXIkeyPress user input.
 */

static void DispatchKeyPress (Widget w, VList callback_list,
			      XKeyPressedEvent *event,
			      Boolean *continue_to_dispatch)
{
    VXInputDataRec input_data;
    double row, column;
    VXInputCb *icb;
    char key[256];
    KeySym keySym;
    XComposeStatus compose;

    /* Determine the image coordinate of the user input: */
    if (VX_App.v.image == NULL ||
	!VImageViewClipToImage (VX_App.x.imageView, event->x, event->y,
				& row, & column)) {
	return;
    } else {
	input_data.row = (int) row;
	input_data.column = (int) column;
    }

    /* Set the input type: */
    input_data.input_type = VXIkeyPress;

    /* Find the ASCII code of the key being pressed: */
    (void) XLookupString(event, key, sizeof (key), &keySym, &compose);
    input_data.value =
	(keySym < XK_space || keySym > XK_asciitilde) ? 0 : key[0];

    /* Set the modifiers field: */
    /* ==> This relies on a correspondence between X and VX definitions. */
    input_data.modifiers = event->state;

    /* call each callback associated with the user input */
    icb = (VXInputCb *) VListFirst (callback_list);
    while (icb != NULL) {
	/* Call the callback function: */
	(icb->callback)(&input_data, icb->client_data);
	icb = (VXInputCb *) VListNext (callback_list);
    }
}


/*
 * Local: DispatchButtonPress
 *
 * Call the input callbacks associated with the VXIbuttonPress user input.
 */

static void DispatchButtonPress (Widget w, VList callback_list,
				 XButtonPressedEvent *event,
				 Boolean *continue_to_dispatch)
{
    VXInputDataRec input_data;
    double row, column;
    VXInputCb *icb;

    /* Determine the image coordinate of the user input: */
    if (VX_App.v.image == NULL ||
	!VImageViewClipToImage (VX_App.x.imageView, event->x, event->y,
				&row, &column)) {
	return;
    } else {
	input_data.row = (int) row;
	input_data.column = (int) column;
    }

    /* Set the input type: */
    /* ==> This relies on a correspondence between X and VX definitions. */
    input_data.input_type = VXIbuttonPress;

    /* Set the value field to the button number: */
    /* ==> This relies on a correspondence between X and VX definitions. */
    input_data.value = event->button;

    /* Set the modifer field: */
    input_data.modifiers = event->state;

    /* call each callback associated with the VXIbuttonPress user input */
    icb = (VXInputCb *) VListFirst (callback_list);
    while (icb != NULL) {
	/* Call the callback function: */
	(icb->callback)(&input_data, icb->client_data);
	icb = (VXInputCb *) VListNext (callback_list);
    }
}


/*
 * Local: DispatchButtonRelease
 *
 * Call the input callbacks associated with the VXIbuttonRelease user input.
 */

static void DispatchButtonRelease (Widget w, VList callback_list,
				   XButtonReleasedEvent *event,
				   Boolean *continue_to_dispatch)
{
    VXInputDataRec input_data;
    double row, column;
    VXInputCb *icb;

    /* Determine the image coordinate of the user input: */
    if (VX_App.v.image == NULL ||
	!VImageViewClipToImage (VX_App.x.imageView, event->x, event->y,
				&row, &column)) {
	return;
    } else {
	input_data.row = (int) row;
	input_data.column = (int) column;
    }

    /* Set the input type: */
    input_data.input_type = VXIbuttonRelease;

    /* Set the value field to the button number: */
    /* ==> This relies on a correspondence between X and VX definitions. */
    input_data.value = event->button;

    /* Set the modifer field: */
    input_data.modifiers = (VXModifierMask) (event->state);

    /* call each callback associated with the VXIbuttonRelease user input */
    icb = (VXInputCb *) VListFirst (callback_list);
    while (icb != NULL) {
	/* Call the callback function: */
	(icb->callback)(&input_data, icb->client_data);
	icb = (VXInputCb *) VListNext (callback_list);
    }
}

/*
 * Local: DispatchPointerMotion
 *
 * Call the input callbacks associated with the VXIPointerMotion user input.
 */

static void DispatchPointerMotion (Widget w, VList callback_list,
				   XMotionEvent *event,
				   Boolean *continue_to_dispatch)
{
    VXInputDataRec input_data;
    double row, column;
    VXInputCb *icb;
    int root_x, root_y, win_x, win_y;
    unsigned int keys_buttons;
    Window root, child;

    /* Only want hints: */
    XQueryPointer (XtDisplay(VX_App.x.imageView), XtWindow(VX_App.x.imageView),
		   &root, &child, &root_x, &root_y, &win_x, &win_y,
		   &keys_buttons);

    /* Determine the image coordinate of the user input: */
    if (VX_App.v.image == NULL ||
	!VImageViewClipToImage (VX_App.x.imageView, win_x, win_y,
				&row, &column)) {
	return;
    } else {
	input_data.row = (int) row;
	input_data.column = (int) column;
    }

    /* Set the input type: */
    input_data.input_type = VXIpointerMotion;

    /* Set the modifier field: */
    input_data.modifiers = (VXModifierMask) (event->state);

    /* Call each callback associated with the VXIpointerMotion user input: */
    icb = (VXInputCb *) VListFirst (callback_list);
    while (icb != NULL) {
	/* Call the callback function: */
	(icb->callback)(&input_data, icb->client_data);
	icb = (VXInputCb *) VListNext (callback_list);
    }
}


/*
 * VXAddInputCallback
 *
 * Associate a callback with a user input.
 */

void VXAddInputCallback (VXInputType input_type, VXInputCallback callback,
			 VPointer client_data)
{
    VXInputCb *icb;
    XtEventHandler event_handler = NULL;
    EventMask event_mask = 0;

    if (! VX_App.initialized)
	VError ("VXAddInputCallback: VX not initialized");

    if (VX_App.in_main_loop)
	VError ("VXAddInputCallback: Main event loop already started");

    /* Check if callback is NULL: */
    if (callback == NULL)
	VError ("VXAddInputCallback: Callback function cannot be NULL");

    /* Check validity of input_type: */
    if (input_type < 0 || input_type >= VXnInputTypes)
	VError ("VXAddInputCallback: Invalid input type %d", input_type);

    /* Allocate a structure to store callback and client_data: */
    icb = VMalloc (sizeof (VXInputCb));
    icb->callback = callback;
    icb->client_data = client_data;

    /* If the appropriate callback list is NULL, allocate space for the list
       and register the appropriate event handler */
    if (l_input.input_cb_list[input_type] == NULL) {

	l_input.input_cb_list[input_type] = VListCreate ();

	switch (input_type) {

	case VXIkeyPress:
	    event_handler = (XtEventHandler) DispatchKeyPress;
	    event_mask = KeyPressMask;
	    break;

	case VXIbuttonPress:
	    event_handler = (XtEventHandler) DispatchButtonPress;
	    event_mask = ButtonPressMask;
	    break;

	case VXIbuttonRelease:
	    event_handler = (XtEventHandler) DispatchButtonRelease;
	    event_mask = ButtonReleaseMask;
	    break;

	case VXIpointerMotion:
	    event_handler = (XtEventHandler) DispatchPointerMotion;
	    event_mask = PointerMotionMask | PointerMotionHintMask;
	    break;

	default:
	    break;
	}

	XtAddEventHandler (VX_App.x.imageView, event_mask, FALSE,
			   event_handler,
			   (XtPointer) l_input.input_cb_list[input_type]);
    }

    /* Add the callback structure to the appropriate list: */
    VListAppend (l_input.input_cb_list[input_type], (VPointer) icb);
}
