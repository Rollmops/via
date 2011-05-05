/*
 *  $Id: file.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *  This file implements file selection, opening, and closing.
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
#include "FsbGQS.h"

/* From the Vista library: */
#include <viaio/mu.h>

/* From the Motif toolkit: */
#include <Xm/FileSB.h>
#include <Xm/List.h>
#include <Xm/MwmUtil.h>

/* File identification string: */
VRcsId ("$Id: file.c 3629 2009-08-20 17:04:30Z proeger $");

/* Later in this file: */
static void FileSelectCB (Widget, XtPointer, XtPointer);
static void FileCancelCB (Widget, XtPointer, XtPointer);
static void FileDestroyCB (Widget, XtPointer, XtPointer);
static int TraverseObjects (View, VAttrList, int, void (*) ());
static void RecordObjects (View, int, int, VAttrListPosn *);
static void ListObjects (View, Widget);


/*
 *  ShowFileSelectDialog
 *
 *  Pop up file selection dialog.
 */

void ShowFileSelectDialog (View view)
{
    Widget dialog;

    if (view && view->file_shell)
	dialog = view->file_shell;
    else {
	dialog = XmCreateFileSelectionDialog (view ? view->view_shell :
					      topLevelShell,
					      "file_select", NULL, 0);
	XtAddCallback (XtParent (dialog), XmNdestroyCallback,
		       FileDestroyCB, (XtPointer) view);
	XtAddCallback (dialog, XmNokCallback, FileSelectCB,
		       (XtPointer) view);
	XtAddCallback (dialog, XmNcancelCallback,
		       FileCancelCB, (XtPointer) view);
	XtAddCallback (dialog, XmNhelpCallback,
		       ShowHelpDialog, (XtPointer) "File Selection");
	if (view)
	    view->file_shell = dialog;
	else nviews++;
    }
    XtManageChild (dialog);
}


/*
 *  File selection dialog callbacks.
 *
 *	FileSelectCB	OK pressed in dialog
 *	FileDestroyCB	dialog cancelled or destroyed via window manager
 */

/* ARGSUSED */
static void FileSelectCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data;
    char *filename;
    VAttrList attributes;

    /* Resolve any use of ~ in the filename entered: */
    filename = XmcFsbGetQualifyString (w);

    /* Open and display the file: */
    if (ReadFile (filename, & attributes)) {
	XtUnmanageChild (w);
	if (view)
	    CloseFile (view);			/* close previous file */
	else {
	    XtDestroyWidget (w);
	    view = CreateView (FALSE);		/* create new view */
	}
	view->filename = filename;
	view->attributes = attributes;
	ShowFile (view, -1);
	XtManageChild (view->view_shell);
    } else XtFree (filename);
}


/* ARGSUSED */
static void FileCancelCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data;

    if (view)
	XtUnmanageChild (view->file_shell);
    else XtDestroyWidget (w);	/* the dialog was for opening a new view */
}

/* ARGSUSED */
static void FileDestroyCB (Widget w, XtPointer cl_data, XtPointer ca_data)
{
    View view = (View) cl_data;

    if (view)
	view->file_shell = NULL;
    else nviews--;		/* the dialog was for opening a new view */
}


/*
 *  ReadFile
 *
 *  Read a Vista data file, returning the attributes found in it.
 */

VBoolean ReadFile (VStringConst filename, VAttrList *attributes)
{
    FILE *file;

    /* Read the file to be displayed: */
    if (strcmp (filename, "-") == 0)
	file = stdin;
    else if (! (file = fopen (filename, "r"))) {
	VWarning ("Unable to open %s", filename);
	return FALSE;
    }
    *attributes = VReadFile (file, NULL);
    if (file != stdin)
	fclose (file);
    return *attributes != NULL;
}


/*
 *  ShowFile
 *
 *  Update a view to display a list of objects just loaded from a file.
 */

