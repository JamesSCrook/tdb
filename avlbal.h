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
	avlbal.h - TDB

    This header file contains the MACRO definition for the code to balance a
binary tree using the AVL algorithm.  It is used in three places, so this is
a (source) code saving excercise.  Note that this MACRO is a block, and as
such, can declare automatic variable(s).  It declares one (tmpptr) of the type
passed as the first parameter (avlbaltype).
*******************************************************************************/

#define BALAVLSUBTREE(avlbaltype, nodeptr, childptr, nodeptrptr) \
{ \
    avlbaltype *tmpptr; \
\
    if ((nodeptr)->balfact == 2) { \
	if ((childptr)->balfact == 1) {		/* case I, rotate left */ \
	    *(nodeptrptr) = (childptr); \
	    (nodeptr)->rightptr = (childptr)->leftptr; \
	    (childptr)->leftptr = (nodeptr); \
	    (nodeptr)->balfact = (childptr)->balfact = 0; \
	} else {		/* child->balfact == -1,  case II, R then L */ \
	    tmpptr = (childptr)->leftptr;	/* rotation 1, around ? */ \
	    (childptr)->leftptr = tmpptr->rightptr; \
	    tmpptr->rightptr = (childptr); \
	    (nodeptr)->rightptr = tmpptr; \
	    *(nodeptrptr) = tmpptr;		/* rotation 2, around ? */ \
	    (nodeptr)->rightptr = tmpptr->leftptr; \
	    tmpptr->leftptr = (nodeptr); \
	    switch(tmpptr->balfact) {		/* adjust balance factors */ \
		case -1: \
		    (nodeptr)->balfact = 0; \
		    (childptr)->balfact = 1; \
		    break; \
		case 0: \
		    (nodeptr)->balfact = 0; \
		    (childptr)->balfact = 0; \
		    break; \
		case 1: \
		    (nodeptr)->balfact = -1; \
		    (childptr)->balfact = 0; \
		    break; \
	    } \
	    tmpptr->balfact = 0; \
	} \
    } else {					/* (nodeptr)->balfact == -2 */ \
	if ((childptr)->balfact == -1) { 		/* case I, R */ \
	    *(nodeptrptr) = (childptr); \
	    (nodeptr)->leftptr = (childptr)->rightptr; \
	    (childptr)->rightptr = (nodeptr); \
	    (nodeptr)->balfact = (childptr)->balfact = 0; \
	} else {	    /* (childptr)->balfact == 1, case II, L then R */ \
	    tmpptr = (childptr)->rightptr;	/* rotation 1, around ? */ \
	    (childptr)->rightptr = tmpptr->leftptr; \
	    tmpptr->leftptr = (childptr); \
	    (nodeptr)->leftptr = tmpptr; \
	    *(nodeptrptr) = tmpptr;		/* rotation 2, around ? */ \
	    (nodeptr)->leftptr = tmpptr->rightptr; \
	    tmpptr->rightptr = (nodeptr); \
	    switch(tmpptr->balfact) {		/* adjust balance factors */ \
		case -1: \
		    (nodeptr)->balfact = 1; \
		    (childptr)->balfact = 0; \
		    break; \
		case 0: \
		    (nodeptr)->balfact = 0; \
		    (childptr)->balfact = 0; \
		    break; \
		case 1: \
		    (nodeptr)->balfact = 0; \
		    (childptr)->balfact = -1; \
		    break; \
	    } \
	    tmpptr->balfact = 0; \
	} \
    } \
}
