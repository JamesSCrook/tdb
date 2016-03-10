aggregate(department, weight, course, name);

foreach(department:) {
    foreach(course: department) {
	printf("%s-%d\n", department, course);
	foreach(name: department, course) {
	    printf("  %-25s %d : ", name, count(department, course, name));
	    totalscore = 0.0;
	    foreach(weight: department, course, name) {
		totalscore += sum(score: weight, department, course, name)
							    * (weight/100.0);
	    }
	    printf("%6.2f\n", totalscore);
	}
    }
}
