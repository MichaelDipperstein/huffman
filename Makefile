############################################################################
# Makefile for bit array library and sample program
# arguements:
#	No argument			Build everything
#	DEBUG=1				Build with debugging output and symbols
#	clean				Delete all compiled/linked output
#
############################################################################

CC = gcc
LD = gcc
CFLAGS = -O2 -Wall -Wextra -pedantic -ansi -c
LDFLAGS = -O2 -o

# Treat NT and non-NT windows the same
ifeq ($(OS),Windows_NT)
	OS = Windows
endif

ifeq ($(OS),Windows)
	EXE = .exe
	DEL = del
else    #assume Linux/Unix
	EXE =
	DEL = rm -f
endif

# Handle debug/no debug
ifneq ($(DEBUG), 1)
	CFLAGS += -DNDEBUG
else
	CFLAGS += -g
endif

all:	sample$(EXE)

sample$(EXE):	sample.o bitarray.o
	$(LD) $^ $(LDFLAGS) $@

sample.o:	sample.c bitarray.h
	$(CC) $(CFLAGS) $<

bitarray.o:	bitarray.c bitarray.h
	$(CC) $(CFLAGS) $<

clean:
	$(DEL) *.o
	$(DEL) sample$(EXE)
