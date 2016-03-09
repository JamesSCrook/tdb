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
	report.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "tdb.h"
#include "y.tab.h"


/*******************************************************************************
Tree traversals:

    The TDB "database" consists of cascading balanced (AVL) B-trees.  There
    is one level of B-tree for each aggregated field. For example, if there
    were three aggregated fields, each data record read  (in  report  mode)
    would  result in an entry made (or changed) at each of the three levels
    of some B-tree.
    
    Each data node in the B-tree has a left and a right pointer  -  as  one
    would  expect.  However,  each node also has an "orthog(inal)" pointer.
    This is the pointer to the B-tree for the next aggregate field level.

    For the purposes of describing the tree traversal, PRETEND FOR A MOMENT
    THAT  THE  INTERNAL  DATA  STRUCTURES USE (SORTED) LINKED LISTS, rather
    than B-trees.

    The following files ...
---------------------------------------------------------------------
    fields file:     data file:         report file:
    ------------     -----------        ------------
    string  code     C1  Me   1 a       aggregate(who, code, let);
    string  who      X3  Me   2 b
    int     num      C1  Me   3 c                buckets:
    string  let      T5  You  4 c      ----------------------------
                     T5  You  5 x      /-----/   |-----|     /---\
                     T5  You  6 a     /     /    |     |    (     )
                     T5  You  7 a    /____c/     |____c|     \__c/
                     R1  You  8 c    level 1     level 2    level 3
                     R1  You  9 c    node        node       node
                     R1  You 10 c    (depth=0)   (depth=1)  (depth=2)

                                   Note: `c' is the "count" of the number
					 of data records in each of the
					 buckets.
---------------------------------------------------------------------
    ... would produce this structure (again, the balanced B-trees
    are replaced with sorted linked lists - ONLY IN THIS DIAGRAM):

root->/-----/                           /-----/
     / Me  /-------------------------->/ You /
    /____3/                           /____7/
       |                                 |
       |                                 |
       v                                 v
    |-----|    /---\     /---\        |-----|    /---\
    | C1  |-->(  a  )-->(  c  )       | R1  |-->(  c  )
    |____2|    \__1/     \__1/        |____3|    \__3/
       |                                 |
       |                                 |
       v                                 v
    |-----|    /---\                  |-----|    /---\     /---\     /---\
    | X3  |-->(  b  )                 | T5  |-->(  a  )-->(  c  )-->(  x  )
    |____1|    \__1/                  |____4|    \__2/     \__1/     \__1/


    The fldbits  are  set  depending  upon  which  fields  are  used  by  a
    particular  TDB  function.  For  example,  in  a  report  with the call
    `count(let, who)' would set the two bits (bit 2 and bit  0)  for  these
    fields (all the other bits would be zero).

    The tree is traversed from depth 0, 1, and so on. Foreach loops - which
    are   frequently  nested,  are  responsible  for  setting  entries  the
    curfldvalptrtbl array. That is, in a program

    The following source fragment illustrates how tree traversal is done:

    if (!(fldbits & 1L<<(long)depth) || (curfldvalptrtbl[depth] != NULL &&
			dataptr->argptr == &(curfldvalptrtbl[depth]->arg))) {
	if (depth == maxdepth)
	    c += dataptr->count;
	else if (dataptr->orthogptr != NULL)
	    c += getcount(dataptr->orthogptr, fldbits, depth+1, maxdepth);
    }

    This says: IF (
	the field for the current traversal depth is NOT one of the
	fields we're looking for - or, otherwise - it is a field we're
	looking for - AND - the VALUE of that field is the one set for
	this pass of the foreach loop.) THEN we look further.
    {
	IF (
	    we've reached the maximum depth (of the fields specified in
	    the count function call)
	) THEN
	{ increment the total count value by the count for this bucket.  }
	ELSE IF (
	    we're not at the maximum depth, another there is another level
	    of data below (at a lower depth)
	)
	{ call this function (getcount) again for the data below }
    }
*******************************************************************************/

#define INITARGSTKSIZE   8
#define ARGSTKMULTFACTOR 2

Axistbl  *axistbl;
Datanode *datarootptr = NULL;

int curstmtidx;		/* current statement index, for error messages */

static Axisnode **curfldvalptrtbl;
static Axisnode **axisarrayptr;

static Exprnode *argstk;	/* the ufnc argument stack (of expressions) */
static int       maxargstkctr;	/* the max number of args on the stack */
static int       argstkctr = 0;	/* the number of args on the stack */
static int       botargstkidx = 0;/* the "bottom" of the current stack frame */

extern Stmt *stmts;
extern int nstmts;


/*******************************************************************************
Copy all the axis tree node addresses into the axis arrays (hence, they will
be sorted by name).
*******************************************************************************/
static int
axis2array(Axisnode *axisnodeptr)
{
    if (axisnodeptr->leftptr != NULL)
	axis2array(axisnodeptr->leftptr);

    *axisarrayptr++ = axisnodeptr;

    if (axisnodeptr->rightptr != NULL)
	axis2array(axisnodeptr->rightptr);
}


