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
	selrej.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tdb.h"
#include "y.tab.h"

/*******************************************************************************
*******************************************************************************/
void
initselrej()
{
    extern int nfields;
    extern Field *fldtbl;
    int idx;

    for (idx=0; idx<nfields; idx++)
	fldtbl[idx].idx = idx;
}


/*******************************************************************************
*******************************************************************************/
Selrej
selrej()
{
    extern Flag runtimeflag;	/* FALSE = compile time, TRUE = runtime */
    extern int curstmtidx;	/* GLOBAL variable used for error msgs */
    extern Stmt *stmts;
    extern int nstmts;
    Exprnode result;
    Selrej retval = SELECTED;

    runtimeflag = TRUE;
    for (curstmtidx=0; curstmtidx<nstmts; curstmtidx++) {
	result = *(stmts[curstmtidx].s.selrej.exprptr);
	evalexpr(&result);
	if (result.typ != INTEGER) {
	    fprintf(stderr, "selrej: non-integer\n");
	    break;
	} else if (((stmts[curstmtidx].typ == REJCT) && result.arg.ival) ||
		    ((stmts[curstmtidx].typ == SELCT) && !result.arg.ival)) {
	    retval = REJECTED;	
	    break;
	}
    }
    return retval;
}


/*******************************************************************************
*******************************************************************************/
void
evalselrejfld(Exprnode *exprptr)
{
    extern Field *fldtbl;
    extern char **inplineargptrtbl;

    exprptr->typ = fldtbl[exprptr->arg.ival].typ;
    switch (exprptr->typ) {
	case STRING:
	    exprptr->arg.sval = inplineargptrtbl[exprptr->arg.ival];
	    break;
	case INTEGER:
	    exprptr->arg.ival = atoi(inplineargptrtbl[exprptr->arg.ival]);
	    break;
	case FLOAT:
	    exprptr->arg.fval = (float)atof(inplineargptrtbl[exprptr->arg.ival]);
	    break;
	default:
	    fatalerrlin("evalselrejfld: SNARK!");
	    break;
    }
}
