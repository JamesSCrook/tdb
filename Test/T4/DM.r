join(string pid: mod, ser);

aggregate(mod, pid, wks, fcd);

#include "headfoot.i"

function headerbottom() {
    printf("                             ");
    foreach(wks:)
	printf("   ---- %s ----", wks);
    printf("\nFault Type                  ");
    foreach(wks:)
	printf("    # Flts Perfm");
    printf("    %% Missed\n------------                 ");
    foreach(wks:)
	printf("   ------ ------");

    printf("   --------\n");
}

repdate = reportdate();
reptime = reporttime();
titlestr="Percentage of Defects Missed By Visual Inspection Area";
initialize();

/**************** Start of main program ******************/
foreach(mod:) {					/* for each board type */
    printf("\nBoard type: %-2s (# boards)", mod);
    foreach(wks:)
	printf("           %5d", number(pid: mod, wks));
    printf("\n============================\n");
    foreach(fcd: mod) {				/* for each fault code */
	nflts_modfcd = count(mod, fcd);
	printf("%-29s", lookup(fcd, fcdexpansionfile));
	non_vis = 0.0;
	foreach(wks:) {			/* for each workstation */
	    nbrds_modwks = number(pid: mod, wks);		/* # faults */
	    nflts_modfcdwks = count(mod, fcd, wks);
	    printf("    %5d", nflts_modfcdwks);

				    /* fault finding rate, FFR (flts/brd) */
	    if (nbrds_modwks != 0) {
		wks_ffr = (float)nflts_modfcdwks / nbrds_modwks;
		printf("%7.2f", wks_ffr);
	    } else {
		wks_ffr = 0.0;
		printf("  *****");
	    }
				    /* save the wks flt finding rate */
	    if (wks == "VIS") {
		vis = wks_ffr;
	    } else {
		non_vis = non_vis + wks_ffr;
	    }
	}
	printf("%11.2f\n", (1 - (vis / (vis+non_vis)))*100);
    }
}
