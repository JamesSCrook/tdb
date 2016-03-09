/*******************************************************************************
    tdb: a text database processing tool
    Copyright (c) 1991-2015 James S. Crook

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
	read.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "tdb.h"
#include "y.tab.h"

#define COMMENTCHAR  '#'
#define INPERRMSG "input error at line #%d in file `%s'\n"

Flag fielddelimflag =	FALSE;
Flag skipline1flag =	FALSE;
char quotechar =	'"';

char **inplineargptrtbl;
char fielddelimchar;

/*******************************************************************************
*******************************************************************************/
void
initreadfile()
{
    extern int nfields;
    extern int repnfields;
    int idx;

    inplineargptrtbl = (char**)calloc((size_t)repnfields, sizeof(char*));
    for (idx=nfields; idx<repnfields; idx++)
	inplineargptrtbl[idx] = (char*)malloc((size_t)MAXLINELEN);

}


/*******************************************************************************
*******************************************************************************/
static void
readfileselrej(char *filename, FILE *data_fp)
{
    extern FILE *output_fp;
    char inpline[2][MAXLINELEN], saveinpline[2][MAXLINELEN];
    int secpartctr, nargsfirst, nargssecond, nargs, linectr = 0, inplineidx = 0;
    extern int nfields;

    nargs = nfields;
    while (fgets(inpline[inplineidx], MAXLINELEN, data_fp) != NULL) {
	linectr++;
	if (*inpline[inplineidx] == COMMENTCHAR) {
	    fputs(inpline[inplineidx], output_fp);
	    continue;
	}

	strcpy(saveinpline[inplineidx], inpline[inplineidx]);
	if (!WHITESPACE(*inpline[inplineidx])) { /* 1st part or complete line */
	    nargsfirst = parseline(inpline[inplineidx], inplineargptrtbl, nfields);
	    if (nargs != nfields)
		fprintf(stderr, INPERRMSG, linectr, filename);
	    nargs = nargsfirst;
	    inplineidx = inplineidx ? 0 : 1;
	    secpartctr = 0;
	} else {		/* not (firstpart or complete line) */
	    nargssecond = parseline(inpline[inplineidx],
			    inplineargptrtbl+nargsfirst, nfields-nargsfirst);
	    if (nargssecond == 0)
		continue;
		
	    nargs = nargsfirst + nargssecond;
	    if (nargs < nfields)
		fprintf(stderr, INPERRMSG, linectr, filename);
	}

	if (nargs == nfields && selrej() == SELECTED) {
	    if (nargs == nargsfirst) {		/* complete line */
		fputs(saveinpline[inplineidx ? 0 : 1], output_fp);
	    } else {		/* not a complete line */
		secpartctr++;
		if (secpartctr == 1)	/* only print the 1st first part */
		    fputs(saveinpline[inplineidx ? 0 : 1], output_fp);
		fputs(saveinpline[inplineidx], output_fp);
	    }
	}
    }
}


/*******************************************************************************
*******************************************************************************/
static void
readfileselrejdelim(char *filename, FILE *data_fp)
{
    extern FILE *output_fp;
    extern int nfields;
    char *chptr, *firstchptr, inpline[MAXLINELEN], saveinpline[MAXLINELEN];
    int nargs, linectr = 0;

    if (skipline1flag) {
	if (fgets(inpline, MAXLINELEN, data_fp) != NULL) {
	    linectr++;
	} else {
	    return;
	}
    }

    while (fgets(inpline, MAXLINELEN, data_fp) != NULL) {
	linectr++;
	strcpy(saveinpline, inpline);

	chptr = inpline;
	nargs = 0;
	firstchptr = chptr;
	int inquoteflag = FALSE;

	while (*chptr) {
	    if (inquoteflag) {	/* already in a quote */
		if (*chptr == quotechar) {	/* end the quote */
		    inquoteflag = FALSE;
		    *chptr++ = '\0';
		} else {			/* continue quote */
		    chptr++;
		}
	    } else {		/* not currently in a quote */
		if (*chptr == quotechar) {  /* begin a quote */
		    inquoteflag = TRUE;
		    chptr++;
		    firstchptr = chptr;
		} else if (*chptr == fielddelimchar || *chptr == '\n') {
		    inplineargptrtbl[nargs++] = firstchptr;
		    *chptr++ = '\0';
		    firstchptr = chptr;
		    if (nargs == nfields) {
			break;
		    }
		} else {
		    chptr++;
		}
	    }
	}

	if (nargs == nfields) {
	    if (selrej() == SELECTED) {
		fputs(saveinpline, output_fp);
	    }
	} else {
	    fprintf(stderr, INPERRMSG, linectr, filename);
	}
    }
}


