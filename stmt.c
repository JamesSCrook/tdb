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
	stmt.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "tdb.h"
#include "y.tab.h"

Stmt *stmts = NULL;
int   nstmts = 0;

static int maxnstmts = 0;

#define INITMAXNSTMTS    200
#define NSTMTSMULTFACTOR   2
/*******************************************************************************
Allocate memory for the statments.
*******************************************************************************/
static void
allocstmts()
{
    extern char *srcfile;
    extern int linenum;

    if (nstmts == maxnstmts) {
	if (stmts == NULL) {
	    maxnstmts = INITMAXNSTMTS;
	    stmts = (Stmt*)malloc((size_t)maxnstmts*sizeof(Stmt));
	} else {
	    maxnstmts *= NSTMTSMULTFACTOR;
	    stmts = (Stmt*)realloc((void*)stmts,(size_t)maxnstmts*sizeof(Stmt));
	}
    }
    stmts[nstmts].srcfile = srcfile;
    stmts[nstmts].linenum = linenum;
}


/*******************************************************************************
Add a SELREJ (select or reject) statment.
*******************************************************************************/
void
addselrejstmt(int stmttyp, Exprnode *exprptr)
{
    extern Opmode opmode;

    if (opmode != SELREJMODE)
	fatalerrlin("addselrejstmt: illegal select/reject statement");

    allocstmts();
    stmts[nstmts].typ = stmttyp;
    stmts[nstmts].s.selrej.exprptr = exprptr;
    nstmts++;
}


static int foreachstmtidx;
/*******************************************************************************
Add a FOREACH statment.
*******************************************************************************/
void
addforeachstmt(int fldidx)
{
    extern Field *fldtbl;
    long l = fldtbl[fldidx].idx;

    allocstmts();
    stmts[nstmts].typ = FOREACH;
    stmts[nstmts].s.foreach.fldidx = l;
    stmts[nstmts].s.foreach.fldbits = 1L << l;
    stmts[nstmts].s.foreach.maxdepth = l;
    stmts[nstmts].s.foreach.restrictflag = FALSE;
    foreachstmtidx = nstmts;
    nstmts++;
}


/*******************************************************************************
Add (append) a field to those already associated with a statment that takes
optional field argument(s) - FOREACH.
*******************************************************************************/
void
tdbstmtaddfld(int fldidx)
{
    extern Field *fldtbl;
    long l = fldtbl[fldidx].idx;

    stmts[foreachstmtidx].s.foreach.fldbits |= 1L << l;
    if (l > stmts[foreachstmtidx].s.foreach.maxdepth)
	stmts[foreachstmtidx].s.foreach.maxdepth = l;
    stmts[foreachstmtidx].s.foreach.restrictflag = TRUE;
}


/*******************************************************************************
Add a WHILE statment.
*******************************************************************************/
void
addwhilestmt(Exprnode *exprptr)
{
    allocstmts();
    stmts[nstmts].typ = WHILE;
    stmts[nstmts].s.while_.exprptr = exprptr;
    nstmts++;
}


/*******************************************************************************
Add a DOWHILE statment.
*******************************************************************************/
void
adddowhilestmt(Exprnode *exprptr)
{
    allocstmts();
    stmts[nstmts].typ = DO;
    stmts[nstmts].s.dowhile.exprptr = exprptr;
    nstmts++;
}


/*******************************************************************************
Add a SORT statment.
*******************************************************************************/
void
addsortstmt(int fldidx, Exprnode *exprptr, Flag reverseflag)
{
    extern Field *fldtbl;

    allocstmts();
    stmts[nstmts].typ = SORT;
    stmts[nstmts].s.sort.fldidx = fldtbl[fldidx].idx;
    stmts[nstmts].s.sort.exprptr = exprptr;
    stmts[nstmts].s.sort.reverseflag = reverseflag;
    nstmts++;
}


/*******************************************************************************
Add a FIRST or LAST (FSTLST) statment.
*******************************************************************************/
void
addfstlststmt(int stmttyp, int fldidx, Exprnode *exprptr)
{
    extern Field *fldtbl;

    allocstmts();
    stmts[nstmts].typ = stmttyp;
    stmts[nstmts].s.fstlst.fldidx = fldtbl[fldidx].idx;
    stmts[nstmts].s.fstlst.exprptr = exprptr;
    nstmts++;
}


