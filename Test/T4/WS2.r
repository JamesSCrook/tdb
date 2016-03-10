join(string pid: mod, ser);

aggregate(opr, dat, pid);

#include "headfoot.i"
#include "weeksum.i"

function headerbottom() {
    headerweeksum("Operator");
}

initialize();

/**************** Start of main program ******************/
foreach(opr:) {					/* for each board type */
    printf("%s\n  Total time (MM:SS)    :  ", lookup(opr, oprfile));
    foreach(dat:)
	printminssecs(sum(tim: opr, dat), 0);
    printminssecs(sum(tim: opr), 1);

    printf("  Total boards processed:  ");
    foreach(dat:)
	print_n_value(number(pid: opr, dat), 0);
    print_n_value(number(pid: opr), 1);

    printf("  Average/board (MM:SS) :  ");
    foreach(dat:)
	print_t_div_n_value(sum(tim: opr, dat), number(pid: opr, dat), 0);
    print_t_div_n_value(sum(tim: opr), number(pid: opr), 1);

    printf("  Total faults processed:  ");
    foreach(dat:)
	print_n_value(count(opr, dat), 0);
    print_n_value(count(opr), 1);

    printf("  Average/fault (MM:SS) :  ");
    foreach(dat:)
	print_t_div_n_value(sum(tim: opr, dat), count(opr, dat), 0);
    print_t_div_n_value(sum(tim: opr), count(opr), 2);
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
