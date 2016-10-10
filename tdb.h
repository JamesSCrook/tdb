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
	tdb.h - TDB
*******************************************************************************/

#define MAXLINELEN 2048

#define COMCHAR      '#'
#define CONTLINECHAR '\\'
#define QUOTECHAR    '"'
#define ESCAPECHAR   '\\'

#if !defined(TRUE)
#define		TRUE  1
#endif /* TRUE */
#if !defined(FALSE)
#define		FALSE 0
#endif /* FALSE */

#if !defined(GOOD)
#define		GOOD   0
#endif /* GOOD */
#if !defined(ERROR)
#define		ERROR -1
#endif /* ERROR */

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ABS(x)   ((x) > 0   ? (x) :-(x))

#define WHITESPACE(c)   ((((c)==' ')||((c)=='\t')||((c)=='\n')) ? TRUE : FALSE)

typedef int Flag;

#define MINSTATSINFOTBLSIZE     1	/* must be >= 1 */

#define MAXARRAYDIMS   32	/* max number of array dimensions */

#define INVALIDFLDIDX  -1		/* all must be < 0 */

#define UNARYOPEXPR   -10	/* expression types */
#define BINARYOPEXPR  -11
#define UFNCCALLEXPR  -12
#define UFNCPARAMEXPR -13
#define ARRAYVAREXPR  -14
#define UNDEFVAREXPR  -15
#define NORETVALEXPR  -16
#define STRFUNCEXPR   -17

#define ASSIGNSTMT    -20	/* statment types */
#define UFNCDEFSTMT   -21
#define EXPRSTMT      -22

#define VARIABLE      -30	/* "variable" types */
#define UFNCNAMVAR    -31
#define ARRAYVAR      -32

#define STDINCODE  "-"
#define ASSIGNCHAR '='

#define MAXAVLDEPTH 22	/* 22 yeilds a minimum of 2^20 or 1,048,576 nodes */

typedef enum { ERRORMODE, SELREJMODE, REPORTMODE } Opmode;

typedef enum { REJECTED, SELECTED } Selrej;

typedef union {
    int   ival;
    float fval;
    char  *sval;
} Intfltstr;

typedef struct axisnode {	/* axis (AVL) binary tree node */
    Intfltstr	     arg;
    Flag	     seenflag;
    Intfltstr        sortarg;
    struct axisnode *leftptr;	/* left and right pointers */
    struct axisnode *rightptr;
    int	             balfact;	/* balance factor (for AVL agorithm) */
} Axisnode;

typedef struct {	/* axis table element */
    int        number;	/* number of records (at this depth) */
    int        first;	/* first index */
    int        last;	/* last index */
    Axisnode  *rootptr;	/* root of the axis tree */
    Axisnode **array;	/* array (for arbitrary sorting) */
} Axistbl;

typedef struct {	/* The calculations performed for fields with stats */
    Intfltstr sum;	/* sum x */
    Intfltstr sumsqrd;	/* sum x^2 */
} Statscalcs;

typedef struct datanode {	/* data (AVL) binary tree node */
    int	   	     count;
    Intfltstr	    *argptr;
    Axisnode	    *axisptr;
    struct datanode *leftptr;	/* left and right pointers */
    struct datanode *orthogptr;	/* orthog for next "depth" level */
    struct datanode *rightptr;
    int		     balfact;	/* balance factor (for AVL agorithm) */
	/* statstbl MUST BE THE LAST ITEM in the STRUCTURE!!! */
    Statscalcs       statstbl[MINSTATSINFOTBLSIZE];
} Datanode;

typedef struct exprptrlistnode {
    struct exprnode        *exprptr;
    struct exprptrlistnode *next;
} Exprptrlistnode;

typedef struct {		/* stack of expression-pointer lists */
    Exprptrlistnode *exprptrlistptr;
    int exprctr;
} Exprptrliststk;

typedef struct exprnode {	/* expression binary tree node */
    int typ;
    union {
	int   ival;
	float fval;
	char  *sval;
	int   optyp;			/* operator type */
	int   casttyp;			/* for type casts */
	struct {			/* for the internal TDB functions */
	    long  fldbits;	/* field bit flags */
	    short depth;
	    short maxdepth;
	} tdbfnc;
	struct {		/* for array variables */
	    int functyp;
	    struct exprnode *expr3ptr;	/* for 3rd arg to str func(s) */
	} strfnc;
	struct {		/* for the user defined functions */
	    short  begstmtidx;	/* stmt # of function body */
	    short  argctr;	/* number of argument(s) */
	    Exprptrlistnode *exprptrlistptr;
	} ufnc;
	struct {		/* for array variables */
	    Exprptrlistnode *exprptrlistptr;
	} arrvar;
    } arg;
    struct varnode  *varptr;	/* used by variable & arrvar expressions */
    struct exprnode *leftptr;	/* left and right pointers */
    struct exprnode *rightptr;
} Exprnode;

typedef struct ufncparamnode {
    char                 *name;
    Exprnode		  expr;
    struct ufncparamnode *nextptr;
} Ufncparamnode;

