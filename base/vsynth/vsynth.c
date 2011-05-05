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
 * $Id: vsynth.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vsynth - produce a synthetic image

\par Description
vsynth  creates  a  one-band  image with a specified size, pixel representation, and
pattern of pixel values.
The program knows how to create several different types  of  patterns. 

\par Usage

        <code>vsynth</code>

        \param -out         output image
        \param -pattern     Specifies the type of pattern to be used to set pixel values.
        \param -nrows       number of rows in the output image
        \param -ncolumns    number of columns in the output image
        \param -repn        output pixel representation 
	(bit | ubyte | sbyte | short | long | float | double). Default: float.
        \param -name        Specifies the name  to  be  given  the  synthesized  image.  
	Default: ``image''.

\par Patterns
Available patterns are:

<ul>
<li><code>-pattern constant</code><br>
<code>-base b</code> <br>
All pixel values are set to b.

<li><code>-pattern impulse</code><br>
<code>-amplitude a -base b -origin i j</code><br>
The  pixel  at row i, column j is set to a; all others are set to b.

<li><code>-pattern ramp</code><br>
<code>-amplitude a -base b -origin i j -orientation d</code><br>
The image is filled with a ramp whose slope is a units per pixel. The ramp is
centered  at row i, column j, where it has the value b. The ramp increases in
the direction d, which is measured CCW  in  degrees  from  the  direction  of
increasing column number.

<li><code>-pattern sine</code> <br>
<code>-amplitude a -base b -origin i j -phase w</code><br>
The  image is filled with a sine grating. The grating has a DC component of b
and an amplitude of a (or 2a peak-to-peak). Its frequency and  direction  may
be  specified by any of three combinations of frequency (in cycles per pixel,
measured either in the direction of the grating or as horizontal and vertical
components),  period  (in  pixels per cycle), and orientation (in degrees CCW
from the direction of increasing column number).  The grating is  shifted  so
that at row i, column j it has phase w radians.

<li><code>-pattern zone</code><br>
<code>-amplitude a -base b -origin i j -period t -phase w </code><br>
The image is filled with the zone plate pattern defined by
f(r) = a cos(r^2/t + w) + b, where r is the radial distance from the origin at row i,  column j.


<li><code>-pattern uniform</code><br>
<code>-amplitude a -base b -seed s</code><br>
Pixels are set to random values drawn independently from [b,b+a] according to
a uniform distribution.  The random number generator is initialized with the seed s.


<li><code>-pattern normal</code><br>
<code>-amplitude a -base b -seed s</code><br>
Pixel values are set to random values drawn independently according to a normal 
(Gaussian) distribution with mean b and standard deviation a. The  random
number generator is initialized with the seed s.

<li><code>-pattern binomial</code><br>
<code>-amplitude a -base b -density p -seed s</code><br>
Pixel  values  are  drawn  at random from a binomial distribution. A pixel is
given the value b with probability 1 -p, and it is given the value  b+a  with
probability p.
</ul>

\par Known bugs
none.

\file vsynth.c
\author Arthur Pope, UBC Laboratory for Computational Intelligence
*/




/*
 *  The vsynth program, which synthesizes an image containing one of a
 *  repertoire of possible patterns.
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
 *  Author: Arthur Pope, UBC Laboratory for Computational Intelligence
 */

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"

/* From the standard C library: */
#include <math.h>


/* Types of pixel value patterns: */
enum {
    binomialPattern,
    constantPattern,
    impulsePattern,
    normalPattern,
    rampPattern,
    sineGratingPattern,
    uniformPattern,
    zonePlatePattern
};
static VDictEntry pattern_dict[] = {
    { "binomial", binomialPattern },
    { "constant", constantPattern },
    { "impulse", impulsePattern },
    { "normal", normalPattern },
    { "ramp", rampPattern },
    { "sine", sineGratingPattern },
    { "uniform", uniformPattern },
    { "zone", zonePlatePattern },
    { NULL }
};

/* Program options: */
static VLong type;
static VLong nrows = 256, ncolumns = 256;
static VLong repn = VFloatRepn;

static VStringConst object_name = "image";

static VDouble origin_arg[2] = { 0.0, 0.0 };
static double origin[2];
static VBoolean origin_found;

static VDouble amplitude = 1.0, base = 0.0;
static VDouble frequency = 0.0, orientation = 0.0, period = 8.0, phase = 0.0;
static VDouble freq[2] = { 0.0, 0.0 };
static VBoolean frequency_found, orientation_found, period_found;
static VBoolean freq_found[2];

static VDouble density = 0.5;
static VLong seed;

