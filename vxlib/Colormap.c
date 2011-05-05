/*
 *  $Id: Colormap.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains functions for initializing a standard colormap for
 *  use by an X Windows application.
 *
 *  This code requires X11R5 with public patch 21 installed. Previous
 *  versions of Xmu produced bad standard colormaps.
 *
 *  ==> To do:
 *		Enforce an upper limit of 256 for nreds, ngreens, nblues by
 *		dividing them down while multiplying up their red_mult, etc.
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
 *  Authors: Dan Razzell, Art Pope
 *	     UBC Laboratory for Computational Intelligence
 */

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "colormap.h"
#include "viaio/mu.h"
#include "viaio/os.h"

/* From X Windows libraries: */
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/Xmu/StdCmap.h>

/* File identification string: */
VRcsId ("$Id: Colormap.c 3177 2008-04-01 14:47:24Z karstenm $");

/* Constants: */
#define maxXColorValue		65535		/* see XColor(3X11) man page */
#define minGrayShades		17		/* see AllocGrays() routine */
#define maxGrayShades		256		/* see AllocGrays() routine */
#define maxPrimaryShades	10		/* see CreateCustomCmap() */

/* Later in this file: */
static VColormap CreateMonoVCmap (Screen *);
static VColormap CreateStdVCmap (Screen *, Atom, int, XVisualInfo []);
static VBoolean GetStdCmap (Screen *, Atom, int, XVisualInfo [], VColormap *);
static VColormap CreateCustomCmap (Screen *, Atom, XVisualInfo *);
static void AllocGrays (VColormap);
static VColormap AllocVColormap (Screen *, Atom, XVisualInfo *,
				 XStandardColormap *);
static void SetRGBMultipliers (VColormap);
static VBoolean ExistsVisualForProperty (Atom, int, XVisualInfo *);
static XVisualInfo *GetBestVisual (Screen *, Atom, int, XVisualInfo *);
static XVisualInfo *GetDeepestVisual (int, XVisualInfo *, int);


/*----------------------------------------------------------------------*/
/*  Public functions							*/
/*----------------------------------------------------------------------*/

VColormap VCreateColormap (Screen *screen, Atom property,
			   long vinfo_mask, XVisualInfo *vinfo_template)
{
  XVisualInfo template, *vinfos;
  int nvinfos;
  VColormap vc;

  /* 1-bit deep screens are a special case because standard colormaps
     aren't used with them. */
  if (PlanesOfScreen (screen) == 1)
    return CreateMonoVCmap (screen);

  /* Get a list of all visuals consistent with the constraints specified
     in the vinfo template: */
  if (vinfo_template)
    template = *vinfo_template;
  else if (vinfo_mask) {
    VWarning ("VCreateColormap: No vinfo template supplied");
    vinfo_mask = 0;
  }
  vinfo_mask |= VisualScreenMask;
  template.screen = XScreenNumberOfScreen (screen);
  vinfos = XGetVisualInfo (DisplayOfScreen (screen), vinfo_mask,
			   & template, & nvinfos);
  if (! vinfos) {
    VWarning ("VCreateColormap: No visuals match arguments");
    return NULL;
  }

  if (property != None) {

    /* If a property was specified or obtained by default, get a standard
       colormap for it: */
    vc = CreateStdVCmap (screen, property, nvinfos, vinfos);
    if (! vc) {
      VWarning ("VCreateColormap: "
		"No visual suitable for requested property");
      return NULL;
    }
  } else {

    /* Otherwise, try for standard colormaps in the order DEFAULT then
       BEST then GRAY: */
    vc = CreateStdVCmap (screen, XA_RGB_DEFAULT_MAP, nvinfos, vinfos);
    if (! vc)
      vc = CreateStdVCmap (screen, XA_RGB_BEST_MAP, nvinfos, vinfos);
    if (! vc)
      vc = CreateStdVCmap (screen, XA_RGB_GRAY_MAP, nvinfos, vinfos);
  }

  /* If the colormap doesn't already contain a good selection of
     grays, allocate some: */
  if (vc)
    AllocGrays (vc);

  VFree (vinfos);
  return vc;
}


/*
 *  VDestroyColormap
 *
 *  Frees a VColormap and everything hanging off of it.
 *  Does not attempt to remove any installed standard colormaps.
 */

