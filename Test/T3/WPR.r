join(string pid: mod, ser);

aggregate(mod, fcd, pid);

function headerindent() {
    foreach(mod:)
	printf("     ");
}

function header() {
    printf("<WPR>\n%-64s\n\n", titlestr);
    headerindent();
    printf("     Weekly Production Report\n");
    headerindent();
    printf("     ========================\n");
    headerindent();
    printf("Period from: %s", formatdate("%d/%m/%y", startdate));
    printf(" to: %s\n\n", formatdate("%d/%m/%y", enddate));

    printf("\t\t\t\t\t\t\t\tDate: %s\n", formatdate("%d/%m/%y", repdate));
    printf("\t\t\t\t\t\t\t\tTime: %s\n\n", formattime("%H:%M", reptime));

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

function footer() {		/* footerlength lines long */
    printf("\n\t\t\t\t    - %d -\n\n", pagenumber);
}

/**************** Start of main program ******************/
trap(0, header());
footerlength = 3;
trap(pagelength-footerlength, footer());

printf("# boards done");
foreach(mod:)
    printf("%5d%s", number(pid: mod), formattime(" %H:%M", sum(tim: mod)));
printf("%5d%s", number(pid:), formattime(" %H:%M\n\n", sum(tim:)));

foreach(fcd:) {
    printf("%-13s", lookup(fcd, fcdexpansionfile));
    foreach(mod:)
	printf("%5d%s", count(fcd, mod), formattime(" %H:%M", sum(tim: fcd, mod)));
    printf("%5d%s\n", count(fcd), formattime(" %H:%M", sum(tim: fcd)));
}

printf("             ");
foreach(mod:)
    printf(" ----------", mod);
printf(" ----------\n");
printf("Totals       ");
foreach(mod:)
    printf("%5d%s", count(mod), formattime(" %H:%M", sum(tim: mod)));
printf("%5d%s\n\n", count(), formattime(" %H:%M", sum(tim:)));
