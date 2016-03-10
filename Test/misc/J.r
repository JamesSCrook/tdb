aggregate(dat);

function header() {
    printf("H\nHHHHHHHHHHHHH -%d- HHHHHHHHHHHHHH\nH\n", pagenumber);
}

function footer() {
    printf("F\nFFFFFFFFFFFFF -%d- FFFFFFFFFFFFFF\nF\n", pagenumber);
}

pagelength = 16;
footerlength = 3;
trap(0, header());
trap(pagelength-footerlength, footer());

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