/*******************************************************************************
This function checks for fields specified inside intrinsic functions (COUNT,
etc.) that are referenced outside a foreach loop - which is meaningless as well
as erroneous.
*******************************************************************************/
static void
chk4badflds(long fldbits, int maxdepth)
{
    extern Field *aggrfldtbl;
    int depth;

    for (depth=0; depth<=maxdepth; depth++) {
	if ((fldbits & 1L<<(long)depth) && curfldvalptrtbl[depth] == NULL) {
	    fprintf(stderr, "field `%s'", aggrfldtbl[depth].name);
	    fatalerrlin(" is referenced outside a foreach loop for that field");
	}
    }
}


/*******************************************************************************
This function evaluates the "count" function by traversing the data tree and
incrementing a counter for each node that satisifies the conditions of the
search.
*******************************************************************************/
static int
getcount(Datanode *dataptr, long fldbits, int depth, int maxdepth)
{
    int c = 0;

    if (dataptr->leftptr != NULL)
	c += getcount(dataptr->leftptr, fldbits, depth, maxdepth);

    if (!(fldbits & 1L<<(long)depth) || (curfldvalptrtbl[depth] != NULL &&
			dataptr->argptr == &(curfldvalptrtbl[depth]->arg))) {
	if (depth == maxdepth)
	    c += dataptr->count;
	else if (dataptr->orthogptr != NULL)
	    c += getcount(dataptr->orthogptr, fldbits, depth+1, maxdepth);
    }

    if (dataptr->rightptr != NULL)
	c += getcount(dataptr->rightptr, fldbits, depth, maxdepth);

    return c;
}


/*******************************************************************************
This MACRO contains the code to evaluate the sum/sumsqrd functions for both
INTEGER  and  FLOAT  field types. It is used in four places, this is only a
(source) code saving exercise.
*******************************************************************************/
#define GETSUM(funcname, sumtyp, initval, sumparam) \
    sumtyp s = initval; \
\
    if (dataptr->leftptr != NULL) \
	s += funcname(dataptr->leftptr, fldbits, depth, maxdepth, sumdepth); \
\
    if (!(fldbits & 1L<<(long)depth) || (curfldvalptrtbl[depth] != NULL && \
			dataptr->argptr == &(curfldvalptrtbl[depth]->arg))) { \
	if (depth == maxdepth) \
	    s += dataptr->statstbl[sumdepth].sumparam; \
	else if (dataptr->orthogptr != NULL) \
	    s += funcname(dataptr->orthogptr, fldbits, depth+1, maxdepth, \
								sumdepth); \
    } \
\
    if (dataptr->rightptr != NULL) \
	s += funcname(dataptr->rightptr, fldbits, depth, maxdepth, sumdepth); \
\
    return s	/* note: no terminating ';' !!! */


/*******************************************************************************
Evaluates "sum" function for INTEGER field types (see GETSUM definition).
*******************************************************************************/
static int
getintsum(Datanode *dataptr, long fldbits, int depth, int maxdepth, int sumdepth)
{
    GETSUM(getintsum, int, 0, sum.ival);	/* Note: a MACRO!!! */
}


/*******************************************************************************
Evaluates "sum" function for FLOAT field types (see GETSUM definition).
*******************************************************************************/
static float
getfltsum(Datanode *dataptr, long fldbits, int depth, int maxdepth, int sumdepth)
{
    GETSUM(getfltsum, float, 0.0, sum.fval);	/* Note: a MACRO!!! */
}



/*******************************************************************************
Evaluates "sumsqrd" function for INTEGER field types (see GETSUM definition).
*******************************************************************************/
static int
getintsumsqrd(Datanode *dataptr, long fldbits, int depth, int maxdepth, int sumdepth)
{
    GETSUM(getintsumsqrd, int, 0, sumsqrd.ival);	/* Note: a MACRO!!! */
}


/*******************************************************************************
Evaluates "sumsqrd" function for FLOAT field types (see GETSUM definition).
*******************************************************************************/
static float
getfltsumsqrd(Datanode *dataptr, long fldbits, int depth, int maxdepth, int sumdepth)
{
    GETSUM(getfltsumsqrd, float, 0.0, sumsqrd.fval);	/* Note: a MACRO!!! */
}


static Axisnode *numaxisptr;
/*******************************************************************************
Traverse the data tree for evaluating the "number" statment (see getnumber).
*******************************************************************************/
static int
getnumber2(Datanode *dataptr, long fldbits, int depth, int maxdepth, int numdepth)
{
    int c = 0;

    if (dataptr->leftptr != NULL)
	c += getnumber2(dataptr->leftptr,fldbits,depth,maxdepth,numdepth);

    if (!(fldbits & 1L<<(long)depth) || (curfldvalptrtbl[depth] != NULL &&
			dataptr->argptr == &(curfldvalptrtbl[depth]->arg))) {
	if (depth == numdepth)
	    numaxisptr = dataptr->axisptr;
	if (depth == maxdepth) {
	    if (numaxisptr->seenflag == FALSE) {
		numaxisptr->seenflag = TRUE;
		c++;
	    }
	}
	else if (dataptr->orthogptr != NULL)
	    c += getnumber2(dataptr->orthogptr, fldbits, depth+1, maxdepth,
								    numdepth);
    }

    if (dataptr->rightptr != NULL)
	c += getnumber2(dataptr->rightptr,fldbits,depth,maxdepth,numdepth);
    
    return c;
}


