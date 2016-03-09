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
	print.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "tdb.h"
#include "y.tab.h"

#define PAGLENVARNAM "pagelength"
#define DFLTPAGLEN   66
#define PAGNUMVARNAM "pagenumber"
#define FSTPAGNUM    1
#define LINNUMVARNAM "linenumber"
#define FSTLINNUM    0

#define MAXNTRAPS 5

typedef struct {
    int       linnum;
    Exprnode *exprptr;
} Trap;

static int *outputpaglenptr;
static int *outputpagnumptr;
static int *outputlinnumptr;

static Trap traptbl[MAXNTRAPS];
static int ntraps = 0;
static int inatrapflag = FALSE;


/*******************************************************************************
*******************************************************************************/
void
setoutputvar(char *varname, int **paramptrptr, int assignval)
{
    Varnode *varptr;
    Flag addedflag;

    varptr = accessvar(varname, TRUE, &addedflag);
    *paramptrptr = &varptr->arg.varexpr.arg.ival;
    if (addedflag) {
	varptr->typ = VARIABLE;
	varptr->arg.varexpr.typ = INTEGER;
	varptr->arg.varexpr.varptr = varptr;
	**paramptrptr = assignval;
    }
}


/*******************************************************************************
*******************************************************************************/
void
inittraptbl()
{
    setoutputvar(PAGLENVARNAM, &outputpaglenptr, DFLTPAGLEN);
    if (*outputpaglenptr < 1) {
	fprintf(stderr, "bad pagelength value: %d", *outputpaglenptr);
	fatalerrlin("");
    }
    setoutputvar(PAGNUMVARNAM, &outputpagnumptr, FSTPAGNUM);
    setoutputvar(LINNUMVARNAM, &outputlinnumptr, FSTLINNUM);
}


/*******************************************************************************
*******************************************************************************/
static int
trapcmp(Trap *trapptr1, Trap *trapptr2)
{
    return trapptr1->linnum - trapptr2->linnum;
}


/*******************************************************************************
*******************************************************************************/
void
updatetraptbl(Exprnode *linnumexprptr, Exprnode *ufncexprptr)
{
    Exprnode result;
    Trap loctrap, *trapptr;

    result = *linnumexprptr;
    evalexpr(&result);
    if (result.typ != INTEGER)
	fatalerrlin("trap: first argument must be an integer expr");

    loctrap.linnum = result.arg.ival;
    if ((trapptr=(Trap*)bsearch((void*)&loctrap, (void*)traptbl, (size_t)ntraps,
				    sizeof(Trap), (int(*)())trapcmp)) == NULL) {
	if (ntraps >= MAXNTRAPS)
	    fatalerrlin("too many traps");
	traptbl[ntraps].linnum = result.arg.ival;
	traptbl[ntraps].exprptr = ufncexprptr;
	ntraps++;
	qsort((void*)traptbl, ntraps, sizeof(Trap), (int(*)())trapcmp);
    } else {
	trapptr->exprptr = ufncexprptr;
    }
}


/*******************************************************************************
*******************************************************************************/
void
chkoutputtraps()
{
    Exprnode result;
    Trap loctrap, *trapptr;

    if (inatrapflag)
	return;

    loctrap.linnum = *outputlinnumptr;
    while ((trapptr=(Trap*)bsearch((void*)&loctrap, (void*)traptbl,
		    (size_t)ntraps, sizeof(Trap), (int(*)())trapcmp)) != NULL) {
	inatrapflag = TRUE;
	result = *trapptr->exprptr;
	evalexpr(&result);
	inatrapflag = FALSE;
	loctrap.linnum = *outputlinnumptr;
    }
}


/*******************************************************************************
*******************************************************************************/
void
printftrap(char *rsltstr)
{
    extern FILE *output_fp;
    char *begptr, *endptr;

    begptr = endptr = rsltstr;
    while (*endptr) {
	if (*endptr == '\n' || *endptr == '\f') {
	    chkoutputtraps();
	    fwrite(begptr, endptr-begptr+1, 1, output_fp);
	    if (*endptr == '\n') {
		*outputlinnumptr = (*outputlinnumptr+1) % *outputpaglenptr;
	    } else {
		*outputlinnumptr = 0;
	    }
	    if (*outputlinnumptr == 0)
		(*outputpagnumptr)++;
	    begptr = endptr+1;
	}
	endptr++;
    }
    if (*begptr) {
	chkoutputtraps();
	fwrite(begptr, endptr-begptr, 1, output_fp);
    }
}


/*******************************************************************************
*******************************************************************************/
void
printendtraps()
{
    extern FILE *output_fp;
    int prevlinnum;
    Exprnode result;
    Trap loctrap, *trapptr;

    if (ntraps == 0 || *outputlinnumptr == 0)
	return;

    while (*outputlinnumptr < *outputpaglenptr) {
	loctrap.linnum = prevlinnum = *outputlinnumptr;
	if ((trapptr=(Trap*)bsearch((void*)&loctrap, (void*)traptbl,
		    (size_t)ntraps, sizeof(Trap), (int(*)())trapcmp)) != NULL) {
	    inatrapflag = TRUE;
	    result = *trapptr->exprptr;
	    evalexpr(&result);
	    inatrapflag = FALSE;
	}

	if (*outputlinnumptr == prevlinnum) { /* no trap lines printed */
	    fprintf(output_fp, "\n");
	    (*outputlinnumptr)++;
	} else if (*outputlinnumptr > prevlinnum) {
	    *outputlinnumptr += prevlinnum-*outputlinnumptr;
	} else {
	    *outputlinnumptr += prevlinnum-*outputlinnumptr+*outputpaglenptr;
	}
    }
}


#define NOTRAPSBELOW 999999
/*******************************************************************************
*******************************************************************************/
int
need(Exprnode *exprptr)
{
    extern FILE *output_fp;
    int trapidx, linesleft, eoplinesleft, traplinesleft = NOTRAPSBELOW;

    if (exprptr->typ != INTEGER || exprptr->arg.ival <= 0)
	fatalerrlin("need: argument must be an integer greater than zero");
    
    chkoutputtraps();
    eoplinesleft = *outputpaglenptr - *outputlinnumptr;
				/* find any traps BELOW the current line */
    for (trapidx=0; trapidx<ntraps; trapidx++) {
	if (traptbl[trapidx].linnum >= *outputlinnumptr) {
	    traplinesleft = traptbl[trapidx].linnum - *outputlinnumptr;
	    break;
	}
    }

    linesleft = MIN(eoplinesleft, traplinesleft); /* use that trap, or EOP */

    if (exprptr->arg.ival > linesleft) {	/* need more then we've got */
	*outputlinnumptr = (*outputlinnumptr+linesleft) % *outputpaglenptr;
	while (linesleft-- > 0)
	    fprintf(output_fp, "\n");
	return 0;
    } else {			/* we've got at least as much as is needed */
	return 1;
    }
}
