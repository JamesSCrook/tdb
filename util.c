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
	util.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "tdb.h"
#include "y.tab.h"


/*******************************************************************************
This function does a "strcmp" but with a slight difference.  It treats sequences
of digits in the strings as integers.  That is, they are compared as numbers.
Hence, the following strings would appear in a sorted list as follows:

"R1", "R4", "R4A", "R12"      and
"R4", "R33", "R222", "R1111"   (exactly the reverse, as strcmp).

This function should be used everywhere in TDB in preference to strcmp.
The only exception is when testing for an exact match, then strcmp is used
(because it should be faster).
*******************************************************************************/
int
mystrcmp(char *str1, char *str2)
{
    char *fstdig1, *fstdig2;
    int r, d;

    while (*str1 && *str2) {
	if (isdigit(*str1) && isdigit(*str2)) {	/* both chars are digits */
	    fstdig1 = str1;	/* save pointers to the first digits */
	    fstdig2 = str2;
	    d = 0;
	    do {		/* find the 1st digit that's different */
		if ((d = *str1 - *str2) != 0)
		    break;	/* found 1st difference in digits */
		str1++;
		str2++;
	    } while (isdigit(*str1) && isdigit(*str2));

	    while (isdigit(*str1))	/* skip any remaining digits of str1 */
		str1++;
	    while (isdigit(*str2))	/* skip any remaining digits of str2 */
		str2++;
	    if ((r=(str1-fstdig1)-(str2-fstdig2)) != 0) /* lengths different? */
		return r;  /* one str is longer (hence larger) than the other*/
	    if (d != 0)
		return d;  /* strings are the same length, but not identical */
	} else {
	    if ((r = *str1 - *str2) != 0)
		return r;
	    str1++;
	    str2++;
	}
    }
    if (*str1 && !*str2)
	return 1;
    if (!*str1 && *str2)
	return -1;
    return 0;
}


/*******************************************************************************
*******************************************************************************/
void
chkfld(int fldidx)
{
    extern Field *fldtbl;

    if (fldtbl[fldidx].idx == INVALIDFLDIDX) {
	fprintf(stderr, "chkfld: field `%s'", fldtbl[fldidx].name);
	fatalerrlin(" is not in the aggregate field list");
    }
}


/*******************************************************************************
This function reads a line of input from a text file (by calling fgets) and
also processes any "continuation lines".  That is, in the case where an input
line ends in CONTLINECHAR, the function continues reading lines in until it
gets one that doesn't end in the continuation character (or until some error
occurs).
*******************************************************************************/
char *
fgetscont(char *buf, int maxlen, FILE *fp)
{
    int len;

    if (fgets(buf, maxlen, fp) == NULL)
	return (char*)NULL;

    len = strlen(buf) - 2;	/* -2 to toss of the `\` and '\n' */
    while (len >= 0 && buf[len] == CONTLINECHAR) {
	if (fgets(buf+len, maxlen-len, fp) == NULL)
	    break;
	len = strlen(buf) - 2;
    }
    return buf;
}


#define NONESCAPEDQUOTE(bolchptr,chptr) \
    (*(chptr) == QUOTECHAR && \
    ( \
	((chptr) > (bolchptr) && *((chptr)-1) != ESCAPECHAR) || \
	(chptr) == (bolchptr) \
    ) \
    ? TRUE : FALSE)

#define ESCAPEDQUOTE(bolchptr,chptr) \
    (*(chptr) == QUOTECHAR && \
    ( \
	((chptr) > (bolchptr) && *((chptr)-1) == ESCAPECHAR) || \
	(chptr) == (bolchptr) \
    ) \
    ? TRUE : FALSE)

/*******************************************************************************
This function parses a text line into a number of arguments, and puts a pointer
to each argument (up to the maximum number maxnargs) in the pointer array.
THIS IS A DESTRUCTIVE FUNCTION.  It puts null termination characters at the
end of each of the arguments in the input line.
*******************************************************************************/
int
parseline(char *bolchptr, char *argptrtbl[], int maxnargs)
{
    Flag escapeflag;
    char *toptr, *fromptr;
    char *chptr = bolchptr;	/* bol = beginning of line */
    int idx, argctr = 0;

				/* if line has ESCAPECHAR anywhere in it */
    escapeflag = (strchr(bolchptr, ESCAPECHAR) != NULL ? TRUE : FALSE);

    for (idx=0; idx<maxnargs; idx++)	/* set all pointers to NULL */
	argptrtbl[idx] = NULL;

    while(WHITESPACE(*chptr))		/* skip any initial whitespaces */
	chptr++;

    while (*chptr) {			/* main loop */

	    /* if a comment or we've already got enough args, get out */
	if (*chptr == COMCHAR || argctr >= maxnargs)
	    break;

	if (NONESCAPEDQUOTE(bolchptr, chptr)) {	/* deal with quotes */
	    argptrtbl[argctr++] = ++chptr;
	    while (*chptr && !NONESCAPEDQUOTE(bolchptr, chptr))
		chptr++;
	} else {				/* normal case, no quotes */
	    argptrtbl[argctr++] = chptr++;
	    while (*chptr && !WHITESPACE(*chptr))
		chptr++;
	}
	*chptr++ = '\0';

	while (*chptr && WHITESPACE(*chptr))	/* skip whitespace after arg */
	    chptr++;
    }

    if (escapeflag) {	/* if _any_ of the args had an <\">, rm all the <\> */
	for (idx=0; idx<argctr; idx++) {
	    fromptr = toptr = bolchptr = argptrtbl[idx];
	    while (*fromptr) {
		if (ESCAPEDQUOTE(bolchptr, fromptr))
		    toptr--;
		*toptr++ = *fromptr++;
	    }
	    *toptr = '\0';
	}
    }
    return argctr;
}