/*******************************************************************************
This function evaluates the "number".  It handles the case where number is
called with only a field name, eg, number(opr), by returning the number in the
array table.  Otherwise, it sets up the seenflags and calls getnumber2 to
traverse the data tree.
*******************************************************************************/
static int
getnumber(Datanode *dataptr, long fldbits, int depth, int maxdepth)
{
    int i;

    if (maxdepth == INVALIDFLDIDX) { /* no maxdepth, return this */
	return axistbl[depth].number;			/* depth's num */
    } else {		/* otherwise, clear all seenflags for this depth, */
	for (i=0; i<axistbl[depth].number; i++)
	    (axistbl[depth].array[i])->seenflag = FALSE;
			    /* and traverse down to MAX(depth, maxdepth) */
	return getnumber2(dataptr, fldbits, 0, MAX(depth,maxdepth), depth);
    }
}


#define STKERRMSG "\
Non-existent user function parameter referenced (stack error).\n\
Check for a trap call from within a user function that references\n\
one of that function's parameters."
/*******************************************************************************
This function evaluates an expression.  It is passed a pointer to an expression
node to evaluate (results are also put here), and it evaluates the expression
tree.
Note: the argument stack is used for user function calls.  Even though it looks
cumbersome, indices have been used instead of stack pointers.  This is because
this is a recursive function, and it is neccessary to save previous stack
information AND the stack is dynamically resizable - that is - it can move at
any time.  Hence, any pointers saved on previous invocations of this function
would be pointing to where the stack USED to be, not where it NOW is.
*******************************************************************************/
void
evalexpr(Exprnode *exprptr)
{
    extern Field *aggrfldtbl, *fldtbl;
    extern Opmode opmode;		/* report or selrej mode */
    extern Statsinfofld *statsinfofldtbl;
    Exprnode lresult, rresult;
    Exprptrlistnode *exprptrlistptr;
    int tmpidx, argctr, intval, savebotargstkidx;
    float floatval;

    switch (exprptr->typ) {
	case INTEGER: case FLOAT: case STRING:
	    break;
	case FIELD:
	    if (opmode == REPORTMODE) {
		if (curfldvalptrtbl[exprptr->arg.ival] == NULL) {
		    fprintf(stderr, "field `%s'", 
					    aggrfldtbl[exprptr->arg.ival].name);
		    fatalerrlin(" is referenced outside a foreach loop");
		}
		exprptr->typ = aggrfldtbl[exprptr->arg.ival].typ;
		switch (exprptr->typ) {
		    case STRING:
			exprptr->arg.sval =
				curfldvalptrtbl[exprptr->arg.ival]->arg.sval;
			break;
		    case INTEGER:
			exprptr->arg.ival =
				curfldvalptrtbl[exprptr->arg.ival]->arg.ival;
			break;
		    case FLOAT:
			exprptr->arg.fval =
				curfldvalptrtbl[exprptr->arg.ival]->arg.fval;
			break;
		    default: fatalerror("evalexpr: SNARK!");
			break;
		}
	    } else {	/* selrej mode */
		evalselrejfld(exprptr);
	    }
	    break;
	case UFNCCALLEXPR:
	    argctr = exprptr->arg.ufnc.argctr;
	    if (argstkctr + argctr > maxargstkctr) {   /* enlarge stk if nec */
		maxargstkctr = MAX(maxargstkctr*ARGSTKMULTFACTOR,
							    argstkctr+argctr);
		argstk = (Exprnode*)realloc((void*)argstk,
				    (size_t)maxargstkctr*sizeof(Exprnode));
	    }
				    /* eval all args & push them on the stk */
	    exprptrlistptr = exprptr->arg.ufnc.exprptrlistptr;
	    while (exprptrlistptr != NULL) {		/* eval all args */
		lresult = *exprptrlistptr->exprptr;
		evalexpr(&lresult);
		*(argstk+argstkctr++) = lresult;	/* & push on stk */
		exprptrlistptr = exprptrlistptr->next;
	    }

	    savebotargstkidx = botargstkidx;
	    botargstkidx = argstkctr-argctr;
	    tmpidx = exprptr->arg.ufnc.begstmtidx;	/* call user func */
	    *exprptr = doblock(tmpidx+1, stmts[tmpidx].s.ufncdef.endstmtidx);
	    argstkctr -= argctr;
	    botargstkidx = savebotargstkidx;
	    break;
	case UFNCPARAMEXPR:
	    if (botargstkidx+exprptr->arg.ival >= argstkctr)
		fatalerror(STKERRMSG);
	    *exprptr = *(&argstk[botargstkidx]+exprptr->arg.ival);
	    break;
	case UNARYOPEXPR:
	    lresult = *(exprptr->leftptr);  evalexpr(&lresult);
	    switch (lresult.typ) {
		case INTEGER:
		    exprptr->arg.ival =
				iunaryop(exprptr->arg.optyp, lresult.arg.ival);
		    break;
		case FLOAT:
		    exprptr->arg.fval =
				funaryop(exprptr->arg.optyp, lresult.arg.fval);
		    break;
		default:
		    fatalerrlin("evalexpr: unary operation on illegal type");
		    break;
	    }
	    exprptr->typ = lresult.typ;
	    break;
	case BINARYOPEXPR:
	    lresult = *(exprptr->leftptr);  evalexpr(&lresult);
	    rresult = *(exprptr->rightptr); evalexpr(&rresult);
	    if ((lresult.typ == INTEGER)&&(rresult.typ == INTEGER)) {
		exprptr->arg.ival = ibinop(lresult.arg.ival,
				    exprptr->arg.optyp, rresult.arg.ival);
		exprptr->typ = INTEGER;
	    } else if (lresult.typ == INTEGER && rresult.typ == FLOAT) {
		    if ((exprptr->typ=fbinop((float)lresult.arg.ival,
				exprptr->arg.optyp, rresult.arg.fval,
				&floatval, &intval)) == FLOAT)
		    exprptr->arg.fval = floatval;
		else
		    exprptr->arg.ival = intval;
	    } else if (lresult.typ == FLOAT && rresult.typ == INTEGER) {
		    if ((exprptr->typ=fbinop(lresult.arg.fval,
				exprptr->arg.optyp, (float)rresult.arg.ival,
				&floatval, &intval)) == FLOAT)
		    exprptr->arg.fval = floatval;
		else
		    exprptr->arg.ival = intval;
	    } else if (lresult.typ == FLOAT && rresult.typ == FLOAT) {
		    if ((exprptr->typ=fbinop(lresult.arg.fval,
				exprptr->arg.optyp, rresult.arg.fval,
				&floatval, &intval)) == FLOAT)
		    exprptr->arg.fval = floatval;
		else
		    exprptr->arg.ival = intval;
	    } else if (lresult.typ == STRING && rresult.typ == STRING) {
		sbinop(lresult.arg.sval, rresult.arg.sval, exprptr);
	    } else {
		fatalerrlin( "evalexpr: illegal binary op argument type(s)");
	    }
	    break;
	case COUNT:
	    if (exprptr->arg.tdbfnc.maxdepth != INVALIDFLDIDX) {
		chk4badflds(exprptr->arg.tdbfnc.fldbits,
						exprptr->arg.tdbfnc.maxdepth);
		exprptr->arg.ival = getcount(datarootptr,
					    exprptr->arg.tdbfnc.fldbits, 0,
					    exprptr->arg.tdbfnc.maxdepth);
	    } else {
		exprptr->arg.ival = getcount(datarootptr,
					    exprptr->arg.tdbfnc.fldbits, 0, 0);
	    }
	    exprptr->typ = INTEGER;
	    break;
	case NUMBER:
	    chk4badflds(exprptr->arg.tdbfnc.fldbits,
						exprptr->arg.tdbfnc.maxdepth);
	    exprptr->arg.ival = getnumber(datarootptr,
		    exprptr->arg.tdbfnc.fldbits,
		    exprptr->arg.tdbfnc.depth, exprptr->arg.tdbfnc.maxdepth);
	    exprptr->typ = INTEGER;
	    break;
	case SUM:
	    intval = statsinfofldtbl[exprptr->arg.tdbfnc.depth].typ;
	    switch (intval) {
		case INTEGER:
		    if (exprptr->arg.tdbfnc.maxdepth != INVALIDFLDIDX) {
			chk4badflds(exprptr->arg.tdbfnc.fldbits,
						exprptr->arg.tdbfnc.maxdepth);
			exprptr->arg.ival = getintsum(datarootptr,
						exprptr->arg.tdbfnc.fldbits, 0,
						exprptr->arg.tdbfnc.maxdepth,
						exprptr->arg.tdbfnc.depth);
		    } else {
			exprptr->arg.ival = getintsum(datarootptr,
						exprptr->arg.tdbfnc.fldbits, 0,
						0, exprptr->arg.tdbfnc.depth);
		    }
		    break;
		case FLOAT:
		    if (exprptr->arg.tdbfnc.maxdepth != INVALIDFLDIDX) {
			chk4badflds(exprptr->arg.tdbfnc.fldbits,
						exprptr->arg.tdbfnc.maxdepth);
			exprptr->arg.fval = getfltsum(datarootptr,
						exprptr->arg.tdbfnc.fldbits, 0,
						exprptr->arg.tdbfnc.maxdepth,
						exprptr->arg.tdbfnc.depth);
		    } else {
			exprptr->arg.fval = getfltsum(datarootptr,
						exprptr->arg.tdbfnc.fldbits, 0,
						0, exprptr->arg.tdbfnc.depth);
		    }
		    break;
		default:
		    fatalerrlin("evalexpr: illegal SUM field type");
		    break;
	    }
	    exprptr->typ = intval;


	    break;
	case SUMSQRD:
	    intval = statsinfofldtbl[exprptr->arg.tdbfnc.depth].typ;
	    switch (intval) {
		case INTEGER:
		    if (exprptr->arg.tdbfnc.maxdepth != INVALIDFLDIDX) {
			chk4badflds(exprptr->arg.tdbfnc.fldbits,
						exprptr->arg.tdbfnc.maxdepth);
			exprptr->arg.ival = getintsumsqrd(datarootptr,
						exprptr->arg.tdbfnc.fldbits, 0,
						exprptr->arg.tdbfnc.maxdepth,
						exprptr->arg.tdbfnc.depth);
		    } else {
			exprptr->arg.ival = getintsumsqrd(datarootptr,
						exprptr->arg.tdbfnc.fldbits, 0,
						0, exprptr->arg.tdbfnc.depth);
		    }
		    break;
		case FLOAT:
		    if (exprptr->arg.tdbfnc.maxdepth != INVALIDFLDIDX) {
			chk4badflds(exprptr->arg.tdbfnc.fldbits,
						exprptr->arg.tdbfnc.maxdepth);
			exprptr->arg.fval = getfltsumsqrd(datarootptr,
						exprptr->arg.tdbfnc.fldbits, 0,
						exprptr->arg.tdbfnc.maxdepth,
						exprptr->arg.tdbfnc.depth);
		    } else {
			exprptr->arg.fval = getfltsumsqrd(datarootptr,
						exprptr->arg.tdbfnc.fldbits, 0,
						0, exprptr->arg.tdbfnc.depth);
		    }
		    break;
		default:
		    fatalerrlin("evalexpr: illegal SUM field type");
		    break;
	    }
	    exprptr->typ = intval;


	    break;
	case TYPE:
	    lresult = *(exprptr->leftptr);  evalexpr(&lresult);
	    switch (lresult.typ) {
		case INTEGER:		/* we've got an integer, make a ... */
		    if (exprptr->arg.casttyp == FLOAT) {
			exprptr->arg.fval = (float)lresult.arg.ival;
			exprptr->typ = FLOAT;
		    } else {		/* an integer */
			exprptr->arg.ival = lresult.arg.ival;
			exprptr->typ = INTEGER;
		    }
		    break;
		case FLOAT:		/* we've got a float, make an ... */
		    if (exprptr->arg.casttyp == FLOAT) {
			exprptr->arg.fval = lresult.arg.fval;
			exprptr->typ = FLOAT;
		    } else {		/* an integer */
			exprptr->arg.ival = (int)lresult.arg.fval;
			exprptr->typ = INTEGER;
		    }
		    break;
		default:
		    fatalerrlin("evalexpr: illegal type cast");
		    break;
	    }
	    break;
	case REPORTDT:			/* get the report date or time */
            exprptr->arg.ival = getreportdt(exprptr->arg.ival);
            exprptr->typ = INTEGER;
	    break;
	case FORMATDT:			/* format a date or time */
            lresult = *(exprptr->leftptr);  evalexpr(&lresult);
            rresult = *(exprptr->rightptr); evalexpr(&rresult);
            exprptr->arg.sval = formatdt(exprptr->arg.ival, &lresult, &rresult);
            exprptr->typ = STRING;
	    break;
        case STRFUNCEXPR:
            strfunc(exprptr);
	    break;
	case ATOI:
            lresult = *(exprptr->leftptr);  evalexpr(&lresult);
	    if (lresult.typ != STRING)
		fatalerrlin("evalexpr: atoi of non-string expression");
            exprptr->arg.ival = atoi(lresult.arg.sval);
            exprptr->typ = INTEGER;
	    break;
	case ATOF:
            lresult = *(exprptr->leftptr);  evalexpr(&lresult);
	    if (lresult.typ != STRING)
		fatalerrlin("evalexpr: atof of non-string expression");
            exprptr->arg.fval = (float) atof(lresult.arg.sval);
            exprptr->typ = FLOAT;
	    break;
	case MATHFUNC:
	    lresult = *(exprptr->leftptr);  evalexpr(&lresult);
	    switch (lresult.typ) {
		case INTEGER:
		    exprptr->arg.fval =
			mathfunc(exprptr->arg.optyp, (double)lresult.arg.ival);
		    break;
		case FLOAT:
		    exprptr->arg.fval =
			mathfunc(exprptr->arg.optyp, (double)lresult.arg.fval);
		    break;
		default:
		    fatalerrlin("evalexpr: math function on illegal type");
		    break;
	    }
	    exprptr->typ = FLOAT;
	    break;
	case ARRAYVAREXPR:
	    evalarrvar(exprptr);
	    break;
	case NEED:
	    lresult = *(exprptr->leftptr);  evalexpr(&lresult);
	    exprptr->typ = INTEGER;
	    exprptr->arg.ival = need(&lresult);
	    break;
	case UNDEFVAREXPR:
	    fprintf(stderr, "evalexpr: variable `%s'",exprptr->varptr->name);
	    fatalerrlin(" is undefined");
	    break;
	case NORETVALEXPR:
	    fatalerrlin("function returns no value (one is required)");
	    break;
	default:
	    fatalerrlin("evalexpr: illegal expr type");
	    break;
    }
}


