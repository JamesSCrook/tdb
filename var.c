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
	var.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "tdb.h"
#include "avlbal.h"
#include "y.tab.h"

/*******************************************************************************
Below is a diagram of the structure of variables, which comprise "normal"
variables, functions, and array variables.  The expression contained within the
Varnode structure is "dashed" ("e e e e") because it is only present for
"normal" variables, (this is where the value of that variable is stored).
Array variables are the most complicated case.  The case of a two-dimensional
array variable named "fred" is shown below.


v: Varnode			         eeeeeee(typ=ARRVAREXPR)
a: Arrvarnode                     varptr e     e
e: Exprnode          +-------------------e     e---+ arg.arrvar.exprptrlistptr
l: Exprptrlistnode   |                   eeeeeee   V
                     |                           lllllllllll   lllllllllll
 varrootptr-+        |                           l exprptr l   l exprptr l
            |        |                           l    next l-->l    next l-+
            V        |                           lllllllllll   lllllllllll |
       vvvvvvvvvv<---+               aaaaaaaaaa                            V
      v  "fred"  v                  a          a                          null
     v arg.arrvar.rootptr--------->a     idxtbl-a------>iiiiiii
    v e e e e      v              a eeeeeee      a      i typei
   v  e     e       v            a  e     e       a     i IFS i
    v e     e      v              a e     e      a      iiiiiii
     ve e e e     v                aeeeeeee     a       i typei
      vLptr  Rptrv                  aLptr  Rptra        i IFS i
       /vvvvvvvv\                    /aaaaaaaa\         iiiiiii
      V          V                  V          V

i: arrvaridxnode
IFS: abbreviation for Intfltstr (int, float, string)
*******************************************************************************/

static Varnode  *curufncvarptr = NULL;
static Varnode *varrootptr = NULL;


/*******************************************************************************
This function searches the "variable" tree (containing global variables and
user function names for the "variable" varname.  If varname is already in the
tree, it returns it's address.  If it is not in the tree and addreqflag is
TRUE, it is added to the tree, and it's address is again returned (the added
result flag pointer variable is also set).  If it is not in the tree and
addreqflag is FALSE, it returns NULL.
*******************************************************************************/
Varnode*
accessvar(char *varname, Flag addreqflag, Flag *addedresflagptr)
{
    Varnode *varptr, *newvarptr;
    int r;

    varptr = varrootptr;
    while (varptr != (Varnode*)NULL) {
	r = mystrcmp(varname, varptr->name);
	if (r == 0) {
	    *addedresflagptr = FALSE;
	    return varptr;
	} else if (r < 0 && varptr->leftptr != NULL) {
	    varptr = varptr->leftptr;
	} else if (r > 0 && varptr->rightptr != NULL) {
	    varptr = varptr->rightptr;
	} else {
	    break;
	}
    }

    if (addreqflag) {
	newvarptr = (Varnode*)malloc(sizeof(Varnode));
	newvarptr->name = (char*)malloc((size_t)(strlen(varname)+1));
	(void) strcpy(newvarptr->name, varname);
	newvarptr->leftptr = newvarptr->rightptr = (Varnode*)NULL;
	*addedresflagptr = TRUE;

	if (varptr == NULL) {
	    varrootptr = newvarptr;
	} else if (r < 0) {
	    varptr->leftptr = newvarptr;
	} else {
	    varptr->rightptr = newvarptr;
	}
	*addedresflagptr = TRUE;
	return newvarptr;
    } else {
	*addedresflagptr = FALSE;
	return NULL;
    }
}


/*******************************************************************************
Called to add the DEFINITION of a new user function name.
*******************************************************************************/
void
addufncname(char *ufncname, int begstmtidx)
{
    Varnode *varptr;
    Flag addedflag;

    varptr = accessvar(ufncname, TRUE, &addedflag);
    if (addedflag) {
	varptr->typ = UFNCNAMVAR;
	varptr->arg.ufnc.begstmtidx = begstmtidx;
	varptr->arg.ufnc.paramctr = 0;
	varptr->arg.ufnc.paramlistptr = NULL;
	curufncvarptr = varptr;
    } else {
	fprintf(stderr, "function `%s' ", ufncname);
	fatalerrlin("is defined more than once");
    }
}


