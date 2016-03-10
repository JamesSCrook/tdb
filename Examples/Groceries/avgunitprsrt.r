join(string store: chain, ", ", location);
join(string item_unit: item, " (", unit, ")");

aggregate(store, item_unit, price, quantity, size);

printf("                                         Avg. Price / Qty.\n");
printf("  Store                                     ($/unit)\n");
printf("  -----------------------------------    -----------------\n");

foreach(store:)
    tot_cost[store] = 0.0;	/* must be a float */

foreach(item_unit:) {
    tot_price = 0.0;
    tot_amount = 0.0;
    foreach(store: item_unit) {
	tot_store_price = 0.0;
	tot_store_amount = 0.0;
	foreach(price: item_unit, store) {
	    foreach(quantity: item_unit, store, price) {
		foreach(size: item_unit, store, price, quantity) {
		    c = count(size, quantity, price, item_unit, store);
		    tot_store_price += c * price;
		    tot_store_amount += c * quantity * size;
		}
	    }
	}
	tot_cost[store] = tot_store_price/tot_store_amount;
	tot_price += tot_store_price;
	tot_amount += tot_store_amount;
    }

    printf("%-38s%8.2f\n", item_unit, tot_price/tot_amount);
    sort(store: tot_cost[store]);
    foreach(store: item_unit)
	printf(" %-49s%8.2f\n", store, tot_cost[store]);
    printf("\n");
}
