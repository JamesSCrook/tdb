join(string pid: mod, ser);

aggregate(opr, mod, pid, crf);

printf("\n\t\t\tFaults on Each Side\n\n");

printf("\t\t        side1        side2        Total\n");
printf("\t\t     -----------  -----------  -----------\n");
foreach(mod:) {
    printf("%s : %s\n", mod, lookup(mod, modfile));
    crffile = fcsdir + lookup(mod, modfile, 6);
    side1count = 0;
    side2count = 0;
    foreach(crf:) {
	c = count(mod,crf);
	if (c > 0) {
	    side = lookup(crf, crffile, 3);
	    if (side == "1") {
		side1count = side1count + c;
	    } else if (side == "2") {
		side2count = side2count + c;
	    } else {
		printf("BAD SIDE: `%s'\n", side);
	    }
	}
    }
    totalcount = side1count + side2count;
    printf("\t\t  %5d (%5.1f%%)", side1count, 100.0*side1count/totalcount);
    printf("%5d (%5.1f%%)", side2count, 100.0*side2count/totalcount);
    printf("%6d\n", totalcount);
}

printf("\n\n\t\t\tNumber of Boards Processed\n\n");
printf("Board Types\t\t  ");
foreach(mod:)
    printf("%-6s", mod);
printf("All types\n\n");

sort(opr: lookup(opr, oprfile), reverse);
foreach(opr:) {
    printf("%-18s : ", lookup(opr, oprfile));
    foreach(mod:)
	printf("%6d", number(pid: mod,opr));
    printf("%8d\n", number(pid: opr));
}

printf("\nTotal # Boards\t     ");
foreach(mod:)
    printf("%6d", number(pid: mod));
printf("%8d\n", number(pid:));