/*******************************************************************************
Backwards linked list
*******************************************************************************/
void
addufncparam(char *paramname)
{
    Ufncparamnode *ufncparamptr, *newufncparamptr;

    ufncparamptr = curufncvarptr->arg.ufnc.paramlistptr;
    while (ufncparamptr != NULL) {
	if (!strcmp(paramname, ufncparamptr->name)) {
	    fprintf(stderr, "duplicate parameter: `%s'", paramname);
	    fatalerrlin("");
	}
	ufncparamptr = ufncparamptr->nextptr;
    }

    newufncparamptr = (Ufncparamnode*)malloc(sizeof(Ufncparamnode));
    newufncparamptr->name = (char*)malloc((size_t)strlen(paramname)+1);
    (void) strcpy(newufncparamptr->name, paramname);
    newufncparamptr->expr.typ = UFNCPARAMEXPR;
    newufncparamptr->expr.arg.ival = curufncvarptr->arg.ufnc.paramctr++;

    newufncparamptr->nextptr = curufncvarptr->arg.ufnc.paramlistptr;
    curufncvarptr->arg.ufnc.paramlistptr = newufncparamptr;
}


/*******************************************************************************
*******************************************************************************/
Varnode *
ufncdefined(char *ufncname)
{
    Varnode *varptr;
    Flag addedflag;

    if ((varptr=accessvar(ufncname, FALSE, &addedflag)) != NULL) {
	if (varptr->typ != UFNCNAMVAR) {
	    fprintf(stderr, "function `%s' ", ufncname);
	    fatalerrlin("is already defined as a non-function");
	}
	return varptr;
    } else {
	return NULL;
    }
}


/*******************************************************************************
*******************************************************************************/
Exprnode *
getufncparam(char *paramname)
{
    Ufncparamnode *ufncparamptr;

    ufncparamptr = curufncvarptr->arg.ufnc.paramlistptr;
    while (ufncparamptr != NULL) {
	if (!strcmp(paramname, ufncparamptr->name))
	    return &ufncparamptr->expr;
	ufncparamptr = ufncparamptr->nextptr;
    }
    return NULL;
}


/*******************************************************************************
This func always returns a pointer to an expression node.  If there is already
a variable of this name, a pointer to its expression node is returned.  If the
variable doesn't already exist (in the variable tree), it is created, and a
pointer to the expression node is returned.
*******************************************************************************/
Exprnode *
getvaraddr(char *varname)
{
    Varnode *varptr;
    Exprnode *exprptr;
    Flag addedflag;

    if (curufncvarptr != NULL) {	/* "in" a function? */
	exprptr = getufncparam(varname);  /* is this var a function parameter */
	if (exprptr != NULL)
	    return exprptr;		/* yes, return a pointer to it */
    }

    varptr = accessvar(varname, TRUE, &addedflag); 
    if (addedflag) {
	varptr->typ = VARIABLE;
	varptr->arg.varexpr.typ = UNDEFVAREXPR;
	varptr->arg.varexpr.varptr = varptr;
    } else if (varptr->typ != VARIABLE) {
	fprintf(stderr, "getvaraddr: '%s' is not a variable", varname);
	fatalerrlin("");
    }
    return &(varptr->arg.varexpr);
}


/*******************************************************************************
*******************************************************************************/
void
addcmdlinevar(char *name_value, int exprtyp)
{
    Exprnode *exprptr;
    char *name, *value;

    if ((value=strchr(name_value, ASSIGNCHAR)) == NULL) {
	fprintf(stderr,
		"bad command line variable assignment: %s (use name%cvalue)",
		name_value, ASSIGNCHAR);
	return;
    }

    name = name_value;
    *value++ = '\0';
    exprptr = getvaraddr(name);
    switch(exprtyp) {
	case INTEGER: exprptr->arg.ival = atoi(value); break;
	case FLOAT: exprptr->arg.fval = (float)atof(value); break;
	case STRING: exprptr->arg.sval = value; break;
	default: fatalerror("addcmdlinevar: illegal expr type"); break;
    }
    exprptr->typ = exprtyp;
}



typedef struct frwdufncnode {
    char                *name;
    Exprnode            *exprptr;
    struct frwdufncnode *nextptr;
} Frwdufncnode;

static Frwdufncnode *frwdufnclistptr = NULL;
/*******************************************************************************
*******************************************************************************/
void
addfrwdufnc(char *ufncname, Exprnode *exprptr)
{
    Frwdufncnode *frwdufncptr;

    if (curufncvarptr == NULL) {
	fprintf(stderr, "user function `%s' is undefined", ufncname);
	fatalerrlin("");
    }
    frwdufncptr = (Frwdufncnode*)malloc(sizeof(Frwdufncnode));
    frwdufncptr->name       = ufncname;
    frwdufncptr->exprptr    = exprptr;
    frwdufncptr->nextptr    = frwdufnclistptr;
    frwdufnclistptr = frwdufncptr;
}


