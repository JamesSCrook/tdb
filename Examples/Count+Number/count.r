aggregate(store, fruit);

printf("
The 'count' of times each fruit was purchased
at each store; fruit and store are both sorted
by the least number of purchases.\n\n");

sort(fruit: count(fruit));
sort(store: count(store));

printf("%-12s", "");
foreach (store:) {
    printf("%-7s", store);
}
printf("  TOTALS\n");

foreach (fruit:) {
    printf("%-8s", fruit);
    foreach (store:) {
	printf("%7d", count(fruit, store));
    }
    printf("%9d\n", count(fruit));
}

printf("\n%-8s", "TOTALS");
foreach (store:) {
    printf("%7d", count(store));
}
printf("%9d\n", count());
