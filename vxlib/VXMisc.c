/*
 *  $Id: VXMisc.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains VX miscellaneous routines.
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
#include "viaio/VX.h"
#include "viaio/VXPrivate.h"

/* From X11R5 Xt and Motif: */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>

/* File identification string: */
VRcsId ("$Id: VXMisc.c 3177 2008-04-01 14:47:24Z karstenm $");


/*
 * VXDisplayMessage
 *
 * Display a message in the message area.
 */

void VXDisplayMessage (VBooleanPromoted overwrite, VStringConst format, ...)
{
    va_list ap;
    char str[1024];
    XmTextPosition pos;

    if (! VX_App.initialized)
	VWarning ("VXDisplayMessage: VX not initialized");

    va_start (ap, format);

    /* Call vsprintf to format the message string: */
    vsprintf (str, format, ap);

    va_end (ap);

    /* Display the message string: */
    if (overwrite == TRUE) {
	/* Replace current text by new string: */
	XmTextSetString (VX_App.x.msgArea, str);
	XmTextSetTopCharacter (VX_App.x.msgArea, (XmTextPosition) 0);
    } else {
	/* Append new string to end of existing text: */
	pos = XmTextGetLastPosition (VX_App.x.msgArea);
	XmTextInsert(VX_App.x.msgArea, pos, str);
	XmTextSetTopCharacter (VX_App.x.msgArea, pos);
    }
}


/*
 * VXShowMessageArea
 *
 * Show message area.
 */

void VXShowMessageArea (void)
{
    if (! VX_App.initialized)
	VError ("VXShowMessageArea: VX not initialized");

    if (VX_App.x.msg_area_nlines > 0 &&
	! XtIsManaged (VX_App.x.msgAreaFrame)) {
	XtManageChild (VX_App.x.msgAreaFrame);
    }
}


/*
 * VXHideMessageArea
 *
 * Hide message area.
 */

void VXHideMessageArea (void)
{
    if (! VX_App.initialized)
	VError ("VXHideMessageArea: VX not initialized");

    if (XtIsManaged (VX_App.x.msgAreaFrame)) {
	XtUnmanageChild (VX_App.x.msgAreaFrame);
    }
}


/*
 * VXGetApplicationShell
 *
 * Return top-level application shell widget.
 */

Widget VXGetApplicationShell (void)
{
    if (! VX_App.initialized)
	VError ("VXGetApplicationShell: VX not initialized");

    return VX_App.x.topLevel;
}


/*
 * VXGetImageViewWidget
 *
 * Return the ImageImageView widget.
 */

Widget VXGetImageViewWidget(void)
{
    if (! VX_App.initialized)
	VError ("VXGetImageViewWidget: VX not initialized");

    return VX_App.x.imageView;
}


/*
 *  VXIsColorDisplay
 *
 *  Return TRUE if the VX's window is capable of showing colors.
 */

VBoolean VXIsColorDisplay (void)
{
    Display *display;
    Visual *visual;

    if (! VX_App.initialized)
	VError ("VXIsColorDisplay: VX not initialized");

    if (VX_App.x.vcolormap)
	visual = VColormapVisual (VX_App.x.vcolormap);
    else {
	display = XtDisplay (VX_App.x.imageView);
	visual = DefaultVisual (display, DefaultScreen (display));
    }

    return ! (visual->class == GrayScale || visual->class == StaticGray);
}
