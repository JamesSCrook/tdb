join(string store: chain, ", ", location);
join(string item_unit: item, " (", unit, ")");

aggregate(store, item_unit, quantity, size, price);

printf("                                         Avg. Price / Qty.\n");
printf("  Store                                     ($/unit)\n");
printf("  -----------------------------------    -----------------\n");

foreach(item_unit:) {
    printf("%s\n", item_unit);

    foreach(store: item_unit) {
	sum_prices = 0.0;
	sum_amounts = 0.0;
	foreach(size: item_unit, store) {
	    foreach(quantity: item_unit, store, size) {
		foreach(price: item_unit, store, size, quantity) {
		    c = count(price, item_unit, store, size, quantity);
		    sum_prices += price*c;
		    sum_amounts += size*quantity*c;
		}
	    }
	}
	printf(" %-49s%8.2f\n", store, sum_prices/sum_amounts);
    }
    printf("\n");
}