void VDestroyColormap (VColormap vc)
{
  unsigned long i, ncolors;
    
  if (! vc)
    return;
    
  /* If we allocated our own colormap, free it: */
  if (vc->stdcmap.killid == ReleaseByFreeingColormap)
    XFreeColormap (DisplayOfScreen (vc->screen), vc->stdcmap.colormap);
  else {

    /* Otherwise we have to free any individual colors we allocated. */
    if (vc->indcmap) {
      ncolors = (vc->stdcmap.red_max + 1) * (vc->stdcmap.green_max + 1) *
	(vc->stdcmap.blue_max + 1);
      for (i = 0; i < ncolors; i++)
	if (vc->indcmap_alloced[i])
	  XFreeColors (DisplayOfScreen (vc->screen),
		       vc->stdcmap.colormap, & vc->indcmap[i], 1, 0);
    }

    if (vc->invgmap) {
      for (i = 0; i < vc->ngrays; i++)
	if (vc->invgmap_alloced[i])
	  XFreeColors (DisplayOfScreen (vc->screen),
		       vc->stdcmap.colormap, & vc->invgmap[i], 1, 0);
    }
  }

  VFree (vc->indcmap);
  VFree (vc->indcmap_alloced);
  VFree (vc->invgmap);
  VFree (vc->invgmap_alloced);
  VFree (vc);
}


/*
 *  VColormapRGBPixel, VColormapGrayPixel
 *
 *  These return the pixel value for displaying a specified RGB color
 *  or shade of gray.
 */

void VColormapRGBPixel (VColormap vc, XColor *color)
{
  int r, g, b, t;

  if (vc->vinfo.class == StaticGray || vc->vinfo.class == GrayScale) {

    /* The visual allows only gray shades. Convert the RGB color to
       a gray shade and compute the pixel value for that shade: */
    color->red = 0.299 * color->red + 0.587 * color->green +
      0.114 * color->blue;
    if (color->red > maxXColorValue)
      color->red = maxXColorValue;
    VColormapGrayPixel (vc, color);

  } else {

    r = ((long) color->red * vc->stdcmap.red_max +
	 (maxXColorValue / 2)) / maxXColorValue;
    g = ((long) color->green * vc->stdcmap.green_max +
	 (maxXColorValue / 2)) / maxXColorValue;
    b = ((long) color->blue * vc->stdcmap.blue_max +
	 (maxXColorValue / 2)) / maxXColorValue;
    t = r * vc->stdcmap.red_mult + g * vc->stdcmap.green_mult +
      b * vc->stdcmap.blue_mult;
    color->pixel = vc->indcmap ?
      vc->indcmap[t] : (t + vc->stdcmap.base_pixel) & 0xFFFFFFFF;
    color->red = r * maxXColorValue / vc->stdcmap.red_max;
    color->green = g * maxXColorValue / vc->stdcmap.green_max;
    color->blue = b * maxXColorValue / vc->stdcmap.blue_max;
  }
}

void VColormapGrayPixel (VColormap vc, XColor *color)
{
  int r = ((long) color->red * (vc->ngrays - 1) +
	   (maxXColorValue / 2)) / maxXColorValue;

  color->pixel = vc->invgmap[r];
  color->red = color->green = color->blue =
    r * maxXColorValue / (vc->ngrays - 1);
}


/*----------------------------------------------------------------------*/
/*  Private functions for color map allocation.				*/
/*----------------------------------------------------------------------*/

/*
 *  CreateMonoVCmap
 *
 *  Set up a VColormap for a 1-bit deep screen.
 */

