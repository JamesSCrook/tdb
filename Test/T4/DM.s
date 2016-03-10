select(wks == "REP" || wks == "VIS");
reject(dat < startdate || dat > enddate);
