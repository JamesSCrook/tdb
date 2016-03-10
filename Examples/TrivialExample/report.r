aggregate(metrocity, station, date, low_temp, hi_temp);

function lines() {
    printf("  --------   -----  -----\n");
}

printf("  date       low    high\n");
lines();

foreach (metrocity:) {
    printf("%s\n", metrocity);
    foreach (date: metrocity) {
	count_metrocity_date = count(metrocity, date);
	printf("  %8d  %5.1f  %5.1f", date,
	    sum(low_temp: metrocity, date)/(float)count_metrocity_date,
	    sum(hi_temp: metrocity, date)/(float)count_metrocity_date);

	foreach (station: metrocity, date) {
	    printf(" %s:%d", station, count(metrocity, date, station));
	}
	printf("\n");
    }
    lines();
    count_metrocity = count(metrocity);

    printf("  %-8s  %3d    %3d\n", "Debug",
		sum(low_temp: metrocity), sum(hi_temp: metrocity));

    printf("  %-8s  %5.1f  %5.1f\n\n", "Average",
		sum(low_temp: metrocity)/(float)count_metrocity,
		sum(hi_temp: metrocity)/(float)count_metrocity);
}
