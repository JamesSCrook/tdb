join(string pid: mod, ser);

aggregate(mod, dat, pid);

#include "headfoot.i"
#include "weeksum.i"

function headerbottom() {
    headerweeksum("Board Type");
}

initialize();

/**************** Start of main program ******************/
foreach(mod:) {					/* for each board type */
    printf("%s\n  Total time (MM:SS)    :  ", lookup(mod, modfile));
    foreach(dat:)
	printminssecs(sum(tim: mod, dat), 0);
    printminssecs(sum(tim: mod), 1);

    printf("  Total boards processed:  ");
    foreach(dat:)
	print_n_value(number(pid: mod, dat), 0);
    print_n_value(number(pid: mod), 1);

    printf("  Average/board (MM:SS) :  ");
    foreach(dat:)
	print_t_div_n_value(sum(tim: mod, dat), number(pid: mod, dat), 0);
    print_t_div_n_value(sum(tim: mod), number(pid: mod), 1);

    printf("  Total faults processed:  ");
    foreach(dat:)
	print_n_value(count(mod, dat), 0);
    print_n_value(count(mod), 1);

    printf("  Average/fault (MM:SS) :  ");
    foreach(dat:)
	print_t_div_n_value(sum(tim: mod, dat), count(mod, dat), 0);
    print_t_div_n_value(sum(tim: mod), count(mod), 2);
}

doubleline();
printf("Overall time spent (HH:MM):");
foreach(dat:) {
    t = sum(tim: dat);
    printf("%5d:%02d", t/3600, t%3600/60);
}
t = sum(tim:);
printf("%5d:%02d\n", t/3600, t%3600/60);

printf("Overall boards processed  :");
foreach(dat:)
    print_n_value(number(pid: dat), 0);
print_n_value(number(pid:), 1);

printf("Overall avg/board (MM:SS) :");
foreach(dat:)
    print_t_div_n_value(sum(tim: dat), number(pid: dat), 0);
print_t_div_n_value(sum(tim:), number(pid:), 1);

printf("Overall faults processed  :");
foreach(dat:)
    print_n_value(count(dat), 0);
print_n_value(count(), 1);

printf("Overall avg/fault (MM:SS) :");
foreach(dat:)
    print_t_div_n_value(sum(tim: dat), count(dat), 0);
print_t_div_n_value(sum(tim:), count(), 1);
doubleline();
