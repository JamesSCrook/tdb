aggregate(dat);

function header() {
    printf("H\nHHHHHHHHHHHHH -%d- HHHHHHHHHHHHHH\nH\n", pagenumber);
}

pagelength = 15;
trap(0, header());

c = 1;
while (c <= 39) {
    ndatalines = c%5 + 1;
    need(ndatalines+1);
    while (ndatalines > 0) {
	printf("    c=%2d      (ndatalines=%d)\n", c, ndatalines);
	c += 1;
	ndatalines -= 1;
    }
    printf("    ------------------------\n");
}
