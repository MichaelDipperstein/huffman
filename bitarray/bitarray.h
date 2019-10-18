/***************************************************************************
*                         Arrays of Arbitrary Bit Length
*
*   File    : bitarray.h
*   Purpose : Header file for library supporting the creation and
*             manipulation of arbitrary length arrays of bits.
*   Author  : Michael Dipperstein
*   Date    : January 30, 2004
*
****************************************************************************
*
* Bitarray: An ANSI C library for manipulating arbitrary length bit arrays
* Copyright (C) 2004, 2006-2007, 2014 by
*   Michael Dipperstein (mdipperstein@gmail.com)
*
* This file is part of the bit array library.
*
* The bit array library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* The bit array library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
* General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************/
#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
/* incomplete type to hide implementation */
struct bit_array_t;
typedef struct bit_array_t bit_array_t;

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/* create/destroy functions */
bit_array_t *BitArrayCreate(const unsigned int bits);
void BitArrayDestroy(bit_array_t *ba);

/* debug functions */
void BitArrayDump(const bit_array_t *const ba, FILE *outFile);

/* set/clear functions */
void BitArraySetAll(const bit_array_t *const ba);
void BitArrayClearAll(const bit_array_t *const ba);
int BitArraySetBit(const bit_array_t *const ba, const unsigned int bit);
int BitArrayClearBit(const bit_array_t *const ba, const unsigned int bit);

/* raw bit access */
void *BitArrayGetBits(const bit_array_t *const ba);

/* bit test function */
int BitArrayTestBit(const bit_array_t *const ba, const unsigned int bit);

/* copy functions */
int BitArrayCopy(const bit_array_t *const dest, const bit_array_t *const src);
bit_array_t *BitArrayDuplicate(const bit_array_t *const src);

/* logical operations */
int BitArrayAnd(const bit_array_t *const dest,
    const bit_array_t *const src1,
    const bit_array_t *const src2);

int BitArrayOr(const bit_array_t *const dest,
    const bit_array_t *const src1,
    const bit_array_t *const src2);

int BitArrayXor(const bit_array_t *const dest,
    const bit_array_t *const src1,
    const bit_array_t *const src2);

int BitArrayNot(const bit_array_t *const dest,
    const bit_array_t *const src);

/* bit shift functions */
int BitArrayShiftLeft(const bit_array_t *const ba, unsigned int shifts);
int BitArrayShiftRight(const bit_array_t *const ba, unsigned int shifts);

/* increment/decrement */
int BitArrayIncrement(const bit_array_t *const ba);
int BitArrayDecrement(const bit_array_t *const ba);

/* comparison */
int BitArrayCompare(const bit_array_t *ba1, const bit_array_t *ba2);

#endif  /* ndef BIT_ARRAY_H */
