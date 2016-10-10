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
	field.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "tdb.h"
#include "y.tab.h"

#define STRINGSTR  "string"
#define INTEGERSTR "int"
#define FLOATSTR   "float"

Field *aggrfldtbl;
Field *fldtbl;
int nfields = 0;
int joinnfields = 0;
int repnfields = 0;



#define NUMFIELDARGS 2
#define FIELDTYPEIDX 0
#define FIELDNAMEIDX 1
/*******************************************************************************
Read the fields file, caclulate number of fields (nfields)
*******************************************************************************/
void
readfieldfile(char *fieldfilename)
{
    char *fieldargs[NUMFIELDARGS], line[MAXLINELEN];
    int nargs, linectr;
    FILE *field_fp;

    if ((field_fp=fopen(fieldfilename, "r")) == NULL) {
	fprintf(stderr, "Could not open the field file: `%s'\n", fieldfilename);
	exit(1);
    }

    linectr = 0;
    while (fgetscont(line, MAXLINELEN, field_fp) != NULL) {
	if (*line == COMCHAR || *line == '\n' ||
				parseline(line, fieldargs, NUMFIELDARGS) < 1)
	    continue;
	linectr++;
    }

    fldtbl = (Field*)malloc((size_t)linectr*sizeof(Field));
    /* this will need to be expanded (realloc-ed) if there are join fields */

    linectr = 0;
    rewind(field_fp);
    while (fgetscont(line, MAXLINELEN, field_fp) != NULL) {
	if (*line == COMCHAR || *line == '\n' ||
			(nargs=parseline(line, fieldargs, NUMFIELDARGS)) < 1)
	    continue;

	if (nargs < NUMFIELDARGS) {
	    fprintf(stderr, "Bad field input line starting `%s' in file %s'\n",
							line, fieldfilename);
	}

	fldtbl[linectr].name = (char*)malloc(strlen(fieldargs[FIELDNAMEIDX])+1);
	strcpy(fldtbl[linectr].name, fieldargs[FIELDNAMEIDX]);
	if (!strcmp(fieldargs[FIELDTYPEIDX], STRINGSTR)) {
	    fldtbl[linectr].typ = STRING;
	} else if (!strcmp(fieldargs[FIELDTYPEIDX], INTEGERSTR)) {
	    fldtbl[linectr].typ = INTEGER;
	} else if (!strcmp(fieldargs[FIELDTYPEIDX], FLOATSTR)) {
	    fldtbl[linectr].typ = FLOAT;
	} else {
	    fprintf(stderr, "illegal field type `%s' in file `%s'\n",
					fieldargs[FIELDTYPEIDX], fieldfilename);
	    exit(1);
	}

	linectr++;
    }
    nfields = linectr;
    repnfields = nfields;
}


/*******************************************************************************
Lookup a field name in the field table.
*******************************************************************************/
int
lookupfieldname(char *name)
{
    int idx;

    for (idx=0; idx<repnfields; idx++) {
	if (!strcmp(fldtbl[idx].name, name))
	    return idx;
    }
    return INVALIDFLDIDX;
}
