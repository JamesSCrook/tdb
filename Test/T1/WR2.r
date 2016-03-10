join(string pid: mod, ser);

aggregate(opr, dat, pid);

function printDDMMYY(date) {
    printf("%02d",  date % 100);
    printf("/%02d", date / 100 % 100);
    printf("/%02d", date / 10000 % 100);
}

function printheader() {
    printf("\t\t\tRepair Area Summary (2) Weekly Report\n");
    printf("\t\t\t=====================================\n\n\n");
    printf("\t\t\t\t------------ Week ending ");
    printDDMMYY(ENDDATE);
    printf(" ------------\nBoard\nType\t\t\t");
    foreach(dat:) {
	printf("\t %02d", dat%100);
	printf("/%02d", dat/100%100);
    }
    printf("\t Total \n--------\t\t");
    foreach(dat:)
	printf("\t------", dat);
    printf("\t------\n");
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
	printf("\t%3d:", t/60);
	printf("%02d", t%60);
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
	printf("\t%3d:", t_div_n/60);
	printf("%02d", t_div_n%60);
    } else {
	printf("\t      ");
    }
    while (returncount > 0) {
	printf("\n");
	returncount = returncount - 1;
    }
}

/**************** Start of main program ******************/
printf("\f<WR2>\n");
printheader();

foreach(opr:) {					/* for each board type */
    printf("%s\n", opr);

    printf("  Total time (MM:SS)    :");
    foreach(dat:)
	print_t_value(sum(tim: opr, dat), 0);
    print_t_value(sum(tim: opr), 1);

    printf("  Total boards processed:");
    foreach(dat:)
	print_n_value(number(pid: opr, dat), 0);
    print_n_value(number(pid: opr), 1);

    printf("  Average/board (MM:SS) :");
    foreach(dat:)
	print_t_div_n_value(sum(tim: opr, dat), number(pid: opr, dat), 0);
    print_t_div_n_value(sum(tim: opr), number(pid: opr), 1);

    printf("  Total faults processed:");
    foreach(dat:)
	print_n_value(count(opr, dat), 0);
    print_n_value(count(opr), 1);

    printf("  Average/fault (MM:SS) :");
    foreach(dat:)
	print_t_div_n_value(sum(tim: opr, dat), count(opr, dat), 0);
    print_t_div_n_value(sum(tim: opr), count(opr), 2);
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
