/*
 *  $Id: VXText.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains VX text drawing routines.
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
VRcsId ("$Id: VXText.c 3177 2008-04-01 14:47:24Z karstenm $");


/*
 * Record for storing text related data.
 */

typedef struct {
    Pixel color;		/* current text color */
    Font fid;			/* current font ID */
    VList ns_texts;		/* List of texts not saved by
				   VXStoreOverlays. */
    VList s_texts;		/* List of saved texts. */
    VBoolean saved_consistent;	/* TRUE iff s_texts should be displayed */
    GC gc;			/* Graphic context ID. */
} TextRec;
static TextRec l_text;


/*
 * Record for storing data of a text string.
 */
typedef struct {
    Font fid;
    Pixel color;
    VStringConst str;
    float r, c;
} TextStatesRec, *TextStates;


/* Later in this file: */
static void DrawTextInMemory (VStringConst, float, float);
static void DrawTextOnScreen (VStringConst, float, float);
static void DrawText (VStringConst, float, float);
static void DrawTextsOnScreen (VList);
static void FreeText (VPointer);


/*
 * Extern: VX_InitText
 *
 * Initialize the text module.
 */

extern VBoolean VX_InitText (void)
{
    /* Allocate storage for text lists: */
    l_text.ns_texts = VListCreate();
    l_text.s_texts = VListCreate();

    l_text.saved_consistent = FALSE;

    /* Set default font: */
    VXSetTextFont ("9x15");

    /* Set default text color: */
    if (VXIsColorDisplay())
	VXSetTextColor ("yellow");
    else VXSetTextColor ("white");

    return TRUE;
}


/*
 * Extern: VX_GetTextGC
 *
 * Get a GC for drawing texts.
 */

extern void VX_GetTextGC (void)
{
    XGCValues gcv;

    /* Create a graphic context for drawing lines: */
    l_text.gc = XCreateGC (XtDisplay(VX_App.x.imageView),
			   XtWindow(VX_App.x.imageView), 0, & gcv);
}


/*
 * Extern: VX_RedrawTexts
 *
 * Redraw texts on exposure.
 */

extern void VX_RedrawTexts (void)
{
    if (l_text.saved_consistent && !VX_App.o.pixmap_consistent)
	DrawTextsOnScreen (l_text.s_texts);

    DrawTextsOnScreen (l_text.ns_texts);
}


/*
 * Extern: VX_StoreTexts
 *
 * Save the states of existing texts.
 */

extern void VX_StoreTexts (void)
{
    if (l_text.saved_consistent) {
	/* combine s_texts and ns_texts: */
	VListConcat (l_text.s_texts, l_text.ns_texts);
    } else {
	/* Free s_texts: */
	VListDestroy (l_text.s_texts, FreeText);
	/* Remember currently visible Texts: */
	l_text.s_texts = l_text.ns_texts;
	l_text.saved_consistent = TRUE;
    }

    /* Reset ns_texts to empty list: */
    l_text.ns_texts = VListCreate ();
}


/*
 * Extern: VX_RestoreTexts
 *
 * Revert to previously stored state.
 */

extern void VX_RestoreTexts (void)
{
    /* Free ns_texts: */
    VListDestroy (l_text.ns_texts, FreeText);
    l_text.ns_texts = VListCreate ();

    /* Display saved texts: */
    l_text.saved_consistent = TRUE;
}


/*
 * Local: DrawTextInMemory
 *
 * Save text info in memory.
 */

static void DrawTextInMemory (VStringConst str, float r, float c)
{
    TextStates text;

    /* Add text to text list: */
    text = VMalloc (sizeof (TextRec));

    text->fid = l_text.fid;

    text->color = l_text.color;

    text->str = XtNewString (str);

    text->r = r;
    text->c = c;

    VListAppend (l_text.ns_texts, (VPointer) text);
}


/*
 * Local: DrawTextOnScreen
 *
 * Draw text directly to screen.
 */

static void DrawTextOnScreen (VStringConst str, float r, float c)
{
    int win_x, win_y;

    /* Convert image coordinates to window coordinates: */
    VImageViewImageToWindow (VX_App.x.imageView, r, c, &win_x, &win_y);

    XDrawString (XtDisplay (VX_App.x.imageView), XtWindow (VX_App.x.imageView),
		 l_text.gc, win_x, win_y, str, strlen (str));
}


/*
 * Local: DrawText
 *
 * DrawTextInMemory + DrawTextOnScreen
 */