typedef struct {
    int		typ;
    Intfltstr	idx;
} Arrvaridxnode;

typedef struct arrvarnode {
    Exprnode             expr;
    Arrvaridxnode	*idxtbl;
    struct arrvarnode   *leftptr;
    struct arrvarnode   *rightptr;
    int			 balfact;	/* balance factor (for AVL agorithm) */
} Arrvarnode;

	    /* Variables include both "ordinary" variables and usr functions */
typedef struct varnode {
    char      *name;
    int        typ;	/* UFNCDEF, VARIABLE, or ARRAYVAR */
    union {
	struct {			/* for UFNCDEFs */
	    int    begstmtidx;
	    int    paramctr;
	    Ufncparamnode *paramlistptr;
	} ufnc;
	Exprnode varexpr;		/* for VARIABLEs */
	struct {			/* for ARRAYVARs */
	    int exprctr;	/* number of 'args' between [ and ] */
	    Arrvarnode *rootptr;
	} arrvar;
    } arg;
    struct     varnode *leftptr;
    struct     varnode *rightptr;
} Varnode;

typedef struct {		/* statment */
    int       typ;
    char*     srcfile;		/* the name of the source file */
    int       linenum;		/* line number of the source file */
    union {
	struct {
	    int        endstmtidx;	/* stmt number of end of block */
	    int        defargctr;
	} ufncdef;
	struct {
	    int        optyp;		/* type of assign, eg +=, =, */
	    Exprnode  *lhsexprptr;	/* LHS expression pointer */
	    Exprnode  *rhsexprptr;	/* RHS expression pointer */
	} assign;
	struct {
	    long       fldbits;		/* field bit flags */
	    short      next;		/* next statment number */
	    short      fldidx;		/* field index */
	    short      maxdepth;	/* max depth */
	    Flag       restrictflag;	/* for "inner joins" */
	} foreach;
	struct {			/* Sort stmt */
	    short      fldidx;		/* field index */
	    Exprnode  *exprptr;		/* expression pointer */
	    Flag       reverseflag;	/* reverse sort flag */
	} sort;
	struct {			/* First or Last stmt */
	    short      fldidx;		/* field index */
	    Exprnode  *exprptr;		/* expression pointer */
	} fstlst;
	struct {
	    Exprptrlistnode *exprptrlistptr; /* expression list pointer */
	    short            nargs;	/* number of arguments */
	    Exprnode        *lhsexprptr;/* result expr ptr (sprintf only)*/
	} printf_;
	struct {
	    Exprnode  *exprptr;		/* expression pointer */
	    short      next;		/* next statment number */
	} while_;
	struct {
	    Exprnode  *exprptr;		/* expression pointer */
	    short      next;		/* next statment number */
	} dowhile;
	struct {
	    Exprnode  *exprptr;		/* expression pointer */
	    short      next;		/* next statment number */
	} if_;
	struct {
	    short      next;		/* next statment number */
	} else_;
	struct {
	    Exprnode  *exprptr;		/* expression pointer */
	} system_;
	struct {
	    Exprnode  *exprptr;		/* expression pointer */
	} return_;
	struct {
	    Exprnode  *exprptr;		/* expression pointer */
	} expr;
	struct {
	    Exprnode  *linnumexprptr;	/* trap line num expr pointer */
	    Exprnode  *ufncexprptr;	/* usr func expression pointer */
	} trap;
	struct {
	    Exprnode  *exprptr;		/* expression pointer */
	} selrej;
    } s;
} Stmt;

typedef struct {
    char *name;
    int   typ;
    int   idx;		/* see Field.idx note below */
    Flag  statsflag;
    int   statsidx;	/* only used in report mode by fldtbl (not aggrfldtbl)*/
} Field;

    /*****************************************************************
    Field.idx is used in report mode as follows:
	fldtbl.idx references the associated aggrfldtbl entry
	aggrfldtbl.idx references the associated fldtbl entry
    Field.idx is used in select mode as follows:
	fldtbl.idx references the associated fldtbl entry (ie, itself)
    *****************************************************************/

typedef struct {
    char *code;
    char *expand;
} Expandnode;

typedef struct {
    int fldidx;
    int typ;		/* INTEGER or FLOAT */
} Statsinfofld;

typedef struct joinfldargnode {
    int       fldargtyp;	/* FIELD or STRING */
    union {
	char *strptr;
	int   fldidx;
    } arg;
    struct joinfldargnode *next;
} Joinfldargnode;

typedef struct joinfldnode {
    char               *fldname;
    int                 fldtyp;	/* INTEGER, FLOAT or STRING*/
    int                 fldargctr;
    Joinfldargnode     *argllistptr;
    struct joinfldnode *next;
} Joinfldnode;


