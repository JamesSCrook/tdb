aggregate(date);

minyear = 9999;
maxyear = 0;

foreach(date:) {

    if (STARTDATE <= date) {
	begdate = STARTDATE;
	enddate = date;
	sign = 1;
    } else {
	begdate = date;
	enddate = STARTDATE;
	sign = -1;
    }

    begyear = atoi(formatdate("%Y", begdate));
    begday  = atoi(formatdate("%j", begdate));
    endyear = atoi(formatdate("%Y", enddate));
    endday  = atoi(formatdate("%j", enddate));

    ndays = endday - begday;
    year = begyear;
    while (year < endyear) {
	sprintf(enddayofyear, "%d1231", year);
	    /****
	    printf("  enddayofyear=%s", enddayofyear);
	    printf("  year: ndays=%d ->", ndays);
	    n = atoi(formatdate("%j", atoi(enddayofyear)));
	    ****/
	ndays += atoi(formatdate("%j", atoi(enddayofyear)));
	    /****
	    printf(" + %d = %d\n", n, ndays);
	    ****/
	year += 1;
    }

    printf("From %d to %d is %d days\n", STARTDATE, date, ndays*sign);

    if (minyear > begyear)
	minyear = begyear;
    if (maxyear < endyear)
	maxyear = endyear;
}

year = minyear;
while (year <= maxyear) {
    sprintf(begdayofyear, "%d0101", year);
    sprintf(enddayofyear, "%d1231", year);
    ndays = atoi(formatdate("%j", atoi(enddayofyear)))
	  - atoi(formatdate("%j", atoi(begdayofyear))) + 1;
    printf("year %d has %d days", year, ndays);
    if (ndays != 365)
	printf(" - a leap year");
    printf("\n");
    year += 1;
}
