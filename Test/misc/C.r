join(string pid: mod, ser);

aggregate(dat);

function factorial(n) {		/* test of recursion */
    if (n > 1)
	return n * factorial(n-1);
    else
	return 1;
}

function square(n) {
    return n*n;
}

function sum2(x, y) {
    return x + y;
}

function sumtenargs(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) {
    return a1+a2+a3+a4+a5+a6+a7+a8+a9+a10;
}

function teststrfuncs(patternstr) {
    printf("indexstr of '%s' in the week days (+strlen, substr)\n", patternstr);
    foreach(dat:) {
	dayofweek = formatdate("%A", dat);
	len = strlen(dayofweek);
	idx = indexstr(patternstr, dayofweek);
	printf("    indexstr(%s, %s)=%d", dayofweek, patternstr, idx);
	if (idx > 0) {
	    printf("  :  substr(%s, %d, %d)=%s\n", dayofweek, idx, len,
					substr(dayofweek, idx, len));
	} else {
	    printf("\n");
	}
    }
}

n = 1;
while (n <= 12) {
    printf("factorial(%2d) = %12d\n", n, factorial(n));
    n = n + 1;
}

printf("----------------------\n");
printf("square(6)=%d\n", square(6));
printf("sum(8,9)=%d\n", sum2(8,9));
printf("square(sum(3,5))=%d\n", square(sum2(3,5)));
printf("square(sum(-1,-2)) + square(sum(-4,-5))=%d\n",
	square(sum2(-1,-2)) + square(sum2(-4,-5))  );
printf("sum(square(sum(-1,-2)), square(sum(-4,-5)))=%d\n",
        sum2(square(sum2(-1,-2)), square(sum2(-4,-5)))  );
m = 3;
n = 4;
printf("square(%d)=%d\n", m, square(m));
printf("sum(%d,%d)=%d\n", m, n, sum2(m,n));
printf("square(sum(%d,%d))=%d\n", m, n, square(sum2(m,n)));
printf("sum(square(sum(%d,%d)), square(sum(%d,%d)))=%d\n",
			m, n,
					   2*m,2*n,
	sum2(square(sum2( m, n)), square(sum2(2*m,2*n)))
);
printf("sumtenargs(1,2,3,4,5,6,7,8,9,10)=%d\n",
	sumtenargs(1,2,3,4,5,6,7,8,9,10));

printf("----------------------\n");
patternstr = "e";
printf("Days of the week matching the pattern `%s':\n", patternstr);
foreach(dat:) {
    dayofweek = formatdate("%a", dat);
    if (patmatch(patternstr, dayofweek))
	printf("    %s : YES\n", dayofweek);
    else
	printf("    %s : NO\n", dayofweek);
}

patternstr = "^T";
printf("Days of the week matching the pattern `%s':\n", patternstr);
foreach(dat:) {
    dayofweek = formatdate("%a", dat);
    if (patmatch(patternstr, dayofweek))
	printf("    %s : YES\n", dayofweek);
    else
	printf("    %s : NO\n", dayofweek);
}

patternstr = "^[MW]";
printf("Days of the week matching the pattern `%s':\n", patternstr);
foreach(dat:) {
    dayofweek = formatdate("%a", dat);
    if (patmatch(patternstr, dayofweek))
	printf("    %s : YES\n", dayofweek);
    else
	printf("    %s : NO\n", dayofweek);
}

patternstr = "[eu]$";
printf("Days of the week matching the pattern `%s':\n", patternstr);
foreach(dat:) {
    dayofweek = formatdate("%a", dat);
    if (patmatch(patternstr, dayofweek))
	printf("    %s : YES\n", dayofweek);
    else
	printf("    %s : NO\n", dayofweek);
}


teststrfuncs("n");
teststrfuncs("es");
teststrfuncs("sd");
