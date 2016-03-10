join(string pid: mod, ser);

aggregate(mod, dat, pid);

function header() {
    printf("<WR1>\n%-64s\n\n", titlestr);
    printf("\t\t\tRepair Area Summary (1) Weekly Report\n");
    printf("\t\t\t=====================================\n\n\n");
    printf("\t\t\t\t------------ Week ending %s", formatdate("%d/%m/%y",enddate));
    printf(" ------------\n\n");

    printf("\t\t\t\t\t\t\t\tDate: %s\n", formatdate("%d/%m/%y", repdate));
    printf("\t\t\t\t\t\t\t\tTime: %s\n\n", formattime("%H:%M", reptime));

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

function print_t_value(t, returncount) {
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

foreach(mod:) {					/* for each board type */
    printf("%s\n", lookup(mod, modfile));

    printf("  Total time (MM:SS)    :");
    foreach(dat:)
	print_t_value(sum(tim: mod, dat), 0);
    print_t_value(sum(tim: mod), 1);

    printf("  Total boards processed:");
    foreach(dat:)
	print_n_value(number(pid: mod, dat), 0);
    print_n_value(number(pid: mod), 1);

    printf("  Average/board (MM:SS) :");
    foreach(dat:)
	print_t_div_n_value(sum(tim: mod, dat), number(pid: mod, dat), 0);
    print_t_div_n_value(sum(tim: mod), number(pid: mod), 1);

    printf("  Total faults processed:");
    foreach(dat:)
	print_n_value(count(mod, dat), 0);
    print_n_value(count(mod), 1);

    printf("  Average/fault (MM:SS) :");
    foreach(dat:)
	print_t_div_n_value(sum(tim: mod, dat), count(mod, dat), 0);
    print_t_div_n_value(sum(tim: mod), count(mod), 2);
}

printf("\t\t\t\t==============================================\n");
printf("Overall time spent (HH:MM):");
foreach(dat:) {
    t = sum(tim: dat);
    printf("\t%3d:", t/3600);
    printf("%02d", t%3600/60);
}
t = sum(tim:);
printf("\t%3d:", t/3600);
printf("%02d\n", t%3600/60);

printf("Overall boards processed  :");
foreach(dat:)
    print_n_value(number(pid: dat), 0);
print_n_value(number(pid:), 1);

printf("Overall avg/board (MM:SS) :");
foreach(dat:)
    print_t_div_n_value(sum(tim: dat), number(pid: dat), 0);
print_t_div_n_value(sum(tim:), number(pid:), 1);

printf("Overall faults processed  :");
foreach(dat:)
    print_n_value(count(dat), 0);
print_n_value(count(), 1);

printf("Overall avg/fault (MM:SS) :");
foreach(dat:)
    print_t_div_n_value(sum(tim: dat), count(dat), 0);
print_t_div_n_value(sum(tim:), count(), 1);
printf("\t\t\t\t==============================================\n");
