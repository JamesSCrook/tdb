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
	expr.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "tdb.h"
#include "y.tab.h"

Exprnode *funcexprptr;


/*******************************************************************************
Add an INTEGER expression node.
*******************************************************************************/
Exprnode*
addinteger(int intvalue)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = INTEGER;
    exprptr->arg.ival = intvalue;
    exprptr->leftptr = exprptr->rightptr = NULL;
    return exprptr;
}


/*******************************************************************************
Add a FLOAT expression node.
*******************************************************************************/
Exprnode*
addfloat(float floatvalue)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = FLOAT;
    exprptr->arg.fval = floatvalue;
    exprptr->leftptr = exprptr->rightptr = NULL;
    return exprptr;
}


/*******************************************************************************
Add a STRING function) expression node.
*******************************************************************************/
Exprnode*
addstring(char *stringvalue)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = STRING;
    exprptr->arg.sval = stringvalue;
    exprptr->leftptr = exprptr->rightptr = NULL;
    return exprptr;
}


#define INITEXPRPTRLISTSTKSIZE 8
#define EXPRPTRLISTSTKMULTFACT 2
Exprptrliststk *exprptrliststk;
int exprptrliststkidx = -1;
static int maxexprptrliststkidx = 0;
/*******************************************************************************
*******************************************************************************/
void
setupexprptrlist()
{
    exprptrliststkidx++;
    if (exprptrliststkidx >= maxexprptrliststkidx) {
	if (maxexprptrliststkidx == 0) {
	    maxexprptrliststkidx = INITEXPRPTRLISTSTKSIZE;
	    exprptrliststk = (Exprptrliststk*)malloc(maxexprptrliststkidx*
							sizeof(Exprptrliststk));
	} else {
	    maxexprptrliststkidx *= EXPRPTRLISTSTKMULTFACT;
	    exprptrliststk = (Exprptrliststk*)realloc((void*)exprptrliststk,
				maxexprptrliststkidx*sizeof(Exprptrliststk));
	}
    }
    exprptrliststk[exprptrliststkidx].exprptrlistptr = NULL;
    exprptrliststk[exprptrliststkidx].exprctr = 0;
}


/*******************************************************************************
Add the expression pointer (argexprptr) to the linked list of expression list
nodes.
*******************************************************************************/
void
addexprptrlistarg(Exprnode *argexprptr)
{
    Exprptrlistnode *exprptrlistptr, *newexprptrlistptr;

    newexprptrlistptr = (Exprptrlistnode*)malloc(sizeof(Exprptrlistnode));
    newexprptrlistptr->exprptr = argexprptr;
    newexprptrlistptr->next = NULL;
    exprptrlistptr = exprptrliststk[exprptrliststkidx].exprptrlistptr;
    if (exprptrlistptr == NULL) {	/* new node is the first in llist */
	exprptrliststk[exprptrliststkidx].exprptrlistptr = newexprptrlistptr;
    } else {				/* add new node to END of the  llist */
	while (exprptrlistptr->next != NULL)
	    exprptrlistptr = exprptrlistptr->next;
	exprptrlistptr->next = newexprptrlistptr;
    }
    exprptrliststk[exprptrliststkidx].exprctr++;	/* update counter */
}


#define NUFNCARGMSG \
"function `%s': defined with %d argument(s), called with %d"
/*******************************************************************************
Add to linked list.
*******************************************************************************/
Exprnode *
updateufnccall(char *ufncname)
{
    Varnode  *varptr;
    Exprnode *ufncexprptr;

    ufncexprptr = (Exprnode*)malloc(sizeof(Exprnode));
    ufncexprptr->typ = UFNCCALLEXPR;
    ufncexprptr->arg.ufnc.exprptrlistptr =
			    exprptrliststk[exprptrliststkidx].exprptrlistptr;
    ufncexprptr->arg.ufnc.argctr = exprptrliststk[exprptrliststkidx--].exprctr;
    if ((varptr=ufncdefined(ufncname)) != NULL) {	/* NOT forward func */
	if (varptr->arg.ufnc.paramctr != ufncexprptr->arg.ufnc.argctr) {
	    fprintf(stderr,NUFNCARGMSG, ufncname, varptr->arg.ufnc.paramctr,
						ufncexprptr->arg.ufnc.argctr);
	    fatalerrlin("");
	}
	ufncexprptr->arg.ufnc.begstmtidx = varptr->arg.ufnc.begstmtidx;
    } else {		/* a forward referenced user function */
	addfrwdufnc(ufncname, ufncexprptr);
    }
    return ufncexprptr;
}


/*******************************************************************************
Add to linked list.
*******************************************************************************/
Exprnode *
updatearrvar(char *varnam)
{
    Exprnode *arrvarexprptr;

    if (exprptrliststk[exprptrliststkidx].exprctr < 1 ||
		    exprptrliststk[exprptrliststkidx].exprctr > MAXARRAYDIMS)
	fatalerrlin("too many array arguments between `[' and `]'");
    arrvarexprptr = ctgetarrvaraddr(varnam,
			    exprptrliststk[exprptrliststkidx].exprptrlistptr,
			    exprptrliststk[exprptrliststkidx].exprctr);
    exprptrliststkidx--;
    return arrvarexprptr;
}


/*******************************************************************************
Add a FIELD expression node.
*******************************************************************************/
Exprnode*
addfld(int fldidx)
{
    Exprnode *exprptr;
    extern Field *fldtbl;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = FIELD;
    exprptr->arg.ival = fldtbl[fldidx].idx;
    exprptr->leftptr = exprptr->rightptr = NULL;
    return exprptr;
}