/*******************************************************************************
Add a PRINTF or SPRINTF statment.
Note: lhsexprptr is only used by sprintf, not by printf
*******************************************************************************/
void
addprintfstmt(int stmttyp, Exprnode *lhsexprptr)
{
    extern Exprptrliststk *exprptrliststk;
    extern int exprptrliststkidx;

    allocstmts();
    stmts[nstmts].typ = stmttyp;
    stmts[nstmts].s.printf_.exprptrlistptr =
			    exprptrliststk[exprptrliststkidx].exprptrlistptr;
    stmts[nstmts].s.printf_.nargs = exprptrliststk[exprptrliststkidx--].exprctr;
    if (stmts[nstmts].s.printf_.nargs < 1)
	fatalerrlin("s/printf has no format expression");
    if (stmttyp == SPRINTF)
	stmts[nstmts].s.printf_.lhsexprptr = lhsexprptr;
    nstmts++;
}


/*******************************************************************************
Add an IF statment.
*******************************************************************************/
void
addifstmt(Exprnode *exprptr)
{
    allocstmts();
    stmts[nstmts].typ = IF;
    stmts[nstmts].s.if_.exprptr = exprptr;
    nstmts++;
}


/*******************************************************************************
Add an ELSE statment.
*******************************************************************************/
void
addelsestmt()
{
    allocstmts();
    stmts[nstmts].typ = ELSE;
    nstmts++;
}


/*******************************************************************************
Add any of the ASSIGNment statments (eg =, +=, etc).
*******************************************************************************/
void
addassignstmt(Exprnode *lhsexprptr, int optyp, Exprnode *rhsexprptr)
{
    allocstmts();
    stmts[nstmts].typ = ASSIGNOP;	/* assign ops are always STATMENTS */
    stmts[nstmts].s.assign.optyp = optyp;
    stmts[nstmts].s.assign.lhsexprptr = lhsexprptr;
    stmts[nstmts].s.assign.rhsexprptr = rhsexprptr;
    nstmts++;
}


/*******************************************************************************
Add a user function definition (UFNCDEF) statment.
*******************************************************************************/
void
addufncdefstmt(char *ufncname, int begstmtidx)
{
    allocstmts();
    stmts[nstmts].typ = UFNCDEFSTMT;
    addufncname(ufncname, begstmtidx);
    nstmts++;
}


/*******************************************************************************
Add a SYSTEM statment.
*******************************************************************************/
void
addsystemstmt(Exprnode *exprptr)
{
    allocstmts();
    stmts[nstmts].typ = SYSTEM;
    stmts[nstmts].s.system_.exprptr = exprptr;
    nstmts++;
}


/*******************************************************************************
Add a RETURN statment.
*******************************************************************************/
void
addreturnstmt(Exprnode *exprptr)
{
    allocstmts();
    stmts[nstmts].typ = RETURN;
    stmts[nstmts].s.return_.exprptr = exprptr;
    nstmts++;
}


/*******************************************************************************
Add a EXPRSTMT statment.
*******************************************************************************/
void
addexprstmt(Exprnode *exprptr)
{
    allocstmts();
    stmts[nstmts].typ = EXPRSTMT;
    stmts[nstmts].s.expr.exprptr = exprptr;
    nstmts++;
}


/*******************************************************************************
Add a TRAP statment.
*******************************************************************************/
void
addtrapstmt(Exprnode *linnumexprptr, Exprnode *ufncexprptr)
{
    allocstmts();
    stmts[nstmts].typ = TRAP;
    stmts[nstmts].s.trap.linnumexprptr = linnumexprptr;
    stmts[nstmts].s.trap.ufncexprptr = ufncexprptr;
    nstmts++;
}


Joinfldnode *rootjoinfldptr = (Joinfldnode*)NULL;
static Joinfldnode *newjoinfldptr;

/*******************************************************************************
Initialize a node for a join statment to the reverse llist @ rootjoinfldptr.
*******************************************************************************/
void
initjoinfldstmt(int fldtyp, char *fldname)
{
    extern Opmode opmode;
    Joinfldnode *tmpjoinfldptr;

    if (opmode != REPORTMODE)
	fatalerrlin("illegal join statement");

    newjoinfldptr = (Joinfldnode*)malloc((size_t)sizeof(Joinfldnode));
    newjoinfldptr->fldname = (char*)malloc((size_t)strlen(fldname)+1);
    strcpy(newjoinfldptr->fldname, fldname);
    newjoinfldptr->fldtyp = fldtyp;
    newjoinfldptr->fldargctr = 0;
    newjoinfldptr->argllistptr = (Joinfldargnode*)NULL;
    if (rootjoinfldptr == (Joinfldnode*)NULL) {	/* first one in list */
	newjoinfldptr->next = (Joinfldnode*)NULL;
    } else {		/* not the first one, build a reverse linked list */
	tmpjoinfldptr = rootjoinfldptr;		/* check for dup join fld */
	while (tmpjoinfldptr != (Joinfldnode*)NULL) {
	    if (!strcmp(fldname, tmpjoinfldptr->fldname))
		fatalerrlin("duplicate join field");
	    tmpjoinfldptr = tmpjoinfldptr->next;
	}
	newjoinfldptr->next = rootjoinfldptr;
    }
    rootjoinfldptr = newjoinfldptr;
}


