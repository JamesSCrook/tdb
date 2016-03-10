join(string pid: mod, ser);

aggregate(mod,opr,dat,pid);

/*********************************************************/
function printheader() {
    printf("\n  ");
    foreach(dat:)
	printf("    ");
    printf("Number of Faults Found\n\nDate:     ");
    foreach(dat:)
	printf("  %06d", dat%1000000);
    printf("      Totals\n\n");
}

/*********************************************************/
function dashedline() {
    printf("         ------- ");
    foreach(dat:)
	printf("------- ");
    printf("  ------\n");
}

/*********************************************************/
function singleline() {
    printf("       ------------");
    foreach(dat:)
	printf("--------");
    printf("------\n\n");
}

/*********************************************************/
function doubleline() {
    printf("===================");
    foreach(dat:)
	printf("========");
    printf("======\n");
}

/*********************************************************/
function  printsubtotal() {
    dashedline();
    printf("Subtot: ");
    foreach(dat:)
	printf("%8d", count(opr,dat));
    printf("%8d%8.1f%%\n", nflts_opr, 100.0*nflts_opr/nflts);
    singleline();
}

/*********************************************************/
function printtotals() {
    doubleline();
    printf("Totals: ");
    foreach(dat:)
	printf("%8d", count(dat));
    printf("%8d\n", nflts);

    printf("        ");
    foreach(dat:)
	printf("%7.1f%%", 100.0*count(dat)/nflts);
    printf("\n");
}

/**********************************************************
   The main program 
**********************************************************/
nflts = count();
sort(opr: -number(pid: opr));
printheader();
foreach(opr:) {				/* for each operator */
    nflts_opr = count(opr);
    printf("%s : %d boards\n", lookup(opr, oprfile), number(pid: opr));
    sort(mod: -count(opr,mod));
    foreach(mod: opr) {			/* for each board type */
	nflts_oprmod = count(opr,mod);
	printf("  %-6s", mod);
	foreach(dat:)		/* for each date */
	    printf("%8d", count(opr,mod,dat));
	printf("%8d%8.1f%% (%d brd)\n", nflts_oprmod,
			100.0*nflts_oprmod/nflts, number(pid: opr,mod));
    }
    printsubtotal();
}
printtotals();

printf("SECSPERHOUR=%d\n", SECSPERHOUR);
printf("PI=%f\n", PI);
printf("e=%f\n", e);
printf("name=%s\n", name);
