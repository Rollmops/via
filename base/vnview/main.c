/****************************************************************
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id: main.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*
 *  $Id: main.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *  Top level code for the vxview program, which displays the contents
 *  of Vista data files under X Windows.
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* From the local directory: */
#include "vxview.h"

/* From the Vista library: */
#include "viaio/mu.h"
#include "viaio/option.h"

/* From Motif, X windows, and the Xt intrinsics: */
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/Xmu/Converters.h>
#include <X11/ShellP.h>
#include <X11/Xatom.h>
#include <Xm/CascadeBG.h>
#include <Xm/DialogS.h>
#include <Xm/MessageB.h>
#include <Xm/MwmUtil.h>
#include <Xm/PushBG.h>
#include <Xm/SashP.h>

/* File identification string: */
VRcsId ("$Id: main.c 3629 2009-08-20 17:04:30Z proeger $");

/* Later in this file: */
static void RealizeWrapper (Widget, Mask *, XSetWindowAttributes *);
static void InstallResourceConverters (void);
static void CvtStringToIntensity (XrmValue *, Cardinal *,
				  XrmValue *, XrmValue *);
static void CreateInitialViews (int, VStringConst *, int, int);
static void WarningHandler (VStringConst);
static void PopupWarning (void);


/*
 *  Application resources.
 */

static XtResource appResources[] = {

    /* Various things: */
    { "appDefaultsFileFound", "AppDefaultsFileFound", XtRBoolean,
	  sizeof (Boolean),
	  XtOffset (AppDataPtr, app_defaults_file_found), XtRBoolean, FALSE },
    { "selectRange", "Range", XtRInt, sizeof (int),
	  XtOffset (AppDataPtr, select_range), XtRString, "2" },
    { "size", "Size", XtRInt, sizeof (int),
	  XtOffset (AppDataPtr, size), XtRString, "256" },

    /* Colors: */
    { "hilite", "Foreground", XtRPixel, sizeof (Pixel),
	  XtOffset (AppDataPtr, hilite_color), XtRString, "red" },
    { "color0", "EdgeColor", XtRPixel, sizeof (Pixel),
	  XtOffset (AppDataPtr, edge_set_colors[0]), XtRString, "green" },
    { "color1", "EdgeColor", XtRPixel, sizeof (Pixel),
	  XtOffset (AppDataPtr, edge_set_colors[1]), XtRString, "blue" },
    { "color2", "EdgeColor", XtRPixel, sizeof (Pixel),
	  XtOffset (AppDataPtr, edge_set_colors[2]), XtRString, "yellow" },
    { "color3", "EdgeColor", XtRPixel, sizeof (Pixel),
	  XtOffset (AppDataPtr, edge_set_colors[3]), XtRString, "cyan" },
    { "color4", "EdgeColor", XtRPixel, sizeof (Pixel),
	  XtOffset (AppDataPtr, edge_set_colors[4]), XtRString, "magenta" },
    { "color5", "EdgeColor", XtRPixel, sizeof (Pixel),
	  XtOffset (AppDataPtr, edge_set_colors[5]), XtRString, "purple" },
    { "color6", "EdgeColor", XtRPixel, sizeof (Pixel),
	  XtOffset (AppDataPtr, edge_set_colors[6]), XtRString, "orange" },
    { "color7", "EdgeColor", XtRPixel, sizeof (Pixel),
	  XtOffset (AppDataPtr, edge_set_colors[7]), XtRString, "tomato4" }
};


/*
 *  Global variables.
 */

AppData appData;			/* values of application resources */
View views;				/* list of existing views */
int nviews;				/* number of views open or pending */
Display *myDisplay;			/* X windows display */
Widget topLevelShell;			/* application's top level shell */
Pixmap iconPixmap;			/* icon for shells */
XmString noneString;			/* the string (None) */
VColormap vcolormap;			/* Vista colormap */

static VString pendingWarning;

static XtRealizeProc orig_shell_realize; /* original Shell realize method */


/*
 *  main
 *
 *  Program entry point.
 */

