/*******************************************************************************
    tdb: a text database processing tool
    Copyright (c) 1991-2016 James S. Crook

    This file is part of tdb.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

/*******************************************************************************
	lookup.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tdb.h"
#include "y.tab.h"


typedef struct lookupnode {
    char  *filnam;
    char **linetbl;
    int    linectr;
    struct lookupnode *leftlookupptr;
    struct lookupnode *rightlookupptr;
} Lookupnode;

/*******************************************************************************
The comparison function used by srchlookuptbl to search a lookup table.
*******************************************************************************/
static int
lookupcmp(char **chptrptr1, char **chptrptr2)
{
    return mystrcmp(*chptrptr1, *chptrptr2);
}


#define MAXNLOOKUPARGS 16
/*******************************************************************************
This function actually loads (and then sorts) a "lookup table".  Lookup tables
are created from text files (typically files like the oprfile, etc) where all
the fields (up to MAXNLOOKUPARGS) are extracted from the file and stored into
the lookup table.  There is one table entry per valid line (not commented out,
with at lease one field) and these fields are read in AS STRINGS and stored in 
one big "string".  They are NULL SEPARATED, and DOUBLE NULL TERMINATED.
For example, the source line:
    JSC		"CROOK, JAMES S.	JSC	INS
would get stored: `JSC@CROOK, JAMES S.@JSC@INS@@' (where @ is the zero byte)
*******************************************************************************/
static void
loadlookuptbl(Lookupnode *lookupptr)
{
    FILE *fp;
    char *toptr, *fromptr, *args[MAXNLOOKUPARGS], line[MAXLINELEN];
    int n, nargs, linectr, linelen;

    if ((fp=fopen(lookupptr->filnam, "r")) == NULL) {
	fprintf(stderr, "Cannot open lookup file `%s'", lookupptr->filnam);
	fatalerrlin("");
    }

    linectr = 0;		/* count the number of "data" lines */
    while (fgetscont(line, MAXLINELEN, fp) != NULL) {
	if (*line == COMCHAR || *line == '\n' ||
				    parseline(line, args, MAXNLOOKUPARGS) < 1)
	continue;
	linectr++;
    }

    lookupptr->linetbl = (char**)malloc((size_t)linectr*sizeof(char*));
    lookupptr->linectr = linectr;

    linectr = 0;
    rewind(fp);			/* read the "data" line into memory */
    while (fgetscont(line, MAXLINELEN, fp) != NULL) {
	linelen = strlen(line);
	if (*line == COMCHAR || *line == '\n' ||
			    (nargs=parseline(line, args, MAXNLOOKUPARGS)) < 1)
	    continue;

	lookupptr->linetbl[linectr] = (char*)malloc((size_t)linelen+2);
	toptr = lookupptr->linetbl[linectr];
	for (n=0; n<nargs; n++) {
	    fromptr = args[n];
	    while (*fromptr)
		*toptr++ = *fromptr++;
	    *toptr++ = '\0';	/* copy the null byte too */
	}
	*toptr++ = '\0';	/* terminate with a second null byte */
	linectr++;
    }
    fclose(fp);
    qsort((void*)lookupptr->linetbl, linectr,sizeof(char*),(int(*)())lookupcmp);
}

/*******************************************************************************
This function searches a lookup table (which has already been loaded and
sorted) for the code string of interest.  If it is found, the (line) "field"
of interest is extracted, and a pointer to it is returned.  If the function
doesn't find what it's looking for, a pointer to the code string is returned.
Remember that the line data is NULL SEPARATED, and DOUBLE NULL TERMINATED.
*******************************************************************************/
static char *
srchlookuptbl(char *codstr, int argnum, Lookupnode *lookupptr)
{
    char **lineptr, *chptr;

    if ((lineptr=(char**)bsearch((void*)&codstr, (void*)lookupptr->linetbl,
				    (size_t)lookupptr->linectr, sizeof(char*),
				    (int(*)())lookupcmp)) != NULL) {
	chptr = *lineptr;
	while (--argnum && *chptr) {	/* skip up to the argnum'th arg */
	    while (*chptr)
		chptr++;
	    chptr++;
	}
	if (*chptr)
	    return chptr;
    }
    return codstr;
}


static Lookupnode *lookuprootptr = NULL;
/*******************************************************************************
This function maintains a binary tree of files that it has already loaded into
"lookup tables" (memory).  New files are added to the tree.
*******************************************************************************/
char *
dolookup(char *codstr, char *filnamstr, int argnum)
{
    Lookupnode *lookupptr, *newlookupptr;
    int r;

    lookupptr = lookuprootptr;
    while (lookupptr != NULL) {
	r = mystrcmp(filnamstr, lookupptr->filnam);
	if (r == 0) {
	    return srchlookuptbl(codstr, argnum, lookupptr);
	} else if (r < 0 && lookupptr->leftlookupptr != NULL) {
	    lookupptr = lookupptr->leftlookupptr;
	} else if (r > 0 && lookupptr->rightlookupptr != NULL) {
	    lookupptr = lookupptr->rightlookupptr;
	} else {
	    break;
	}
    }

    newlookupptr = (Lookupnode*)malloc(sizeof(Lookupnode));
    newlookupptr->filnam = (char*)malloc(strlen(filnamstr)+1);
    strcpy(newlookupptr->filnam, filnamstr);
    loadlookuptbl(newlookupptr);
    newlookupptr->leftlookupptr = NULL;
    newlookupptr->rightlookupptr = NULL;

    if (lookupptr == NULL) {
	lookuprootptr = newlookupptr;
    } else if (r < 0) {
	lookupptr->leftlookupptr  = newlookupptr;
    } else {
	lookupptr->rightlookupptr = newlookupptr;
    }
    return srchlookuptbl(codstr, argnum, newlookupptr);
}
