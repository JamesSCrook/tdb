%{
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
	tdb.y - TDB
*******************************************************************************/
#include <stdio.h>
#include "tdb.h"

#define NOTUSED 0

static void pushstmtno();
static int  popstmtno();

extern Exprnode *funcexprptr;

extern Stmt *stmts;
extern int nstmts;

extern void tdbstmtaddfld(int);
extern void addjoinfldarg(int, int, char *);
extern void addjoinfldstmt();
extern void initjoinfldstmt(int, char *);
extern void includejoinfields();
extern int yyerror(char *);
extern int yylex();

%}

%union {
    int      ival;
    float    fval;
    char     *sval;
    Exprnode *exprptr;
}

%type <exprptr> expr and_expr equal_expr relat_expr add_expr mult_expr
%type <exprptr> cast_expr unary_expr primary_expr constant variable
%type <exprptr> ufnc_call_expr datafnc_expr strfnc_expr miscfnc_expr
%type <ival> agg_stmt

%token <sval> IDENTIFIER
%token <ival> TYPE	INTEGER FLOAT STRING

%token <ival> SELREJ	SELCT REJCT

%token <ival> AGGREGATE
%token <ival> JOIN
%token <ival> PRINTF
%token <ival> SPRINTF
%token <ival> FOREACH
%token <ival> WHILE	DO
%token <ival> IF ELSE
%token <ival> COUNT
%token <ival> NUMBER
%token <ival> SUMFUNC   SUM SUMSQRD
%token <ival> SORT	REVERSE
%token <ival> FSTLST	FIRST LAST
%token <ival> REPORTDT	FORMATDT DAT TIM
%token <ival> ATOIF	ATOI ATOF
%token <ival> PATMATCH
%token <ival> INDEXSTR
%token <ival> STRLEN
%token <ival> SUBSTR
%token <ival> LOOKUP
%token <ival> MATHFUNC	SIN COS TAN ASIN ACOS ATAN LOG LOG10 EXP
%token <ival> SYSTEM
%token <ival> RETURN
%token <ival> TRAP
%token <ival> NEED

%token <ival> FUNCTION

%token <ival> FIELD

%token <ival> OR
%token <ival> AND
%token <ival> EQUALOP	EQUAL NOTEQUAL
%token <ival> ADDOP	ADD SUBTRACT
%token <ival> MULTOP	MULTIPLY DIVIDE MOD POWER
%token <ival> ASSIGNOP	ASSIGN ASSIGNADD ASSIGNSUBTRACT ASSIGNMULTIPLY
			ASSIGNDIVIDE ASSIGNMOD ASSIGNPOWER
%token <ival> RELATOP	LESTHAN GRTTHAN LESTHANEQ GRTTHANEQ
%token <ival> NOT

%start	prog
%%
prog		: selrej_stmt_list			/* select program */
		| join_fld_stmt_list			/* report program */
			{
			    includejoinfields();
			    initaxistbl();
			    init_b4_aggr();
			    inittraptbl();
			}
		  agg_stmt ufnc_def_list
			{ resolvefrwdufncs(); savestartstmt(nstmts); }
		  stmt_list
		;

selrej_stmt_list:			/* empty, select/reject statment list */
		| selrej_stmt_list selrej_stmt
		| selrej_stmt_list error
		;

selrej_stmt	: SELREJ '(' expr ')' ';'	/* select/reject statment */
			{ addselrejstmt($1, $3); }
		;

join_fld_stmt_list : /* empty	joinfield statment list */
		| join_fld_stmt_list join_fld_stmt
		;

join_fld_stmt	: JOIN '(' TYPE IDENTIFIER ':'
			{ initjoinfldstmt($3, $<sval>4); }
		  join_fld_arg_list ')' ';'
			{ addjoinfldstmt(); }
		;

join_fld_arg_list : join_fld_arg
		| join_fld_arg_list ',' join_fld_arg
		;