int main (int argc, char **argv)
{
    static VArgVector in_filenames;
    static VBoolean in_found, size_found;
    static VLong object = -1, band, size;
    static VLong palette = None;
    static VDictEntry palette_dict[] = {
	{ "any", None },
	{ "default", XA_RGB_DEFAULT_MAP }, { "best", XA_RGB_BEST_MAP },
        { "gray", XA_RGB_GRAY_MAP }, { "grey", XA_RGB_GRAY_MAP },
	NULL
    };
    static VOptionDescRec options[] = {
	{ "in", VStringRepn, 0, & in_filenames, & in_found, NULL,
	      "File to display" },
        { "object", VLongRepn, 1, & object, VOptionalOpt, NULL,
	      "Object within file" },
        { "band", VLongRepn, 1, & band, VOptionalOpt, NULL,
	      "Band of image object" },
	{ "palette", VLongRepn, 1, & palette, VOptionalOpt,
	      palette_dict, "Color palette to use" },
	{ "size", VLongRepn, 1, & size, & size_found, NULL,
	      "Initial image window dimension" }
    };
    static XtActionsRec actions[] = {
        { "ImageMove", ImageMoveAction },
        { "ImageReport", ImageReportAction },
        { "ImageSelect", ImageSelectAction },
        { "ImageTrack", ImageTrackAction }
    };
    XtAppContext app_context;
    XVisualInfo vinfo_template;
    XEvent event;
    char prg[50];	
    sprintf(prg,"vnview V%s", getVersion());
    fprintf (stderr, "%s\n", prg);


    /* Motif creates shells for things like menus. To ensure they get the
       visual we're using for other application widgets, we wrap our
       own routine around the Realize method for Shell widgets: */
    orig_shell_realize = shellClassRec.core_class.realize;
    shellClassRec.core_class.realize = RealizeWrapper;

    /* Parse command line arguments, initializing X Windows with them: */
    if (! VParseCommand (VNumber (options), options, & argc, argv))
	goto Usage;
    topLevelShell = XtVaAppInitialize (& app_context, programClass, NULL, 0,
#if ! defined(XtSpecificationRelease) || XtSpecificationRelease < 5
				       (Cardinal *)
#endif
				       & argc, (String *) argv, NULL,
				       XtNheight, 10, XtNwidth, 10,
				       (char *) NULL);
    if (! VIdentifyFiles (VNumber (options), options, "in", & argc, argv, 0))
	goto Usage;
    if (argc > 1) {
	VReportBadArgs (argc, argv);
Usage:	VReportUsage (argv[0], VNumber (options), options, "file ...");
	fprintf (stderr, "    plus X Windows options.\n\n");
	exit (EXIT_FAILURE);
    }
    myDisplay = XtDisplay (topLevelShell);

    /* Any visual we use has to have the same depth as the root window
       because Motif assumes that: */
    vinfo_template.depth = DefaultDepthOfScreen (XtScreen (topLevelShell));
    vcolormap = VCreateColormap (XtScreen (topLevelShell), (Atom) palette,
				 VisualDepthMask, & vinfo_template);
    if (vcolormap)
	XtVaSetValues (topLevelShell,
		       XtNcolormap, VColormapColormap (vcolormap),
		       XtNvisual, VColormapVisual (vcolormap),
		       (char *) NULL);
    else VWarning ("Failed to get %s standard colormap",
		   XGetAtomName (XtDisplay (topLevelShell), (Atom) palette));

    /* Install actions and resource type converters: */
    XtAppAddActions (app_context, actions, XtNumber (actions));
    InstallResourceConverters ();

    /* Get application resources: */
    XtGetApplicationResources (topLevelShell, (XtPointer) & appData,
			       appResources, XtNumber (appResources), NULL, 0);
    if (! appData.app_defaults_file_found)
	VError ("X Windows application defaults file, \"%s\", not found",
		programClass);

    /* Any -size switch on the command line overrules the size
       application resource: */
    if (size_found)
	appData.size = size;

    noneString = XmStringCreateSimple ("(None)");
    XtRealizeWidget (topLevelShell);

    /* Create a pixmap for the shell icon: */
    iconPixmap = XmGetPixmap (XtScreen (topLevelShell), iconName,
			      BlackPixelOfScreen (XtScreen (topLevelShell)),
			      WhitePixelOfScreen (XtScreen (topLevelShell)));

    /* Create windows corresponding to the files named with -in: */
    CreateInitialViews (in_filenames.number,
			(VStringConst *) in_filenames.vector,
			(int) object, (int) band);

    /* Register our own handler for warnings: */
    VSetWarningHandler (WarningHandler);

    /* Handle events until there are no visible pop-ups: */
    while (nviews > 0) {

	if (pendingWarning)
	    PopupWarning ();

	XtAppNextEvent (app_context, & event);
	XtDispatchEvent (& event);
    }

    return EXIT_SUCCESS;
}


