/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* we need the delaration of the VPointer type returned by VMalloc
 * to prevent gcc from throwing compiler warnings. */
#include <viaio/Vlib.h>

char * getVersion() {

	char url[] = "$HeadURL: https://svnserv.cbs.mpg.de/svn/lipsia/trunk/VIA/viaio/GetVersion.c $";
	char * ver = (char *)VMalloc(sizeof(char)*20);
	char * pch;


	/* check if this version was checked out from the trunk */
	if(strstr(url, "/trunk/")) {
		sprintf(ver, "#TRUNK#");
	}
	/* check if this version comes from a '/tags/' subdir */
	else {
		if ((pch = strstr (url,"/tags/"))) {
			/* in 'tags' VIA should reside in a directory similar to 'VIA-X.X.X/'.
				 X.X.X is the version string we are interested in. */
			pch = strstr(url, "VIA-");
			pch = strtok(pch, "-");
			pch = strtok(NULL, "/");
				
			if (!pch)
				sprintf(ver,"0.0.0");
			else  
				strcpy(ver, pch);

		}
		/* obviously, this version comes from outer space. Hence, there is no number. */
		else
			sprintf(ver,"0.0.0");
	}
		
	return ver;
}
