join(string store: chain, ", ", location);

aggregate(store, item, price);

function sortfunc(sum_val, count_val) {
    if (count_val != 0)
	return sum_val/count_val;
    else
	return 999999.9;		/* must be a float */
}

printf("  Store                                      Average Expenditure\n");
printf("  -----------------------------------        -------------------\n");

foreach(item:) {
    printf("%-42s%8.2f\n", item, sum(price: item)/count(item));

    sort(store: sortfunc(sum(price: item, store), count(item, store)));
    foreach(store: item) {
	cnt_item_store = count(item, store);
	if (cnt_item_store > 0) {
	    printf("  %-54s%8.2f\n", store,
					sum(price: item, store)/cnt_item_store);
	}
    }
    printf("\n");
}
