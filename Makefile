############################################################################
# Makefile for huffman encode/decode programs
#
#   $Id: Makefile,v 1.8 2007/09/20 03:30:30 michael Exp $
#   $Log: Makefile,v $
#   Revision 1.8  2007/09/20 03:30:30  michael
#   Replace getopt with optlist.
#   Changes required for LGPL v3.
#
#   Revision 1.7  2005/05/23 03:18:04  michael
#   Moved internal routines and definitions common to both canonical and
#   traditional Huffman coding so that they are only declared once.
#
#   Revision 1.6  2004/06/15 13:34:52  michael
#   Build sample with canonical and traditional huffman codes.
#
#   Revision 1.5  2004/02/26 04:47:55  michael
#   Compile to executable sample program, which links to either huffman or
#   chuffman.
#
#   Revision 1.3  2004/01/13 15:35:47  michael
#   Add CVS Log
#
#
############################################################################

CC = gcc
LD = gcc
CFLAGS = -O3 -Wall -ansi -pedantic -c
LDFLAGS = -O3 -o

# libraries
LIBS = -L. -lhuffman -loptlist

# Treat NT and non-NT windows the same
ifeq ($(OS),Windows_NT)
	OS = Windows
endif

ifeq ($(OS),Windows)
	EXE = .exe
	DEL = del
else	#assume Linux/Unix
	EXE =
	DEL = rm
endif

all:		sample$(EXE)

sample$(EXE):	sample.o libhuffman.a liboptlist.a
		$(LD) $^ $(LIBS) $(LDFLAGS) $@

sample.o:	sample.c huffman.h optlist.h
		$(CC) $(CFLAGS) $<

libhuffman.a:	huffman.o chuffman.o huflocal.o bitarray.o bitfile.o
		ar crv libhuffman.a huffman.o chuffman.o huflocal.o\
		bitarray.o bitfile.o
		ranlib libhuffman.a

huffman.o:	huffman.c huflocal.h bitarray.h bitfile.h
		$(CC) $(CFLAGS) $<

chuffman.o:	chuffman.c huflocal.h bitarray.h bitfile.h
		$(CC) $(CFLAGS) $<

huflocal.o:	huflocal.c huflocal.h
		$(CC) $(CFLAGS) $<

bitarray.o:	bitarray.c bitarray.h
		$(CC) $(CFLAGS) $<

bitfile.o:	bitfile.c bitfile.h
		$(CC) $(CFLAGS) $<

liboptlist.a:	optlist.o
		ar crv liboptlist.a optlist.o
		ranlib liboptlist.a

optlist.o:	optlist.c optlist.h
		$(CC) $(CFLAGS) $<

clean:
		$(DEL) *.o
		$(DEL) *.a
		$(DEL) sample$(EXE)