#define NUFNCARGMSG \
"(forward) function `%s': defined with %d argument(s), called with %d"
/*******************************************************************************
*******************************************************************************/
void
resolvefrwdufncs()
{
    Frwdufncnode *frwdufncptr;
    Varnode      *varptr;
    Flag addedflag;

    frwdufncptr = frwdufnclistptr;
    while (frwdufncptr != NULL) {
	if ((varptr=accessvar(frwdufncptr->name, FALSE, &addedflag)) != NULL) {
	    if (varptr->arg.ufnc.paramctr !=
					frwdufncptr->exprptr->arg.ufnc.argctr) {
		fprintf(stderr, NUFNCARGMSG, frwdufncptr->name,
					varptr->arg.ufnc.paramctr,
					frwdufncptr->exprptr->arg.ufnc.argctr);
		fatalerror("");	/* do NOT use fatalerrlin here!!! */
	    }
	    frwdufncptr->exprptr->arg.ufnc.begstmtidx =
						    varptr->arg.ufnc.begstmtidx;
	} else {
	    fprintf(stderr, "function `%s' is ", frwdufncptr->name);
	    fatalerror("not defined");	/* do NOT use fatalerrlin here!!! */
	}
	frwdufncptr = frwdufncptr->nextptr;
    }

    curufncvarptr = NULL;
}


/*******************************************************************************
Compile time
*******************************************************************************/
Exprnode *
ctgetarrvaraddr(char *varnam, Exprptrlistnode *exprptrlistptr, int exprctr)
{
    Varnode *varptr;
    Exprnode *exprptr;
    Flag addedflag;

    varptr = accessvar(varnam, TRUE, &addedflag); 
    if (addedflag) {
	varptr->typ = ARRAYVAR;
	varptr->arg.arrvar.exprctr = exprctr;
	varptr->arg.arrvar.rootptr = NULL;
    } else if (varptr->typ != ARRAYVAR) {
	fprintf(stderr, "getarrvaraddr: array '%s':", varnam);
	fatalerrlin(" used elsewhere as a non array");
    } else if (varptr->arg.arrvar.exprctr != exprctr) {
	fprintf(stderr, "getarrvaraddr: array '%s':", varnam);
	fatalerrlin(" incorrect number of indices");
    }

    exprptr = (Exprnode*)malloc(sizeof(Exprnode));
    exprptr->typ = ARRAYVAREXPR;
    exprptr->arg.arrvar.exprptrlistptr = exprptrlistptr;
    exprptr->varptr = varptr;
    return exprptr;
}


/*******************************************************************************
*******************************************************************************/
int
arrvaridxcmp(Arrvaridxnode *arrvaridxtbl1, Arrvaridxnode *arrvaridxtbl2, int dimctr)
{
    int dimidx, r;
    float f;

    for (dimidx=0; dimidx<dimctr; dimidx++) {	/* for each dimension */
	if ((r=arrvaridxtbl1[dimidx].typ-arrvaridxtbl2[dimidx].typ) != 0)
	    return r;
	switch(arrvaridxtbl1[dimidx].typ) {
	    case INTEGER:
		r=arrvaridxtbl1[dimidx].idx.ival-arrvaridxtbl2[dimidx].idx.ival;
		if (r != 0)
		    return r;
		break;
	    case FLOAT:
		f=arrvaridxtbl1[dimidx].idx.fval-arrvaridxtbl2[dimidx].idx.fval;
		if (f < 0.0)
		    return -1;
		else if (f > 0.0)
		    return 1;
		break;
	    case STRING:
		r = mystrcmp(arrvaridxtbl1[dimidx].idx.sval,
						arrvaridxtbl2[dimidx].idx.sval);
		if (r != 0)
		    return r;
		break;
	    default:
		fatalerror("arrvaridxcmp: hit default");
	}
    }
    return 0;
}


