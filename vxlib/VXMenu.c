/*
 *  $Id: VXMenu.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains VX menu routines.
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
#include "viaio/VList.h"
#include "viaio/VX.h"
#include "viaio/VXPrivate.h"

/* From X11R5 Xt and Motif: */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>

/* File identification string: */
VRcsId ("$Id: VXMenu.c 3177 2008-04-01 14:47:24Z karstenm $");

/* Later in this file: */
static void DispatchMenuEvents (Widget, XtPointer, XtPointer);
static void AddMenuEntry (Widget, VString, VString, VXMenuCallback, VPointer);
static void ExtractMnemonic (VStringConst, VString *, VString *);
static void MakeAccString (VString, VString *);
static int CompareWidgetIDs (VPointer, VPointer);


/*
 * Record for storing menu related data.
 */

typedef struct {
    VList menu_callbacks;	/* menu callback list */
} MenuRec;
static MenuRec l_menu;

/*
 * Record for storing a menu callback and its related data.
 */

typedef struct {
    Widget button;		/* menu button widget ID */
    VXMenuCallback callback;	/* callback function */
    VPointer client_data;	/* client data */
} VXMenuCb;


/*
 * Extern: VX_InitMenu
 *
 * Initialize the menu module.
 */

extern VBoolean VX_InitMenu (void)
{
    l_menu.menu_callbacks = VListCreate ();
    return TRUE;
}


/*
 * Local: DispatchMenuEvents
 *
 * Call the appropriate menu callback depending on the menu button
 * being pressed.
 */

static void DispatchMenuEvents (Widget w, XtPointer client_data,
				XtPointer call_data)
{
    VXMenuCb * mcb;

    /* Search for the appropriate callback function: */
    (void) VListFirst(l_menu.menu_callbacks);
    mcb = (VXMenuCb *) VListSearch (l_menu.menu_callbacks,
				   CompareWidgetIDs,
				   (VPointer) w);

    if (mcb)
	(mcb->callback)(mcb->client_data);
}


/*
 * Local: AddMenuEntry
 *
 * Add a menu entry to a menu.
 */

static void AddMenuEntry (Widget menu,
			  VString menu_label, VString accelerator,
			  VXMenuCallback menu_callback, VPointer client_data)
{
    Widget menuEntryButton;
    XmString str;
    VXMenuCb *mcb;
    VString menu_name, menu_mnemonic, acc_str;
    XrmValue from, to;

    /* Extract the mnemonic and menu name: */
    ExtractMnemonic (menu_label, &menu_name, &menu_mnemonic);

    /* Create the menu entry button: */
    menuEntryButton = XtVCMW (menu_name, xmPushButtonWidgetClass, menu,
			      (char *) NULL);

    /* Add mnemonic if it exists: */
    if (*menu_mnemonic != '\0') {
	/* Convert mnemonic from String type to KeySym type: */
	from.size = strlen (from.addr = menu_mnemonic);
	to.addr = NULL;
	if (!XtConvertAndStore (menuEntryButton, XmRString, &from,
				XmRKeySym, &to))
	    VError ("VXAddMenu : Error in converting mnemonic to KeySym");

	/* Set the mnemonic resource: */
	XtVSV (menuEntryButton, XmNmnemonic, (* (KeySym *) to.addr),
	       (char *) NULL);
    }

    /* Add accelerator if it exists: */
    if (accelerator != NULL) {
	/* Create the accelerator string: */
	MakeAccString (accelerator, &acc_str);

	/* Set the accelerator resource: */
	str = XmStringCreateSimple (acc_str);
	XtVSV (menuEntryButton,
	       XmNaccelerator, accelerator,
	       XmNacceleratorText, str, (char *) NULL);
	XmStringFree (str);
    }

    /* Add menu callback if it exists: */
    if (menu_callback != NULL) {
	/* Store the callback and its associated data to the callback list: */
	mcb = VMalloc (sizeof (VXMenuCb));
	mcb->button = menuEntryButton;
	mcb->callback = menu_callback;
	mcb->client_data = client_data;
	VListAppend(l_menu.menu_callbacks, (VPointer) mcb);

	/* Add the generic callback "DispatchMenuEvents" to the button: */
	XtAddCallback (menuEntryButton, XmNactivateCallback,
		       DispatchMenuEvents, (XtPointer) NULL);
    }
}


/*
 * Local: ExtractMnemonic
 *
 * Extract the mnemonic and name of a menu entry from the
 * encoded menu entry label string.
 * Note: Storage of returned strings is maintained by ExtractMnemonic.
 */

