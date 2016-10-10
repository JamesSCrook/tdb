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
	calc.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <regex.h>
#include "tdb.h"
#include "y.tab.h"

#define MKTIMEYEAROFFSET 1900


/*******************************************************************************
Evaluate binary operations on two integer arguments.
*******************************************************************************/
int
ibinop(int i1, int operator, int i2)
{
    int result = 0;

    switch (operator) {
	case ADD:	result = i1 +  i2; break;
	case SUBTRACT:	result = i1 -  i2; break;
	case MULTIPLY:	result = i1 *  i2; break;
	case DIVIDE:
	    if (i2 == 0)
		fatalerrlin("ibinop: integer divide by zero");
	    result = i1 / i2;
	    break;
	case MOD:
	    if (i2 == 0)
		fatalerrlin("ibinop: integer mod by zero");
	    result = i1 % i2;
	    break;
	case LESTHAN:	result = i1 <  i2; break;
	case LESTHANEQ:	result = i1 <= i2; break;
	case GRTTHAN:	result = i1 >  i2; break;
	case GRTTHANEQ:	result = i1 >= i2; break;
	case EQUAL:	result = i1 == i2; break;
	case NOTEQUAL:	result = i1 != i2; break;
	case AND:	result = i1 && i2; break;
	case OR:	result = i1 || i2; break;
	case POWER:	result = (int)pow((double)i1, (double)i2); break;
	default: fatalerrlin("ibinop: illegal integer binary operation"); break;
    }
    return result;
}


/*******************************************************************************
Evaluate binary operations on two float arguments.
*******************************************************************************/
int
fbinop(float f1, int operator, float f2, float *floatptr, int *intptr)
{		/* Note: MOD is not valid for floats */
    double pow();

    switch (operator) {
	case ADD:	*floatptr = f1 + f2; return FLOAT;
	case SUBTRACT:	*floatptr = f1 - f2; return FLOAT;
	case MULTIPLY:	*floatptr = f1 * f2; return FLOAT;
	case DIVIDE:
	    if (f2 == 0.0)
		fatalerrlin("fbinop: floating point divide by zero");
	    *floatptr = f1 / f2;
	    return FLOAT;
	case POWER:
	    *floatptr = (float)pow((double)f1, (double)f2);
	    return FLOAT;
    }
    switch (operator) {
	case LESTHAN:	*intptr = (f1 <  f2) ? TRUE: FALSE; break;
	case LESTHANEQ:	*intptr = (f1 <= f2) ? TRUE: FALSE; break;
	case GRTTHAN:	*intptr = (f1 >  f2) ? TRUE: FALSE; break;
	case GRTTHANEQ:	*intptr = (f1 >= f2) ? TRUE: FALSE; break;
	case EQUAL:	*intptr = (f1 == f2) ? TRUE: FALSE; break;
	case NOTEQUAL:	*intptr = (f1 != f2) ? TRUE: FALSE; break;
	case AND:	*intptr = (f1 && f2) ? TRUE: FALSE; break;
	case OR:	*intptr = (f1 || f2) ? TRUE: FALSE; break;
	default:
	    fatalerrlin("fbinop: illegal floating point binary operation");
	    break;
    }
    return INTEGER;
}


/*******************************************************************************
Evaluate binary operations on two string (char*) arguments.
*******************************************************************************/
void
sbinop(char *s1, char *s2, Exprnode *exprptr)
{
    switch (exprptr->arg.optyp) {
	case LESTHAN:	exprptr->arg.ival = mystrcmp(s1, s2) <  0; break;
	case LESTHANEQ:	exprptr->arg.ival = mystrcmp(s1, s2) <= 0; break;
	case GRTTHAN:	exprptr->arg.ival = mystrcmp(s1, s2) >  0; break;
	case GRTTHANEQ:	exprptr->arg.ival = mystrcmp(s1, s2) >= 0; break;
	case EQUAL:	exprptr->arg.ival = mystrcmp(s1, s2) == 0; break;
	case NOTEQUAL:	exprptr->arg.ival = mystrcmp(s1, s2) != 0; break;
	case ADD:
	    exprptr->arg.sval =
			    (char*)malloc((size_t)(strlen(s1)+strlen(s2)+1));
	    (void) strcpy(exprptr->arg.sval, s1);
	    (void) strcat(exprptr->arg.sval, s2);
	    exprptr->typ = STRING;
	    return;
	default:
	    fatalerrlin("sbinop: illegal string binary operation");
    }
    exprptr->typ = INTEGER;
}