join_fld_arg	: FIELD
			{ addjoinfldarg(FIELD, $1, NULL); } /* NULL not used */
		| STRING
			{ addjoinfldarg(STRING, 0, $<sval>1); } /* 0 not used */
		;

agg_stmt	: AGGREGATE '(' aggfldlst ')' ';'  /* MUST have >= 1 field(s) */
		;

aggfldlst	: FIELD
			{ aggelement($1); }
		| aggfldlst ',' FIELD
			{ aggelement($3); }
		;

ufnc_def_list	: /* empty	usr function DEFINITION statment list */
		| ufnc_def_list ufnc_def
		;

ufnc_def	: FUNCTION IDENTIFIER '('
			{ addufncdefstmt($2, nstmts); /* nstmts incremented */ }
		  ufnc_param_list ')' '{'
			{ $<ival>$ = nstmts-1; } 	/* hence, -1 */
		  stmt_list '}'
			{ stmts[$<ival>8].s.ufncdef.endstmtidx = nstmts-1; }
		;

ufnc_param_list	: /* empty */
		| ufnc_param_list2
		;

ufnc_param_list2: IDENTIFIER
			{ addufncparam($<sval>1); }
		| ufnc_param_list2 ',' IDENTIFIER
			{ addufncparam($<sval>3); }
		;

stmt_list	: /* empty */
		| stmt_list stmt
		| stmt_list error
		;

compound_stmt	: '{' stmt_list '}'
		;

stmt		: foreach_stmt
		| sort_stmt		/* sort statment */
		| fstlst_stmt		/* first or last statment */
		| while_stmt
		| do_while_stmt
		| if_stmt
		| printf_stmt
		| sprintf_stmt
		| assign_stmt
		| compound_stmt
		| system_stmt
		| return_stmt
		| expr_stmt
		| trap_stmt
		;

foreach_stmt	: FOREACH '(' FIELD ':'
			{
			    $<ival>$=nstmts;	/* this is $5 below */
			    chkfld($3); addforeachstmt($3);
			}
		  stmt_fld_list ')' stmt
			{ stmts[$<ival>5].s.foreach.next = nstmts; }
		;

stmt_fld_list	: /* empty */
		| stmt_fld_list2
		;

stmt_fld_list2	: FIELD
			{ tdbstmtaddfld($1); }
		| stmt_fld_list2 ',' FIELD
			{ tdbstmtaddfld($3); }
		;

sort_stmt	: SORT '(' FIELD ':' expr ')' ';'
			{ chkfld($3); addsortstmt($3, $5, FALSE); }
		| SORT '(' FIELD ':' expr ',' REVERSE ')' ';'
			{ chkfld($3); addsortstmt($3, $5, TRUE); }
		;

fstlst_stmt	: FSTLST '(' FIELD ':' expr ')' ';'
			{ chkfld($3); addfstlststmt($1, $3, $5); }
		;

while_stmt	: WHILE '(' expr ')'
			{ $<ival>$ = nstmts; addwhilestmt($3); }
		  stmt
			{ stmts[$<ival>5].s.while_.next = nstmts; }
		;

do_while_stmt	: DO
			{ $<ival>$ = nstmts; /* this is $2 below */ }
		  stmt WHILE '(' expr ')' ';'
			{
			  adddowhilestmt($6);	/* $6 is expr */
			  stmts[nstmts-1].s.dowhile.next = $<ival>2;
			}
		;

if_stmt		: IF '(' expr ')'
			{ pushstmtno(nstmts); addifstmt($3); }
		  if_tail
		;

if_tail		: stmt
			{ stmts[popstmtno()].s.if_.next = nstmts; }
		| stmt ELSE
			{  /* nstmts+1 to bump it 1 past the ELSE stmt */
			  stmts[popstmtno()].s.if_.next = nstmts+1;
			  pushstmtno(nstmts);
			  addelsestmt();
			}
		  stmt
			{ stmts[popstmtno()].s.else_.next = nstmts; }
		;