static void DrawText (VStringConst str, float r, float c)
{
    DrawTextInMemory (str, r, c);
    if (XtIsRealized (VX_App.x.imageView)) {
	XSetFont (XtDisplay (VX_App.x.imageView), l_text.gc, l_text.fid);
	XSetForeground (XtDisplay (VX_App.x.imageView),
			l_text.gc, l_text.color);
	DrawTextOnScreen (str, r, c);
    }
}

/*
 * Local: DrawTextsOnScreen
 *
 * Draw multiple texts in a text list.
 */

static void DrawTextsOnScreen (VList text_list)
{
    TextStates text;
    Font prev_fid = 0;
    Pixel prev_color = 0;

    if (! XtIsRealized (VX_App.x.imageView))
	return;

    text = (TextStates) VListFirst (text_list);

    /* Just to make prev fid different from current ones: */
    if (text != NULL) {
	prev_fid = text->fid + 1;
	prev_color = text->color + 1;
    }

    while (text != NULL) {
	/* If fid has changed then reset fid: */
	if (text->fid != prev_fid) {
	    XSetFont (XtDisplay (VX_App.x.imageView),
		      l_text.gc,
		      text->fid);
	    prev_fid = text->fid;
	}

	if (text->color != prev_color) {
	    XSetForeground (XtDisplay(VX_App.x.imageView),
			    l_text.gc,
			    text->color);
	    prev_color = text->color;
	}

	/* Draw a text: */
	DrawTextOnScreen (text->str, text->r, text->c);
	text = (TextStates) VListNext (text_list);
    }
}


/*
 * Local: FreeText
 *
 * Free an instance of TextRec *.
 */

static void FreeText (VPointer data)
{
    VFree ((VString) ((TextStates) data)->str);
    VFree ((TextStates) data);
}


/*
 * VXSetTextColor
 *
 * Set Text color.
 */
VBoolean VXSetTextColor (VStringConst color_name)
{
    XColor color;
    Display *display;
    int screen;
    Colormap cmap;

    if (! VX_App.initialized)
	VError ("VXSetTextColor: VX not initialized");

    display = XtDisplay (VX_App.x.imageView);
    screen = DefaultScreen (display);
    cmap = DefaultColormap (display, screen);

    if (! XParseColor (display, cmap, color_name, & color)) {
	VWarning ("VXSetTextColor: Color %s not recognized", color_name);
	return FALSE;
    }

    if (VX_App.x.vcolormap) {
	VColormapRGBPixel (VX_App.x.vcolormap, & color);
	l_text.color = color.pixel;
    } else if (XAllocColor (display, cmap, & color)) {
	l_text.color = color.pixel;
    } else {
	VWarning ("VXSetTextColor: Cannot allocate color %s", color_name);
	return FALSE;
    }

    return TRUE;
}


/*
 * VXSetTextFont
 *
 */

VBoolean VXSetTextFont (VStringConst fontname)
{
    char **font_list;
    int nfonts;

    if (! VX_App.initialized)
	VError ("VXSetTextFont: VX not initialized");

    /* Find a font that match "fontname": */
    font_list = XListFonts (XtDisplay (VX_App.x.imageView), fontname,
			    1, & nfonts);
    if (nfonts == 0) { /* no match */
	VWarning ("VXSetTextFont: Cannot find font %s", fontname);
	return FALSE;
    }

    /* Load and store current Font: */
    l_text.fid = XLoadFont (XtDisplay (VX_App.x.imageView), font_list[0]);
    if (l_text.fid == 0) {
	VWarning ("VXSetTextFont: Cannot load font %s", fontname);
	return FALSE;
    }

    XFreeFontNames (font_list);

    return TRUE;
}


/*
 * VXDrawText
 *
 * Draw a Text and add it to the Text list.
 */

VBoolean VXDrawText (VStringConst str, double r, double c)
{
    if (! VX_App.initialized)
	VError ("VXDrawText: VX not initialized");

    if (VX_App.v.image == NULL) {
	VWarning ("VXDrawText: Cannot draw on NULL image");
	return FALSE;
    }

    DrawText (str, r, c);
    return TRUE;
}


/*
 * VXClearTexts
 *
 * Clear Text from screen and Text list.
 */

void VXClearTexts (void)
{
    if (! VX_App.initialized)
	VError ("VXClearTexts: VX not initialized");

    /* Free ns_texts: */
    VListDestroy (l_text.ns_texts, FreeText);
    l_text.ns_texts = VListCreate ();

    /* Saved texts should no longer be displayed: */
    l_text.saved_consistent = FALSE;

    /* Overlays pixmap is no longer consistent: */
    VX_App.o.pixmap_consistent = FALSE;

    /* Redraw image and overlays: */
    if (XtIsRealized (VX_App.x.imageView))
	VImageViewRedraw (VX_App.x.imageView);
}