Arrvarnode* accessarrvar(Exprnode *exprptr, Flag addreqflag);
Exprnode doblock(int begstmtidx, int endstmtidx);
Exprnode* addatoif(int functyp, Exprnode *argexprptr);
Exprnode* addbinop(Exprnode *larg, int operator, Exprnode *rarg);
Exprnode* addcast(int casttyp, Exprnode *arg);
Exprnode* addfld(int fldidx);
Exprnode* addfloat(float floatvalue);
Exprnode* addformatdt(int fldidx, Exprnode *fmtexprptr, Exprnode *dtexprptr);
Exprnode* addinteger(int intvalue);
Exprnode* addmathfunc(int operatortyp, Exprnode *argexprptr);
Exprnode* addneedfunc(Exprnode *argexprptr);
Exprnode* addreportdt(int fldidx);
Exprnode* addstrfunc(int functyp, Exprnode *expr1ptr, Exprnode *expr2ptr, Exprnode *expr3ptr);
Exprnode* addstring(char *stringvalue);
Exprnode* addunaryop(int operator, Exprnode *argexprptr);
Exprnode* ctgetarrvaraddr(char *varnam, Exprptrlistnode *exprptrlistptr, int exprctr);
Exprnode* getufncparam(char *paramname);
Exprnode* getvaraddr(char *varname);
Exprnode* rtgetarrvaraddr(Exprnode *lhsexprptr);
Exprnode* updatearrvar(char *varnam);
Exprnode* updateufnccall(char *ufncname);
Selrej selrej();
Varnode* accessvar(char *varname, Flag addreqflag, Flag *addedresflagptr);
Varnode* ufncdefined(char *ufncname);
char* dolookup(char *codstr, char *filnamstr, int argnum);
char* fgetscont(char *buf, int maxlen, FILE *fp);
char* formatdt(int fldidx, Exprnode *fmtexprptr, Exprnode *dtexprptr);
float funaryop(int operator, float f);
float mathfunc(int operator, double d);
int arrvaridxcmp(Arrvaridxnode *arrvaridxtbl1, Arrvaridxnode *arrvaridxtbl2, int dimctr);
void evalselrejfld(Exprnode *exprptr);
int fbinop(float f1, int operator, float f2, float *floatptr, int *intptr);
int getreportdt(int fldidx);
int ibinop(int i1, int operator, int i2);
int iunaryop(int operator, int i);
int lookupfieldname(char *name);
int main(int argc, char *argv[]);
int mystrcmp(char *str1, char *str2);
int need(Exprnode *exprptr);
int parseline(char *p, char *chptrtbl[], int maxnargs);
int patmatch(Exprnode *patexprptr, Exprnode *strexprptr);
int strfunc(Exprnode *exprptr);
void addassignstmt(Exprnode *lhsexprptr, int optyp, Exprnode *rhsexprptr);
void addcmdlinevar(char *name_value, int exprtyp);
void adddatanode(Datanode **dataptrptr, Field *aggrfldptr, int depth);
void adddowhilestmt(Exprnode *exprptr);
void addelsestmt();
void addexprptrlistarg(Exprnode *argexprptr);
void addexprstmt(Exprnode *exprptr);
void addforeachstmt(int fldidx);
void addfrwdufnc(char *ufncname, Exprnode *exprptr);
void addfstlststmt(int stmttyp, int fldidx, Exprnode *exprptr);
void addifstmt(Exprnode *exprptr);
void addprintfstmt(int stmttyp, Exprnode *lhsexprptr);
void addreturnstmt(Exprnode *exprptr);
void addselrejstmt(int stmttyp, Exprnode *exprptr);
void addsortstmt(int fldidx, Exprnode *exprptr, Flag reverseflag);
void addsystemstmt(Exprnode *exprptr);
void addtrapstmt(Exprnode *linnumexprptr, Exprnode *ufncexprptr);
void addufncdefstmt(char *ufncname, int begstmtidx);
void addufncname(char *ufncname, int begstmtidx);
void addufncparam(char *paramname);
void addwhilestmt(Exprnode *exprptr);
void aggelement(int fldidx);
void assignop(Exprnode *lhsexprptr, int optyp, Exprnode *rhsexprptr);
void chkfld(int fldidx);
void chkoutputtraps();
void evalarrvar(Exprnode *exprptr);
void evalexpr(Exprnode *exprptr);
void fatalerrlin(char *msg);
void fatalerror(char *msg);
void tdbfncaddfld(int fldidx);
void tdbfncinit(int functyp, int fldidx);
void init_b4_aggr();
void initaxistbl();
void initreadfile();
void initselrej();
void inittraptbl();
void printctrstats();
void printendtraps();
void printfmt(int stmtidx);
void printftrap(char *rsltstr);
void printstart();
void readfieldfile(char *fieldfilename);
void readpathname(char *pathname, Flag verboseflag, Flag progisstdinflag);
void resolvefrwdufncs();
void savestartstmt(int stmtidx);
void sbinop(char *s1, char *s2, Exprnode *exprptr);
void setoutputvar(char *varname, int **paramptrptr, int assignval);
void setupexprptrlist();
void updatetraptbl(Exprnode *linnumexprptr, Exprnode *ufncexprptr);
