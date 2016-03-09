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
	avltrees.c - TDB
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "tdb.h"
#include "avlbal.h"
#include "y.tab.h"

int *axisnodectrtbl;
int *datanodectrtbl;


/*******************************************************************************
This MACRO is called to compare Intfltstr (ifs) values for sorting the data
tree.  It is implemented as a macro only to avoid a function call; for speed.
*******************************************************************************/
#define CMPIFS(rv, typ, strptr, ifsval) \
{ \
    float floatdiff; \
 \
    switch(typ) { \
	case INTEGER: \
	    rv = atoi(strptr) - (ifsval).ival; \
	break; \
	case STRING: \
	    rv = mystrcmp(strptr, (ifsval).sval); \
	break; \
	case FLOAT: \
	    floatdiff = (float)atof(strptr) - (ifsval).fval; \
	    if (floatdiff == 0.0) { \
		rv =  0; \
	    } else if (floatdiff > 0.0) { \
		rv =  1; \
	    } else { \
		rv =  -1; \
	    } \
	break; \
	default: \
	    fatalerrlin("CMPIFS: SNARK!"); \
	break; \
    } \
}


/*******************************************************************************
This function is called to add a new axis node to the axis (AVL) binary tree,
for a particular (field) depth.  If the node already exists, it is not added
again.
*******************************************************************************/
static Axisnode*
addaxisnode(int depth, Field *aggrfldptr)
{
    extern Axistbl *axistbl;
    extern char **inplineargptrtbl;
    Axisnode *curaxisptr, **curaxisptrptr, *newaxisptr;
    Axisnode **possaxisptrptr, *axisptrstk[MAXAVLDEPTH];
    int r, didx, prevbalfact, stkdepth;

    stkdepth = 0;
    curaxisptrptr = &axistbl[depth].rootptr;
    curaxisptr =     axistbl[depth].rootptr;
    while (curaxisptr != NULL) {
	CMPIFS(r, aggrfldptr->typ, inplineargptrtbl[aggrfldptr->idx],
							    curaxisptr->arg);
	if (r == 0)
	    return curaxisptr;
	axisptrstk[stkdepth++] = curaxisptr;
	if (ABS(curaxisptr->balfact) == 1)
	    possaxisptrptr = curaxisptrptr;
	if ((r < 0) && (curaxisptr->leftptr != NULL)) {
	    curaxisptrptr = &curaxisptr->leftptr;
	    curaxisptr =     curaxisptr->leftptr;
	} else if ((r > 0) && (curaxisptr->rightptr != NULL)) {
	    curaxisptrptr = &curaxisptr->rightptr;
	    curaxisptr =     curaxisptr->rightptr;
	} else {
	    break;
	}
    }

    newaxisptr = (Axisnode*) malloc(sizeof(Axisnode));
    axisnodectrtbl[depth]++;

    switch (aggrfldptr->typ) {
	case STRING:
	    newaxisptr->arg.sval = (char*) malloc((size_t)
			    strlen(inplineargptrtbl[aggrfldptr->idx])+1);
	    (void)strcpy(newaxisptr->arg.sval,inplineargptrtbl[aggrfldptr->idx]);
	    break;
	case INTEGER:
	    newaxisptr->arg.ival = atoi(inplineargptrtbl[aggrfldptr->idx]);
	    break;
	case FLOAT:
	    newaxisptr->arg.fval =(float)atof(inplineargptrtbl[aggrfldptr->idx]);
	    break;
	default: 
	    fatalerrlin("addaxisnode: SNARK!");
	    break;
    }

    newaxisptr->balfact = 0;
    newaxisptr->leftptr = NULL; newaxisptr->rightptr = NULL;

    if (axistbl[depth].rootptr == NULL) {
	axistbl[depth].rootptr = newaxisptr;
    } else {
	if (r < 0) {
	    curaxisptr->leftptr = newaxisptr;
	} else {
	    curaxisptr->rightptr = newaxisptr;
	}

	axisptrstk[stkdepth++] = newaxisptr;
	for (didx=stkdepth-2; didx>=0; didx--) {
	    prevbalfact = axisptrstk[didx]->balfact;
	    if (axisptrstk[didx+1] == axisptrstk[didx]->leftptr) {
		axisptrstk[didx]->balfact--;
	    } else {
		axisptrstk[didx]->balfact++;
	    }
	    if (ABS(axisptrstk[didx]->balfact) == 2) {
			    /* Note: this is a MACRO, no ';' needed !!!*/
		BALAVLSUBTREE(Axisnode, axisptrstk[didx], axisptrstk[didx+1],
								possaxisptrptr)
		break;
	    } else if (ABS(axisptrstk[didx]->balfact) <= ABS(prevbalfact)) {
		break;
	    }
	}
    }

    axistbl[depth].number++;
    return newaxisptr;
}