#define FORWARDSORTVALUE  1
#define REVERSESORTVALUE -1
static int sortdirection;	/* forward (1), reverse (-1) multiplier value */
/*******************************************************************************
Comparison function for integers (called by sortaxisarray).
*******************************************************************************/
static int
axisintcmp(Axisnode **arg1, Axisnode **arg2)
{
    return sortdirection*((*arg1)->sortarg.ival - (*arg2)->sortarg.ival);
}


/*******************************************************************************
Comparison function for floats (called by sortaxisarray).
*******************************************************************************/
static int
axisfltcmp(Axisnode **arg1, Axisnode **arg2)
{
    float diff = (*arg1)->sortarg.fval - (*arg2)->sortarg.fval;
    if (diff == 0.0)
	return 0;
    else if (diff > 0)
	return sortdirection;
    else
	return -sortdirection;
}


/*******************************************************************************
Comparison function for strings (called by sortaxisarray).
*******************************************************************************/
static int
axisstrcmp(Axisnode **arg1, Axisnode **arg2)
{
    return sortdirection*mystrcmp((*arg1)->sortarg.sval, (*arg2)->sortarg.sval);
}


/*******************************************************************************
This function is called when a "sort(FIELD, expr)" statement is encountered.
It evaluates expr for each instance of FIELD, determines the type of these
values, (int, float, or string), selects the appropriate comparison function,
and then calls qsort to do the sorting.
*******************************************************************************/
static void
sortaxisarray(Exprnode *exprptr, int newdepth, Flag reverseflag)
{
    Exprnode result;
    int (*cmpfuncptr)();
    int i;

    axistbl[newdepth].first = 0;
    axistbl[newdepth].last = axistbl[newdepth].number - 1;

    for (i=axistbl[newdepth].first; i<=axistbl[newdepth].last; i++) {
	curfldvalptrtbl[newdepth] = axistbl[newdepth].array[i];
	result = *exprptr;
	evalexpr(&result);
	switch(result.typ) {
	    case INTEGER:
		(axistbl[newdepth].array[i])->sortarg.ival = result.arg.ival;
		break;
	    case FLOAT:
		(axistbl[newdepth].array[i])->sortarg.fval = result.arg.fval;
		break;
	    case STRING:
		(axistbl[newdepth].array[i])->sortarg.sval = result.arg.sval;
		break;
	    default:  fatalerrlin("sortaxisarray: illegal expr type (msg 1)");
		break;
	}
    }
    if (reverseflag) {
	sortdirection = REVERSESORTVALUE;
    } else {
	sortdirection = FORWARDSORTVALUE;
    }
    curfldvalptrtbl[newdepth] = NULL;
    switch(result.typ) {
	case INTEGER: cmpfuncptr = axisintcmp; break;
	case FLOAT:   cmpfuncptr = axisfltcmp; break;
	case STRING:  cmpfuncptr = axisstrcmp; break;
	default:  fatalerrlin("sortaxisarray: illegal expr type (msg 2)");
	    break;
    }

    qsort((void*)axistbl[newdepth].array, axistbl[newdepth].number,
					sizeof(Axisnode*), cmpfuncptr);
}