/*******************************************************************************
*******************************************************************************/
Arrvarnode *
accessarrvar(Exprnode *exprptr, Flag addreqflag)
{
    Exprnode result;
    Exprptrlistnode *exprptrlistptr;
    Arrvaridxnode arrvaridxtbl[MAXARRAYDIMS];   /* max # of array dimensions */
    Arrvarnode *curarrvarptr, *newarrvarptr, *arrvarptrstk[MAXAVLDEPTH];
    Arrvarnode **posarrvarptrptr, **curarrvarptrptr;
    int r, dimidx, prevbalfact, stkdepth, didx, dimctr = 0;

			/* calculate the index value(s) */
    exprptrlistptr = exprptr->arg.arrvar.exprptrlistptr;
    while (exprptrlistptr != NULL) {
	result = *exprptrlistptr->exprptr;
	evalexpr(&result);
	arrvaridxtbl[dimctr].typ = result.typ;
	switch (result.typ) {
	    case INTEGER: arrvaridxtbl[dimctr].idx.ival = result.arg.ival;break;
	    case FLOAT: arrvaridxtbl[dimctr].idx.fval = result.arg.fval; break;
	    case STRING: arrvaridxtbl[dimctr].idx.sval = result.arg.sval;break;
	    default: fatalerrlin("accessarrvar: hit default");
	}
	exprptrlistptr = exprptrlistptr->next;
	dimctr++;
    }

    stkdepth = 0;
    curarrvarptrptr = &exprptr->varptr->arg.arrvar.rootptr;
    curarrvarptr =     exprptr->varptr->arg.arrvar.rootptr;
    while (curarrvarptr != NULL) {
	r = arrvaridxcmp(arrvaridxtbl, curarrvarptr->idxtbl, dimctr);
	if (r == 0)
	    return curarrvarptr;
	arrvarptrstk[stkdepth++] = curarrvarptr;
	if (ABS(curarrvarptr->balfact) == 1)
	    posarrvarptrptr = curarrvarptrptr;
	if (r < 0 && curarrvarptr->leftptr != NULL) {
	    curarrvarptrptr = &curarrvarptr->leftptr;
	    curarrvarptr =     curarrvarptr->leftptr;
	} else if (r > 0 && curarrvarptr->rightptr != NULL) {
	    curarrvarptrptr = &curarrvarptr->rightptr;
	    curarrvarptr =     curarrvarptr->rightptr;
	} else {
	    break;
	}
    }

    if (addreqflag) {
	newarrvarptr = (Arrvarnode*)malloc(sizeof(Arrvarnode));
	newarrvarptr->idxtbl =
		(Arrvaridxnode*)malloc((size_t)dimctr*sizeof(Arrvaridxnode));
	newarrvarptr->expr.typ = UNDEFVAREXPR;
	newarrvarptr->expr.varptr = exprptr->varptr;
	newarrvarptr->balfact = 0;
	newarrvarptr->leftptr = newarrvarptr->rightptr = NULL;
	for (dimidx=0; dimidx<dimctr; dimidx++)
	    newarrvarptr->idxtbl[dimidx] = arrvaridxtbl[dimidx];

	if (curarrvarptr == NULL) {
	    exprptr->varptr->arg.arrvar.rootptr = newarrvarptr;
	} else {
	    if (r < 0) {
		curarrvarptr->leftptr = newarrvarptr;
	    } else {
		curarrvarptr->rightptr = newarrvarptr;
	    }
	    arrvarptrstk[stkdepth++] = newarrvarptr;
	    for (didx=stkdepth-2; didx>=0; didx--) {
		prevbalfact = arrvarptrstk[didx]->balfact;
		if (arrvarptrstk[didx+1] == arrvarptrstk[didx]->leftptr) {
		    arrvarptrstk[didx]->balfact--;
		} else {
		    arrvarptrstk[didx]->balfact++;
		}
		if (ABS(arrvarptrstk[didx]->balfact) == 2) {
			    /* Note: this is a MACRO, no ';' needed !!!*/
		    BALAVLSUBTREE(Arrvarnode, arrvarptrstk[didx],
					arrvarptrstk[didx+1], posarrvarptrptr)
		    break;
		} else if (ABS(arrvarptrstk[didx]->balfact)<= ABS(prevbalfact)){
		    break;
		}
	    }
	}
	return newarrvarptr;
    } else {		/* could not find it */
	return NULL;
    }
}


/*******************************************************************************
*******************************************************************************/
void
evalarrvar(Exprnode *exprptr)
{
    Arrvarnode *arrvarptr;

    if ((arrvarptr=accessarrvar(exprptr, FALSE)) == NULL ||
					arrvarptr->expr.typ == UNDEFVAREXPR) {
	fprintf(stderr, "array variable `%s'", exprptr->varptr->name);
	fatalerrlin(" is undefined");
    } else {
	*exprptr = arrvarptr->expr;
    }
}


/*******************************************************************************
Run time.
*******************************************************************************/
Exprnode *
rtgetarrvaraddr(Exprnode *lhsexprptr)
{
    Arrvarnode *arrvarptr;

    arrvarptr = accessarrvar(lhsexprptr, TRUE);
    return &arrvarptr->expr;
}
