############################################################################
# Makefile for huffman encode/decode programs
#
#   $Id: Makefile,v 1.6 2004/06/15 13:34:52 michael Exp $
#   $Log: Makefile,v $
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
CFLAGS = -O2 -Wall -ansi -c
LDFLAGS = -O2 -o

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

sample$(EXE):	sample.o huffman.o chuffman.o getopt.o bitfile.o bitarray.o
		$(LD) $^ $(LDFLAGS) $@

sample.o:	sample.c huffman.h getopt.h
		$(CC) $(CFLAGS) $<

huffman.o:	huffman.c bitarray.h bitfile.h
		$(CC) $(CFLAGS) $<

chuffman.o:	chuffman.c bitarray.h bitfile.h
		$(CC) $(CFLAGS) $<

getopt.o:	getopt.c getopt.h
		$(CC) $(CFLAGS) $<

bitarray.o:	bitarray.c bitarray.h
		$(CC) $(CFLAGS) $<

bitfile.o:	bitfile.c bitfile.h
		$(CC) $(CFLAGS) $<

clean:
		$(DEL) *.o
		$(DEL) sample$(EXE)