static VColormap CreateMonoVCmap (Screen *screen)
{
  VColormap vc = AllocVColormap (screen, None, NULL, NULL);

  vc->vinfo.visual = DefaultVisualOfScreen (screen);
  vc->vinfo.visualid = XVisualIDFromVisual (vc->vinfo.visual);
  vc->vinfo.screen = XScreenNumberOfScreen (screen);
  vc->vinfo.depth = 1;
  vc->vinfo.class = StaticGray;
  vc->vinfo.red_mask = vc->vinfo.green_mask = vc->vinfo.blue_mask = 0;
  vc->vinfo.colormap_size = 2;
  vc->vinfo.bits_per_rgb = 1;
  vc->stdcmap.colormap = DefaultColormapOfScreen (screen);
  vc->stdcmap.red_max = vc->stdcmap.red_mult = 1;
  vc->stdcmap.green_max = vc->stdcmap.green_mult = 0;
  vc->stdcmap.blue_max = vc->stdcmap.blue_mult = 0;
  vc->stdcmap.killid = vc->stdcmap.visualid = vc->vinfo.visualid;
  vc->ngrays = 2;
  vc->invgmap = VMalloc (2 * sizeof (unsigned long));
  vc->invgmap[0] = BlackPixelOfScreen (screen);
  vc->invgmap[1] = WhitePixelOfScreen (screen);
  vc->invgmap_alloced = VMalloc (2 * sizeof (VBoolean));
  vc->invgmap_alloced[0] = vc->invgmap_alloced[1] = FALSE;
  return vc;
}


/*
 *  CreateStdVCmap
 *
 *  Tries to locate or create a standard colormap as described by the
 *  specified property. The standard colormap must be for one of the
 *  visuals listed in vinfos. The strategy is:
 *    (a) use an existing standard colormap if a suitable one has been
 *	  registered as a root window property
 *    (b) create and register a standard colormap if one of that type
 *	  hasn't already been registered for another visual (or if we're
 * 	  looking for a RGB_DEFAULT_MAP one, for which there can be
 *	  multiple registered)
 *    (c) if we can't create and register one because another of that
 *	  type is already registered, create a private one and don't
 *	  register it
 *    (d) allocate colors ourselves, getting a random set of pixel
 *	  values, and set up an indirect colormap to map them
 */

static VColormap CreateStdVCmap (Screen *screen, Atom property,
				 int nvinfos, XVisualInfo vinfos[])
{
  VBoolean prop_defined;
  XVisualInfo *best_vinfo;
  VColormap vc;
  unsigned long red_max, green_max, blue_max;
  XStandardColormap *stdcmap;
#if 0
  static char *visual_class_name[6] = {
    "StaticGray", "GrayScale", "StaticColor", "PseudoColor",
    "TrueColor", "DirectColor"
  };
#endif

  /* Ensure that the property is consistent with at least one of the
     visuals we've been given: */
  if (! ExistsVisualForProperty (property, nvinfos, vinfos))
    return NULL;

  /* Try to use a standard colormap that has already been defined for
     the property: */
  prop_defined = GetStdCmap (screen, property, nvinfos, vinfos, & vc);
  if (prop_defined && vc)
    return vc;

  /* If none is already defined, try to create one. Choose our best
     visual for it: */
  best_vinfo = GetBestVisual (screen, property, nvinfos, vinfos);

  if ( ! prop_defined || property == XA_RGB_DEFAULT_MAP) {

    /* There is not already a standard colormap with the property
       we're interested in, or if there is its an RGB_DEFAULT_MAP
       (of which there are allowed to be multiple registered).
       Try creating and registering one: */
    if (XmuLookupStandardColormap (DisplayOfScreen (screen),
				   XScreenNumberOfScreen (screen),
				   best_vinfo->visualid,
				   best_vinfo->depth,
				   property, FALSE, TRUE)) {
      if (GetStdCmap (screen, property, 1, best_vinfo, & vc) && vc) {
#if 0
	VWarning ("VCreateColormap: "
		  "Created %d-bit %s %s", best_vinfo->depth,
		  visual_class_name[best_vinfo->class],
		  XGetAtomName (DisplayOfScreen (screen), property));
#endif
	return vc;
      }
      VWarning ("VCreateColormap: Internal error in CreateStdVCmap");
    }
  } else {

    /* There's already a standard colormap registered but its visual
       isn't one we can use. Try creating one but not registering it: */
    if (! XmuGetColormapAllocation (best_vinfo, property,
				    & red_max, & green_max, & blue_max))
      return NULL;
    stdcmap = XmuStandardColormap (DisplayOfScreen (screen),
				   XScreenNumberOfScreen (screen),
				   best_vinfo->visualid,
				   best_vinfo->depth,
				   property, None,
				   red_max, green_max, blue_max);
    if (stdcmap) {
      vc = AllocVColormap (screen, property, best_vinfo, stdcmap);
      XFree ((char *) stdcmap);
      return vc;
    }
  }

  /* If that failed, its usually that we're trying to create an
     RGB_DEFAULT_MAP for the screen's default visual, in which case
     XmuLookupStandardColormap would have tried to use the default
     colormap rather than allocate a new one. It would be nice to use
     the default colormap to avoid flashing, so we try allocating colors
     individually, and not necessarily contiguously. */
  return CreateCustomCmap (screen, property, best_vinfo);
}


