aggregate(num, quotestring);

foreach(num:) {
    foreach(quotestring: num) {
	printf("%2d : quotestring=<%s>\n", num, quotestring);
    }
}
