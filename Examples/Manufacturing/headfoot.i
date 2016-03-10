/*****************************************************************
Setup the variables needed for the header and/or footer (so we
don't have to calculate them each time, and so the report time
is the same on every page.  Also, create a sprintf format string
(headertop_fmt) which is centered, and a line of '=' characters
of the same length.
*****************************************************************/
function initialize() {
    reportdatestr = formatdate("%d, %b %Y", reportdate());
    reporttimestr = formattime("%H:%M", reporttime());
    startdatestr = formatdate("%y/%m/%d", startdate);
    enddatestr   = formatdate("%y/%m/%d", enddate);
    sprintf(headertop_fmt, "%%%ds%%s\n", (80-strlen(titlestr))/2);
    understr = substr(
	"====================================================================",
	1, strlen(titlestr));
    trap(0, header());
    footerlength = 3;
    trap(pagelength-footerlength, footer());
}

/*****************************************************************
Print the top of the header (title, underline, date/time)
*****************************************************************/
function header() {
    printf("\n%-57s%s %s\n\n", "ACME Manufacturing Report",
				    reportdatestr, reporttimestr);
    printf(headertop_fmt, " ", titlestr);
    printf(headertop_fmt, " ", understr);
    printf("%20sPeriod from: %s", " ", startdatestr);
    printf(" to: %s\n\n", enddatestr);
    headerweeksum("Fault");
}

/*****************************************************************
*****************************************************************/
function footer() {		/* footerlength lines long */
    printf("\n%36s- %d -\n\n", " ", pagenumber);
}