static VOptionDescRec options[] = {
    { "pattern", VLongRepn, 1, & type, VRequiredOpt, pattern_dict,
	  "Type of pixel value pattern" },
    { "nrows", VLongRepn, 1, & nrows, VOptionalOpt, NULL,
	  "Number of rows" }, 
    { "ncolumns", VLongRepn, 1, & ncolumns, VOptionalOpt, NULL,
	  "Number of columns" }, 
    { "repn", VLongRepn, 1, & repn, VOptionalOpt, VNumericRepnDict,
	  "Pixel representation" },
    { "name", VStringRepn, 1, & object_name, VOptionalOpt, NULL,
	  "Name to be given created image" },
    { "origin", VLongRepn, 2, origin, & origin_found, NULL,
	  "Origin of pattern" },
    { "amplitude", VDoubleRepn, 1, & amplitude, VOptionalOpt, NULL,
	  "Amplitude of pattern" },
    { "base", VDoubleRepn, 1, & base, VOptionalOpt, NULL,
	  "Amplitude of background or DC component" },
    { "frequency", VDoubleRepn, 1, & frequency, & frequency_found, NULL,
	  "Frequency of grating pattern (cycles/pixel)" },
    { "orientation", VDoubleRepn, 1, & orientation, & orientation_found, NULL,
	  "Orientation of pattern (degrees, CCW from X axis)" },
    { "period", VDoubleRepn, 1, & period, & period_found, NULL,
	  "Period of grating pattern (pixels/cycle)" },
    { "phase", VDoubleRepn, 1, & phase, VOptionalOpt, NULL,
	  "Phase of grating pattern at origin (in [0,period])" },
    { "xfreq", VDoubleRepn, 1, & freq[0], & freq_found[0], NULL,
	  "Frequency of grating in X direction (cycles/pixel)" },
    { "yfreq", VDoubleRepn, 1, & freq[1], & freq_found[1], NULL,
	  "Frequency of grating in Y direction (cycles/pixel)" },
    { "density", VDoubleRepn, 1, & density, VOptionalOpt, NULL,
	  "Dot density for -type binomial (in [0,1])" },
    { "seed", VLongRepn, 1, & seed, VOptionalOpt, NULL,
	  "Seed for initializing random number generator" }
};


/*
 *  Program entry point.
 */

int main (int argc, char **argv)
{
    time_t now;
    FILE *out_file;
    VImage result;
    VAttrList list;
    char prg[50];	
    sprintf(prg,"vsynth V%s", getVersion());
    fprintf (stderr, "%s\n", prg);

    /* Choose a random seed: */
    time (& now);
    VRandomSeed ((long) now);
    seed = VRandomLong ();

    /* Parse command line arguments: */
    VParseFilterCmd (VNumber (options), options, argc, argv, NULL, & out_file);

    /* Create an image of the specified size and pixel representation: */
    if (nrows <= 0)
	VError ("%s must be >= 0", "nrows");
    if (ncolumns <= 0)
	VError ("%s must be >= 0", "ncolumns");
    if(! (result = VCreateImage (1, nrows, ncolumns, repn)))
	exit (EXIT_FAILURE);

    /* If origin isn't specified, it defaults to the image centre: */
    if (origin_found) {
	origin[0] = origin_arg[0];
	origin[1] = origin_arg[1];
    } else {
	origin[0] = nrows / 2.0;
	origin[1] = ncolumns / 2.0;
    }

    /* Convert orientation to radians: */
    orientation *= M_PI / 180.0;

    /* Initialize the random number generator: */
    VRandomSeed (seed);

    /* Fill the image with the synthesized pattern: */
    switch (type) {

    case binomialPattern:
	VBinomialNoiseImage (result, 0, base, base + amplitude, 1.0 - density);
	break;

    case constantPattern:
	VFillImage (result, 0, base);
	break;

    case impulsePattern:
	VFillImage (result, 0, base);
	if (origin[0] >= 0 && origin[0] < VImageNRows (result) &&
	    origin[1] >= 0 && origin[1] < VImageNColumns (result))
	    VSetPixel (result, 0, (int) origin[0], (int) origin[1], amplitude);
	break;

    case normalPattern:
	VNormalNoiseImage (result, 0, base, amplitude);
	break;

    case rampPattern:
	VRampImage (result, 0, origin, orientation, base, amplitude);
	break;

    case sineGratingPattern:
	if ((frequency_found + period_found +
	     (freq_found[0] || freq_found[1]) > 1) ||
	    ((freq_found[0] || freq_found[1]) && orientation_found)) {
	    fprintf (stderr, "%s: Specify one of:\n", argv[0]);
	    fprintf (stderr, "\t-frequency and -orientation\n");
	    fprintf (stderr, "\t-period and -orientation\n");
	    fprintf (stderr, "\t-xfreq and -yfreq\n");
	    exit (EXIT_FAILURE);
	}
	if (period_found)
	    frequency = (period == 0) ? 0.0 : 2.0 * M_PI / period;
	else if (freq_found[0] || freq_found[1]) {
	    orientation = atan2 (freq[1], freq[0]);
	    frequency = sqrt (freq[0] * freq[0] + freq[1] * freq[1]);
	}
	VSineGratingImage (result, 0, origin, amplitude, base, frequency,
			   orientation, phase);
	break;

    case uniformPattern:
	VUniformNoiseImage (result, 0, base, base + amplitude);
	break;

    case zonePlatePattern:
	VZonePlateImage (result, 0, origin, amplitude, base,
			 (period == 0) ? DBL_MAX : 1.0 / period, phase);
    }

    /* Write the image to the output file: */
    list = VCreateAttrList ();
    VHistory(VNumber(options),options,prg,&list,&list);
    VAppendAttr (list, object_name, NULL, VImageRepn, result);
    if (! VWriteFile (out_file, list))
	exit (EXIT_FAILURE);
    
    fprintf (stderr, "%s: Created %d x %d %s image.\n",
	     argv[0], (int) nrows, (int) ncolumns, VRepnName ((int) repn));

    return EXIT_SUCCESS;
}