/*******************************************************************************
*******************************************************************************/
static void
readfilereport(char *filename, FILE *data_fp)
{
    extern Datanode *datarootptr;
    extern Field *aggrfldtbl;
    extern int nfields;
    extern Joinfldnode *rootjoinfldptr;
    extern int repnfields;
    Joinfldnode *joinfldptr;
    Joinfldargnode *joinfldargptr;
    char inpline[2][MAXLINELEN];
    int fldidx, nargsfirst, nargssecond, nargs, linectr = 0, inplineidx = 0;

    nargs = nfields;
    while (fgets(inpline[inplineidx], MAXLINELEN, data_fp) != NULL) {
	linectr++;
	if (*inpline[inplineidx] == COMMENTCHAR)
	    continue;

	if (!WHITESPACE(*inpline[inplineidx])) { /* 1st part or complete line */
	    nargsfirst = parseline(inpline[inplineidx], inplineargptrtbl, nfields);
	    if (nargs != nfields)
		fprintf(stderr, INPERRMSG, linectr, filename);
	    nargs = nargsfirst;
	    inplineidx = inplineidx ? 0 : 1;
	} else {		/* not (firstpart or complete line) */
	    nargssecond = parseline(inpline[inplineidx],
			    inplineargptrtbl+nargsfirst, nfields-nargsfirst);
	    if (nargssecond == 0)
		continue;

	    nargs = nargsfirst + nargssecond;
	    if (nargs < nfields)
		fprintf(stderr, INPERRMSG, linectr, filename);
	}

	if (nargs == nfields) {
	    if (repnfields > nfields) {		/* >= 1 join field(s) */
		joinfldptr = rootjoinfldptr;
		fldidx = nfields;
		while (joinfldptr != (Joinfldnode*)NULL) {
		    *inplineargptrtbl[fldidx] = '\0';
		    joinfldargptr = joinfldptr->argllistptr;
		    while (joinfldargptr != (Joinfldargnode*)NULL) {
			if (joinfldargptr->fldargtyp == FIELD) {
			    strcat(inplineargptrtbl[fldidx],
				    inplineargptrtbl[joinfldargptr->arg.fldidx]);
			} else {
			    strcat(inplineargptrtbl[fldidx],
				    joinfldargptr->arg.strptr);
			}
			joinfldargptr = joinfldargptr->next;
		    }
		    fldidx++;
		    joinfldptr = joinfldptr->next;
		}
	    }
	    adddatanode(&datarootptr, aggrfldtbl, 0);
	}
    }
}


/*******************************************************************************
*******************************************************************************/
static void
readfilereportdelim(char *filename, FILE *data_fp)
{
    extern Datanode *datarootptr;
    extern Field *aggrfldtbl;
    extern int nfields;
    extern Joinfldnode *rootjoinfldptr;
    extern int repnfields;
    Joinfldnode *joinfldptr;
    Joinfldargnode *joinfldargptr;
    char *chptr, *firstchptr, inpline[MAXLINELEN];
    int fldidx, nargs, linectr = 0;

    if (skipline1flag) {
	if (fgets(inpline, MAXLINELEN, data_fp) != NULL) {
	    linectr++;
	} else {
	    return;
	}
    }

    while (fgets(inpline, MAXLINELEN, data_fp) != NULL) {
	linectr++;

	chptr = inpline;
	nargs = 0;
	firstchptr = chptr;
	int inquoteflag = FALSE;

	while (*chptr && nargs <= nfields) {
	    if (inquoteflag) {	/* already in a quote */
		if (*chptr == quotechar) {	/* end the quote */
		    inquoteflag = FALSE;
		    *chptr++ = '\0';
		} else {			/* continue quote */
		    chptr++;
		}
	    } else {		/* not currently in a quote */
		if (*chptr == quotechar) {  /* begin a quote */
		    inquoteflag = TRUE;
		    chptr++;
		    firstchptr = chptr;
		} else if (*chptr == fielddelimchar || *chptr == '\n') {
		    inplineargptrtbl[nargs++] = firstchptr;
		    *chptr++ = '\0';
		    firstchptr = chptr;
		    if (nargs == nfields) {
			break;
		    }
		} else {
		    chptr++;
		}
	    }
	}

	if (nargs == nfields) {
	    if (repnfields > nfields) {		/* >= 1 join field(s) */
		joinfldptr = rootjoinfldptr;
		fldidx = nfields;
		while (joinfldptr != (Joinfldnode*)NULL) {
		    *inplineargptrtbl[fldidx] = '\0';
		    joinfldargptr = joinfldptr->argllistptr;
		    while (joinfldargptr != (Joinfldargnode*)NULL) {
			if (joinfldargptr->fldargtyp == FIELD) {
			    strcat(inplineargptrtbl[fldidx],
				    inplineargptrtbl[joinfldargptr->arg.fldidx]);
			} else {
			    strcat(inplineargptrtbl[fldidx],
				    joinfldargptr->arg.strptr);
			}
			joinfldargptr = joinfldargptr->next;
		    }
		    fldidx++;
		    joinfldptr = joinfldptr->next;
		}
	    }
	    adddatanode(&datarootptr, aggrfldtbl, 0);
	}
    }
}