/*
 *  RealizeWrapper
 *
 *  Ensure that a Shell widget about to be realized is given the appropriate
 *  visual. If this isn't done, shell-derived widgets created by the Motif
 *  library for things like menus inherit the root window's visual because
 *  they are descended directly from the root window.
 */

static void RealizeWrapper (Widget wid, Mask *vmask,
			    XSetWindowAttributes *attr)
{
    ShellWidget w = (ShellWidget) wid;

    if (vcolormap)
	w->shell.visual = VColormapVisual (vcolormap);
    orig_shell_realize (wid, vmask, attr);
}


/*
 *  InstallResourceConverters
 *
 *  Install some Xt resource type converters.
 */

static void InstallResourceConverters (void)
{
    static XtConvertArgRec sw_cvt_args[] = {
        { XtBaseOffset, (XtPointer) XtOffset (Widget, core.parent),
	      sizeof (Widget) }
    };

    XtAddConverter (XmRString, XmRWidget, XmuCvtStringToWidget,
		    sw_cvt_args, XtNumber (sw_cvt_args));
    XtAddConverter (XmRString, XmRWindow, XmuCvtStringToWidget,
		    sw_cvt_args, XtNumber (sw_cvt_args));
    XtAddConverter (XmRString, IntensityRT, CvtStringToIntensity, NULL, 0);
}


/*
 *  CvtStringToIntensity
 *
 *  A resource type converter for converting the strings
 *  INTENSITY_ABSOLUTE and INTENSITY_SIGNED to the internal
 *  constants IntensityAbsolute and IntensitySigned.
 */

/* ARGSUSED */
static void CvtStringToIntensity (XrmValue *args, Cardinal *num_args,
				  XrmValue *from, XrmValue *to)
{
    static n;

    if (strcmp (from->addr, "INTENSITY_ABSOLUTE") == 0)
	n = IntensityAbsolute;
    else if (strcmp (from->addr, "INTENSITY_SIGNED") == 0)
	n = IntensitySigned;
    else {
	String params[1];
	Cardinal num_params = 1;

	params[0] = from->addr;
	XtWarningMsg ("CvtStringToIntensity", "badValue",
		      "XtToolkitError", "Bad intensity meaning \"%s\"",
		      params, & num_params);
	return;
    }
    to->size = sizeof (n);
    to->addr = (XtPointer) & n;
}


/*
 *  CreateInitialViews
 *
 *  Create views for files specified on the command line.
 */

static void CreateInitialViews (int nfilenames, VStringConst *filenames,
				int object, int band)
{
    View view;
    int i;
    VAttrList *attrs;
    VBoolean status;

    if (nfilenames == 0)
	VError ("CreateInitialViews: No filenames supplied");

    /* Before displaying anything, read all files to ensure they're okay: */
    attrs = VMalloc (nfilenames * sizeof (VAttrList));
    for (i = 0, status = TRUE; i < nfilenames; i++)
	status &= ReadFile (filenames[i], & attrs[i]);
    if (! status)
	exit (EXIT_FAILURE);

    /* Only if they're all okay, create views: */
    for (i = 0; i < nfilenames; i++) {
	view = CreateView (TRUE);
	view->filename = VNewString (filenames[i]);
	view->attributes = attrs[i];
	view->band = band;
	ShowFile (view, object);
	XtManageChild (view->view_shell);
    }
    VFree (attrs);
}


/*
 *  WarningHandler
 *
 *  Handler for a Vista warning, which reports it with a pop-up dialog.
 */

static void WarningHandler (VStringConst msg)
{
    int i;
    char *cp;

    /* Record the message without its "Program: " and "Warning: " prefixes: */
    for (i = 0; i < 2; i++)
	if (cp = strchr (msg, ':')) {
	    cp++;
	    if (*cp == ' ')
		cp++;
	    msg = cp;
	}
    i = pendingWarning ? strlen (pendingWarning) : 0;
    pendingWarning = VRealloc (pendingWarning, i + strlen (msg));
    strcpy (pendingWarning + i, msg);
}

