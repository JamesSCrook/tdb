aggregate(fcd, dat);

function header() {
    printf("<WR3>\n%-64s\n\n", titlestr);
    printf("\t\t\tRepair Area Summary (3) Weekly Report\n");
    printf("\t\t\t=====================================\n\n\n");
    printf("\t\t\t\t------------ Week ending %s",formatdate("%d/%m/%y",enddate));
    printf(" ------------\n\n");

    printf("\t\t\t\t\t\t\t\tDate: %s\n", formatdate("%d/%m/%y", reportdate()));
    printf("\t\t\t\t\t\t\t\tTime: %s\n\n", formattime("%H:%M", reporttime()));

    printf("Board\nType\t\t\t");
    foreach(dat:)
	printf("\t %s", formatdate("%a", dat));
    printf("\t Total \n--------\t\t");
    foreach(dat:)
	printf("\t------");
    printf("\t------\n");
}

function footer() {		/* footerlength lines long */
    printf("\n\t\t\t\t    - %d -\n\n", pagenumber);
}

function print_n_value(n, returncount) {
    if (n > 0)
	printf("\t%6d", n);
    else
	printf("\t      ");
    while (returncount > 0) {
	printf("\n");
	returncount = returncount - 1;
    }
}

function printMMSS(t, returncount) {
    if (t > 0) {
	printf("\t%3d:%02d", t/60, t%60);
    } else {
	printf("\t      ");
    }
    while (returncount > 0) {
	printf("\n");
	returncount = returncount - 1;
    }
}

function printHHMM(t, returncount) {
    if (t > 0) {
	printf("\t%3d:%02d", t/3600, t%3600/60);
    } else {
	printf("\t      ");
    }
    while (returncount > 0) {
	printf("\n");
	returncount = returncount - 1;
    }
}

function print_t_div_n_value(t, n, returncount) {
    if (t > 0 && n > 0) {
	t_div_n = (int) ((float)t / n);
	printf("\t%3d:%02d", t_div_n/60, t_div_n%60);
    } else {
	printf("\t      ");
    }
    while (returncount > 0) {
	printf("\n");
	returncount = returncount - 1;
    }
}

/**************** Start of main program ******************/
trap(0, header());
footerlength = 3;
trap(pagelength-footerlength, footer());

foreach(fcd:) {					/* for each board type */
    printf("%s\n", lookup(fcd, fcdexpansionfile));

    printf("  Total time (MM:SS)    :");
    foreach(dat:)
	printMMSS(sum(tim: fcd, dat), 0);
    printMMSS(sum(tim: fcd), 1);

    printf("  Total faults processed:");
    foreach(dat:)
	print_n_value(count(fcd, dat), 0);
    print_n_value(count(fcd), 1);

    printf("  Average/fault (MM:SS): ");
    foreach(dat:)
	print_t_div_n_value(sum(tim: fcd, dat), count(fcd, dat), 0);
    print_t_div_n_value(sum(tim: fcd), count(fcd), 2);
}

printf("\t\t\t\t==============================================\n");
printf("Overall time spent (HH:MM):");

foreach(dat:)
    printHHMM(sum(tim: dat), 0);
printHHMM(sum(tim:), 1);

printf("Overall faults processed  :");
foreach(dat:)
    print_n_value(count(dat), 0);
print_n_value(count(), 1);

printf("Overall avg/fault (MM:SS) :");
foreach(dat:)
    print_t_div_n_value(sum(tim: dat), count(dat), 0);
print_t_div_n_value(sum(tim:), count(), 1);
printf("\t\t\t\t==============================================\n");
