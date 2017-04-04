################################################################################
#   tdb: a text database processing tool
#   Copyright (c) 1991-2016 James S. Crook
#
#   This file is part of tdb.
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

##### Linux variables #############
CC			= gcc
CFLAGS			= -s -O -D_FLEX -D_BISON
#CFLAGS			= -s -O2 -pedantic -Wextra -Wshadow -Wpointer-arith -Wcast-qual -D_FLEX -D_BISON
LEXICAL_ANALYZER	= flex	
PARSER_GENERATOR	= bison
PARS_GEN_FLAGS		= -d -o y.tab.c

##### non Linux variables #############
#CC			= gcc
#CFLAGS			= -s -O
#LEXICAL_ANALYZER	= lex
#PARSER_GENERATOR	= yacc
#PARS_GEN_FLAGS		= -d

########## common variables #########
PROG			= tdb

# Yacc dependent/independent sources & objects
TDBSRCS = avltrees.c calc.c expr.c field.c lookup.c main.c print.c \
	  read.c report.c selrej.c stmt.c util.c var.c
TDBOBJS = $(TDBSRCS:.c=.o)

# All objects
ALLOBJS = $(TDBOBJS) y.tab.o

$(PROG): $(ALLOBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(ALLOBJS) -lm

$(TDBOBJS): tdb.h y.tab.h

y.tab.c y.tab.h: tdb.y tdb.h
	@echo "NOTE: 1 shift/reduce conflict (the if/else one) is OK"
	$(PARSER_GENERATOR) $(PARS_GEN_FLAGS) tdb.y

y.tab.o: lex.yy.c
#	$(CC) $(CFLAGS) -c $<

lex.yy.c: tdb.l
	$(LEXICAL_ANALYZER) tdb.l

avltrees.o var.o: avlbal.h

clean: 
	rm -f $(ALLOBJS) lex.yy.c y.tab.h y.tab.c y.output core
