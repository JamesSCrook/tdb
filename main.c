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
	main.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "tdb.h"
#include "y.tab.h"

#define MAXERRORMSGLEN 127

FILE  *output_fp;
Opmode opmode = ERRORMODE;

#define GETOPTSTR "f:s:r:vd:1q:o:I:F:S:"

#define USAGETEXT "\
\n\
usage (v0.1.2):\n\
\n\
%s -f field_file -s select_file [OPTIONS] data_file [...]\n\
    or\n\
%s -f field_file -r report_file [OPTIONS] data_file [...]\n\
\n\
where OPTIONS are any of:\n\
  -v                                    : verbose/statistics mode\n\
  -d delimiter_character                : data file field delimiter character\n\
  -1                                    : skip line #1 of delimited data file(s)\n\
  -q quote_character (default \")        : quote character for delimited data file(s)\n\
  -o output_file                        : output file\n\
  -I variable_name=integer_value        : define an integer cmd line variable\n\
  -F variable_name=floating_point_value : define a float cmd line variable\n\
  -S variable_name=string_value         : define a string cmd line variable\n\
\n\
Notes:\n\
  Use `-' to specify stdin as a `data_file'\n\
  Define variables like this: -I foobar=42     (ie, no spaces around the `=')\n\
\n\
tdb Copyright (C) 1991-2016 James S. Crook\n\
This program comes with ABSOLUTELY NO WARRANTY.\n\
This is free software, and you are welcome to redistribute it under certain conditions.\n\
This program is licensed under the terms of the GNU General Public License as published\n\
by the Free Software Foundation, either version 3 of the License, or (at your option) any\n\
later version (see <http://www.gnu.org/licenses/>).\n\
\n\
"


void checkparsing();


/*******************************************************************************
*******************************************************************************/
int
main(int argc, char *argv[])
{
    extern FILE *yyin;			/* lex input file */
    extern int optind;
    extern char *optarg;
    extern char *srcfile;
    extern Flag fielddelimflag;
    extern Flag skipline1flag;
    extern char fielddelimchar;
    extern char quotechar;
    int c;
    char *programfilename = NULL;
    char *outputfilename = NULL;
    char *fieldfilename = NULL;
    Flag errflag = FALSE;
    Flag verboseflag = FALSE;
    Flag progisstdinflag = FALSE;

    while ((c=getopt(argc, argv, GETOPTSTR)) != EOF) {
        switch(c) {
	    case 'd': fielddelimflag = TRUE; fielddelimchar = *optarg; break;
	    case '1': skipline1flag = TRUE; break;
	    case 'q': quotechar = *optarg; break;
	    case 'f': fieldfilename = optarg; break;
	    case 'v': verboseflag = TRUE; break;
	    case 's':
		if (opmode != ERRORMODE)
		    errflag = TRUE;
		programfilename = optarg;
		opmode = SELREJMODE;
		break;
	    case 'r':
		if (opmode != ERRORMODE)
		    errflag = TRUE;
		programfilename = optarg;
		opmode = REPORTMODE;
		break;
	    case 'o': outputfilename = optarg; break;
	    case 'I': addcmdlinevar(optarg, INTEGER); break;
	    case 'F': addcmdlinevar(optarg, FLOAT); break;
	    case 'S': addcmdlinevar(optarg, STRING); break;
	    case '?': errflag = TRUE; break;
        }
    }

    if (errflag || opmode == ERRORMODE || fieldfilename == NULL) {
	fprintf(stderr, USAGETEXT, argv[0], argv[0]);
        return 1;
    }

    readfieldfile(fieldfilename);

    if (!strcmp(programfilename, STDINCODE)) {
	yyin = stdin;
	progisstdinflag = TRUE;
    } else {
	if ((yyin=fopen(programfilename, "r")) == NULL) {
	    fprintf(stderr, "Could not open the program file: `%s'\n",
							    programfilename);
	    return 1;
	}
	srcfile = programfilename;
    }

    if (outputfilename == NULL) {
	output_fp = stdout;
    } else if ((output_fp=fopen(outputfilename, "w")) == NULL) {
	fprintf(stderr,"Could not open the output file: `%s'\n",outputfilename);
	return 1;
    }

    switch(opmode) {
	case SELREJMODE:
	    initselrej();
	    yyparse();			/* parse the selrej program */
	    checkparsing();		/* fatal error on errors */
	    initreadfile();
	    for (c=optind; c<argc; c++)
		readpathname(argv[c], verboseflag, progisstdinflag);
	    break;
	case REPORTMODE:
	    yyparse();			/* parse the report program */
	    checkparsing();		/* fatal error on errors */
	    initreadfile();
	    for (c=optind; c<argc; c++)
		readpathname(argv[c], verboseflag, progisstdinflag);
	    printstart();		/* print the report */
	    if (verboseflag)
		printctrstats();
	    break;
	default:
	    fatalerror("main: illegal operation mode");
	    break;
    }

    return 0;
}


/*******************************************************************************
*******************************************************************************/
void
fatalerror(char *msg)
{
    fprintf(stderr, "%s\nfatal error\n", msg);
    exit(1);
}


Flag runtimeflag = FALSE;	/* FALSE = compile time, TRUE = runtime */
/*******************************************************************************
*******************************************************************************/
void
fatalerrlin(char *msg)
{
    extern int curstmtidx;
    extern Stmt *stmts;
    extern int linenum;
    extern char *srcfile;
    int loclinenum;
    char *locsrcfile;

    if (runtimeflag) {				/* "run" time error */
	locsrcfile = stmts[curstmtidx].srcfile;
	loclinenum = stmts[curstmtidx].linenum;
    } else {					/* "compile" time error */
	locsrcfile = srcfile;
	loclinenum = linenum;
    }

    fprintf(stderr, "%s\nfatal error at/near line %d of source file `%s'\n",
						msg, loclinenum, locsrcfile);
    exit(1);
}