int statsinfofldctr = 0;
/*******************************************************************************
Initialize an intrinsic function (COUNT, NUMBER, SUM, or SUMSQRD) expression
node.
*******************************************************************************/
void
tdbfncinit(int functyp, int fldidx)
{
    extern Field *fldtbl;
    extern Opmode opmode;
    extern Statsinfofld *statsinfofldtbl;

    if (opmode != REPORTMODE)
	fatalerrlin("tdbfncinit: function only available in report mode");

    funcexprptr = (Exprnode*)malloc(sizeof(Exprnode));
    funcexprptr->typ = functyp;
    funcexprptr->leftptr = funcexprptr->rightptr = NULL;
    funcexprptr->arg.tdbfnc.fldbits = 0L;
    funcexprptr->arg.tdbfnc.maxdepth = INVALIDFLDIDX;
    switch (functyp) {
	case NUMBER:
	    funcexprptr->arg.tdbfnc.depth = fldtbl[fldidx].idx;
	    break;
	case SUM:
	case SUMSQRD:
	    if (fldtbl[fldidx].typ == STRING)
		fatalerrlin("data (first) field's type must be numerical");
	    if (!fldtbl[fldidx].statsflag) {
		fldtbl[fldidx].statsflag = TRUE;
		fldtbl[fldidx].statsidx = statsinfofldctr;
		statsinfofldtbl[statsinfofldctr].fldidx = fldidx;
		statsinfofldtbl[statsinfofldctr].typ = fldtbl[fldidx].typ;
		statsinfofldctr++;
	    }
	    funcexprptr->arg.tdbfnc.depth = fldtbl[fldidx].statsidx;
	    break;
    }
}


/*******************************************************************************
Add (append) a field to those already associated with an intrinsic function
(COUNT, NUMBER, SUM, SUMSQRD) expression node.
*******************************************************************************/
void
tdbfncaddfld(int fldidx)
{
    extern Field *fldtbl;

    long l = fldtbl[fldidx].idx;
    funcexprptr->arg.tdbfnc.fldbits |= 1L << l;
    if (l > funcexprptr->arg.tdbfnc.maxdepth)
	funcexprptr->arg.tdbfnc.maxdepth = l;
}


/*******************************************************************************
Add a unary operation expression node.
*******************************************************************************/
Exprnode*
addunaryop(int operator, Exprnode *argexprptr)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = UNARYOPEXPR;
    exprptr->arg.optyp = operator;
    exprptr->leftptr = argexprptr;
    return exprptr;
}


/*******************************************************************************
Add a binary operation expression node.
*******************************************************************************/
Exprnode*
addbinop(Exprnode *larg, int operator, Exprnode *rarg)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = BINARYOPEXPR;
    exprptr->arg.optyp = operator;
    exprptr->leftptr = larg;
    exprptr->rightptr = rarg;
    return exprptr;
}


/*******************************************************************************
Add a TYPE (cast) operation expression node.
*******************************************************************************/
Exprnode*
addcast(int casttyp, Exprnode *arg)
{
    Exprnode *exprptr;

    if (casttyp != INTEGER && casttyp != FLOAT)
	fatalerrlin("illegal type cast type");
    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = TYPE;
    exprptr->arg.casttyp = casttyp;
    exprptr->leftptr = arg;
    exprptr->rightptr = NULL;
    return exprptr;
}


/*******************************************************************************
Add a REPORTDT (DAT or TIM) expression node.
*******************************************************************************/
Exprnode*
addreportdt(int fldidx)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = REPORTDT;
    exprptr->arg.ival = fldidx;
    exprptr->leftptr = exprptr->rightptr = NULL;
    return exprptr;
}


/*******************************************************************************
Add a FORMATDT (DAT or TIM) expression node.
*******************************************************************************/
Exprnode*
addformatdt(int fldidx, Exprnode *fmtexprptr, Exprnode *dtexprptr)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = FORMATDT;
    exprptr->arg.ival = fldidx;
    exprptr->leftptr = fmtexprptr;
    exprptr->rightptr = dtexprptr;
    return exprptr;
}


/*******************************************************************************
Add a STRFUNCEXPR (built-in string function: PATMATCH INDEXSTR STRLEN SUBSTR
LOOKUP) expression node.  The parameters expr2ptr, expr3ptr are passed
NULL pointers when the function does not require that many arguments.
*******************************************************************************/
Exprnode*
addstrfunc(int functyp, Exprnode *expr1ptr, Exprnode *expr2ptr, Exprnode *expr3ptr)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = STRFUNCEXPR;
    exprptr->leftptr = expr1ptr;
    exprptr->rightptr = expr2ptr;
    exprptr->arg.strfnc.functyp = functyp;
    exprptr->arg.strfnc.expr3ptr = expr3ptr;
    return exprptr;
}


/*******************************************************************************
Add an ATOIF (ATOI or ATOF) expression node.
*******************************************************************************/
Exprnode*
addatoif(int functyp, Exprnode *argexprptr)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = functyp;
    exprptr->leftptr = argexprptr;
    return exprptr;
}


/*******************************************************************************
Add a MATHFUNC (SIN COS TAN ARCSIN ARCCOS ARCTAN LOG LOG10 EXP)
expression node.
*******************************************************************************/
Exprnode*
addmathfunc(int operatortyp, Exprnode *argexprptr)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = MATHFUNC;
    exprptr->arg.optyp = operatortyp;
    exprptr->leftptr = argexprptr;
    return exprptr;
}


/*******************************************************************************
Add a NEED function expression node.
*******************************************************************************/
Exprnode*
addneedfunc(Exprnode *argexprptr)
{
    Exprnode *exprptr;

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = NEED;
    exprptr->leftptr = argexprptr;
    return exprptr;
}
