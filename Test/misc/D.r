aggregate(opr, dat);

printf("\t\t      ");
foreach(dat:)
    printf("%s", formatdate(" %d/%m", dat));
printf("    Total  #DaysWkd\n\n");

foreach(opr:) {
    printf("%-18s : ", lookup(opr, oprfile));
    foreach(dat:)
	printf("%6d", count(opr,dat));
    printf("%8d", count(opr));
    printf("%8d", number(dat: opr));
    printf("\n");
}

printf("\nTotals:\t\t     ");
foreach(dat:)
    printf("%6d", count(dat));
printf("%8d", count());
printf("%8d\n", number(dat:));

printf("Oprs on date:\t     ");
foreach(dat:)
    printf("%6d", number(opr: dat));
printf("%8d\n", number(opr:));
