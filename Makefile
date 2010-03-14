# Makefile for huffman
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

huffman$(EXE):	huffman.o getopt.o bitop256.o
		$(LD) $^ $(LDFLAGS) $@

huffman.o:	huffman.c bitop256.h getopt.h
		$(CC) $(CFLAGS) $<

chuffman$(EXE):	chuffman.o getopt.o bitop256.o
		$(LD) $^ $(LDFLAGS) $@

chuffman.o:	chuffman.c bitop256.h getopt.h
		$(CC) $(CFLAGS) $<

getopt.o:	getopt.c getopt.h
		$(CC) $(CFLAGS) $<

bitop256.o:	bitop256.c bitop256.h
		$(CC) $(CFLAGS) $<

clean:
		$(DEL) *.o
		$(DEL) huffman$(EXE)
		$(DEL) chuffman$(EXE)
