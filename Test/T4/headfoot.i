function headertop() {
    printf("\n%-64s%s %s\n\n", "ACME Manufacturing Report",
							repdatestr, reptimestr);
    printf(headertop_fmt, " ", titlestr);
    printf(headertop_fmt, " ", understr);
    printf("%23sPeriod from: %s", " ", startdatestr);
    printf(" to: %s\n\n", enddatestr);
}

function footer() {		/* footerlength lines long */
    printf("\n%36s- %d -\n\n", " ", pagenumber);
}

function header() {
    headertop();
    headerbottom();
}

function initialize() {
    repdatestr = formatdate("%d/%m/%y", repdate);
    reptimestr = formattime("%H:%M", reptime);
    startdatestr = formatdate("%d/%m/%y", startdate);
    enddatestr   = formatdate("%d/%m/%y", enddate);
    sprintf(headertop_fmt, "%%%ds%%s\n", (80-strlen(titlestr))/2);
    understr = substr(
	"=====================================================================",
	1, strlen(titlestr));
    trap(0, header());
    footerlength = 3;
    trap(pagelength-footerlength, footer());
}