/*
 *  GetStdCmap
 *
 *  Look among properties on the root window for a standard colormap
 *  defined for one of the visuals of interest.
 */

static VBoolean GetStdCmap (Screen *screen, Atom property, int nvinfos,
			    XVisualInfo vinfos[], VColormap *vc)
{
  XStandardColormap *stdcmaps;
  int nstdcmaps, i, j, k;
  XVisualInfo *vinfo_subset = VMalloc (nvinfos * sizeof (XVisualInfo));
  XVisualInfo *best_vinfo;

  /* Find out what standard colormaps have been defined for the
     specified property: */
  if (! XGetRGBColormaps (DisplayOfScreen (screen),
			  RootWindowOfScreen (screen),
			  & stdcmaps, & nstdcmaps, property))
    return FALSE;	/* none defined */

  /* Sanity check: complain if there's more than one unless they're
     DEFAULT maps: */
  if (nstdcmaps > 1 && property != XA_RGB_DEFAULT_MAP)
    VWarning ("VCreateColormap: Multiple %s standard colormaps",
	      XGetAtomName (DisplayOfScreen (screen), property));

  /* Select the subset of visuals mentioned in the standard colormaps: */
  for (i = j = 0; i < nvinfos; i++)
    for (k = 0; k < nstdcmaps; k++)
      if (stdcmaps[k].visualid == vinfos[i].visualid)
	vinfo_subset[j++] = vinfos[i];

  /* Select the best of the subset according to the property: */
  best_vinfo = GetBestVisual (screen, property, j, vinfo_subset);
  if (best_vinfo) {
	
    /* Note the standard colormap that goes with that visual: */
    for (i = 0; i < nstdcmaps; i++)
      if (stdcmaps[i].visualid == best_vinfo->visualid)
	break;
    if (i == nstdcmaps)
      VError ("VCreateColormap: Internal error in GetStdCmap");
	
    /* Create a VColormap using that standard colormap: */
    *vc = AllocVColormap (screen, property, best_vinfo, & stdcmaps[i]);
  } else *vc = NULL;

  VFree (stdcmaps);
  VFree (vinfo_subset);
  return TRUE;
}


/*
 *  CreateCustomCmap
 *
 *  Allocate a palette of colors, not necessarily contiguously, and return
 *  a VColormap describing them.
 */

static VColormap CreateCustomCmap (Screen *screen, Atom property,
				   XVisualInfo *vinfo)
{
  unsigned long red_max, green_max, blue_max;
  unsigned long nreds, ngreens, nblues, ncolors;
  unsigned long r, g, b, *pp;
  unsigned short rvalue, gvalue;
  XColor color;
  VBoolean *ap, alloc_success = TRUE;
  VColormap vc;

  /* Use Xmu's rule of thumb for deciding how many colors to use
     with this property and visual: */
  if (! XmuGetColormapAllocation (vinfo, property,
				  & red_max, & green_max, & blue_max))
    return NULL;

  /* Limit the numbers because we're probably allocating them in the
     default colormap, and also because we don't want to allocate storage
     for too large an indirect colormap: */
  red_max = VMin (red_max, maxPrimaryShades - 1);
  green_max = VMin (green_max, maxPrimaryShades - 1);
  blue_max = VMin (blue_max, maxPrimaryShades - 1);
       
  nreds = red_max + 1;
  ngreens = green_max + 1;
  nblues = blue_max + 1;
  ncolors = nreds * ngreens * nblues;

  /* Allocate and initialize a VColormap incorporating an indirect index: */
  vc = AllocVColormap (screen, property, vinfo, NULL);
  pp = vc->indcmap = VMalloc (ncolors * sizeof (unsigned long));
  ap = vc->indcmap_alloced = VMalloc (ncolors * sizeof (VBoolean));

  /* Simulate a standard colormap: */
  vc->stdcmap.red_max = red_max;
  vc->stdcmap.green_max = green_max;
  vc->stdcmap.blue_max = blue_max;
  SetRGBMultipliers (vc);
  vc->stdcmap.base_pixel = 0;
  vc->stdcmap.visualid = vinfo->visualid;
  if (property == XA_RGB_DEFAULT_MAP &&
      vinfo->visual == DefaultVisualOfScreen (screen)) {
    vc->stdcmap.killid = RootWindowOfScreen (screen);
    vc->stdcmap.colormap = DefaultColormapOfScreen (screen);
  } else {
    vc->stdcmap.killid = ReleaseByFreeingColormap;
    vc->stdcmap.colormap = XCreateColormap (DisplayOfScreen (screen),
					    RootWindowOfScreen (screen),
					    vinfo->visual, AllocNone);
  }

  /* Allocate colors. Allocation of a color could fail if the colormap
     is already completely allocated with read/write cells, which may
     happen with the default colormap. A failed color's pixel value is set
     to black (which we take to be black as defined in the default
     colormap). */
  for (r = 0; r < nreds; r++) {
    rvalue = (r * maxXColorValue) / red_max;
    for (g = 0; g < ngreens; g++) {
      gvalue = (g * maxXColorValue) / green_max;
      for (b = 0; b < nblues; b++) {
	color.red = rvalue;
	color.green = gvalue;
	color.blue = (b * maxXColorValue) / blue_max;
	if (XAllocColor (DisplayOfScreen (screen), 
			 vc->stdcmap.colormap, & color)) {
	  *pp++ = color.pixel;
	  *ap++ = TRUE;
	} else {
	  *pp++ = BlackPixelOfScreen (screen);
	  alloc_success = *ap++ = FALSE;
	}
      }
    }
  }
  if (! alloc_success)
    VWarning ("VInitColormap: Unable to allocate colors");

  return vc;
}


