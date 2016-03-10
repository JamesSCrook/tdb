aggregate(name, mass);

function sample_stddev(n, sumxsqrd, xbar) {
    return (sumxsqrd-xbar*xbar*n)/(n)^0.5;
}

printf("Name            mass\n");
printf("______________  _____\n");
foreach(name:) {
    foreach(mass: name)
	printf("%-15s %5.1f\n", name, mass);
}
total_mass = sum(mass:);
numpeople = count();
avg_mass = total_mass/numpeople;
printf("---------------------\n");
printf("tot mass:       %5.1f\n", total_mass);
printf("---------------------\n");
printf("count:          %5d\n", numpeople);
printf("---------------------\n");
printf("average:        %5.2f\n", avg_mass);
printf("---------------------\n");
printf("sumsqrd:    %9.2f\n", sumsquared(mass:));
printf("---------------------\n");
printf("sample std dev: %5.2f\n",
    sample_stddev(numpeople, sumsquared(mass:), avg_mass));
