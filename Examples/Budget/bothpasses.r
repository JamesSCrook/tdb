aggregate(year, type, month, category, price, item);


/***************************************************************************/
function line(str1, str2, str3) {
    printf(str1);
    foreach(month:) {
	printf(str2);
    }
    printf(str3);
}


/***************************************************************************/
function header() {
    line("========", "======", "\n");
    printf("%d Buget (P=Plan, A=Actual, D=Delta) - Month:\n", year);
    foreach(month:) {
	printf("%6d", month);
    }
    line("   Total\n========", "======", "\n");
}


/***************************************************************************/
function footer() {
    line("========", "======", "\n");
    line("", "   ", "");
    printf("Page %d\n", pagenumber);
}


/***************************************************************************/
function printvalue(narrowfmt, widefmt, value) {
    if (value < -9999 || value > 99999)
	printf(widefmt, value/1000);
    else
	printf(narrowfmt, value);
}


/***************************************************************************/
function do_yearcatitem() {
    nlines = ((number(type: year, category, item)>1)+1)*2;	/* 2 or 4 */
    need(nlines);
    printf("%s: %s\n", category, item);
    foreach(month:) {
	catitemmondelta[month] = 0.0;
    }
    /****** one line for each of Plan and/or Actual ******/
    foreach(type: year, category, item) {
	printf("%s", type);
	foreach(month:) {
	    s = sum(price: year, category, item, type, month);
	    printvalue(tblfmtnarrow, tblfmtwide, s);
	    if (type == "A") {
		catitemmondelta[month] += s;
	    } else {
		catitemmondelta[month] -= s;
	    }
	}
	printvalue(totfmtnarrow, totfmtwide,
		    sum(price: year, category, item, type));
	printf("\n");
    }

    /****** print a delta line "iff" we have both Plan AND Actual ******/
    if (number(type: year, category, item) > 1) {
	printf("D");
	catitemdelta = 0.0;
	foreach(month:) {
	    printvalue(tblfmtnarrow, tblfmtwide, catitemmondelta[month]);
	    catitemdelta += catitemmondelta[month];
	}
	printvalue(totfmtnarrow, totfmtwide, catitemdelta);
	printf("\n");
    }
}


/***************************************************************************/
function do_yearcat() {
    foreach(item: year, category) {
	do_yearcatitem();
	printf("\n");
    }

    if (number(item: year, category) > 1 ) {
	foreach(month:) {
	    yearcatmondelta[month] = 0.0;
	}

	nlines = ((number(type: year, category)>1)+1)*2;	/* 2 or 4 */
	need(nlines);
	printf("%s: } Total\n", category);
	foreach(type: year, category) {
	    printf("%s", type);
	    foreach(month:) {
		s = sum(price: year, category, type, month);
		printvalue(tblfmtnarrow, tblfmtwide, s);
		if (type == "A") {
		    yearcatmondelta[month] += s;
		} else {
		    yearcatmondelta[month] -= s;
		}
	    }
	    printvalue(totfmtnarrow, totfmtwide,
				    sum(price: year, category, type));
	    printf("\n");
	}

	if (number(type: year, category) > 1) {
	    yearcatdelta = 0.0;
	    printf("D");
	    foreach(month:) {
		printvalue(tblfmtnarrow, totfmtwide, yearcatmondelta[month]);
		yearcatdelta += yearcatmondelta[month];
	    }
	    printvalue(totfmtnarrow, totfmtwide, yearcatdelta);
	    printf("\n");
	}
	printf("\n");
    }
    if (need(3))
	line("--------", "------", "\n");
}


/***************************************************************************/
function do_grandtotals() {
    need(6);
    printf("} Yearly Total: \n");
    foreach(month:) {
	totmondelta[month] = 0.0;
    }
    foreach(type: year) {
	printf("%s", type);
	foreach(month:) {
	    s = sum(price: year, type, month);
	    printvalue(tblfmtnarrow, tblfmtwide, s);
	    if (type == "A") {
		totmondelta[month] += s;
	    } else {
		totmondelta[month] -= s;
	    }
	}
	printvalue(totfmtnarrow, totfmtwide, sum(price: year, type));
	printf("\n");
    }

    totaldelta = 0.0;
    printf("D");
    foreach(month:) {
	printvalue(tblfmtnarrow, tblfmtwide, totmondelta[month]);
	totaldelta += totmondelta[month];
    }
    printvalue(totfmtnarrow, tblfmtwide, totaldelta);
    line("\n\n--------", "------", "\n");
}


/****************************************************************************
Main program
****************************************************************************/

typesokflag = 1;
foreach(type:) {
    if (type != "A" && type != "P") {
	printf("illegal type: %s\n", type);
	typesokflag = 0;
    }
}

if (typesokflag) {
    pagelength = 66;
    trap(0, header());
    trap(pagelength-2, footer());

    tblfmtnarrow = "%6.0f"; tblfmtwide = "%5.0fk";
    totfmtnarrow = "%7.0f"; totfmtwide = "%6.0fk";

    sort(type: type, reverse);
    foreach(year:) {
	foreach(category: year) {
	    do_yearcat();
	}
	do_grandtotals();
    }
}
