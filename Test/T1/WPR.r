join(string pid: mod, ser);

aggregate(mod, fcd, pid);

function printDDMMYY(date) {
    printf("%02d",  date % 100);
    printf("/%02d", date / 100 % 100);
    printf("/%02d", date / 10000 % 100);
}

function headerindent() {
    foreach(mod:)
	printf("     ");
}

function printheader() {
    headerindent();
    printf("     Weekly Production Report\n");
    headerindent();
    printf("     ========================\n");
    headerindent();
    printf("Period from: ");
    printDDMMYY(STARTDATE);
    printf(" to: ");
    printDDMMYY(ENDDATE);
    printf("\n\n");

    printf("-------------");
    foreach(mod:)
	printf(" ----------", mod);
    printf(" ----------\n");

    printf("             ");
    foreach(mod:)
	printf(" -- %-4s --", mod);
    printf(" -- Total -\n");

    printf("Description  ");
    foreach(mod:)
	printf("    # HH:MM", mod);
    printf("    # HH:MM\n");

    printf("-------------");
    foreach(mod:)
	printf(" ----------", mod);
    printf(" ----------\n");
}

function printHHMM(t, returncount) {
    printf(" %2d", t/3600);
    printf(":%02d", t%3600/60);
    while (returncount > 0) {
	printf("\n");
	returncount = returncount - 1;
    }
}

/**************** Start of main program ******************/
printf("\f<WPR>\n");

/*
sort(mod: -sum(tim: mod));
sort(fcd: count(fcd));
*/
printheader();

printf("# boards done");
foreach(mod:) {
    printf("%5d", number(pid: mod));
    printHHMM(sum(tim: mod), 0);
}
printf("%5d", number(pid:));
printHHMM(sum(tim:), 2);

foreach(fcd:) {
    printf("%-13s", fcd);
    foreach(mod:) {
	printf("%5d", count(fcd, mod));
	printHHMM(sum(tim: fcd, mod), 0);
    }
    printf("%5d", count(fcd));
    printHHMM(sum(tim: fcd), 1);
}

printf("             ");
foreach(mod:)
    printf(" ----------", mod);
printf(" ----------\n");
printf("Totals       ");
foreach(mod:) {
    printf("%5d", count(mod));
    printHHMM(sum(tim: mod), 0);
}
printf("%5d", count());
printHHMM(sum(tim:), 2);