/*
 *  AllocGrays
 *
 *  Try to get ensure that there are at least a few gray shades allocated
 *  so that rendering of grayscale images is okay.
 */

static void AllocGrays (VColormap vc)
{
  unsigned long true_ngrays, near_ngrays, reqd_ngrays;
  unsigned long nreds, ngreens, nblues, i, t, *pp;
  XColor color;
  VBoolean *ap, alloc_success = TRUE;
    
  /* Determine how many neutral gray shades can be represented by
     what's already been allocated:
     true_ngrays - the number of true, neutral gray shades
     near_ngrays - the number of approximately neutral gray shades */
  if (vc->property == XA_RGB_GRAY_MAP &&
      (vc->vinfo.class != TrueColor && vc->vinfo.class != DirectColor))
    true_ngrays = near_ngrays = vc->stdcmap.red_max +
      vc->stdcmap.green_max + vc->stdcmap.blue_max + 1;
  else {
    true_ngrays = (vc->stdcmap.red_max == vc->stdcmap.green_max &&
		   vc->stdcmap.green_max == vc->stdcmap.blue_max) ?
      vc->stdcmap.red_max + 1 : 0;
    near_ngrays = VMax (vc->stdcmap.red_max, vc->stdcmap.green_max);
    near_ngrays = VMax (vc->stdcmap.blue_max, near_ngrays) + 1;
  }

  /* Determine how many we ought to have. There's no point in trying
     for more than the visual will support. */
  reqd_ngrays = VMin (vc->vinfo.colormap_size, 1 << vc->vinfo.bits_per_rgb);
  reqd_ngrays = VMin (minGrayShades, reqd_ngrays);

  if (true_ngrays < reqd_ngrays &&
      vc->property != XA_RGB_BEST_MAP && vc->property != XA_RGB_GRAY_MAP) {

    /* If there aren't enough neutral gray shades among the colors we've
       already got, and this isn't a BEST or GRAY map (where we wouldn't
       have a hope of allocating more colors), try allocating some
       additional neutral gray shades. */

    vc->ngrays = reqd_ngrays;
    pp = vc->invgmap = VMalloc (reqd_ngrays * sizeof (unsigned long));
    ap = vc->invgmap_alloced = VMalloc (reqd_ngrays * sizeof (VBoolean));
    t = reqd_ngrays - 1;
    for (i = 0; i < reqd_ngrays; i++) {
      color.red = color.green = color.blue = (i * maxXColorValue) / t;
      if (XAllocColor (DisplayOfScreen (vc->screen), 
		       vc->stdcmap.colormap, & color)) {
	*pp++ = color.pixel;
	*ap++ = TRUE;
      } else {
	*pp++ = BlackPixelOfScreen (vc->screen);
	alloc_success = *ap++ = FALSE;
      }
    }
    if (! alloc_success)
      VWarning ("VInitColormap: Unable to allocate gray shades");

  } else {

    /* Otherwise assemble a ramp of approximate greys using whatever colors
       we've already got. Use no more grays than the rendering code can
       support (256). */

    vc->ngrays = VMin (maxGrayShades, near_ngrays);
    vc->invgmap = VMalloc (vc->ngrays * sizeof (unsigned long));
    vc->invgmap_alloced = VMalloc (vc->ngrays * sizeof (VBoolean));
    memset (vc->invgmap_alloced, 0, vc->ngrays * sizeof (VBoolean));
    nreds = vc->stdcmap.red_max + 1;
    ngreens = vc->stdcmap.green_max + 1;
    nblues = vc->stdcmap.blue_max + 1;
    for (i = 0; i < vc->ngrays; i++) {
      t = i * nreds / vc->ngrays * vc->stdcmap.red_mult +
	i * ngreens / vc->ngrays * vc->stdcmap.green_mult +
	i * nblues / vc->ngrays * vc->stdcmap.blue_mult;
      vc->invgmap[i] = vc->indcmap ?
	vc->indcmap[t] : (t + vc->stdcmap.base_pixel) & 0xFFFFFFFF;
    }
  }
}


