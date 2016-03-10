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
function do_yearcat() {

    foreach(item: year, category) {
	n_types = number(type: year, category, item);	/* 1 or 3 */
	need(n_types+1);				/* 2 or 4 */
	printf("%s: %s\n", category, item);

	foreach(type: year, category, item) {
	    printf("%s", type);
	    foreach(month:) {
		printvalue(tblfmtnarrow, tblfmtwide,
				sum(price: year, category, item, type, month));
	    }
	    printvalue(totfmtnarrow, totfmtwide,
					sum(price: year, category, item, type));
	    printf("\n");
	}
	printf("\n");
    }

    if (need(3))
	line("--------", "------", "\n");
}


/****************************************************************************
Main program
****************************************************************************/
pagelength = 66;
trap(0, header());
trap(pagelength-2, footer());

tblfmtnarrow = "%6.0f"; tblfmtwide = "%5.0fk";
totfmtnarrow = "%7.0f"; totfmtwide = "%6.0fk";

typesorttbl["P"] = 1;
typesorttbl["A"] = 2;
typesorttbl["D"] = 3;
sort(type: typesorttbl[type]);

foreach(year:) {
    foreach(category: year) {
	do_yearcat();
    }
}
