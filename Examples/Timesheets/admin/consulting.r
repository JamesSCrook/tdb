aggregate(week, dayofweek, category, days);

function header() {
    printf("\n%-53s%s %s\n\n", TITLE, repdatestr, reptimestr);
    if (detailflag)
	daysofweekheader();
}

function footer() {		/* footerlength lines long */
    printf("\n%36s- %d -\n\n", " ", pagenumber);
}

function daysofweekheader() {			/* days of week header */
    printf("Wk  Cat.");
    foreach(dayofweek:) {
	printf("%7s", dayofweek);
    }
    printf("   Total   Cumul\n");
    printf("==  ====");
    foreach(dayofweek:) {
	printf("   ====");
    }
    printf("   =====   =====\n");
}


pagelength = 64;
repdatestr = formatdate("%d/%m/%y", 19990125);	/* should be reportdate() */
reptimestr = formattime("%H:%M", 50000);	/* should be reporttime() */
trap(0, header());
footerlength = 3;
trap(pagelength-footerlength, footer());

dayofweekordertbl["Mon"] = 0;
dayofweekordertbl["Tue"] = 1;
dayofweekordertbl["Wed"] = 2;
dayofweekordertbl["Thu"] = 3;
dayofweekordertbl["Fri"] = 4;
dayofweekordertbl["Sat"] = 5;
dayofweekordertbl["Sun"] = 6;
sort(dayofweek: dayofweekordertbl[dayofweek]);

detailflag = 0;
need(2+number(category:)); 	/* summary */
printf("--------------- Summary ------------\n");
ntotal = 0.0;
foreach(category:) {
    printf(" %-4s:", category); 
    ndays = sum(days: category);
    printf("%6.1f  days  %s\n", ndays, lookup(category, "admin/categories"));
    ntotal += ndays;
}
printf("Total:%6.1f days\n\n", ntotal);

detailflag = 1;
need(6); 					/* detail header */
printf("Detail (full days)\n");
daysofweekheader();

cumul = 0.0;
foreach(week:) {			/* detail body */
    need(number(category: week)+2);
    firstcategoryflag = 1;
    foreach(dayofweek:)
	n4wk_day[dayofweek] = 0.0;
    foreach(category: week) {
	if (firstcategoryflag == 1) {
	    printf("%02d", week); 
	    firstcategoryflag = 0;
	} else {
	    printf("  "); 
	}
	printf("  %-4s", category); 

	n4wk_cat = 0.0;
	foreach(dayofweek:) {
	    ndays = sum(days: week, category, dayofweek);
	    printf("%7.1f", ndays);
	    n4wk_cat += ndays;
	    n4wk_day[dayofweek] += ndays;
	}
	printf("%8.1f\n", n4wk_cat);

    }

    printf("        ");			/* detail totals */
    foreach(dayofweek:)
	printf("   ----");
    printf("   -----\n");

    printf("        ");
    n4wk = 0.0;
    foreach(dayofweek:) {
	printf("%7.1f", n4wk_day[dayofweek]);
	n4wk += n4wk_day[dayofweek];
    }
    cumul += n4wk;
    printf("%8.1f%8.1f\n\n", n4wk, cumul);
}
