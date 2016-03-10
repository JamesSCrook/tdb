aggregate(opr, crf);

function testparams(intparam, strparam) {
    printf("intparam=%d, strparam=<%s>\n", intparam, strparam);
    intvar = intparam + 4;
    intparam = intvar + 10;
    inttbl[intvar] = intparam;
    printf("intparam=%d, inttbl[%d]=%d\n", intparam, intvar, inttbl[intvar]);
    strvar = strparam + " crook";
    strparam = strvar + " was here";
    printf("strparam=<%s>, strvar=<%s>\n", strparam, strvar);
    sprintf(strparam, "my name is: %s", strvar);
    printf("strparam=<%s>\n", strparam);
}

printf("---- Test of op= ------------------------------------------\n");
ctr = 1500;
printf("%d\n", ctr);
while (ctr < 2500) {
    ctr %= 43;
    printf("%%=43 -> %4d, ", ctr);
    ctr += 4;
    printf("+=4 -> %4d, ", ctr);
    ctr -= 1;
    printf("-=1 -> %4d, ", ctr);
    ctr *= 3;
    printf("*=3 -> %4d, ", ctr);
    ctr /= 9;
    printf("/=9 -> %4d, ", ctr);
    ctr ^= 3;
    printf("^=3 -> %4d\n", ctr);
}

printf("---- Test of array variables ------------------------------\n");
idx = 0;
while (idx < 10) {
    array[idx] = 2*idx;
    array[idx+0.5] = 2*idx+0.5;
    sprintf(strtbl[idx, "jc"], "0x%05x", 256*idx);
    idx += 1;
}
strtbl["cj", 5] = "abc";
strtbl["cj", 6] = 3.14;
idx = 9;
while (idx >= 0) {
    printf("array[%d]=%2d, ", idx, array[idx]);
    printf("array[%.3f]=%6.3f, ", idx+0.5, array[idx+0.5]);
    printf("strtbl[%d, \"jc\"]=<%s>\n", idx-1, strtbl[idx-1, "jc"]);
    idx -= 2;
}
printf("strtbl[\"cj\", 5]=<%s>, strtbl[\"cj\", 5]=%5.3f\n",
			    strtbl["cj", 5], strtbl["cj", 6]);
idx = 0;
while (idx < 5) {
    printf("array[idx]:array[%2d]=%2d\n", idx, array[idx]);
    printf("    array[array[idx]]: array[array[%d]]=%2d\n",
					idx, array[array[idx]]);
    printf("    array[array[array[idx]]/4]: array[array[array[%d]]/4]=%2d\n",
			idx, array[array[array[idx]]/4]);
    idx += 1;
}

printf("--------------------------------\n");
testparams(5, "jim");
printf("--------------------------------\n");

foreach(opr:) {
    printf("<%s>, ", opr);
    oprnametbl[opr] = lookup(opr, oprfile);
}
printf("\n");

printf("--------------------------------\n");
foreach(opr:)
    printf("<%s>\n", oprnametbl[opr]);

printf("--------------------------------\n");
sort(opr: oprnametbl[opr]);
foreach(opr:) {
    printf("<%s>\n", oprnametbl[opr]);
}

printf("--------------------------------\n");
foreach(crf:) {
    printf("<%s>, ", crf);
    crfpnotbl[crf] = lookup(crf, crffile);
    crfdscrtbl[crf] = lookup(crf, crffile, 7);
}
printf("\n");

printf("--------------------------------\n");
printf("Sorted by text description\n");
printf("--------------------------------\n");
sort(crf: crfdscrtbl[crf]);
foreach(crf:)
    printf("<%s>:<%s>:<%s>\n", crf, crfpnotbl[crf], crfdscrtbl[crf]);

printf("--------------------------------\n");
printf("Reverse sorted by part number\n");
printf("--------------------------------\n");
sort(crf: crfpnotbl[crf], reverse);
foreach(crf:)
    printf("<%s>:<%s>:<%s>\n", crf, crfpnotbl[crf], crfdscrtbl[crf]);
