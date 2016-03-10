join(string pid: mod, ser);

aggregate(mod, fcd, pid);

#include "headfoot.i"

function headerbottom() {
    printf("-------------");
    foreach(mod:)
	printf(" ----------", mod);
    printf(" ----------\n");

    printf("             ");
    foreach(mod:)
	printf(" -- %-4s --", mod);
    printf(" -- Total -\n");

    printf("Description  ");
    foreach(mod:)
	printf("    # HH:MM", mod);
    printf("    # HH:MM\n");

    printf("-------------");
    foreach(mod:)
	printf(" ----------", mod);
    printf(" ----------\n");
}

titlestr="Weekly Production Report";
initialize();

/**************** Start of main program ******************/
printf("# boards done");
foreach(mod:)
    printf("%5d%s", number(pid: mod), formattime(" %H:%M", sum(tim: mod)));
printf("%5d%s", number(pid:), formattime(" %H:%M\n\n", sum(tim:)));

foreach(fcd:) {
    printf("%-13s", lookup(fcd, fcdexpansionfile));
    foreach(mod:)
	printf("%5d%s", count(fcd, mod), formattime(" %H:%M", sum(tim: fcd, mod)));
    printf("%5d%s\n", count(fcd), formattime(" %H:%M", sum(tim: fcd)));
}

printf("             ");
foreach(mod:)
    printf(" ----------", mod);
printf(" ----------\n");
printf("Totals       ");
foreach(mod:)
    printf("%5d%s", count(mod), formattime(" %H:%M", sum(tim: mod)));
printf("%5d%s\n\n", count(), formattime(" %H:%M", sum(tim:)));