static void PopupWarning (void)
{
    Widget dialog;
    Dimension width, height;
    Arg al[] = { { XmNmessageString },
		 { XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL } };
    Screen *screen = XtScreen (topLevelShell);

    al[0].value = (XtArgVal)
	XmStringCreateLtoR (pendingWarning, XmSTRING_DEFAULT_CHARSET);
    VFree (pendingWarning);
    pendingWarning = NULL;

    /* Create a dialog for displaying the message: */
    dialog =
	XmCreateWarningDialog (topLevelShell, "warning", al, XtNumber (al));
    XmStringFree ((XmString) al[0].value);
    XtVaSetValues (XtParent (dialog),
		   XmNmappedWhenManaged, FALSE,
		   XmNdeleteResponse, XmDESTROY, (char *) NULL);
    XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_HELP_BUTTON));
    XtAddCallback (dialog, XmNokCallback,
		   (XtCallbackProc) XtDestroyWidget, NULL);
    XtManageChild (dialog);

    /* Center the dialog: */
    XtVaGetValues (dialog, XmNheight, & height, XmNwidth, & width,
		   (char *) NULL);
    XtVaSetValues (dialog,
		   XmNx, (Position) (WidthOfScreen (screen) - width) / 2,
		   XmNy, (Position) (HeightOfScreen (screen) - height) / 2,
		   (char *) NULL);

    /* Make it visible: */
    XtSetMappedWhenManaged (XtParent (dialog), TRUE);
}


/*
 *  UnmanageCallback, DestroyCallback
 *
 *  Callbacks that pop down the dialog widget corresponding to client_data.
 */

/* ARGSUSED */
void UnmanageCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
    XtUnmanageChild ((Widget) client_data);
}

/* ARGSUSED */
void DestroyCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
    XtDestroyWidget ((Widget) client_data);
}


/*
 *  SetMenuSensitivity
 *
 *  Enable or disable a menu or menu item.
 */

void SetMenuSensitivity (Widget menu, int item, VBoolean sensitive)
{
    WidgetList children;
    Cardinal num_children;
    WidgetClass class;
    int i;

    /* Locate the item'th CascadeButtonGadget or PushButtonGadget
       in the menu: */
    XtVaGetValues (menu, XmNchildren, & children,
		   XmNnumChildren, & num_children, (char *) NULL);
    for (i = 0; i < num_children; i++) {
	class = XtClass (children[i]);
	if (class == xmCascadeButtonGadgetClass ||
	    class == xmPushButtonGadgetClass)
	    if (item)
		item--;
	    else {

		/* Set it's sensitivity resource: */
		XtVaSetValues (children[i], XmNsensitive, sensitive,
			       (char *) NULL);
		return;
	    }
    }

    VError ("Internal error in SetMenuSensitivity: Didn't find item");
}


/*
 *  MapWidgetView
 *
 *  Map a widget to the view containing it.
 */

View MapWidgetToView (Widget w)
{
    View view;

    /* Find the shell widget: */
    while (XtParent (w) != topLevelShell)
	w = XtParent (w);

    /* Look through current views to find the one with that shell: */
    for (view = views; view && view->view_shell != w; view = view->next) ;

    if (! view)
	VWarning ("Internal error in MapWidgetToView");

    return view;
}


/*
 *  FixButtonPane
 *
 *  Uses tricks from O'Reilly Vol. 6 to prevent keyboard traversal from
 *  including a pane's sash widgets, and to fix the height of a button
 *  area pane to its current height. Passed a button widget; its parent
 *  should be a form (containing the row of buttons), and its grandparent
 *  should be a paned window (containing the form as its bottom pane).
 */

void FixButtonPane (Widget button)
{
    Dimension h, m;
    Widget *children;
    int n;

    /* Fix the buttons widget to its current height plus some margin: */
    XtVaGetValues (button, XmNheight, & h, XmNmarginHeight, & m,
		   (char *) NULL);
    h += m + m;
    XtVaSetValues (XtParent (button),
		   XmNpaneMaximum, h, XmNpaneMinimum, h, (char *) NULL);

    /* Disable keyboard traversal for all of the pane's sashes: */
    XtVaGetValues (XtParent (XtParent (button)),
		   XmNchildren, & children, XmNnumChildren, & n,
		   (char *) NULL);
    while (n-- > 0)
	if (XmIsSash (children[n]))
	    XtVaSetValues (children[n], XmNtraversalOn, FALSE,
			   (char *) NULL);
}