#define MAXPRINTFFMTLEN   15	/* formt code lenths (eg, "%-25.2f") */
#define MAXPRINTFSTRLEN 1023	/* max result string lenth */
static char printfcodes[] = "dfsioxXueEgG";
/*******************************************************************************
This function does the formatted printing (both PRINTF and SPRINTF).  It first
evaluates the user's format expression (argexprptrtbl[0]) - this STRING is
referenced by usrfmtptr.  It then sequences through the user's format string
copying everything except s/printf format (%) codes to rsltstr.  As % codes are
encountered, they are copied into tmpfmtstr.  This string is used by really
calling sprintf with tmpfmtstr as the format string and argexprptrtbl[n]
(evaluated) as its agument.  The result from sprintf is added to the end of
rsltstr (which has been accumulating).
*******************************************************************************/
void
printfmt(int stmtidx)
{
    Exprnode result, *lhsexprptr;
    Exprptrlistnode *exprptrlistptr;
    char *usrfmtptr, *rsltptr, *tmpfmtptr, *codeptr;
    char tmpfmtstr[MAXPRINTFFMTLEN+1], rsltstr[MAXPRINTFSTRLEN+1];
    int argidx;

    exprptrlistptr = stmts[stmtidx].s.printf_.exprptrlistptr;
    result = *exprptrlistptr->exprptr;	/* evaluate the FORMAT expr */
    evalexpr(&result);
    if (result.typ != STRING) 		/* it MUST be a string */
	fatalerrlin( "s/printf statment: format argument must be string expr");
    usrfmtptr = result.arg.sval;
    if (usrfmtptr == NULL)
	fatalerrlin("internal error: tried to print a null string");

    argidx = 1;
    rsltptr = rsltstr;
    while (*usrfmtptr) {		/* sequence thru user format string */
	if (*usrfmtptr == '%' && *(usrfmtptr+1)) {
	    tmpfmtptr = tmpfmtstr;
	    *tmpfmtptr++ = *usrfmtptr++;	/* copy the % to tmpfmtstr */
	    if (*usrfmtptr == '%') {		/* if user fmt str has "%%" */
		*rsltptr++ = *usrfmtptr++;	/* copy a % to rsltstr & cont */
		continue;
	    }
	    while (*usrfmtptr) {	/* copy all chars upto & including a */
		codeptr = printfcodes;	/* valid printf code to tmpfmtstr */
		while (*codeptr) {		/* break on a printf code ch */
		    if (*usrfmtptr == *codeptr) /* eg, 'd', 's', 'f', etc */
			break;
		    codeptr++;
		}		/* exit anyway, if we've checked them all */
		*tmpfmtptr++ = *usrfmtptr++;	/* copy this ch to tmpfmtstr */
		if (*codeptr)	/* break if we've found a printf code char */
		    break;
	    }
	    *tmpfmtptr = '\0';		/* terminate tmpfmtstr */

	    if (argidx < stmts[stmtidx].s.printf_.nargs) {
		exprptrlistptr = exprptrlistptr->next;
		result = *exprptrlistptr->exprptr;  /* evaluate the arg expr */
		evalexpr(&result);
		argidx++;
	    } else {	/* if there are more %d, %f, etc than argument(s) */
		fatalerrlin("s/printf format string: too many % codes");
	    }
	    switch (result.typ) {	/* sprintf it to the end of rsltstr */
		case INTEGER:
		    if (*codeptr == 's' || *codeptr == 'f')
			fatalerrlin("bad s/printf format code for an integer");
		    sprintf(rsltptr, tmpfmtstr, result.arg.ival);
		    break;
		case FLOAT:
		    if (*codeptr == 's' || *codeptr == 'd')
			fatalerrlin("bad s/printf format code for a float");
		    sprintf(rsltptr, tmpfmtstr, result.arg.fval);
		    break;
		case STRING:
		    if (*codeptr != 's')
			fatalerrlin("s/printf: use `%s' to print strings");
		    sprintf(rsltptr, tmpfmtstr, result.arg.sval);
		    break;
		case NORETVALEXPR:
		    fatalerrlin("printfmt: user function must return a value");
		default:
		    fatalerrlin("printfmt: s/printf hit default");
	    }
	    while (*rsltptr)	/* move rsltptr up to the end of rsltstr */
		rsltptr++;
	} else {			/* ordinary character, copy it as is */
	    *rsltptr++ = *usrfmtptr++;
	}
    }
    *rsltptr = '\0';		/* terminate rsltstr */

    if (stmts[stmtidx].typ == PRINTF) {		/* PRINTF: print it */
	printftrap(rsltstr);
    } else {						/* SPRINTF */
	switch (stmts[stmtidx].s.printf_.lhsexprptr->typ) {
	    case UFNCPARAMEXPR:
		lhsexprptr = &argstk[botargstkidx] +
				stmts[stmtidx].s.printf_.lhsexprptr->arg.ival;
		break;
	    case ARRAYVAREXPR:
		lhsexprptr=rtgetarrvaraddr(stmts[stmtidx].s.printf_.lhsexprptr);
		break;
	    default:			/* an "ordinary" variable */
		lhsexprptr = stmts[stmtidx].s.printf_.lhsexprptr;
		break;
	}
	lhsexprptr->arg.sval = (char*)malloc(strlen(rsltstr)+1);
	strcpy(lhsexprptr->arg.sval, rsltstr);
	lhsexprptr->typ = STRING;
    }
}


