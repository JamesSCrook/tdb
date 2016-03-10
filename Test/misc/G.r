aggregate(opr, dat, crf, pno);

function horizline() {
    foreach (dat:)
	printf("----------");
    printf("----------------------------\n");
}

function printheader() {
    printf("\t\t\t\t\t\t\t%s\n", formatdate("%a %b %d, %Y", reportdate()));
    printf("\t\t\t\t\t\t\t%s\n\n\n", formattime("%H:%M:%S", reporttime()));

    printf("Operator name\t  ");
    foreach (dat:)
	printf("  %s", formatdate("%d/%m/%y", dat));
    printf("  Opr Tots\n");
    horizline();
}

/************************ Main program ************************/

foreach (crf:) {
    printf("%-5s: ", crf);
    foreach (pno: crf)
	printf(" %s", pno);
    printf("\n");
}
printf("\n");

foreach (pno:) {
    printf("%s: ", pno);
    foreach (crf: pno)
	printf(" %s", crf);
    printf("\n");
}

/*************************************/
sort(opr: lookup(opr, oprfile), reverse);
printheader();

foreach(opr:) {
    printf("%-18s", lookup(opr, oprfile));
    foreach (dat:)
	printf("  %8d", count(opr, dat));
    printf("  %8d\n", count(opr));

    printf("\t\t  ");
    foreach (dat:)
	printf("  %8d", sum(tim: opr, dat));
    printf("  %8d\n", sum(tim: opr));
}

horizline();

printf("   Date Totals:\t  ");
foreach (dat:)
    printf("  %8d", count(dat));
printf("  %8d\n", count());

printf("\t\t  ");
foreach (dat:)
    printf("  %8d", sum(tim: dat));
printf("  %8d\n", sum(tim:));
