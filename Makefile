############################################################################
# Makefile for huffman encode/decode programs
#
#   $Id: Makefile,v 1.4 2004/02/04 15:31:14 michael Exp $
#   $Log: Makefile,v $
#   Revision 1.4  2004/02/04 15:31:14  michael
#   replace bitop with bitarry library.
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

all:		huffman$(EXE) chuffman$(EXE)

huffman$(EXE):	huffman.o getopt.o bitarray.o bitfile.o
		$(LD) $^ $(LDFLAGS) $@

huffman.o:	huffman.c bitarray.h bitfile.h getopt.h
		$(CC) $(CFLAGS) $<

chuffman$(EXE):	chuffman.o getopt.o bitarray.o bitfile.o
		$(LD) $^ $(LDFLAGS) $@

chuffman.o:	chuffman.c bitarray.h bitfile.h getopt.h
		$(CC) $(CFLAGS) $<

getopt.o:	getopt.c getopt.h
		$(CC) $(CFLAGS) $<

bitarray.o:	bitarray.c bitarray.h
		$(CC) $(CFLAGS) $<

bitfile.o:	bitfile.c bitfile.h
		$(CC) $(CFLAGS) $<

clean:
		$(DEL) *.o
		$(DEL) huffman$(EXE)
		$(DEL) chuffman$(EXE)