/*******************************************************************************
This function is called to sequence through "blocks" of statements.  The entire
program is the outer-most block, and each foreach statement begins another one.
That is, this function is RECURSIVE!!!
*******************************************************************************/
Exprnode
doblock(int begstmtidx, int endstmtidx)
{
    Exprnode  *lhsexprptr, result;
    int       newdepth, i;

    int stmtidx;
    stmtidx = begstmtidx;
    while (stmtidx <= endstmtidx) {
	curstmtidx = stmtidx;			/* global, for error messages */
	switch (stmts[stmtidx].typ) {
	    case PRINTF: case SPRINTF:
		printfmt(stmtidx);
		stmtidx++;
		break;
	    case FOREACH:
		newdepth = stmts[stmtidx].s.foreach.fldidx;
		for (i=axistbl[newdepth].first; i<=axistbl[newdepth].last; i++){
		    curfldvalptrtbl[newdepth] = axistbl[newdepth].array[i];

		    /* this skips "outer joins" where there is no data */
		    if (stmts[stmtidx].s.foreach.restrictflag) {
			chk4badflds(stmts[stmtidx].s.foreach.fldbits,
					    stmts[stmtidx].s.foreach.maxdepth);
			if (getcount(datarootptr,
					stmts[stmtidx].s.foreach.fldbits, 0,
					stmts[stmtidx].s.foreach.maxdepth) == 0)
			    continue; /* skip doblock */
		    }

		    doblock(stmtidx+1, stmts[stmtidx].s.foreach.next-1);
		}
		curfldvalptrtbl[newdepth] = NULL;
		stmtidx = stmts[stmtidx].s.foreach.next;
		break;
	    case WHILE:
		result = *(stmts[stmtidx].s.while_.exprptr);
		evalexpr(&result);
		while (result.arg.ival) {
		    doblock(stmtidx+1, stmts[stmtidx].s.while_.next-1);
		    result = *(stmts[stmtidx].s.while_.exprptr);
		    evalexpr(&result);
		}
		stmtidx = stmts[stmtidx].s.while_.next;
		break;
	    case DO:
		result = *(stmts[stmtidx].s.dowhile.exprptr);
		evalexpr(&result);
		if (result.arg.ival)
		    stmtidx = stmts[stmtidx].s.dowhile.next;
		else
		    stmtidx++;
		break;
	    case IF:
		result = *(stmts[stmtidx].s.if_.exprptr);
		evalexpr(&result);
		if (result.arg.ival)
		    stmtidx++;
		else
		    stmtidx = stmts[stmtidx].s.if_.next;
		break;
	    case ELSE:
		stmtidx = stmts[stmtidx].s.else_.next;
		break;
	    case ASSIGNOP:			/* assign ops are STATMENTS */
		switch (stmts[stmtidx].s.assign.lhsexprptr->typ) {
		    case UFNCPARAMEXPR:
			lhsexprptr = &argstk[botargstkidx]+stmts[stmtidx].s.
						    assign.lhsexprptr->arg.ival;
			break;
		    case ARRAYVAREXPR:
			lhsexprptr = rtgetarrvaraddr(
					    stmts[stmtidx].s.assign.lhsexprptr);
			break;
		    default:			/* an "ordinary" variable */
			lhsexprptr = stmts[stmtidx].s.assign.lhsexprptr;
			break;
		}
		assignop(lhsexprptr, stmts[stmtidx].s.assign.optyp,
					    stmts[stmtidx].s.assign.rhsexprptr);
		stmtidx++;
		break;
	    case SORT:
		newdepth = stmts[stmtidx].s.sort.fldidx;
		sortaxisarray(stmts[stmtidx].s.sort.exprptr, newdepth, 
					    stmts[stmtidx].s.sort.reverseflag);
		stmtidx++;
		break;
	    case FIRST:
		newdepth = stmts[stmtidx].s.fstlst.fldidx;
		result = *(stmts[stmtidx].s.fstlst.exprptr);
		evalexpr(&result);
		axistbl[newdepth].last =
		    MIN(axistbl[newdepth].first+result.arg.ival-1,
			axistbl[newdepth].last);
		stmtidx++;
		break;
	    case LAST:
		newdepth = stmts[stmtidx].s.fstlst.fldidx;
		result = *(stmts[stmtidx].s.fstlst.exprptr);
		evalexpr(&result);
		axistbl[newdepth].first =
		    MAX(axistbl[newdepth].last-result.arg.ival+1,
			axistbl[newdepth].first);
		stmtidx++;
		break;
	    case SYSTEM:
		result = *(stmts[stmtidx].s.system_.exprptr);
		evalexpr(&result);
		if (result.typ == STRING) {
		    (void) system(result.arg.sval);
		} else {
		    fprintf(stderr,
			    "`system' must be called with a string argument");
		}
		stmtidx++;
		break;
	    case RETURN:
		if (stmts[stmtidx].s.return_.exprptr != NULL) {
		    result = *(stmts[stmtidx].s.return_.exprptr);
		    evalexpr(&result);
		} else {
		    result.typ = NORETVALEXPR;
		}
		return result;
	    case EXPRSTMT:
		result = *(stmts[stmtidx].s.expr.exprptr);
		evalexpr(&result);
		stmtidx++;
		break;
	    case TRAP:
		updatetraptbl(stmts[stmtidx].s.trap.linnumexprptr,
				    stmts[stmtidx].s.trap.ufncexprptr);
		stmtidx++;
		break;
	    default:
		fatalerrlin("doblock: illegal statement type");
		stmtidx++;
		break;
	}
    }
    result.typ = NORETVALEXPR;
    return result;
}


