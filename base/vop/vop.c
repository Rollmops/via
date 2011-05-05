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
 * $Id: vop.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vop - perform arithmetic and logical operations on images

\par Description
vop performs arithmetic and logical operations on image pixels.  It has three modes:
<p>
1. It can perform a unary (single-operand) operation on each  pixel  of  an  input
image.
<p>
2. It  can perform a binary (two-operand) operation between each pixel of an input
image and a scalar value.
<p>
3. It can perform a binary (two-operand) operation between each pixel of an  input
image and the corresponding pixel of a second image.
The  mode,  operation,  and  operands  are specified by command line options.  Input
images come from a data file; the specified operation is performed on each  to  pro­
duce an output image of the same properties.
<p>
When  the  operation  is a binary one between two images (case 3, above), the second
input comes from a separate file, and it may have
the same number of bands as the first input image, in which case band i of  the
output image is formed from band i of both input image; or
formed from band i of the first input image and band 0 of the second.
<p>
By  means  of  the -min and -max options, you can specify bounds for clipping output
pixel values.
<p>
When -op specifies a binary operation, either -image or -value must be specified.
The and, or, and xor operations can only be performed  with  images  having  integer
pixel representations (not float or double).
A  floating point result (such as what exp produces) is rounded to the nearest inte­
ger for storing in an integer output pixel.
Clipping of output pixel values and arithmetic exceptions are reported by  means  of
warning messages.


\par Usage

        <code>vop</code>

        \param -in    Specifies a Vista data file containing the input images. These images
                      serve as the first operand of a binary operation, or the only operand
                      of a unary one.
        \param -out   output image

        \param -op    Specifies the operation. Possible values:
                      abs,exp,log,not,sqrt,square,add,and,dist,div,max,min,mult,or,sub,xor

        \param -value  Specifies  a  scalar  constant  to be used as the second operand 
                       of a binary operation.
        \param -image  Specifies a Vista data file containing a single image to serve as the
                       second operand of a binary operation.
        \param -min    Sets  a  lower  bound  for clipping output pixel values. Default: the
                       minimum value that can be represented by an output pixel.


        \param -max    Sets  an upper bound  for clipping output pixel values. Default: the
                       maximum value that can be represented by an output pixel.




\par Examples
<br>
\par Known bugs
none.

\file vop.c
\author Ralph Horstmann, UBC Laboratory for Computational Intelligence
*/



/*
 * 
 * The vop program performs arithmetic and logical operations on images.
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
 *  Authors:
 *  Ralph Horstmann and Art Pope, UBC Laboratory for Computational Intelligence
 */

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"
#include <signal.h>

/* From the standard C library: */
#include <math.h>

#ifdef sgi
#include <sigfpe.h>
#include <signal.h>
#endif

#ifdef SunOS_4
#include <signal.h>
#endif

#if defined (SunOS_5) && defined (HasSunC)
#include <floatingpoint.h>
#include <siginfo.h>
#include <sunmath.h>
#endif


/* Names for the operations supported by VImageOp: */
static VDictEntry op_dict[] = {
  { "abs", VImageOpAbs },
  { "add", VImageOpAdd },
  { "and", VImageOpAnd },
  { "dist", VImageOpDist },
  { "div", VImageOpDiv },
  { "exp", VImageOpExp },
  { "log", VImageOpLog },
  { "max", VImageOpMax },
  { "min", VImageOpMin },
  { "mult", VImageOpMult },
  { "not", VImageOpNot },
  { "or", VImageOpOr },
  { "sqrt", VImageOpSqrt },
  { "square", VImageOpSquare },
  { "sub", VImageOpSub },
  { "xor", VImageOpXor },
  { NULL }
};

#define Unary(op)							\
    (op == VImageOpAbs || op == VImageOpExp || op == VImageOpLog ||	\
     op == VImageOpNot || op == VImageOpSqrt || op == VImageOpSquare)

/* Record of errors signalled by exceptions: */
static struct {
  int int_division;
  int flt_division;
  int flt_overflow;
  int flt_underflow;
  int flt_inexact;
  int flt_invalid;
} error_counts;

/* Later in this file: */
#ifdef sgi
void SgiTrapHandler (int, int, void *);
#endif
#ifdef SunOS_4
static void SunFPEHandler (int, int, void *, char *);
#endif
#if defined (SunOS_5) && defined (HasSunC)
static void SunFPEHandler (int, siginfo_t *, void *);
#endif


/*
 *  Program entry point.
 */