/*
 *  AllocVColormap
 *
 *  Allocate and initialize a VColormap.
 */

static VColormap AllocVColormap (Screen *screen, Atom property,
				 XVisualInfo *vinfo,
				 XStandardColormap *stdcmap)
{
  VColormap vc = VNew (struct V_ColormapRec);

  vc->screen = screen;
  vc->property = property;
  if (vinfo)
    vc->vinfo = *vinfo;
  if (stdcmap) {
    vc->stdcmap = *stdcmap;

    /* ==> XmuStandardColormap creates XA_RGB_GRAY_MAP standard colormaps
       with the red, green, and blue multipliers all set to 1, regardless
       of the visual. Consequently, pixel values are calculated incorrectly
       for some XA_RGB_GRAY_MAP visuals. Here we correct the
       multipliers. */
    if (property == XA_RGB_GRAY_MAP)
      SetRGBMultipliers (vc);
  }
  vc->indcmap = vc->invgmap = NULL;
  vc->indcmap_alloced = vc->invgmap_alloced = NULL;
  vc->ngrays = 0;
  return vc;
}


/*
 *  SetRGBMultipliers
 *
 *  Among the problems with Xmu's treatment of RBG_GRAY_MAP standard
 *  colormaps is an assumption in XmuStandardColormap that an RGB_GRAY_MAP
 *  will not be on a DirectColor visual -- it doesn't set red_mult,
 *  green_mult, blue_mult correctly for such stdcmaps. This routine
 *  sets them to what they should be.
 */

static void SetRGBMultipliers (VColormap vc)
{
#define lowbit(x) ((x) & (~(x) + 1))

  if (vc->vinfo.class == TrueColor || vc->vinfo.class == DirectColor) {
    vc->stdcmap.red_mult = lowbit (vc->vinfo.red_mask);
    vc->stdcmap.green_mult = lowbit (vc->vinfo.green_mask);
    vc->stdcmap.blue_mult = lowbit (vc->vinfo.blue_mask);
    if (vc->property == XA_RGB_GRAY_MAP) {
      vc->stdcmap.red_max =
	vc->vinfo.red_mask / vc->stdcmap.red_mult;
      vc->stdcmap.green_max =
	vc->vinfo.green_mask / vc->stdcmap.green_mult;
      vc->stdcmap.blue_max =
	vc->vinfo.blue_mask / vc->stdcmap.blue_mult;
    }
  } else if (vc->property == XA_RGB_GRAY_MAP) {
    vc->stdcmap.red_mult = vc->stdcmap.green_mult =
      vc->stdcmap.blue_mult = 1;
  } else {
    vc->stdcmap.red_mult = (vc->stdcmap.red_max > 0) ?
      (vc->stdcmap.green_max + 1) * (vc->stdcmap.blue_max + 1) : 0;
    vc->stdcmap.green_mult = (vc->stdcmap.green_max > 0) ?
      (vc->stdcmap.blue_max + 1) : 0;
    vc->stdcmap.blue_mult = (vc->stdcmap.blue_max > 0) ? 1 : 0;
  }

#undef lowbit
}


