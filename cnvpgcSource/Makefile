#
# Makefile for ISDS 'c' Source Code from IDKK
# This file last updated 2013.08.15:14.15
#                       2013.06.03:16.00
#
# This file is the Makefile for the following programs:
#   cnvseg2sth
#   cnvsth2seg
#   cnvshp2csv
#   cnvgethdr
#   pgcwhpolygon
#
# Note that this Makefile removes the gcc symbol directories, if they exist

CFLAGS = -g -lm  -Wall 

# Target

all: pgcwhpolygon cnvshp2csv cnvsth2seg cnvseg2sth cnvgethdr

# Program cnvgethdr has one primary source file - cnvgethdr.c
cnvgethdr: cnvgethdr.o cnvgethdr.h
	gcc $(CFLAGS) cnvgethdr.c -o cnvgethdr
	rm -rf cnvgethdr.dSYM

# Program cnvshp2csv has one primary source file - cnvshp2csv.c
cnvshp2csv: cnvshp2csv.o cnvswap.o cnvswap.h cnvendian.h
	gcc $(CFLAGS) cnvshp2csv.c cnvswap.c -o cnvshp2csv
	rm -rf cnvshp2csv.dSYM

# Program cnvsth2seg has two primary source files - cnvsth2seg.c cnvfloat.c - and also
# one include file - cnvsth2seg.h
# File cnvfloat.c is shared with program cnvseg2sth
cnvsth2seg: cnvsth2seg.o cnvfloat.o cnvfloat.h cnvswap.o cnvswap.h cnvendian.h cnvsth2seg.h
	gcc $(CFLAGS) cnvsth2seg.c cnvfloat.c cnvswap.o -o cnvsth2seg
	rm -rf cnvsth2seg.dSYM

# Program cnvseg2sth has two primary source files - cnvseg2sth.c cnvfloat.c - and also
# one include file - cnvseg2sth.h
# File cnvfloat.c is shared with program cnvsth2seg
cnvseg2sth: cnvseg2sth.o cnvfloat.o cnvfloat.h cnvendian.h cnvswap.o cnvswap.h cnvseg2sth.h
	gcc $(CFLAGS) cnvseg2sth.c cnvfloat.c cnvswap.c -o cnvseg2sth
	rm -rf cnvseg2sth.dSYM

# Program pgcpolygon has four primary source files - pgcwhpolygon.c pgcwhpolyring.c
# pgcwhpolypoint.c pgcwhpolyhull.c, and a single include file - pgcwhpolygon.h
POLYSRC = pgcwhpolygon.c pgcwhpolyring.c pgcwhpolypoint.c pgcwhpolyhull.c cnvswap.c
pgcwhpolygon: $(POLYSRC) pgcwhpolygon.h cnvendian.h cnvswap.h
	gcc $(CFLAGS) -lm -o pgcwhpolygon $(POLYSRC) 
	rm -rf pgcwhpolygon.dSYM

# The "clean" process removes all the object files that may exist, as well
# as the executable files - and the gcc symbol directories, if they exist
clean:
	rm -rf *.o pgcwhpolygon cnvshp2csv cnvsth2seg cnvseg2sth cnvgethdr 
	rm -rf *.dSYM

#
# End of Makefile
