aggregate(fcd, dat);

#include "headfoot.i"
#include "weeksum.i"

initialize();	/* must be executed before any printing */

/**************** Start of main program ******************/
foreach(fcd:) {					/* for each board type */
    printf("%s\n  Total time (MM:SS)    :  ", lookup(fcd, fcdexpansionfile));
    foreach(dat:)
	printminssecs(sum(tim: fcd, dat), 0);
    printminssecs(sum(tim: fcd), 1);

    printf("  Total faults processed:  ");
    foreach(dat:)
	print_n_value(count(fcd, dat), 0);
    print_n_value(count(fcd), 1);

    printf("  Average/fault (MM:SS) :  ");
    foreach(dat:)
	print_t_div_n_value(sum(tim: fcd, dat), count(fcd, dat), 0);
    print_t_div_n_value(sum(tim: fcd), count(fcd), 2);
}

doubleline();
printf("Overall time spent (HH:MM):");

foreach(dat:)
    printhoursmins(sum(tim: dat), 0);
printhoursmins(sum(tim:), 1);

printf("Overall faults processed  :");
foreach(dat:)
    print_n_value(count(dat), 0);
print_n_value(count(), 1);

printf("Overall avg/fault (MM:SS) :");
foreach(dat:)
    print_t_div_n_value(sum(tim: dat), count(dat), 0);
print_t_div_n_value(sum(tim:), count(), 1);
doubleline();