/*----------------------------------------------------------------------*/
/*  Private functions embodying policy for selecting visuals.		*/
/*----------------------------------------------------------------------*/

/*
 *  ExistsVisualForProperty
 *
 *  Returns TRUE iff a set of visuals includes some compatible with
 *  a specified property.
 */

static VBoolean ExistsVisualForProperty (Atom property, int nvinfos,
					 XVisualInfo *vinfos)
{
  int i;

  if (property == None)
    return TRUE;
  for (i = 0; i < nvinfos; i++) {
    switch (vinfos[i].class) {

    case StaticGray:
    case GrayScale:
      if (property == XA_RGB_GRAY_MAP)
	return TRUE;
      break;

    default:
      return TRUE;
    }
  }
  return FALSE;
}


/*
 *  GetBestVisual
 *
 *  Determines the best among a set of visuals according to a given property.
 *  Return any visual if all else fails.
 *  There should always be at least one visual.
 *  We do not allocate a copy of the object we return, so the caller is
 *  responsible for doing so if necessary.
 */

static XVisualInfo *GetBestVisual (Screen *screen, Atom property,
				   int nvinfos, XVisualInfo *vinfos)
{
  XVisualInfo *v1, *v2;
  int i;
    
  /* For an RGB_DEFAULT_MAP standard colormap, we prefer the screen's
     default visual: */
  if (property == XA_RGB_DEFAULT_MAP) {
    for (i = 0; i < nvinfos; i++)
      if (vinfos[i].visual == DefaultVisualOfScreen (screen))
	return & vinfos[i];
  }

  /* For an RGB_BEST_MAP standard colormap, we prefer the deepest
     TrueColor visual, else the deepest StaticColor visual: */
  if (property == XA_RGB_BEST_MAP)
    if ((v1 = GetDeepestVisual (nvinfos, vinfos, TrueColor)) ||
	(v1 = GetDeepestVisual (nvinfos, vinfos, StaticColor)))
      return v1;
    
  /* For an RGB_GRAY_MAP standard colormap, we prefer the deepest
     GrayScale visual, else the deepest StaticGray visual: */
  if (property == XA_RGB_GRAY_MAP)
    if ((v1 = GetDeepestVisual (nvinfos, vinfos, GrayScale)) ||
	(v1 = GetDeepestVisual (nvinfos, vinfos, StaticGray)))
      return v1;
    
  /* For any type of standard colormap, our next choice is the deepest
     DirectColor or PseudoColor, preferring a DirectColor. They both
     have read/write colormaps. */
  v1 = GetDeepestVisual (nvinfos, vinfos, DirectColor);
  v2 = GetDeepestVisual (nvinfos, vinfos, PseudoColor);
  if (v1) {
    if (! v2 || ((v1->red_mask | v1->green_mask | v1->blue_mask) + 1 >=
		 v2->colormap_size))
      return v1;
  }
  if (v2)
    return v2;

  /* Our last choice is the deepest TrueColor or StaticColor, not really
     caring which. They both have read-only collormaps. */
  v1 = GetDeepestVisual (nvinfos, vinfos, TrueColor);
  v2 = GetDeepestVisual (nvinfos, vinfos, StaticColor);
  if (v1) {
    if (! v2 || ((v1->red_mask | v1->green_mask | v1->blue_mask) + 1 >=
		 v2->colormap_size))
      return v1;
  }
  if (v2)
    return v2;
    
  /* If we get this far, there must be no visual compatible with the
     specified property. */
  return NULL;
}


/*
 *  GetDeepestVisual
 *
 *  Determines the visual of greatest depth in a given visual class.
 *  If no such visual exists, returns NULL.  
 */

static XVisualInfo *GetDeepestVisual (int nvinfos, XVisualInfo *vinfos,
				      int class)
{
  int	i;
  unsigned int maxdepth = 0;
  XVisualInfo *v = NULL;
    
  for (i = 0; i < nvinfos; i++, vinfos++)
    if (vinfos->class == class && vinfos->depth > maxdepth) {
      maxdepth = vinfos->depth;
      v = vinfos;
    }
    
  return v;
}