/*******************************************************************************
This function is only called when in report mode, NEVER from select mode.
*******************************************************************************/
void
assignop(Exprnode *lhsexprptr, int optyp, Exprnode *rhsexprptr)
{
    Exprnode lresult, expr, rresult;

    rresult = *rhsexprptr;
    evalexpr(&rresult);
    if (rresult.typ == NORETVALEXPR)
	fatalerrlin("function returns no value (one is required)");

    if (optyp == ASSIGN) {			/* simple assign (=) */
	*lhsexprptr = rresult;
    } else {				/* arithmetic assign (+=, etc) */
	lresult = *lhsexprptr;
	evalexpr(&lresult);
	if (lresult.typ == NORETVALEXPR)
	    fatalerrlin("function returns no value (one is required)");
	expr.typ = BINARYOPEXPR;
	switch (optyp) {
	    case ASSIGNADD:	 expr.arg.optyp = ADD; break;
	    case ASSIGNSUBTRACT: expr.arg.optyp = SUBTRACT; break;
	    case ASSIGNMULTIPLY: expr.arg.optyp = MULTIPLY; break;
	    case ASSIGNDIVIDE:	 expr.arg.optyp = DIVIDE; break;
	    case ASSIGNMOD:	 expr.arg.optyp = MOD; break;
	    case ASSIGNPOWER:	 expr.arg.optyp = POWER; break;
	    default: fatalerrlin("assignop: hit default"); break;
	}
	expr.leftptr = &lresult;
	expr.rightptr = &rresult;
	evalexpr(&expr);
	*lhsexprptr = expr;
    }
}


/*******************************************************************************
Evaluate unary operations on one integer argument.
*******************************************************************************/
int
iunaryop(int operator, int i)
{
    int result = 0;

    switch (operator) {
	case ADD:	result =  i; break;
	case SUBTRACT:	result = -i; break;
	case NOT:	result = !i; break;
	default:
	    fatalerrlin("iunaryop: illegal integer unary operation"); break;
    }
    return result;
}


/*******************************************************************************
Evaluate unary operations on one float argument.
*******************************************************************************/
float
funaryop(int operator, float f)
{
    float result = 0.0;

    switch (operator) {
	case ADD:	result =  f; break;
	case SUBTRACT:	result = -f; break;
	default:
	    fatalerrlin("funaryop: fatal floating point unary operation");break;
    }
    return result;
}


/*******************************************************************************
Evaluate unary operations on one float argument.
*******************************************************************************/
float
mathfunc(int operator, double d)
{
    double result = 0.0;

    switch (operator) {
	case SIN:	result = sin(d); break;
	case COS:	result = cos(d); break;
	case TAN:	result = tan(d); break;
	case ASIN:	result = asin(d); break;
	case ACOS:	result = acos(d); break;
	case ATAN:	result = atan(d); break;
	case LOG:	result = log(d); break;
	case LOG10:	result = log10(d); break;
	case EXP:	result = exp(d); break;
	default:
	    fatalerrlin("mathfunc: ilegal operation"); break;
    }
    return (float)result;
}


#define SECSPERHR 3600
#define SECSPERMIN  60
/*******************************************************************************
Get the report date or time, REPORTDT (DAT or TIM), from the system.
*******************************************************************************/
int
getreportdt(int fldidx)
{
    struct tm *ts, *localtime();
    time_t ti;
    int dt = 0;

    ti = time((time_t*)0);
    ts = localtime(&ti);
    switch (fldidx) {
	case DAT:
	    dt = (ts->tm_year+MKTIMEYEAROFFSET)*10000 + (ts->tm_mon+1)*100 +
								    ts->tm_mday;
	    break;
	case TIM:
	    dt = ts->tm_hour*SECSPERHR + ts->tm_min*SECSPERMIN + ts->tm_sec;
	    break;
	default:
	    fatalerrlin("getreportdt: hit default");
	    break;
    }
    return dt;
}


#define MAXDTSTRLEN 128
#define MONTHOFFSET   1	/* because Jan=0, Dec=11 */
/*******************************************************************************
*******************************************************************************/
char *
formatdt(int fldidx, Exprnode *fmtexprptr, Exprnode *dtexprptr)
{
    struct tm *timstructptr, *localtime();
    time_t loctim;
    char *resultptr, resultstr[MAXDTSTRLEN];
    size_t len;
    time_t dt;

    if (fmtexprptr->typ != STRING)
	fatalerrlin("formatdt: first argument MUST be a string");
    if (dtexprptr->typ != INTEGER)
	fatalerrlin("formatdt: second argument MUST be an integer");

    loctim = time((time_t*)NULL);
    timstructptr = localtime(&loctim);
    dt = dtexprptr->arg.ival;
    switch (fldidx) {
	case DAT:
	    timstructptr->tm_mday  = dt%100;
	    timstructptr->tm_mon   = dt/100%100 - MONTHOFFSET;
	    timstructptr->tm_year  = dt/10000 - MKTIMEYEAROFFSET;
	    timstructptr->tm_sec = timstructptr->tm_min =
			    timstructptr->tm_hour = timstructptr->tm_isdst =  0;
	    break;
	case TIM:
	    timstructptr->tm_sec   = dt%3600%60;
	    timstructptr->tm_min   = dt/60%60;
	    timstructptr->tm_hour  = dt/3600;
	    break;
	default:
	    fatalerrlin("formatdt: hit default");
	    break;
    }
    (void) mktime(timstructptr);	/* sets some values in the tm struct */
    timstructptr->tm_isdst = 0;		/* set no daylight savings */
    len = strftime(resultstr, MAXDTSTRLEN, fmtexprptr->arg.sval, timstructptr);
    if (len > 0) {
	resultptr = (char*)malloc(len+1);
	strcpy(resultptr, resultstr);
    } else {
	resultptr = NULL;
    }
    return resultptr;
}


