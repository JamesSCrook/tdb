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
function printsubtotal() {
    dashedline();
    printf("Subtot: ");
    foreach(dat:)
	printf("%8d", count(mod,dat));
    printf("%8d%8.1f%%\n", nflts_mod, 100.0*nflts_mod/nflts);
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
printheader();
foreach(mod:) {				/* for each board type */
    nflts_mod = count(mod);
    printf("%-6s : %d boards\n", lookup(mod, modfile), number(pid: mod));
    foreach(opr: mod) {			/* for each operator */
	nflts_modopr = count(mod,opr);
	printf("  %-6s", opr);
	foreach(dat:)		/* for each date */
	    printf("%8d", count(mod,opr,dat));
	printf("%8d%8.1f%% (%d brd)\n", nflts_modopr,
			    100.0*nflts_modopr/nflts, number(pid: opr,mod));
    }
    printsubtotal();
}
printtotals();
