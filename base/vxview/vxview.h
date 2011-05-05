/*
 *  $Id: vxview.h 3629 2009-08-20 17:04:30Z proeger $
 *
 *  Definitions associated with images: their representation in files and
 *  in memory, and operations that can be performed with them.
 */

#ifndef vxview_h
#define vxview_h 1

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

/* From X windows and the Xt intrinsics: */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/* From the Motif toolkit: */
#include <Xm/Xm.h>

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/VEdges.h"
#include "viaio/VImage.h"
#include "viaio/colormap.h"
#include "viaio/os.h"


/*
 *  Application resources.
 */

/* Representation type names for application-specific resources: */
#define IntensityRT		"Intensity"

/* Number of different colors available for drawing edge sets: */
#define numberEdgeSetColors	8

typedef struct {

    /* Various things: */
    Boolean app_defaults_file_found;	/* set TRUE by app-defaults file */
    int select_range;			/* edge selection range */
    int size;				/* default object window dimension */

    /* Colors: */
    Pixel hilite_color;			/* color for hiliting edge */
    Pixel edge_set_colors[numberEdgeSetColors];	/* colors for drawing edges */
} AppData, *AppDataPtr;

extern AppData appData;			/* application resources */


/*
 *  Display preferences.
 */

typedef struct {

    /* For images in particular: */
    struct {
	float brightness;		/* brightness adjustment */
	float contrast;			/* contrast adjustment */
	VBoolean ignore_BI;		/* ignore band_interp attribute */
	enum {
	    IntensityAbsolute,		/* intensity = abs pixel value */
	    IntensitySigned		/* intensity = signed pixel value */
	} intensity;
    } image;

    /* For edge sets in particular: */
    struct {
	VBoolean points; 		/* show edge points */
	VBoolean lines;			/* show edge lines */
	VBoolean endpoints;		/* show edge endpoints */
    } edge;
} Preferences;

extern Preferences defaultPrefs;	/* defaults for new views */


/*
 *  Displayable object.
 */

/* Types of objects vxview knows how to display: */
typedef enum {
    noObject, imageObject, edgeSetObject
} ViewObjectType;

/* Representation of one object found in a file: */
typedef struct {
    VRepnKind type;			/* image, edge set, etc. */
    int level;				/* level of nesting within file */
    int idx;				/* index number at that level */
    VStringConst name;			/* object's name */
    VAttrListPosn posn;			/* position in some attribute list */
    VBoolean visible;			/* whether currently displayed */
    VImage image;			/* if an image */
    VEdges edges;			/* if an edge set */
} ViewObject;


/*
 *  Menu structure.
 */

/* Menus within the menu bar: */
enum { fileMenu, viewMenu, helpMenu, numberMenus };

/* Buttons in the File menu: */
enum { newButton, removeButton, openButton, rereadButton, quitButton };

/* Buttons in the View menu: */
enum { attrsButton, displayButton, edgeDataButton };

/* Buttons in the Help menu: */
enum { helpIndexButton, helpVersionButton };


/*
 *  View of the objects in a file.
 */

typedef struct _View ViewRec, *View;
struct _View {

    /* For keeping a list of all open views: */
    View next;				/* next open view */

    /* Information about the file being displayed: */
    VStringConst filename;		/* name of the file */
    VStringConst short_name;		/* short version of filename */
    VAttrList attributes;		/* file attributes */
    int nobjects;			/* number of viewable objs in file */
    ViewObject *objects;		/* index of viewable objs in file */

    /* Information about the image and band being displayed: */
    ViewObject *image_object;		/* image object, if any, displayed */
    VImage image;			/* image, if any, being displayed */
    VImage tmp_image;			/* contrast-adjusted image */
    VBand band;				/* which band */
    VBoolean act_color;			/* whether image is actually color */
    VBoolean show_color;		/* whether image is shown as color */

    /* Currently selected edge: */
    VBoolean edge_selected;		/* whether an edge is selected */
    int sel_edges_idx, sel_edge_idx;	/* selected edge set, edge, and pt */
    int sel_pt_idx;
    VEdge sel_edge;
    double sel_row, sel_column;		/* row and column of last click */

    /* View preferences: */
    Preferences prefs;			/* view preferences */

