select(wks == "REP" || wks == "VIS");
reject(dat < STARTDATE || dat > ENDDATE);
