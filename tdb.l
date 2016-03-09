%{
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
	tdb.l - TDB
*******************************************************************************/

#include <stdlib.h>
#include <string.h>

#define COMMENTEND 9999
#define CPPQUOTE '"'
#define MAX_INCLUDE_DEPTH 10

/*****************************************************************************
This struct stores the data necessary to handle #include files.  The index,
srcincludelev, is used to indicate the current level of #include source
file(s).  0 means no include files - that is - the primary source file.
Hence, when operating at a level of N, srcfiletbl[0...N-1] store the
input info for all the lower level input source file(s).
*****************************************************************************/
typedef struct {
#ifdef _FLEX
    YY_BUFFER_STATE yybuffer;
#else
    FILE *yyinfptr;
#endif
    char *srcfile;
    int  linenum;
} Srcfile;

char *srcfile = "the standard input";	/* default srcfile only */
int   linenum = 1;

static Srcfile srcfiletbl[MAX_INCLUDE_DEPTH];

static int     srcincludelev = 0;
static int     commentctr = 0;


/*****************************************************************************
*****************************************************************************/
int
cppinclude(UCHAR *str)
{
    char *filename, *endptr;

    if (srcincludelev >= MAX_INCLUDE_DEPTH) {
	fprintf(stderr, "#include source files nested more than %d deep",
							    MAX_INCLUDE_DEPTH);
	fatalerrlin("");
    }
    if ((filename=strchr(str, CPPQUOTE)) != NULL &&
			(endptr=strchr(++filename, CPPQUOTE)) != NULL) {
	*endptr = '\0';
#ifdef _FLEX
	srcfiletbl[srcincludelev].yybuffer = YY_CURRENT_BUFFER;
#else
	srcfiletbl[srcincludelev].yyinfptr = yyin;
#endif
	srcfiletbl[srcincludelev].srcfile = srcfile;
	srcfiletbl[srcincludelev].linenum = linenum;
	srcincludelev++;

	if ((yyin=fopen(filename, "r")) == NULL) {
	    fprintf(stderr, "Could not open the #include file: `%s'", filename);
	    fatalerrlin("");
	}

	srcfile = malloc(strlen(filename)+1);
	strcpy(srcfile, filename);
	linenum = 1;

#ifdef _FLEX
	yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
#endif
    } else {
	fatalerrlin("include format: #include \"filename\"");
    }
}

/*****************************************************************************
This function copies the string 'from' to the string 'to' but also replaces
"escape" sequences (backslash followed by another character) with the
appropriate character (eg, "\n" is mapped to the newline character).
*****************************************************************************/
static void
do_escapes(char *to, UCHAR *from)
{
    while (*from) {
	if ((*from == '\\') && *(from+1)) {
	    from++;
	    switch (*from) {
		case 'n': *to = '\n'; break;
		case 't': *to = '\t'; break;
		case 'r': *to = '\r'; break;
		case 'f': *to = '\f'; break;
		case 'b': *to = '\b'; break;
		default: *to = *from; break;
	    }
	} else {
	    *to = *from;
	}
	to++;
	from++;
    }
    *to = '\0';
}


/*****************************************************************************
*****************************************************************************/
static int
commentbegin()
{
    int v;

    commentctr++;
    while ((v=yylex()) && (v != COMMENTEND))
	;			/* do nothing */
    if (v != 0)
	return yylex();
    else
	return 0;
}


/*****************************************************************************
*****************************************************************************/
static int
commentend()
{
    commentctr--;
    if (commentctr < 0) {
	fatalerrlin("unmatched comment close (*/)");
	commentctr = 0;
    }
    return COMMENTEND;
}


extern double atof();
static int fieldidx;
/*******************************************************************************
*******************************************************************************/
%}

L	[a-zA-Z_]
D	[0-9]
E	[Ee][-+]?{D}+

%%

\/\*			{ return commentbegin(); }
\*\/			{ return commentend(); }

^#include[ \t]+\"[^"]*\"[ \t]*\n {	/* embedded `\"' are forbidden */
			  if (commentctr == 0)
			      cppinclude(yytext);
			}

[ \t]			;
\n			{ linenum++; }

{D}+			{ yylval.ival = atoi(yytext); return INTEGER; }
{D}*\.{D}+({E})?	|
{D}+\.{D}*({E})?	|
{D}+{E}			{ yylval.fval = (float)atof(yytext); return FLOAT; }

select			{ yylval.ival = SELCT; return SELREJ; }
reject			{ yylval.ival = REJCT; return SELREJ; }

