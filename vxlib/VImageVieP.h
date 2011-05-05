/*
 *  $Id: VImageVieP.h 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains private definitions used by the VImageView widget.
 */

#ifndef V_VImageVieP_h
#define V_VImageVieP_h 1

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
 *  Authors: Arthur Pope, Daniel Ko, Dan Razzell,
 *	     UBC Laboratory for Computational Intelligence
 */

/* From the X Windows libraries: */
#include <X11/IntrinsicP.h>

/* Class public header file: */
#include "viaio/VImageView.h"

/* Superclass private header file: */
#include <X11/Xaw/SimpleP.h>

/* From the Vista library: */
#include "viaio/VImage.h"
#include "viaio/colormap.h"

/* For portability: */
#include <X11/Xfuncproto.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 *  Class record.
 */

typedef struct {
    int empty;
} VImageViewClassPart;

typedef struct V_ImageViewClassRec {
    CoreClassPart core_class;
    VImageViewClassPart v_image_view_class;
} VImageViewClassRec;

extern VImageViewClassRec vImageViewClassRec;


/*
 *  Instance record.
 */

typedef struct {

    /* Resources: */
    Boolean absolute;		/* 1: grey shade = abs pixel value */
    int band;			/* band to be displayed */
    int column_center;		/* column-coord. of zoom center */
    Cursor cursor;		/* cursor to display over image */
    XtCallbackList expose_callback; /* callbacks to draw over image */
    VImage image;		/* image to be displayed */
    XtCallbackList input_callback; /* callbacks to handle input */
    XtCallbackList move_zoom_center_callback; /* callbacks to handle  
						 move zoom center */
    Boolean proportion;		/* 1: maintain image proportions */
    Boolean resize;		/* 1: adjust widget size to image's */
    int row_center;		/* row-coord. of zoom center */
    Boolean use_pixmap;		/* cache image in pixmap at server */
    VColormap v_colormap;	/* info to guide color rendering */
    XtCallbackList zoom_in_callback; /* callbacks to handle zoom in */
    int zoom_level;		/* zoom level */
    XtCallbackList zoom_out_callback; /* callbacks to handle zoom out */
 
    /* Private state: */
    VBoolean render_needed;	/* image must yet be rendered */
    VFloat pixel_aspect_ratio;	/* image's pixel aspect ratio attribute */
    VBoolean is_color;		/* image is to be interpreted as RGB color */
    XImage *ximage;
    Pixmap pixmap;		/* pixmap used to cache image at server */
    unsigned int alloced_height;/* dimensions of alloc'ed ximage and pixmap */
    unsigned int alloced_width;
    GC gc;			/* GC for drawing to pixmap, window */
    struct V_Port {		/* portion of image currently shown: */
	int first_row;		/*   top left image row and column */
	int first_column;
	int nrows;		/*   numbers of image rows and columns */
	int ncolumns;
	unsigned int height;	/*   portion of window occupied */
	unsigned int width;
    } port;
    VBoolean free_vcolormap;	/* 1: widget created is VColormap */
    Window busy_window;         /* mapped when widget is busy */
} VImageViewPart;

typedef struct V_ImageViewRec {
    CorePart core;
    VImageViewPart viv;
} VImageViewRec;


/* Declarations of private functions: */

extern XImage *V_RenderVToX (
#if NeedFunctionPrototypes
    VImageViewWidget	/* vw */			     
#endif
);

#ifdef __cplusplus
}
#endif

#endif /* V_VImageVieP_h */
