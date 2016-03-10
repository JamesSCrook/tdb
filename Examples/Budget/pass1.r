aggregate(year, month, category, item, type);

typesokflag = 1;
foreach(type:) {
    if (type != "A" && type != "P") {
	printf("%s: illegal type\n", type);
	typesokflag = 0;
    }
}

if (typesokflag) {

    FMT = "%-22s %-22s %s %10.2f %4d %2d 00\n";	/* 00 is a bogus day */
    subtotalstr = "\"} Total\"";
    grandtotalstr = "\"} Yearly Total\"";

    foreach(year:) {

	foreach(category: year) {
	    sprintf(catstr, "\"%s\"", category);

	    foreach(item: year, category) {
		sprintf(itemstr, "\"%s\"", item);

		ntypes_yci = number(type: year, category, item);
		foreach(month: year, category, item) {
		    val_ycim["A"] = 0.0;
		    val_ycim["P"] = 0.0;

		    foreach(type: year, category, item, month) {
			val = sum(price: year, category, item, type, month);
			printf(FMT, catstr, itemstr, type, val, year, month);
			val_ycim[type] = val;
		    }

		    if (ntypes_yci == 2) {
			printf(FMT, catstr, itemstr, "D",
				    val_ycim["A"]-val_ycim["P"], year, month);
		    }
		}
	    }

	    ntypes_yc = number(type: year, category);
	    if (number(item: year, category) > 1) {
		foreach(month: year, category) {
		    val_ycm["A"] = 0.0;
		    val_ycm["P"] = 0.0;
		    foreach(type: year, category, month) {
			pr = sum(price: year, category, type, month);
			printf(FMT, catstr, subtotalstr, type, pr, year, month);
			val_ycm[type] = pr;
		    }
		    if (ntypes_yc == 2) {
			printf(FMT, catstr, subtotalstr, "D",
					val_ycm["A"]-val_ycm["P"], year, month);
		    }
		}
	    }
	}

	foreach(month: year) {
	    val_ym["A"] = 0.0;
	    val_ym["P"] = 0.0;
	    foreach(type: year, month) {
		pr = sum(price: year, type, month);
		printf(FMT, grandtotalstr, "\"\"", type, pr, year, month);
		val_ym[type] = pr;
	    }
	    printf(FMT, grandtotalstr, "\"\"", "D", val_ym["A"]-val_ym["P"],
							    year, month);
	}
    }
}