aggregate		{ return AGGREGATE; }
join			{ return JOIN; }
printf			{ return PRINTF; }
sprintf			{ return SPRINTF; }
foreach			{ return FOREACH; }
while			{ return WHILE; }
do			{ return DO; }
if			{ return IF; }
else			{ return ELSE; }
count			{ yylval.ival = COUNT; return COUNT; }
sum			{ yylval.ival = SUM;   return SUMFUNC; }
sumsquared		{ yylval.ival = SUMSQRD;   return SUMFUNC; }
number			{ yylval.ival = NUMBER; return NUMBER; }
sort			{ yylval.ival = SORT;  return SORT; }
reverse			{ yylval.ival = REVERSE;  return REVERSE; }
first			{ yylval.ival = FIRST; return FSTLST; }
last			{ yylval.ival = LAST;  return FSTLST; }
reportdate		{ yylval.ival = DAT; return REPORTDT; }
reporttime		{ yylval.ival = TIM; return REPORTDT; }
formatdate		{ yylval.ival = DAT; return FORMATDT; }
formattime		{ yylval.ival = TIM; return FORMATDT; }
atoi			{ yylval.ival = ATOI; return ATOIF; }
atof			{ yylval.ival = ATOF; return ATOIF; }
patmatch		{ return PATMATCH; }
strlen			{ return STRLEN; }
indexstr		{ return INDEXSTR; }
substr			{ return SUBSTR; }
lookup			{ return LOOKUP; }
sin			{ yylval.ival = SIN;   return MATHFUNC; }
cos			{ yylval.ival = COS;   return MATHFUNC; }
tan			{ yylval.ival = TAN;   return MATHFUNC; }
asin			{ yylval.ival = ASIN;  return MATHFUNC; }
acos			{ yylval.ival = ACOS;  return MATHFUNC; }
atan			{ yylval.ival = ATAN;  return MATHFUNC; }
log			{ yylval.ival = LOG;   return MATHFUNC; }
log10			{ yylval.ival = LOG10; return MATHFUNC; }
exp			{ yylval.ival = EXP;   return MATHFUNC; }
system			{ return SYSTEM; }
return			{ return RETURN; }
trap			{ return TRAP; }
need			{ return NEED; }

function		{ return FUNCTION; }

int			{ yylval.ival = INTEGER; return TYPE; }
float			{ yylval.ival = FLOAT;   return TYPE; }
string			{ yylval.ival = STRING;  return TYPE; }

"+"			{ yylval.ival = ADD;		return ADDOP; }
"-"			{ yylval.ival = SUBTRACT;	return ADDOP; }
"*"			{ yylval.ival = MULTIPLY;	return MULTOP; }
"/"			{ yylval.ival = DIVIDE;		return MULTOP; }
"%"			{ yylval.ival = MOD;		return MULTOP; }
"^"			{ yylval.ival = POWER;		return MULTOP; }

"="			{ yylval.ival = ASSIGN;		return ASSIGNOP; }
"+="			{ yylval.ival = ASSIGNADD;	return ASSIGNOP; }
"-="			{ yylval.ival = ASSIGNSUBTRACT;	return ASSIGNOP; }
"*="			{ yylval.ival = ASSIGNMULTIPLY;	return ASSIGNOP; }
"/="			{ yylval.ival = ASSIGNDIVIDE;	return ASSIGNOP; }
"%="			{ yylval.ival = ASSIGNMOD;	return ASSIGNOP; }
"^="			{ yylval.ival = ASSIGNPOWER;	return ASSIGNOP; }

"<"			{ yylval.ival = LESTHAN;	return RELATOP; }
">"			{ yylval.ival = GRTTHAN;	return RELATOP; }
"<="			{ yylval.ival = LESTHANEQ;	return RELATOP; }
">="			{ yylval.ival = GRTTHANEQ;	return RELATOP; }
"=="			{ yylval.ival = EQUAL;		return EQUALOP; }
"!="			{ yylval.ival = NOTEQUAL;	return EQUALOP; }
"&&"			{ yylval.ival = AND;		return AND; }
"||"			{ yylval.ival = OR;		return OR; }
"!"			{ yylval.ival = NOT;		return NOT; }

{L}({L}|{D})*		{
			  fieldidx = lookupfieldname(yytext);
			  if (fieldidx != INVALIDFLDIDX) {
			      yylval.ival = fieldidx;
			      return FIELD;
			  } else {
			      yylval.sval = malloc((size_t)(strlen(yytext)+1));
			      strcpy(yylval.sval, yytext); return IDENTIFIER;
			  }
			}

\"[^"]*			{ /* double quoted strings (may have embedded `\"') */
			  if (yytext[yyleng-1] == '\\')
			      yymore();		/* handle `\"' */
		  	  else {
			      input();		/* dispose of the final '"' */
			      yylval.sval =malloc((size_t)(strlen(yytext+1)+1));
			      do_escapes(yylval.sval, yytext+1);
			      return STRING;
		  	  }
			}

[^ \t\n]		{ return *yytext; }

%%

/*******************************************************************************
*******************************************************************************/
extern int
yywrap()
{
    if (commentctr > 0)
	fatalerrlin("unmatched comment open (/*)");

    if (--srcincludelev >= 0) {		/* this #include file is done */
#ifdef _FLEX
	yy_delete_buffer(YY_CURRENT_BUFFER);
	yy_switch_to_buffer(srcfiletbl[srcincludelev].yybuffer);
#else
	fclose(yyin);
	yyin    = srcfiletbl[srcincludelev].yyinfptr;
#endif
	srcfile = srcfiletbl[srcincludelev].srcfile;
	linenum = srcfiletbl[srcincludelev].linenum + 1;
	return 0;
    } else {
	return 1;			/* lexical analysis is done */
    }
}