static int startstmtidx;
/*******************************************************************************
*******************************************************************************/
void
savestartstmt(int stmtidx)
{
    startstmtidx = stmtidx;
}


/*******************************************************************************
Allocate space for the axis arrays.
*******************************************************************************/
void
initaxistbl()
{
    extern int repnfields;

    axistbl = (Axistbl*)malloc((size_t)repnfields*sizeof(Axistbl));
    curfldvalptrtbl =
		(Axisnode**)malloc((size_t)(repnfields+1)*sizeof(Axisnode*));
}


/*******************************************************************************
Initialize the axis arrays.
*******************************************************************************/
void
printstart()
{
    extern Flag runtimeflag;	/* FALSE = compile time, TRUE = runtime */
    extern int aggrmaxdepth;
    int depth;

    for (depth=0; depth<=aggrmaxdepth; depth++) {
	axistbl[depth].array =
			(Axisnode**)malloc((size_t)axistbl[depth].number*
							 sizeof(Axisnode*));
	axisarrayptr = axistbl[depth].array;
	if (axistbl[depth].rootptr == NULL)
	    fatalerror("There is no input data");
	axis2array(axistbl[depth].rootptr);
	axistbl[depth].first = 0;
	axistbl[depth].last = axistbl[depth].number - 1;
    }

    maxargstkctr = INITARGSTKSIZE;	/* start the stack off */
    argstk = (Exprnode*)malloc((size_t)maxargstkctr*sizeof(Exprnode));

    runtimeflag = TRUE;
    doblock(startstmtidx, nstmts-1);
    printendtraps();
}