/*******************************************************************************
This function takes two arguments: a "string expression pointer" and a "pattern
expression pointer" and does a pattern match (PATMATCH).  It returns 1 if there
is a match, and 0 if there is no match.
*******************************************************************************/
int
patmatch(Exprnode *patexprptr, Exprnode *strexprptr)
{
    regex_t extregexp;
    int retval = 0;

    if (regcomp(&extregexp, patexprptr->arg.sval, REG_EXTENDED) == 0) {
						    /* compiled OK */
	retval = !regexec(&extregexp, strexprptr->arg.sval, (size_t)0, NULL, 0);
    } else {					/* compile failed */
	perror("patmatch: regexec error");
	fatalerrlin("");
    }
    return retval;
}



/*******************************************************************************
Handle all the built-in string functions (PATMATCH INDEXSTR STRLEN SUBSTR
LOOKUP)
*******************************************************************************/
int
strfunc(Exprnode *exprptr)
{
    Exprnode result1, result2, result3;
    char *chptr;
    int len, newlen;
				    /* all functions have a first argument */
    result1 = *(exprptr->leftptr);  evalexpr(&result1);
    switch (exprptr->arg.strfnc.functyp) {
        case PATMATCH:
            result2 = *(exprptr->rightptr); evalexpr(&result2);
	    if ((result1.typ != STRING) || (result2.typ != STRING))
		fatalerrlin("patmatch: both arguments must be strings");
            exprptr->arg.ival = patmatch(&result1, &result2);
            exprptr->typ = INTEGER;
	    break;
	case STRLEN:
	    if (result1.typ != STRING)
		fatalerrlin("strlen: argument must be a string");
	    exprptr->arg.ival = strlen(result1.arg.sval);
	    exprptr->typ = INTEGER;
	    break;
	case INDEXSTR:
            result2 = *(exprptr->rightptr); evalexpr(&result2);
	    if ((result1.typ != STRING) || (result2.typ != STRING))
		fatalerrlin("indexstr: both arguments must be strings");
	    if ((chptr=strstr(result2.arg.sval, result1.arg.sval)) == NULL) {
		exprptr->arg.ival = 0;
	    } else {
		exprptr->arg.ival = chptr - result2.arg.sval + 1;
	    }
            exprptr->typ = INTEGER;
	    break;
	case SUBSTR:
	    if (result1.typ != STRING)
		fatalerrlin("substr: first argument must be a string");
            result2 = *(exprptr->rightptr); evalexpr(&result2);
	    result3 = *(exprptr->arg.strfnc.expr3ptr); evalexpr(&result3);
	    if (result2.typ != INTEGER || result3.typ != INTEGER)
		fatalerrlin("substr: 2nd and 3rd arguments must be integers");
	    len = strlen(result1.arg.sval);
	    if (result2.arg.ival < 1 || result2.arg.ival > len) {
		fprintf(stderr, "substr: bad index %d", result2.arg.ival);
		fatalerrlin("");
	    }
	    if (result3.arg.ival < 1) {
		fprintf(stderr, "substr: bad length %d", result3.arg.ival);
		fatalerrlin("");
	    }
	    newlen = MIN(len-result2.arg.ival+1, result3.arg.ival);
	    chptr = (char*)malloc((size_t)(newlen+1));
	    strncpy(chptr, result1.arg.sval+result2.arg.ival-1, newlen);
	    *(chptr+newlen) = '\0';
	    exprptr->arg.sval = chptr;
            exprptr->typ = STRING;
	    break;
	case LOOKUP:
	    if (result1.typ != STRING)
		fatalerrlin("lookup: first argument must be a string");
						/* second argument */
            result2 = *(exprptr->rightptr);  evalexpr(&result2);
	    if (result2.typ != STRING)
		fatalerrlin("lookup: second argument must be a string");
						/* optional 3rd arg */
	    if (exprptr->arg.strfnc.expr3ptr == NULL) {
		result3.arg.ival = 2;
	    } else {
		result3 = *(exprptr->arg.strfnc.expr3ptr);
		evalexpr(&result3);
		if (result3.typ != INTEGER)
		    fatalerrlin("lookup: third argument must be an integer");
	    }
	    exprptr->arg.sval = dolookup(result1.arg.sval, result2.arg.sval,
							    result3.arg.ival);
	    exprptr->typ = STRING;
	    break;
	default: fatalerror("strfunc: hit default");
	    break;
    }
}