printf_stmt	: PRINTF '('
			{ setupexprptrlist(); }
		  arg_expr_list ')' ';'
			{ addprintfstmt(PRINTF, NULL); }
		;

sprintf_stmt	: SPRINTF '(' variable ','
			{ setupexprptrlist(); }
		  arg_expr_list ')' ';'
			{ addprintfstmt(SPRINTF, $3); }
		;

assign_stmt	: variable ASSIGNOP expr ';'
			{ addassignstmt($1, $2, $3); }
		;

system_stmt	: SYSTEM '(' expr ')' ';'
			{ addsystemstmt($<exprptr>3); }
		;

return_stmt	: RETURN ';'
			{ addreturnstmt(NULL); }
		| RETURN expr ';'
			{ addreturnstmt($<exprptr>2); }
		;

expr_stmt	: expr ';'
			{ addexprstmt($<exprptr>1); }
		;

trap_stmt	: TRAP '(' expr ',' ufnc_call_expr ')' ';'
			{ addtrapstmt($3, $5); }
		| TRAP '(' expr ')' ';'
			{ addtrapstmt($3, NULL); }
		;

expr		: and_expr
		| expr OR and_expr
			{ $<exprptr>$ = addbinop($1, $2, $3); }
		;

and_expr	: equal_expr
		| and_expr AND equal_expr
			{ $<exprptr>$ = addbinop($1, $2, $3); }
		;

equal_expr	: relat_expr
		| equal_expr EQUALOP relat_expr
			{ $<exprptr>$ = addbinop($1, $2, $3); }
		;

relat_expr	: add_expr
		| relat_expr RELATOP add_expr
			{ $<exprptr>$ = addbinop($1, $2, $3); }
		;

add_expr	: mult_expr
		| add_expr ADDOP mult_expr
			{ $<exprptr>$ = addbinop($1, $2, $3); }
		;

mult_expr	: cast_expr
		| mult_expr MULTOP cast_expr
			{ $<exprptr>$ = addbinop($1, $2, $3); }
		;

cast_expr	: unary_expr
		| '(' TYPE ')' unary_expr
			{ $<exprptr>$ = addcast($2, $4); }
		;

unary_expr	: primary_expr
		| ADDOP primary_expr
			{ $<exprptr>$ = addunaryop($1, $2); }
		| NOT primary_expr
			{ $<exprptr>$ = addunaryop($1, $2); }
		;

primary_expr	: constant
		| variable
		| FIELD
			{ $<exprptr>$ = addfld($<ival>1); chkfld($1); }
		| '(' expr ')'
			{ $<exprptr>$ = $<exprptr>2; }
		| datafnc_expr
                | strfnc_expr
                | miscfnc_expr
		| ufnc_call_expr
		;

datafnc_expr	: COUNT '('
			{ tdbfncinit($1, NOTUSED); }
		  fnc_fld_list ')'
			{ $<exprptr>$ = funcexprptr; }
		| NUMBER '(' FIELD ':'
			{ chkfld($3); tdbfncinit($1, $3); }
		  fnc_fld_list ')'
			{ $<exprptr>$ = funcexprptr; }
		| SUMFUNC '(' FIELD ':'
			{ tdbfncinit($1, $3); }
		  fnc_fld_list ')'
			{ $<exprptr>$ = funcexprptr; }
		;

strfnc_expr	: PATMATCH '(' expr ',' expr ')'
			{ $<exprptr>$ = addstrfunc(PATMATCH, $3, $5, NULL); }
		| STRLEN '(' expr ')'
			{ $<exprptr>$ = addstrfunc(STRLEN, $3, NULL, NULL); }
		| INDEXSTR '(' expr ',' expr ')'
			{ $<exprptr>$ = addstrfunc(INDEXSTR, $3, $5, NULL); }
		| SUBSTR '(' expr ',' expr ',' expr ')'
			{ $<exprptr>$ = addstrfunc(SUBSTR, $3, $5, $7); }
		| LOOKUP '(' expr ',' expr ')'
			{ $<exprptr>$ = addstrfunc(LOOKUP, $3, $5, NULL); }
		| LOOKUP '(' expr ',' expr ',' expr ')'
			{ $<exprptr>$ = addstrfunc(LOOKUP, $3, $5, $7); }
		;

