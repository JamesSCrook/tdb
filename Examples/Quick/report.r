aggregate(fruit, price);

/*************************************************************************/
printf("count()=%d\n", count());

printf("-------------- count(fruit) ------------------------\n");
foreach(fruit:) {
    printf("%-8s %8d\n", fruit, count(fruit));
}

printf("-------------- count(price) ------------------------\n");
foreach(price:) {
    printf("%8.2f %8d\n", price, count(price));
}

printf("-------------- count(fruit,price) ------------------------\n");
printf("          ");
foreach(price:) {
    printf("%8.2f", price);
}
printf("\n");
foreach(fruit:) {
    printf("%-8s", fruit);
    foreach(price:) {
	printf("%8d", count(fruit,price));
    }
    printf("\n");
}

printf("-------------- number(fruit:) ------------------------\n");
printf("%8d :     (", number(fruit:));
foreach(fruit:) {
    printf(" %s", fruit);
}
printf(" )\n");

printf("-------------- number(price:) ------------------------\n");
printf("%8d :     (", number(price:));
foreach(price:) {
    printf(" %.2f", price);
}
printf(" )\n");

printf("-------------- number(price: fruit) ------------------------\n");
foreach(fruit:) {
    printf("%-8s %8d :     (", fruit, number(price: fruit));
    foreach(price: fruit) {
	printf(" %.2f", price);
    }
    printf(" )\n");
}

printf("-------------- number(fruit: price) ------------------------\n");
foreach(price:) {
    printf("%6.2f %8d :     (", price, number(fruit: price));
    foreach(fruit: price) {
	printf(" %s", fruit);
    }
    printf(" )\n");
}
/*************************************************************************/

printf("-------------- sum(price:) ------------------------\n");
printf("%.2f\n", sum(price:));

printf("-------------- sum(price: fruit) ------------------------\n");
foreach(fruit:) {
    printf("%-8s %6.2f :     (", fruit, sum(price: fruit));
    foreach(price: fruit) {
	printf(" %d*%.2f", count(price, fruit), price);
    }
    printf(" )\n");
}

printf("-------------- sumsquared(price:) ------------------------\n");
printf("%.2f\n", sumsquared(price:));

printf("-------------- sumsquared(price: fruit) ------------------------\n");
foreach(fruit:) {
    printf("%-8s %6.2f :     (", fruit, sumsquared(price: fruit));
    foreach(price: fruit) {
	printf(" %d*%.2f", count(price, fruit), price^2);
    }
    printf(" )\n");
}

printf("-------------- lookup ------------------------\n");
foreach(fruit:) {
    printf("%-8s\n", fruit);
    printf("  lookup(%s, file)   =%s\n", fruit, lookup(fruit, "./lookup"));
    printf("  lookup(%s, file, 1)=%s\n", fruit, lookup(fruit, "./lookup", 1));
    printf("  lookup(%s, file, 2)=%s\n", fruit, lookup(fruit, "./lookup", 2));
    printf("  lookup(%s, file, 3)=%s\n", fruit, lookup(fruit, "./lookup", 3));
}
