/*
 *  $Id: Synth.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *  This file contains routines for synthesizing images.
 */

/*
 *  Copyright 1994 University of British Columbia
 *
 *  Permission to use, copy, modify, distribute, and sell this software and its
 *  documentation for any purpose is hereby granted without fee, provided that
 *  the above copyright notice appears in all copies and that both that
 *  copyright notice and this permission notice appear in supporting
 *  documentation. UBC makes no representations about the suitability of this
 *  software for any purpose. It is provided "as is" without express or
 *  implied warranty.
 *
 *  Author: Art Pope, UBC Laboratory for Computational Intelligence
 */

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/os.h>

/* From the standard C libaray: */
#include <math.h>

/* File identification string: */
VRcsId ("$Id: Synth.c 3629 2009-08-20 17:04:30Z proeger $");


/*
 *  VRampImage
 *
 *  Fills an image with a ramp of pixel values.
 */

VBoolean VRampImage (VImage image, VBand band, double origin[2],
		     double orientation, double base, double slope)
{
    int first_band, last_band, row, column;
    double row_slope = slope * - sin (orientation);
    double column_slope = slope * cos (orientation);

    if (! VSelectBand ("VRampImage", image, band, NULL, NULL))
	return FALSE;
    if (band == VAllBands) {
	first_band= 0;
	last_band = VImageNBands (image) - 1;
    } else first_band = last_band = band;

    for (band = first_band; band <= last_band; band++)
	for (row = 0; row < VImageNRows (image); row++)
	    for (column = 0; column < VImageNColumns (image); column++)
		VSetPixel (image, band, row, column,
			   (row - origin[0]) * row_slope +
			   (column - origin[1]) * column_slope + base);

    return TRUE;
}


/*
 *  VSineGratingImage
 *
 *  Fills an image with a sine grating of specified amplitude, orientation,
 *  phase, and frequency.
 */

VBoolean VSineGratingImage (VImage image, VBand band, double origin[2],
			    double amplitude, double base,
			    double frequency, double orientation, double phase)
{
    int first_band, last_band, row, column;
    double row_slope = frequency * - sin (orientation);
    double column_slope = frequency * cos (orientation);
    double t;

    if (! VSelectBand ("VSineGratingImage", image, band, NULL, NULL))
	return FALSE;
    if (band == VAllBands) {
	first_band= 0;
	last_band = VImageNBands (image) - 1;
    } else first_band = last_band = band;

    for (band = first_band; band <= last_band; band++)
	for (row = 0; row < VImageNRows (image); row++)
	    for (column = 0; column < VImageNColumns (image); column++) {
		t = (row - origin[0]) * row_slope +
		    (column - origin[1]) * column_slope;
		VSetPixel (image, band, row, column,
			   sin (t + phase) * amplitude + base);
	    }

    return TRUE;
}


/*
 *  VZonePlateImage
 *
 *  Fills an image with the pattern f(r) = cos (r * r).
 */

VBoolean VZonePlateImage (VImage image, VBand band, double origin[2],
			  double amplitude, double base,
			  double frequency, double phase)
{
    int first_band, last_band, row, column;
    double dx, dy, t, cx = M_PI / VImageNRows (image) * frequency;

    if (! VSelectBand ("VSineGratingImage", image, band, NULL, NULL))
	return FALSE;
    if (band == VAllBands) {
	first_band= 0;
	last_band = VImageNBands (image) - 1;
    } else first_band = last_band = band;

    for (band = first_band; band <= last_band; band++)
	for (row = 0; row < VImageNRows (image); row++)
	    for (column = 0; column < VImageNColumns (image); column++) {
		dx = row - origin[0];
		dy = column - origin[1];
		t = cos ((dx * dx + dy * dy) * cx + phase);
		VSetPixel (image, band, row, column, t * amplitude + base);
	    }

    return TRUE;
}


/*
 *  VBinomialNoiseImage
 *
 *  Fills an image with pixel values of v0 and v1, chosen at random according
 *  to a binomial distribution with P(v0) = p.
 */

VBoolean VBinomialNoiseImage (VImage image, VBand band,
			      VDoublePromoted v0, VDoublePromoted v1, double p)
{
    int first_band, last_band, row, column;

    if (! VSelectBand ("VSineGratingImage", image, band, NULL, NULL))
	return FALSE;
    if (band == VAllBands) {
	first_band= 0;
	last_band = VImageNBands (image) - 1;
    } else first_band = last_band = band;

    for (band = first_band; band <= last_band; band++)
	for (row = 0; row < VImageNRows (image); row++)
	    for (column = 0; column < VImageNColumns (image); column++)
		VSetPixel (image, band, row, column,
			   VRandomDouble () < p ? v0 : v1);

    return TRUE;
}


/*
 *  VNormalNoiseImage
 *
 *  Fills an image with pixel values drawn at random from a normal
 *  distribution with the specified mean and standard deviation.
 */

VBoolean VNormalNoiseImage (VImage image, VBand band,
			    double mean, double std_dev)
{
    int first_band, last_band, row, column;
    double u1, u2, s = 0.0, t = 0.0, v;
    VBoolean phase = FALSE;

    if (! VSelectBand ("VNormalNoiseImage", image, band, NULL, NULL))
	return FALSE;
    if (band == VAllBands) {
	first_band= 0;
	last_band = VImageNBands (image) - 1;
    } else first_band = last_band = band;

    for (band = first_band; band <= last_band; band++)
	for (row = 0; row < VImageNRows (image); row++)
	    for (column = 0; column < VImageNColumns (image); column++) {

		/* Numbers drawn from N(0,1) are generated in pairs from
		   pairs of numbers in U(0,1). */
		if (phase)
		    v = s * sin (t);
		else {
		    do {
			u1 = VRandomDouble ();
		    } while (u1 == 0.0);
		    do {
			u2 = VRandomDouble ();
		    } while (u2 == 0.0);
		    s = sqrt (log (u1) * -2.0);
		    t = u2 * 2.0 * M_PI;
		    v = s * cos (t);
		}
		phase = ! phase;
		VSetPixel (image, band, row, column, mean + v * std_dev);
	    }

    return TRUE;
}


/*
 *  VUniformNoiseImage
 *
 *  Fills an image with pixel values drawn at random from the uniform
 *  distribution [min_value,max_value).
 */

VBoolean VUniformNoiseImage (VImage image, VBand band,
			     VDoublePromoted min_value,
			     VDoublePromoted max_value)
{
    int first_band, last_band, row, column;
    double range = max_value - min_value;

    if (! VSelectBand ("VUniformNoiseImage", image, band, NULL, NULL))
	return FALSE;
    if (band == VAllBands) {
	first_band= 0;
	last_band = VImageNBands (image) - 1;
    } else first_band = last_band = band;

    /* So that, with truncation of integer values, range maps [0,1) onto
       [min_value, max_value]: */
    if (VIsIntegerRepn (VPixelRepn (image)))
	range += 1.0;

    for (band = first_band; band <= last_band; band++)
	for (row = 0; row < VImageNRows (image); row++)
	    for (column = 0; column < VImageNColumns (image); column++)
		VSetPixel (image, band, row, column,
			   VRandomDouble () * range + min_value);

    return TRUE;
}
