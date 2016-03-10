function headerweeksum(str) {
    printf("%-28s", str);
    foreach(dat:)
	printf("  %-6s", formatdate("%a", dat));
    printf("  Total\n------------------------    ");
    foreach(dat:)
	printf(" ------ ");
    printf(" ------ \n");
}

function doubleline() {
    printf("                           ========");
    foreach(dat:)
	printf("========");
    printf("\n");
}

function print_n_returns(returncount) {
    while (returncount > 0) {
	printf("\n");
	returncount -= 1;
    }
}

function print_n_value(n, returncount) {
    if (n > 0)
	printf("%8d", n);
    else
	printf("        ");
    print_n_returns(returncount);
}

function print_t_div_n_value(secs, n, returncount) {
    if (secs > 0 && n > 0) {
	t_div_n = (int) ((float)secs / n);
	printf("%5d:%02d", t_div_n/60, t_div_n%60);
    } else {
	printf("        ");
    }
    print_n_returns(returncount);
}

function printminssecs(secs, returncount) {
    if (secs > 0) {
	printf("%5d:%02d", secs/60, secs%60);
    } else {
	printf("        ");
    }
    print_n_returns(returncount);
}

function printhoursmins(secs, returncount) {
    if (secs > 0) {
	printf("%5d:%02d", secs/3600, secs%3600/60);
    } else {
	printf("        ");
    }
    print_n_returns(returncount);
}
