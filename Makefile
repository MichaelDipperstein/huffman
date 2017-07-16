############################################################################
#
# Makefile for huffman encode/decode sample program
# arguements:
#	No argument			Build everything
#	DEBUG=1				Build with debugging output and symbols
#	clean				Delete all compiled/linked output
#
############################################################################

CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -ansi -pedantic -c
LDFLAGS = -o

# libraries
LIBS = -L. -Lbitfile -Lbitarray -Loptlist -lhuffman -lbitfile -lbitarray -loptlist

# Treat NT and non-NT windows the same
ifeq ($(OS),Windows_NT)
	OS = Windows
endif

ifeq ($(OS),Windows)
	EXE = .exe
	DEL = del
else	#assume Linux/Unix
	EXE =
	DEL = rm -f
endif

# Handle debug/no debug
ifneq ($(DEBUG), 1)
	CFLAGS += -O3 -DNDEBUG
else
	CFLAGS += -g
endif

all:		sample$(EXE)

sample$(EXE):	sample.o libhuffman.a bitfile/libbitfile.a\
				bitarray/libbitarray.a optlist/liboptlist.a
		$(LD) $^ $(LIBS) $(LDFLAGS) $@

sample.o:	sample.c huffman.h optlist/optlist.h
		$(CC) $(CFLAGS) $<

libhuffman.a:	huffman.o canonical.o huflocal.o
		ar crv libhuffman.a huffman.o canonical.o huflocal.o
		ranlib libhuffman.a

huffman.o:	huffman.c huflocal.h bitarray/bitarray.h bitfile/bitfile.h
		$(CC) $(CFLAGS) $<

canonical.o:	canonical.c huflocal.h bitarray/bitarray.h bitfile/bitfile.h
		$(CC) $(CFLAGS) $<

huflocal.o:	huflocal.c huflocal.h
		$(CC) $(CFLAGS) $<

bitfile/libbitfile.a:
		cd bitfile && $(MAKE) libbitfile.a

bitarray/libbitarray.a:
		cd bitarray && $(MAKE) libbitarray.a

optlist/liboptlist.a:
		cd optlist && $(MAKE) liboptlist.a

clean:
		$(DEL) *.o
		$(DEL) *.a
		$(DEL) sample$(EXE)
		cd optlist && $(MAKE) clean
		cd bitfile && $(MAKE) clean
		cd bitarray && $(MAKE) clean
