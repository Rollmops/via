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
 * $Id: vcatobj.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *****************************************************************/

/*! \brief vcatobj - concatenates Vista data files

\par Description
concatenates Vista data files.

\par Usage

        <code>vcatobj</code>

        \param -in      input image(s)
        \param -out     output image

\par Examples
<br>
\par Known bugs
none.

\file vcatobj.c
\author Arthur Pope, UBC Laboratory for Computational Intelligence
*/


/*
 *
 *  The vcatobj program, which concatenates Vista data files.
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

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/file.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"


/*
 *  Program entry point.   
 */

int main (int argc, char *argv[])
{
    static VArgVector in_files;
    static VString out_filename;
    static VBoolean in_found, out_found;
    static VOptionDescRec options[] = {
	{ "in", VStringRepn, 0, & in_files, & in_found, NULL,
	      "Input files" },
	{ "out", VStringRepn, 1, & out_filename, & out_found, NULL,
	      "Output file" }
    };
    FILE *f;
    VStringConst in_filename;
    VAttrList in_list, out_list = VCreateAttrList ();
    VAttrListPosn posn;
    int i, n;
    char prg[50];	
    sprintf(prg,"vcatobj V%s", getVersion());
    fprintf (stderr, "%s\n", prg);


    /* Parse command line arguments: */
    if (! VParseCommand (VNumber (options), options, & argc, argv) ||
        ! VIdentifyFiles (VNumber (options), options, "in", & argc, argv, 0) ||
	! VIdentifyFiles (VNumber (options), options, "out", & argc, argv, -1))
	goto Usage;
    if (argc > 1) {
	VReportBadArgs (argc, argv);
Usage:	VReportUsage (argv[0], VNumber (options), options, NULL);
	exit (EXIT_FAILURE);
    }

    /* For each input file: */
    for (i = 0; i < in_files.number; i++) {
	in_filename = ((VStringConst *) in_files.vector)[i];

	/* Read its contents: */
	if (strcmp (in_filename, "-") == 0)
	    f = stdin;
	else {
	    f = fopen (in_filename, "r");
	    if (! f)
		VError ("Failed to open input file %s", in_filename);
	}
	if (! (in_list = VReadFile (f, NULL)))
	    exit (EXIT_FAILURE);
	fclose (f);

	/* Append the input file's contents to the output list: */
	if (out_list->prev)
	    out_list->prev->next = in_list->next;
	else out_list->next = in_list->next;
	out_list->prev = in_list->prev;
	in_list->next = in_list->prev = NULL;
	VDestroyAttrList (in_list);
    }

    /* Count the number of attributes now in the output list: */
    for (n = 0, VFirstAttr (out_list, & posn); VAttrExists (& posn);
	 VNextAttr (& posn), n++) ;

    /* Open and write the output file: */
    if (strcmp (out_filename, "-") == 0)
	f = stdout;
    else {
	f = fopen (out_filename, "w");
	if (! f)
	    VError ("Failed to open output file %s", out_filename);
    }
    if (! VWriteFile (f, out_list))
	exit (EXIT_FAILURE);

    fprintf (stderr, "%s: Copied %d object%s.\n",
	     argv[0], n, n == 1 ? "" : "s");
    return EXIT_SUCCESS;
}