/*******************************************************************************
This function is called to add a new data node to the data (AVL) binary tree,
for a particular (field) depth.  If the node already exists, it is not added
again.  The count (number of these nodes) variable is adjusted accordingly.
*******************************************************************************/
void
adddatanode(Datanode **dataptrptr, Field *aggrfldptr, int depth)
{
    extern char **inplineargptrtbl;
    extern int statsinfofldctr;
    extern Statsinfofld *statsinfofldtbl;
    Datanode *curdataptr, **curdataptrptr, *newdataptr;
    Datanode **possdataptrptr, *dataptrstk[MAXAVLDEPTH];
    Axisnode *axisptr;
    int r, didx, prevbalfact, stkdepth, statsfldidx;
    Intfltstr t;

    stkdepth = 0;
    curdataptrptr = dataptrptr;
    curdataptr =   *dataptrptr;
    while (curdataptr != (Datanode*)NULL) {
	CMPIFS(r, aggrfldptr->typ, inplineargptrtbl[aggrfldptr->idx],
							*(curdataptr->argptr));
	if (r == 0) {
	    curdataptr->count += 1;
	    for (statsfldidx=0; statsfldidx<statsinfofldctr; statsfldidx++) {
		switch (statsinfofldtbl[statsfldidx].typ) {
		    case INTEGER:
			t.ival = atoi(inplineargptrtbl[statsinfofldtbl[
							statsfldidx].fldidx]);
			curdataptr->statstbl[statsfldidx].sum.ival += t.ival;
			curdataptr->statstbl[statsfldidx].sumsqrd.ival +=
								t.ival*t.ival;
			break;
		    case FLOAT:
			t.fval = (float)atof(inplineargptrtbl[statsinfofldtbl[
							statsfldidx].fldidx]);
			curdataptr->statstbl[statsfldidx].sum.fval += t.fval;
			curdataptr->statstbl[statsfldidx].sumsqrd.fval +=
								t.fval*t.fval;
			break;
		    default:
			fatalerrlin("adddatanode: IN main loop: SNARK!");
			break;
		}
	    }
	    if ((aggrfldptr+1)->typ != INVALIDFLDIDX)
		adddatanode(&curdataptr->orthogptr, aggrfldptr+1, depth+1);
	    return;
	}
	dataptrstk[stkdepth++] = curdataptr;
	if (ABS(curdataptr->balfact) == 1)
	    possdataptrptr = curdataptrptr;
	if ((r < 0) && (curdataptr->leftptr != NULL)) {
	    curdataptrptr = &curdataptr->leftptr;
	    curdataptr =     curdataptr->leftptr;
	} else if ((r > 0) && (curdataptr->rightptr != NULL)) {
	    curdataptrptr = &curdataptr->rightptr;
	    curdataptr =     curdataptr->rightptr;
	} else {
	    break;
	}
    }

    newdataptr = (Datanode*)malloc(sizeof(Datanode)+sizeof(Intfltstr)
	+ (statsinfofldctr-MINSTATSINFOTBLSIZE)*sizeof(Intfltstr));
    datanodectrtbl[depth]++;
    newdataptr->leftptr = newdataptr->rightptr = newdataptr->orthogptr =
							(Datanode*)NULL;
    newdataptr->count = 1;
    for (statsfldidx=0; statsfldidx<statsinfofldctr; statsfldidx++) {
	switch (statsinfofldtbl[statsfldidx].typ) {
	    case INTEGER:
		t.ival =atoi(
			inplineargptrtbl[statsinfofldtbl[statsfldidx].fldidx]);
		newdataptr->statstbl[statsfldidx].sum.ival = t.ival;
		newdataptr->statstbl[statsfldidx].sumsqrd.ival = t.ival*t.ival;
		break;
	    case FLOAT:
		t.fval = (float)atof(
			inplineargptrtbl[statsinfofldtbl[statsfldidx].fldidx]);
		newdataptr->statstbl[statsfldidx].sum.fval = t.fval;
		newdataptr->statstbl[statsfldidx].sumsqrd.fval = t.fval*t.fval;
		break;
	    default:
		fatalerrlin("adddatanode: AFTER main loop: SNARK!");
		break;
	}
    }

    newdataptr->balfact = 0;
    axisptr = addaxisnode(depth, aggrfldptr);
    newdataptr->argptr = &(axisptr->arg);
    newdataptr->axisptr = axisptr;

    if (*dataptrptr == NULL) {
	*dataptrptr = newdataptr;
    } else {
	if (r < 0) {
	    curdataptr->leftptr = newdataptr;
	} else {
	    curdataptr->rightptr = newdataptr;
	}

	dataptrstk[stkdepth++] = newdataptr;
	for (didx=stkdepth-2; didx>=0; didx--) {
	    prevbalfact = dataptrstk[didx]->balfact;
	    if (dataptrstk[didx+1] == dataptrstk[didx]->leftptr) {
		dataptrstk[didx]->balfact--;
	    } else {
		dataptrstk[didx]->balfact++;
	    }
	    if (ABS(dataptrstk[didx]->balfact) == 2) {
			    /* Note: this is a MACRO, no ';' needed !!!*/
		BALAVLSUBTREE(Datanode, dataptrstk[didx], dataptrstk[didx+1],
								possdataptrptr)
		break;
	    } else if (ABS(dataptrstk[didx]->balfact) <= ABS(prevbalfact)) {
		break;
	    }
	}
    }

    if ((aggrfldptr+1)->typ != INVALIDFLDIDX)
	adddatanode(&newdataptr->orthogptr, aggrfldptr+1, depth+1);
}


/*******************************************************************************
This function is called to print out the total number of data tree nodes, axis
tree nodes, and report statements.
*******************************************************************************/
void
printctrstats()
{
    extern Field *aggrfldtbl;
    extern int nstmts;
    extern int aggrmaxdepth;

    int depth, totaxisnodectr = 0, totdatanodectr = 0;

    fprintf(stderr, "\n%d statements\n", nstmts);

    for (depth=0; depth<=aggrmaxdepth; depth++) {
	totaxisnodectr += axisnodectrtbl[depth];
	totdatanodectr += datanodectrtbl[depth];
    }

    for (depth=0; depth<=aggrmaxdepth; depth++) {
	fprintf(stderr,
		"%-20s: %6d axisnodes (%5.1f%%), %6d datanodes (%5.1f%%)\n",
		aggrfldtbl[depth].name,
		axisnodectrtbl[depth],
		axisnodectrtbl[depth]/(float)totaxisnodectr*100,
		datanodectrtbl[depth],
		datanodectrtbl[depth]/(float)totdatanodectr*100);
    }
    fprintf(stderr, "%-20s: %6d axisnodes, %15d datanodes\n", "Totals",
			    totaxisnodectr, totdatanodectr);
}
