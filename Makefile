#
# Makefile for ISDS 'c' Source Code from IDKK
# This file last updated 2013.06.03:16.00
#
# This file is the Makefile for the following programs:
#   cnvseg2sth
#   cnvsth2seg
#   cnvshp2csv
#   pgcwhpolygon
#
# Note that this Makefile removes the gcc symbol directories, if they exist

CFLAGS = -g -Wall 

# Target

all: pgcwhpolygon cnvshp2csv cnvsth2seg cnvseg2sth 

# Program cnvshp2csv has one source file - cnvshp2csv.c
cnvshp2csv: cnvshp2csv.o
	gcc $(CFLAGS) cnvshp2csv.c -o cnvshp2csv
	rm -rf cnvshp2csv.dSYM

# Program cnvsth2seg has two source files - cnvsth2seg.c cnvfloat.c - and also
# one include file - cnvsth2seg.h
# File cnvfloat.c is shared with program cnvseg2sth
cnvsth2seg: cnvsth2seg.o cnvfloat.o cnvsth2seg.h
	gcc $(CFLAGS) cnvsth2seg.c cnvfloat.c -o cnvsth2seg
	rm -rf cnvsth2seg.dSYM

# Program cnvseg2sth has two source files - cnvseg2sth.c cnvfloat.c - and also
# one include file - cnvseg2sth.h
# File cnvfloat.c is shared with program cnvsth2seg
cnvseg2sth: cnvseg2sth.o cnvfloat.o cnvseg2sth.h
	gcc $(CFLAGS) cnvseg2sth.c cnvfloat.c -o cnvseg2sth
	rm -rf cnvseg2sth.dSYM

# Program pgcpolygon has four source files - pgcwhpolygon.c pgcwhpolyring.c
# pgcwhpolypoint.c pgcwhpolyhull.c, and a single include file - pgcwhpolygon.h
POLYSRC = pgcwhpolygon.c pgcwhpolyring.c pgcwhpolypoint.c pgcwhpolyhull.c
pgcwhpolygon: $(POLYSRC) pgcwhpolygon.h
	gcc $(CFLAGS) -lm -o pgcwhpolygon $(POLYSRC) 
	rm -rf pgcwhpolygon.dSYM

# The "clean" process removes all the object files that may exist, as well
# as the executable files - and the gcc symbol directories, if they exist
clean:
	rm -rf *.o pgcwhpolygon cnvshp2csv cnvsth2seg cnvseg2sth 
	rm -rf *.dSYM

#
# End of Makefile
