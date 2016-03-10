aggregate(name, score, height);

function sample_stddev(n, sumxsqrd, xbar) {
    return (sumxsqrd-xbar*xbar*n)/(n)^0.5;
}

function population_stddev(n, sumxsqrd, xbar) {
    return (sumxsqrd-xbar*xbar*n)/(n-1)^0.5;
}

/*
sort(name: sum(height: name));
sort(name: name, reverse);
sort(name: sum(height: name), reverse);
*/
sort(name: sum(score: name), reverse);

printf("Name            score    height\n");
printf("______________  _____    ______\n");
foreach(name:) {
    foreach(score: name) {
	foreach(height: name, score) {
	    printf("%-15s %5d %8d\n", name, score, height);
	}
    }
}
total_score = sum(score:);
numpeople = count();
avg_score = total_score/(float)numpeople;

total_height = sum(height:);
avg_height = total_height/(float)numpeople;

printf("------------------------------\n");
printf("tot:            %5d %8d\n", total_score, total_height);
printf("------------------------------\n");
printf("count:          %5d\n", numpeople);
printf("------------------------------\n");
printf("average:        %5.2f %8.2f\n", avg_score, avg_height);
printf("------------------------------\n");
printf("sumsqrd:        %5d %8d\n", sumsquared(score:), sumsquared(height:));
printf("------------------------------\n");
printf("sample std dev: %5.2f %8.2f\n",
    sample_stddev(numpeople, (float)sumsquared(score:), avg_score),
    sample_stddev(numpeople, (float)sumsquared(height:), avg_height));
printf("------------------------------\n");
printf("popul std dev:  %5.2f %8.2f\n",
    population_stddev(numpeople, (float)sumsquared(score:), avg_score),
    population_stddev(numpeople, (float)sumsquared(height:), avg_height));
