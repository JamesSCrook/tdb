aggregate(week, date, dayofweek, category, days, comment);

pagelength = 64;
repdatestr = formatdate("%d/%m/%y", 19990125);	/* should be reportdate() */
reptimestr = formattime("%H:%M", 60000);	/* should be reporttime() */

detailflag = 0;
printf("\n%-53s%s %s\n\n", TITLE, repdatestr, reptimestr);

baddowfmt = "\n\n
======================================================
BAD DAY OF WEEK for %d: '%s' should be '%s'!!!
======================================================
\n\n";

/************ check for wrong dayofweek only ******/
foreach(date:) {
    foreach(dayofweek: date) {
	datestr = formatdate("%a", date);
	if (datestr != dayofweek) {
	    printf(baddowfmt, date, dayofweek, datestr);
	}
    }
}


if (REVERSEFLAG) {
    sort(date: date, reverse);
}


foreach(week:) {			/* detail body */
    printf("Week %2d  ", week);
    foreach(category: week) {
	printf("%5s", category);
    }
    printf("\n");

    printf("         ");
    foreach(category: week) {
	printf(" ----");
    }
    printf("\n");

    foreach(date: week) {
	foreach(comment: week, date) {
	    printf("%s", formatdate("%a %d/%m", date));
	    foreach(category: week) {
		ndays =
		    sum(days: week, date, category, comment);
		if (ndays > 0) {
		    printf("%5.1f", ndays);
		} else {
		    printf(" ....");
		}
	    }
	    printf(" %s\n", comment);
	}
	printf("\n");
    }

    printf(" Totals: ");
    foreach(category: week) {
	printf("%5.1f", sum(days: week, category));
    }
    printf("%7.1f\n\n\n", sum(days: week));
}