/*******************************************************************************
This function reads the input stream.  The firstpart of a line always starts
at the beginning of a line

Input files have three valid formats:
firstpart		[nargsfirstpart arguments]
    lastpart		[repnfields-nargsfirstpart arguments]
    ...			[optional multiple lastpart(s)]

firstpart lastpart	[repnfields arguments (also called "complete" below]

COMMENTCHAR...		[these are always ignored]
*******************************************************************************/
static void
readfile(char *filename, FILE *data_fp)
{
    extern Opmode opmode;

    switch(opmode) {
	case SELREJMODE:
	    if (fielddelimflag) {
		readfileselrejdelim(filename, data_fp);
	    } else {
		readfileselrej(filename, data_fp);
	    }
	    break;
	case REPORTMODE:
	    if (fielddelimflag) {
		readfilereportdelim(filename, data_fp);
	    } else {
		readfilereport(filename, data_fp);
	    }
	    break;
	default:
	    fatalerror("readfile: illegal operation mode");
	    break;
    }
}


#define MAXPATHLEN 1024
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode)&S_IFDIR) == S_IFDIR)
#endif  /* S_ISDIR */
#ifndef S_ISREG
#define S_ISREG(mode) (((mode)&S_IFREG) == S_IFREG)
#endif  /* S_ISREG */
/*******************************************************************************
This function takes one argument, a pathname.  It examines (stats) this
pathname and if it is a file, calls 'readfile'.  If it is a directory, it opens
the directory, reads all the entries, and calls itself for each entry
(except "." and "..").
*******************************************************************************/
void
readpathname(char *pathname, Flag verboseflag, Flag progisstdinflag)
{
    struct stat statbuf;
    struct dirent *direntryptr;
    DIR *dirptr;
    FILE *data_fp;
    char newpathname[MAXPATHLEN+1];

    if (!strcmp(pathname, STDINCODE)) {
	if (progisstdinflag) {
	    fatalerror("standard input used for both progam and data");
	} else {
	    readfile(pathname, stdin);
	}
	return;
    }

    if (stat(pathname, &statbuf) == 0) {
	if (S_ISDIR(statbuf.st_mode)) {			/* stat succeeded */
	    if ((dirptr=opendir(pathname)) != NULL) {	/* a directory */
							/* read all entries */
		while ((direntryptr=readdir(dirptr)) != NULL) {
		    if (strcmp(direntryptr->d_name, ".") &&
					    strcmp(direntryptr->d_name, "..")) {
			sprintf(newpathname,"%s/%s", pathname,
							direntryptr->d_name);
			readpathname(newpathname, verboseflag, progisstdinflag);
		    }
		}
		(void) closedir(dirptr);
	    } else {
		perror("could not open directory");
		fprintf(stderr, "pathname: %s\n", pathname);
	    }
	} else if (S_ISREG(statbuf.st_mode)) {		/* regular file */
	    if (verboseflag)
		fprintf(stderr, "processing file: %s\n", pathname);

	    if ((data_fp=fopen(pathname, "r")) == NULL) {
		fprintf(stderr, "readpathname: could not open file %s\n",
								    pathname);
	    } else {
		readfile(pathname, data_fp);
		(void) fclose(data_fp);
	    }
	} else {					/* not a file or dir */
	    fprintf(stderr, "pathname: %s\n", pathname);
	}
    } else {						/* stat call failed */
	perror("could not stat file");
	fprintf(stderr, "pathname: %s\n", pathname);
    }
}