static void ExtractMnemonic (VStringConst menu_label, VString *menu_name,
			     VString *menu_mnemonic)
{
    static char name[256], mnemonic[256];
    VString pname, pmnemonic;

    /* Check length of menu entry label: */
    if (strlen (menu_label) >= sizeof (name))
	VError ("VXAddMenu : Menu entry label %s is too long", menu_label);

    *menu_name = name;
    *menu_mnemonic = mnemonic;

    pname = name;
    pmnemonic = mnemonic;

    /* Extract mnemonic and menu name: */
    while (*menu_label != '_' && *menu_label != '\0')
	*pname++ = *menu_label++;
    if (*menu_label == '\0' || *(menu_label+1) == '\0') { /* no mnemonic */
	*pname = '\0';
	*pmnemonic = '\0';
	return;
    }

    menu_label++; /* skip underscore */
    *pname++ = *pmnemonic++ = *menu_label++; /* get mnemonic character */

    /* Copy rest of menu label: */
    while (*menu_label != '\0')
	*pname++ = *menu_label++;
    *pname = '\0';
    *pmnemonic = '\0';
}


/*
 * Local: MakeAccString
 *
 * Generate the accelerator label string from the accelerator description
 * string.
 * The storage of returned string is maintained by MakeAccString.
 */

static void MakeAccString (VString accelerator, VString *acc_str)
{
    char str[256];
    VString pstr, pacc;
    VBoolean modifier_exist;

    /* Check length of accelerator description string: */
    if (strlen(accelerator) >= sizeof (str))
	VError ("VXAddMenu : Accelerator %s is too long", accelerator);

    /* Construct the accelerator label string: */
    pstr = str;
    *acc_str = str;
    modifier_exist = FALSE;
    pacc = accelerator;

    /* Extract modifier: */
    while (*pacc != '<' && *pacc != '\0') {
	modifier_exist = TRUE;
	*pstr++ = *pacc++;
    }

    if (*pacc == '\0')
	VError ("VXAddMenu : Accelerator %s is invalid", accelerator);

    /* Skip "<Key>": */
    while (*pacc != '>' && *pacc != '\0')
	pacc++;

    if (*pacc == '\0' || *(pacc+1) == '\0')
	VError ("VXAddMenu : Accelerator %s is invalid", accelerator);

    /* Add "-" symbol between modifier and character: */
    if (modifier_exist == TRUE)
	*pstr++ = '-';

    /* Copy character: */
    *pstr++ = *(pacc+1);
    *pstr = '\0';
}


/*
 * Local: CompareWidgetIDs
 *
 * Return true if the button field of x1 (of type VXMenuCb *) equals x2.
 */

static int CompareWidgetIDs (VPointer x1, VPointer x2)
{
    Widget w;
    VXMenuCb *mcb;

    mcb = (VXMenuCb *) x1;
    w = (Widget) x2;
    return (mcb->button == w);
}


/*
 * VXAddMenu
 *
 * Add a menu (with its associated callbacks) to the menu bar.
 */

void VXAddMenu (VStringConst menu_label, ...)
{
    va_list ap;
    VString menu_entry_label, accelerator;
    VXMenuCallback menu_callback;
    VPointer client_data;
    Widget menu, menuButton;
    VString menu_name, menu_mnemonic;
    XrmValue from, to;

    if (! VX_App.initialized)
	VError ("VXAddMenu: VX not initialized");

    if (VX_App.in_main_loop)
	VError ("VXAddMenu: Main event loop already started");

    /* Get the menu label string: */
    va_start (ap, menu_label);

    /* Extract mnemonic and menu name: */
    ExtractMnemonic (menu_label, &menu_name, &menu_mnemonic);

    /* Create the menu button: */
    menuButton = XtVCMW (menu_name, xmCascadeButtonWidgetClass,
			 VX_App.x.menuBar, (char *) NULL);

    /* If the menu is a Help menu, right align it: */
    if (strcmp(menu_name, "Help") == 0)
	XtVSV (VX_App.x.menuBar, XmNmenuHelpWidget, menuButton, (char *) NULL);

    /* Add mnemonic if it exists: */
    if (*menu_mnemonic != '\0') {
	/* Convert mnemonic from String type to KeySym type: */
	from.size = strlen (from.addr = menu_mnemonic);
	to.addr = NULL;
	if (!XtConvertAndStore (menuButton, XmRString, &from,
				XmRKeySym, &to))
	    VError ("VXAddMenu : Error in converting mnemonic to KeySym");

	/* Set the mnemonic: */
	XtVaSetValues (menuButton, XmNmnemonic, (* (KeySym *) to.addr),
		       (char *) NULL);
    }

    /* Create a pulldown menu for storing menu entries: */
    menu = XmCreatePulldownMenu (VX_App.x.menuBar, "pullDownMenu", NULL, 0);

    /* Add the menu entries to the pull down menu: */
    menu_entry_label = va_arg (ap, VString);
    while (menu_entry_label != NULL) {
	accelerator = va_arg (ap, VString);
	menu_callback = va_arg (ap, VXMenuCallback);
	client_data = va_arg (ap, VPointer);

	AddMenuEntry (menu, menu_entry_label, accelerator,
		      menu_callback, client_data);

	menu_entry_label = va_arg (ap, VString);
    }

    va_end (ap);

    /* Inform the menu button of its associated menu: */
    XtVSV (menuButton, XmNsubMenuId, menu, (char *) NULL);
}
