aggregate(crf);

function horizline() {
    printf("--------------------------------------------------------------\n");
}

function histagram(titlestr) {
    horizline();
    printf(titlestr);
    printf(" - total of %d faults\n", tot_flts);
    horizline();

    foreach (crf:) {
	printf("%-5s: ", crf);
	n_crf_flts = count(crf);
	i = 60.0/max * n_crf_flts;
	while (i > 0) {
	    printf("*", count(crf));
	    i = i - 1;
	}
	printf(" %d", n_crf_flts);
	printf(" (%.1f%%)\n", 100.0*count(crf)/tot_flts);
    }
    horizline();
    printf("\n");

}

/************************ Main program ************************/
tot_flts = count();
max = -1;
foreach (crf:) {
    n_crf_flts = count(crf);
    if (n_crf_flts > max)
	max = n_crf_flts;
}

histagram("Number of failures: Sorted by component reference");

sort(crf: count(crf), reverse);
histagram("Number of failures: Sorted by number of failures");

first(crf: 4);
histagram("Four most frequent failures");

sort(crf: count(crf), reverse);
last(crf: 3);
histagram("Three least frequent failures");