/*******************************************************************************
*******************************************************************************/
void
addjoinfldarg(int fldargtyp, int fldidx, char *strptr)
{
    Joinfldargnode *joinfldargptr, *newjoinfldargptr;

    newjoinfldargptr = (Joinfldargnode*)malloc((size_t)sizeof(Joinfldargnode));
    newjoinfldargptr->fldargtyp = fldargtyp;
    if (fldargtyp == FIELD) {
	newjoinfldargptr->arg.fldidx = fldidx;
	newjoinfldptr->fldargctr++;
    } else {	/* STRING */
	newjoinfldargptr->arg.strptr = strptr;
    }
    newjoinfldargptr->next = (Joinfldargnode*)NULL;

			/* add to end of llist */
    if (newjoinfldptr->argllistptr == (Joinfldargnode*)NULL) {
	newjoinfldptr->argllistptr = newjoinfldargptr;	/* add first one */
    } else {
	joinfldargptr = newjoinfldptr->argllistptr;	/* add non-first one */
	while (joinfldargptr->next != (Joinfldargnode*)NULL)
	    joinfldargptr = joinfldargptr->next;
	joinfldargptr->next = newjoinfldargptr;
    }
}


/*******************************************************************************
Add a node for a join field statment.
*******************************************************************************/
void
addjoinfldstmt()
{
    extern int joinnfields;

    joinnfields++;
}


/*******************************************************************************

*******************************************************************************/
void
includejoinfields()
{
    extern int nfields, joinnfields, repnfields;
    extern Field *fldtbl;
    Joinfldnode *joinfldptr;
    int fldidx;

    if (joinnfields > 0) {		/* if there are join fields */
	repnfields = nfields + joinnfields;
	fldtbl =(Field*)realloc((void*)fldtbl,(size_t)repnfields*sizeof(Field));
	joinfldptr = rootjoinfldptr;
	fldidx = nfields;
	while (joinfldptr != (Joinfldnode*)NULL) {
	    fldtbl[fldidx].name = joinfldptr->fldname;
	    fldtbl[fldidx].typ  = joinfldptr->fldtyp;
	    fldidx++;
	    joinfldptr = joinfldptr->next;
	}
    }
}


static Flag *aggrfldusedflagtbl;
Statsinfofld *statsinfofldtbl;

/*******************************************************************************
This function initializes the aggregate statement in report mode.
*******************************************************************************/
void
init_b4_aggr()
{
    extern Axistbl *axistbl;
    extern int nfields, joinnfields, repnfields;
    extern Field *aggrfldtbl, *fldtbl;
    extern int *axisnodectrtbl;
    extern int *datanodectrtbl;
    int idx;

    statsinfofldtbl =
	    (Statsinfofld*)malloc((size_t)(repnfields)*sizeof(Statsinfofld));
    aggrfldtbl = (Field*)malloc((size_t)(repnfields+1)*sizeof(Field));
    aggrfldusedflagtbl = (Flag*)malloc((size_t)(repnfields+1)*sizeof(Flag));
    for (idx=0; idx<=repnfields; idx++) {	/* Note: <= */
	aggrfldusedflagtbl[idx] = FALSE;
	aggrfldtbl[idx].typ = INVALIDFLDIDX;
    }

    for (idx=0; idx<repnfields; idx++) {	/* Note: < */
	fldtbl[idx].idx = INVALIDFLDIDX;
	fldtbl[idx].statsflag = FALSE;
	axistbl[idx].rootptr = NULL;
	axistbl[idx].number = 0;
    }

    axisnodectrtbl = (int*)malloc((size_t)repnfields*sizeof(int));
    datanodectrtbl = (int*)malloc((size_t)repnfields*sizeof(int));
}


int aggrmaxdepth;

/*******************************************************************************
This function processes a field in an aggregate stament.
*******************************************************************************/
void
aggelement(int fldidx)
{
    static int aggrfldctr = 0;
    extern Opmode opmode;
    extern Field *aggrfldtbl;
    extern Field *fldtbl;

    if (opmode != REPORTMODE)
	fatalerrlin("illegal aggregate statement");

    if (aggrfldusedflagtbl[fldidx]) {
	fprintf(stderr, "field `%s' used more than once", fldtbl[fldidx].name);
	fatalerrlin("");
    } else {
	aggrfldusedflagtbl[fldidx] = TRUE;
	aggrfldtbl[aggrfldctr] = fldtbl[fldidx];
	aggrfldtbl[aggrfldctr].idx = fldidx;
	fldtbl[fldidx].idx = aggrfldctr;
	aggrfldctr++;
    }
    aggrmaxdepth = aggrfldctr-1;
}
