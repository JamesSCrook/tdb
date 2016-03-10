aggregate(store, fruit);

printf("
An 'X' indicates a particular fruit was purchased
at a particular store (one or more times).  Fruits\n");
printf("are sorted by the most purchased fruit.  Stores
use default (alpha/numeric) sorting by that field.\n\n");

sort(fruit: number(store: fruit), reverse);

printf("%12s", "");
foreach (store:) {
    printf("%-7s", store);
}
printf("  ALL_STORES\n\n");

foreach (fruit:) {
    printf("%-8s", fruit);
    foreach (store:) {
	if (count(store, fruit) > 0) {
	    printf("%7s", "X");
	} else {
	    printf("%7s", ".");
	}
    }
    printf("%12d\n", number(store: fruit));
}

printf("\n%-8s", "ALL   ");
foreach (store:) {
    printf("%7d", number(fruit: store));
}
printf("\n");