    /* Widgets making up the view: */
    Widget view_shell;			/* view's shell */
    int size;				/* dimensions of object display area */
    Widget menu_bar;			/* menu bar XmRowColumn widget */
    Widget menus[numberMenus];		/* each menu's XmRowColumn widget */
    Widget object_list;			/* list of objects in file */
    Widget image_info;			/* blurb about the image */
    Widget band_0;			/* band 0 widget */
    Widget band_scale;			/* band selection XmScale widget */
    Widget band_n;			/* number of bands visual */
    Widget pixel_info;			/* pixel location and value */
    Widget edge_label;
    Widget edge_info;			/* edge identity */
    Widget point_label;
    Widget point_info;			/* edge point location and value */
    Widget image_view;			/* image view widget */
    GC gc;				/* GC for drawing over image */

    /* Widget making up the Attributes dialog: */
    Widget attr_shell;			/* attribute dialog's shell */
    Widget attr_text;			/* attribute dialog's text widget */

    /* Widgets making up the File Selection dialog: */
    Widget file_shell;			/* file selection dialog */

    /* Widgets making up the Preferences dialog: */
    Widget pref_shell;			/* preferences dialog's shell */
    Widget brightness_scale;		/* brightness scale */
    Widget contrast_scale;		/* contrast scale */
    Widget bi_toggle;			/* ignore band interpretation */
    Widget intensity_menu;		/* intensity option menu */
    Widget points_toggle;		/* display edge points */
    Widget lines_toggle;		/* diplsay lines between edge points */
    Widget endpoints_toggle;		/* display edge endpoints */
};

extern View views;			/* list of existing views */
extern int nviews;			/* number of views open or pending */


/*
 *  Miscellaneous stuff.
 */

#define programClass	"Vxview"
#define iconName	"vista_icon"

extern Display *myDisplay;		/* current display */
extern Widget topLevelShell;		/* top level shell */
extern Pixmap iconPixmap;		/* icon for shells */
extern XmString noneString;		/* the string (None) */
extern VColormap vcolormap;		/* Vista colormap */

#define defaultTitle 	"VXView"	/* default window title */

/* Arguments to UpdateView: */
enum {
    imageChange		= 0x01,		/* new choice of image */
    edgeSetChange	= 0x02,		/* new choice of edge sets */
    ignoreBIChange	= 0x04,		/* change whether band_interp used */
    intensityChange	= 0x08,		/* new abs/rel intensity choice */
    contrastChange	= 0x10,		/* new image brightness/contrast */
    edgeChange		= 0x20,		/* new edge display prefs */
    edgeSelChange	= 0x40		/* new edge selected */
};

/* Shorthand for some Xt routines: */
#define XVCW XtVaCreateWidget
#define XVCMW XtVaCreateManagedWidget
#define XVCPS XtVaCreatePopupShell


/*
 *  Procedure declarations.
 */

/* In attr.c: */
void ShowAttrDialog (View);

/* In edge.c: */
void SelectEdge (View, VBoolean, int, int, VEdge, int);
void ShowEdgeData (View);

/* In file.c: */
void ShowFileSelectDialog (View);
VBoolean ReadFile (VStringConst, VAttrList *);
void ShowFile (View, int);
void CloseFile (View);

/* In help.c: */
void ShowVersionDialog (View);
void ShowHelpDialog (Widget, XtPointer, XtPointer);

/* In main.c: */
void UnmanageCallback (Widget, XtPointer, XtPointer);
void DestroyCallback (Widget, XtPointer, XtPointer);
ViewObjectType TypeOfObject (VAttrListPosn *);
void SetMenuSensitivity (Widget, int, VBoolean);
View MapWidgetToView (Widget);
void FixButtonPane (Widget);

/* In pref.c: */
void ShowPrefDialog (View);

/* In view.c: */
View CreateView (VBoolean);
void DestroyView (Widget, XtPointer, XtPointer);
void ImageMoveAction (Widget, XEvent *, String *, Cardinal *);
void ImageReportAction (Widget, XEvent *, String *, Cardinal *);
void ImageSelectAction (Widget, XEvent *, String *, Cardinal *);
void ImageTrackAction (Widget, XEvent *, String *, Cardinal *);
void UpdateView (View, int);

#endif /* vxview_h */
