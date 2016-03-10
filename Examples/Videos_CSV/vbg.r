aggregate(genre, title);

foreach(title:) {
    printf("%s\n\t", title);
    foreach(genre: title) {
	printf(" %s,", genre);
    }
    printf("\n");
}