miscfnc_expr	: REPORTDT '(' ')'
			{ $<exprptr>$ = addreportdt($1); }
		| FORMATDT '(' expr ',' expr ')'
			{ $<exprptr>$ =addformatdt($1,$<exprptr>3,$<exprptr>5);}
		| ATOIF '(' expr ')'
			{ $<exprptr>$ = addatoif($1, $<exprptr>3);}
		| MATHFUNC '(' expr ')'
			{ $<exprptr>$ = addmathfunc($1, $<exprptr>3);}
		| NEED '(' expr ')'
			{ $<exprptr>$ = addneedfunc($3); }
		;

ufnc_call_expr	: IDENTIFIER '('
			{ setupexprptrlist(); }
		  arg_expr_list ')'
			{ $<exprptr>$ = updateufnccall($1); }
		;

arg_expr_list	: /* empty */
		| arg_expr_list2
		;

arg_expr_list2	: expr
			{ addexprptrlistarg($1); }
		| arg_expr_list2 ',' expr
			{ addexprptrlistarg($3); }
		;

fnc_fld_list	: /* empty */
		| fnc_fld_list2
		;

fnc_fld_list2	: FIELD
			{ chkfld($1); tdbfncaddfld($1); }
		| fnc_fld_list2 ',' FIELD
			{ chkfld($3); tdbfncaddfld($3); }
		;

variable	: IDENTIFIER
			{ $<exprptr>$ = getvaraddr($<sval>1); }
		| IDENTIFIER '['
			{ setupexprptrlist(); }
		  arg_expr_list ']'
			{ $<exprptr>$ = updatearrvar($<sval>1); }
		;

constant	: INTEGER
			{ $<exprptr>$ = addinteger($<ival>1); }
		| FLOAT
			{ $<exprptr>$ = addfloat($<fval>1); }
		| STRING
			{ $<exprptr>$ = addstring($<sval>1); }
		;
%%


#include "lex.yy.c"

static Flag parseerrorflag = FALSE;

/*******************************************************************************
*******************************************************************************/
int
yyerror(char *msg)
{
    extern char *srcfile;
    extern int linenum;

    fprintf(stderr, "yyerror: %s at/near line %d of source file `%s'\n",
							msg, linenum, srcfile);
    (void) fflush(stderr);
    parseerrorflag = TRUE;
}

/*******************************************************************************
*******************************************************************************/
void
checkparsing()
{
    if (nstmts < 1)
	fatalerror("program file contains no executable statments");
    if (parseerrorflag)
	fatalerror("aborting after parsing input program");
}


/*******************************************************************************
*******************************************************************************/
#define INITSTACKSIZE  32
#define STACKMULTFACTOR 2

static short *stmtnostack = NULL;
static short  stmtnoidx = 0;
static short  maxstmtnoidx = 0;
/*******************************************************************************
*******************************************************************************/
static void
pushstmtno(int stmtno)
{
    if (stmtnoidx >= maxstmtnoidx) {
	if (stmtnostack == NULL) {
	    maxstmtnoidx = INITSTACKSIZE;
	    stmtnostack = (short*)malloc((size_t)maxstmtnoidx*sizeof(short));
	} else {
	    maxstmtnoidx *= STACKMULTFACTOR;
	    stmtnostack = (short*)realloc((void*)stmtnostack,
					(size_t) maxstmtnoidx*sizeof(short));
	}
    }
    stmtnostack[stmtnoidx++] = stmtno;
}


/*******************************************************************************
*******************************************************************************/
static int
popstmtno()
{
    if (stmtnoidx <= 0)
	fatalerror("popstmtno: popped into negative stack space");

    return (int)stmtnostack[--stmtnoidx];
}
