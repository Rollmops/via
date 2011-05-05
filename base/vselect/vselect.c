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
 * $Id: vselect.c 3629 2009-08-20 17:04:30Z proeger $
 *
 *****************************************************************/

/*! \brief vselect - select objects from an input file

\par Description
vselect copies selected objects from an input file to an output file.<br>
Command line options specify which objects are to be copied. Objects may be selected
by type (e.g., all images), by name, by the value of  some  attribute  (neg.  values
must precede with a double dash), or by position within the input file.

\par Usage

        <code>vselect</code>

        \param -in      input image
        \param -out     output image
        \param -object  Select the ith object. Objects in the input file are numbered 
	                consecutively from 0.
        \param -name    Select an object by its name.
        \param -type    Select an object by its type.
        \param -attr    Select any object having an attribute with  the  specified  name and
                        value.



\par Examples
<p>
To select the first object from a file:
<br>
<code>vselect -object 0 -in image.v -out result.v</code>
<p>
To select all attributes named ``history'' from a file:
<br>
<code> vselect -name history -in image.v -out result.v</code>
<p>
To select everything but images from a file:
<br>
<code>vselect -type image -not -in image.v -out result.v</code>
<p>
To select images with ubyte pixels from a file of images:
<br>
<code>vselect -attr repn ubyte -in image.v -out result.v</code>
<p>
To select images with modality "MR MPIL_DN02_REL" from a file of images:
<br>
<code>vselect -attr modality "MR MPIL_DN02_REL" -in image.v -out result.v</code>

\par Known bugs
none.

\file vselect.c
\author Arthur Pope, UBC Laboratory for Computational Intelligence
*/




/*
 *
 *  The vselect program, which selects certain objects from a data file.
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

/* From the V library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"


/* Later in this file: */
static VStringConst ObjectAttribute (VAttrListPosn *, VStringConst);


int main (int argc, char **argv)
{
  static VShort object;
  static VString name_str, type_str, attr_str[2];
  static VBoolean object_found, name_found, type_found, attr_found, not;
  static VOptionDescRec options[] = {
    { "object", VShortRepn, 1, & object, & object_found, NULL,
      "Object's order within file (0..n-1)" },
    { "name", VStringRepn, 1, & name_str, & name_found, NULL,
      "Object's name" },
    { "type", VStringRepn, 1, & type_str, & type_found, NULL,
      "Object's type" },
    /*    { "attr", VStringRepn, 2, & attr_str[0], & attr_found, NULL,
     * see ../vflow/vflow.c line 52 for futher infomation
     */
    { "attr", VStringRepn, 2, & attr_str[0], & attr_found, NULL,
      "Name and value of object's attribute" },
    { "not", VBooleanRepn, 1, & not, VOptionalOpt, NULL,
      "Invert selection criteria" }
  };
  FILE *in_file, *out_file;
  VAttrList list;
  int i, n;
  VAttrListPosn posn;
  VBoolean selected;
  VStringConst str;
  VBundle b;
  char prg[50];	
  sprintf(prg,"vselect V%s", getVersion());
  fprintf (stderr, "%s\n", prg);


  /* Parse command line arguments and identify the input and output files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,
		   & in_file, & out_file);

  /* Ensure that exactly one of the alternate selection criteria was
     specified: */
  if ((int) object_found + name_found + type_found + attr_found != 1)
    VError ("Specify one of -object, -name, -type, or -attr");
  if (object_found && object < 0)
    VError ("Object index must be >= 0");

  /* ==> Special hack: remove registration of standard object types such as
     "image" and "edges" so that VReadFile will leave them
     as VBundles rather than convert them to VImage, VEdges, etc.
     This simplifies support for such things as selecting images
     by their pixel representation. */
  VRepnInfo[VEdgesRepn].name = "";
  VRepnInfo[VImageRepn].name = "";

  /* Read the contents of the input file: */
  if (! (list = VReadFile (in_file, NULL)))
    exit (EXIT_FAILURE);
    
  /* For each object in that file: */
  i = n = 0;
  VFirstAttr (list, & posn);
  while (VAttrExists (& posn)) {

    /* Determine whether it is one of those selected: */
    if (object_found)
      selected = (i == object);

    else if (name_found)

      selected = (strcmp (VGetAttrName (& posn), name_str) == 0);

    else if (type_found) {

      selected = (VGetAttrRepn (& posn) == VBundleRepn &&
		  VGetAttrValue (& posn, NULL, VBundleRepn, & b) &&
		  strcmp (b->type_name, type_str) == 0);

    } else if (attr_found)

      selected = (str = ObjectAttribute (& posn, attr_str[0])) &&
	(strcmp (str, attr_str[1]) == 0);

    else selected = FALSE;

    /* Delete the attribute if its not among those selected: */
    if (selected != not) {
      n++;
      VNextAttr (& posn);
    } else VDeleteAttr (& posn);
    i++;
  }

  if (object_found && object >= i)
    VWarning ("-object argument (%d) exceeds no. of objects in file (%d)",
	      object, i);

  /* Write the selected attributes to the output file: */
  if (! VWriteFile (out_file, list))
    exit (EXIT_FAILURE);

  fprintf (stderr, "%s: Selected %d object%s out of %d.\n",
	   argv[0], n, n == 1 ? "" : "s", i);
  return EXIT_SUCCESS;
}


/*
 *  ObjectAttribute
 *
 *  Return the string value of a particular attribute of an object.
 */

static VStringConst ObjectAttribute (VAttrListPosn *posn, VStringConst name)
{
  VAttrList sublist;
  VBundle b;
  VStringConst str;

  switch (VGetAttrRepn (posn)) {

  case VAttrListRepn:

    /* If the object is a general attribute list, just search the list
       for the desired attribute: */
    VGetAttrValue (posn, NULL, VAttrListRepn, & sublist);
    break;

  case VBundleRepn:

    /* If the object is a bundle, search its attribute list for the
       desired attribute: */
    VGetAttrValue (posn, NULL, VBundleRepn, & b);
    sublist = b->list;
    break;

  default:
    return NULL;
  }

  return VGetAttr (sublist, name, NULL, VStringRepn, & str) == VAttrFound ?
    str : NULL;
}