void ShowFile (View view, int object)
{
    VStringConst name;
    VBoolean enable_reread;
    int i = 0;

    /* Set the window's title to the file name: */
    if (strcmp (view->filename, "-") == 0) {
	name = "Standard Input";
	enable_reread = FALSE;
    } else {
	name = strrchr (view->filename, '/');
	if (name)
	    name++;
	else name = view->filename;
	enable_reread = TRUE;
    }
    XtVaSetValues (view->view_shell, XmNiconName, name, XmNtitle, name,
		   (char *) NULL);
    view->short_name = VNewString (name);

    /* Enable the Reread menu item iff the file isn't <stdin>: */
    SetMenuSensitivity (view->menus[fileMenu], rereadButton, enable_reread);

    /* Count the number of displayable objects in the file: */
    view->nobjects = TraverseObjects (view, view->attributes, 0, NULL);

    /* Allocate storage for a list of displayable objects: */
    view->objects = VMalloc (view->nobjects * sizeof (ViewObject));

    /* Fill the list: */
    view->nobjects = 0;
    TraverseObjects (view, view->attributes, 0, RecordObjects);

    /* Fill the object selection list: */
    ListObjects (view, view->object_list);

    /* Select and display the specified object, the first image, or the
       first object of any kind: */
    if (view->nobjects) {
	if (object >= 0) {
	    for (i = 0; i < view->nobjects; i++)
		if (view->objects[i].level == 0 &&
		    view->objects[i].idx == object)
		    break;
	    if (i == view->nobjects)
		object = -1;
	}
	if (object < 0) {
	    for (i = 0; i < view->nobjects; i++)
		if (view->objects[i].type == VImageRepn)
		    break;
	    if (i == view->nobjects)
		i = 0;
	}
	view->objects[i].visible = TRUE;
	XmListSelectPos (view->object_list, i + 1, FALSE);
    }
    SelectEdge (view, FALSE, 0, 0, NULL, 0);
    UpdateView (view, imageChange | edgeSetChange);
}


static int TraverseObjects (View view, VAttrList list,
			    int level, void (*fnc) ())
{
    VAttrListPosn posn;
    VAttrList sublist;
    VImage image;
    VEdges edges;
    int idx = 0, n = 0;

    for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
	switch (VGetAttrRepn (& posn)) {

	case VEdgesRepn:
	    VGetAttrValue (& posn, NULL, VEdgesRepn, & edges);
	    sublist = VEdgesAttrList (edges);
	    break;

	case VImageRepn:
	    VGetAttrValue (& posn, NULL, VImageRepn, & image);
	    sublist = VImageAttrList (image);
	    break;

	default:
	    continue;
	}
	if (fnc)
	    (*fnc) (view, level, idx, & posn);
	n++;
	if (sublist)
	    n += TraverseObjects (view, sublist, level + 1, fnc);
	idx++;
    }
    return n;
}


static void RecordObjects (View view, int level, int idx, VAttrListPosn *posn)
{
    ViewObject *obj = & view->objects[view->nobjects];
    VStringConst name;
    static char buf[100];

    obj->type = VGetAttrRepn (posn);
    obj->level = level;
    obj->idx = idx;
    obj->visible = FALSE;
    obj->posn = *posn;

    /* Format a name for the object: */
    switch (VGetAttrRepn (posn)) {

    case VEdgesRepn:
	VGetAttrValue (posn, NULL, VEdgesRepn, & obj->edges);
	if (VGetAttr (VEdgesAttrList (obj->edges), VNameAttr, NULL,
		      VStringRepn, & name) == VAttrFound)
	    sprintf (buf, "%.40s (Edge Set)", name);
	else sprintf (buf, "Edge Set %d", view->nobjects);
	break;

    case VImageRepn:
	VGetAttrValue (posn, NULL, VImageRepn, & obj->image);
	if (VGetAttr (VImageAttrList (obj->image), VNameAttr, NULL,
		      VStringRepn, & name) == VAttrFound)
	    sprintf (buf, "%.40s (Image)", name);
	else sprintf (buf, "Image %d", view->nobjects);
	break;

    default:
	strcpy (buf, "<Unknown object type>");
    }
    obj->name = VNewString (buf);

    view->nobjects++;
}


/*
 *  ListObjects
 *
 *  Fill an XmList widget with a list of the objects in a particular file.
 */

static void ListObjects (View view, Widget w)
{
    int i;
    ViewObject *obj;
    XmString str;
    char buf[100];

    XmListDeleteAllItems (w);

    if (view->nobjects == 0) {

	XmListAddItem (w, noneString, 0);

    } else {

	for (i = 0, obj = view->objects; i < view->nobjects; i++, obj++) {
	    sprintf (buf, "%*s%s", obj->level * 2, "", obj->name);
	    str = XmStringCreateSimple (buf);
	    XmListAddItem (view->object_list, str, 0);
	    XmStringFree (str);
	}
    }

    XtVaSetValues (view->object_list, XmNsensitive, view->nobjects > 0,
		   (char *) NULL);
}

/*
 *  CloseFile
 *
 *  Close a view's currently-open file.
 */

void CloseFile (View view)
{
    int i;

    if (! view->filename)
	return;
    VFree ((VPointer) view->filename);
    VFree ((VPointer) view->short_name);
    VDestroyAttrList (view->attributes);
    for (i = 0; i < view->nobjects; i++)
	VFree ((VPointer) view->objects[i].name);
    VFree (view->objects);
    view->filename = NULL;
    view->short_name = NULL;
    view->attributes = NULL;
    view->nobjects = 0;
    view->image = NULL;
    view->band = 0;
}
