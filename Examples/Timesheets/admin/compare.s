select(week >= FIRSTWEEK && week <= LASTWEEK);
reject(
    category == "admi" ||
    category == "sdvt"
);