int main (int argc, char *argv[])
{
  static VImageOpKind op;
  static VBoolean value_found, image_found, min_found, max_found;
  static VString image_filename;
  static VDouble value, min, max;
  static VOptionDescRec  options[] = {
    { "op", VLongRepn, 1, & op, VRequiredOpt, op_dict,
      "Operation to be performed" },
    { "value", VDoubleRepn, 1, & value, & value_found, NULL,
      "Constant operand for binary operation" },
    { "image", VStringRepn, 1, & image_filename, & image_found, NULL,
      "Image operand for binary operation" },
    { "min", VDoubleRepn, 1, & min, & min_found, NULL,
      "Clipping lower bound" },
    { "max", VDoubleRepn, 1, & max, & max_found, NULL,
      "Clipping upper bound" }
  };
  FILE *in_file, *out_file, *image_file = NULL;
  VAttrList list;
  VAttrListPosn posn;
  VImage *images, src;
  int nimages = 0;
  VDouble *minp, *maxp;
  char prg[50];	
  sprintf(prg,"vop V%s", getVersion());
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments and identify the input and output files: */
  VParseFilterCmd(VNumber (options), options, argc, argv,
		  & in_file, & out_file);
  minp = min_found ? & min : NULL;
  maxp = max_found ? & max : NULL;

  /* Check consistency between type of operation and operands provided: */
  if (Unary (op)) {
    if (value_found || image_found) {
      VWarning ("Unary operation requires neither -value nor -image");
      value_found = image_found = FALSE;
    }
  } else {
    if (! (value_found ^ image_found))
      VError ("Binary operation requires either -value or -image");
    if (image_found) {

      /* Load the image serving as the second operand: */
      if (strcmp (image_filename, "-") == 0)
	image_file = stdin;
      else if (! (image_file = fopen (image_filename, "r")))
	VError ("Failed to open image file %s", image_filename);
      if ((nimages = VReadImages (image_file, & list, & images)) == 0)
	exit (EXIT_FAILURE);
      if (nimages > 1)
	VWarning ("Using only the first image in %s", image_filename);
      VDestroyAttrList (list);
      fclose (image_file);
    }
  }

  /* Read source image(s): */
  if (! (list = VReadFile (in_file, NULL)))
    exit (EXIT_FAILURE);

  /* Set up a handlers for arithmetic and floating point exceptions: */
#ifdef SunOS_4
  if (ieee_handler ("set", "all", SunFPEHandler))
    VError ("ieee_handler() failed");
  SunFPEHandler (0, 0, NULL, NULL);
#endif

#if defined (SunOS_5) && defined (HasSunC)
#ifndef BADSIG
#define BADSIG (void (*)())-1
#endif
  if (ieee_handler ("set", "all", SunFPEHandler))
    VError ("ieee_handler() failed");
  if (sigfpe (FPE_INTDIV, SunFPEHandler) == BADSIG)
    VError ("sigfpe() failed");
#endif /* SunOS_5 && HasSunC */

  /* Operate on each source image: */
  nimages = 0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn)
      continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);
    memset (& error_counts, 0, sizeof (error_counts));
    if (value_found) {

      /* Operation between image and constant: */
      src = VImageOpV (src, src, VAllBands, op, value, minp, maxp);
    } else if (image_found) {

      /* Operation between two images: */
      src = VImageOpI (src, src, VAllBands, op, images[0],
		       VImageNBands (src) == VImageNBands (images[0]) ?
		       VAllBands : 0, minp, maxp);
    } else {

      /* Operation on single image: */
      src = VImageOpU (src, src, VAllBands, op, minp, maxp);
    }
    if (! src)
      exit (EXIT_FAILURE);

    /* Report any errors detected: */
#define ReportError(n,type)					\
	    if (n)							\
		VWarning ("%d %s%s on image %d", n, type,		\
			  n == 1 ? "" : "s", nimages + 1)

    ReportError (error_counts.int_division, "integer zero division");
    ReportError (error_counts.flt_division, "f.p. zero division");
    ReportError (error_counts.flt_overflow, "f.p. overflow");
    ReportError (error_counts.flt_underflow, "f.p. underflow");
    ReportError (error_counts.flt_inexact, "inexact f.p. result");
    ReportError (error_counts.flt_invalid, "invalid f.p. operation");

#undef ReportError

    nimages++;
  }

  /* Write the results to the output file: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list))
    exit (EXIT_FAILURE);
  fprintf (stderr, "%s: Processed %d image%s.\n",
	   argv[0], nimages, nimages == 1 ? "" : "s");
  return EXIT_SUCCESS;
}


/*
 *  Floating point and arithmetic exception handlers.
 */

#ifdef SunOS_4
static void SunFPEHandler (int sig, int code, void *scp, char *addr)
{
  switch (code) {
  case FPE_INTDIV_TRAP:   error_counts.int_division++; break;
  case FPE_FLTINEX_TRAP:  error_counts.flt_inexact++; break;
  case FPE_FLTDIV_TRAP:   error_counts.flt_division++; break;
  case FPE_FLTUND_TRAP:   error_counts.flt_underflow++; break;
  case FPE_FLTOPERR_TRAP: error_counts.flt_invalid++; break;
  case FPE_FLTOVF_TRAP:   error_counts.flt_overflow++; break;
  }
  if ((int) signal (SIGFPE, SunFPEHandler) == -1)
    VSystemError ("Call to signal() failed");
}

int matherr (struct exception *exc)
{
  switch (exc->type) {
  case DOMAIN:    error_counts.flt_invalid++; break;
  case SING:	    error_counts.flt_division++; break;
  case OVERFLOW:  error_counts.flt_overflow++; break;
  case UNDERFLOW: error_counts.flt_underflow++; break;
  }
  return 1;
}
#endif

#if defined (SunOS_5) && defined (HasSunC)
static void SunFPEHandler (int sig, siginfo_t *sip, void *uap)
{
  switch (sip->si_code) {

  case FPE_INTDIV:
    VError ("Divide by zero");

  case FPE_FLTDIV: error_counts.flt_division++; break;
  case FPE_FLTOVF: error_counts.flt_overflow++; break;
  case FPE_FLTUND: error_counts.flt_underflow++; break;
  case FPE_FLTRES: error_counts.flt_inexact++; break;
  case FPE_FLTINV: error_counts.flt_invalid++; break;
  }
}
#endif
