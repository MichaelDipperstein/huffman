/***************************************************************************
*                        256 bit Array Manipulation
*
*   File    : bitop256.c
*   Purpose : Provides for bit operations for an array of 32 8 bit
*             characters (256 bits).
*   Author  : Michael Dipperstein
*   Date    : November 20, 2003
*
****************************************************************************
*   UPDATES
*
*   Date        Change
*   12/01/03    Modified Left and Right shifts to shift the remaining
*               1 to 7 bits all at once.
*   12/01/03    Return overflow/underflow for increment/decrement
*               1 to 7 bits all at once.
*   12/01/03    Added addition and subtraction operations
*
****************************************************************************
*
* Bitop256: An ANSI C library for manulating 8 x 32 bit arrays
* Copyright (C) 2003 by Michael Dipperstein (mdipper@cs.ucsb.edu)
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
***************************************************************************/

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <limits.h>
#include <string.h>

/***************************************************************************
*                                 MACROS
***************************************************************************/
/* use preprocessor to verify type length */
#if (UCHAR_MAX != 0xFF)
#error This program expects unsigned char to be 1 byte
#endif

#define NUM_BITS    256
#define NUM_BYTES   32      /* 32 x 8 = 256 */
#define LAST_BYTE   31      /* bytes are numbered 0 .. 31 */

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : SetAll256
*   Description: This function sets every bit to 1 in 256 contiguous bits
*                of RAM.  This is function uses memset, so it is crucial
*                that the machine implementation of arrays of unsigned char
*                be compatible with memset.
*   Parameters : bits - pointer to an array of 32 unsigned char (256 bits)
*   Effects    : The block of 256 bits starting at bits will be set to 1.
*   Returned   : NONE
****************************************************************************/
void SetAll256(unsigned char *bits)
{
    memset((void *)bits, 0xFF, NUM_BYTES);
}

/****************************************************************************
*   Function   : ClearAll256
*   Description: This function sets every bit to 0 in 256 contiguous bits
*                of RAM.  This is function uses memset, so it is crucial
*                that the machine implementation of arrays of unsigned char
*                be compatible with memset.
*   Parameters : bits - pointer to an array of 32 unsigned char (256 bits)
*   Effects    : The block of 256 bits starting at bits will be set to 0.
*   Returned   : NONE
****************************************************************************/
void ClearAll256(unsigned char *bits)
{
    memset((void *)bits, 0, NUM_BYTES);
}

/****************************************************************************
*   Function   : SetBit256
*   Description: This function sets a single bit to 1 in an array of 32
*                unsigned chars.
*   Parameters : bits - pointer to an array of 32 unsigned chars the msb
*                       of bit[0] is bit 0 of the 256 bits.  The lsb of
*                       of bit[31] is bit 255 of the 256 bits.
*                bit - the bit number to change.
*   Effects    : bit 'bit' in the array pointed to by 'bits' will be set to
*                1.
*   Returned   : NONE
****************************************************************************/
void SetBit256(unsigned char *bits, unsigned char bit)
{
    int byte;

    byte = bit / 8;         /* target byte */
    bit = bit % 8;          /* target bit in byte */

    bits[byte] |= (0x80 >> bit);
}

/****************************************************************************
*   Function   : ClearBit256
*   Description: This function sets a single bit to 0 in an array of 32
*                unsigned chars.
*   Parameters : bits - pointer to an array of 32 unsigned chars the msb
*                       of bit[0] is bit 0 of the 256 bits.  The lsb of
*                       of bit[31] is bit 255 of the 256 bits.
*                bit - the bit number to change.
*   Effects    : bit 'bit' in the array pointed to by 'bits' will be set to
*                0.
*   Returned   : NONE
****************************************************************************/
void ClearBit256(unsigned char *bits, unsigned char bit)
{
    int byte;
    unsigned char mask;

    byte = bit / 8;         /* target byte */
    bit = bit % 8;          /* target bit in byte */

    /* create a mask to zero out desired bit */
    mask = (0x80 >> bit);
    mask = ~mask;

    bits[byte] &= (mask);
}

/****************************************************************************
*   Function   : TestBit256
*   Description: This function returns the value of a single bit in an array
*                of 32 unsigned chars.
*   Parameters : bits - pointer to an array of 32 unsigned chars the msb
*                       of bit[0] is bit 0 of the 256 bits.  The lsb of
*                       of bit[31] is bit 255 of the 256 bits.
*                bit - the bit number to test.
*   Effects    : NONE
*   Returned   : 0 if the bit test is 0, 1 otherwise.
****************************************************************************/
int TestBit256(const unsigned char *bits, unsigned char bit)
{
    int byte;
    unsigned char mask;

    byte = bit / 8;         /* target byte */
    bit = bit % 8;          /* target bit in byte */

    mask = (0x80 >> bit);

    return((bits[byte] & mask) != 0);
}

/****************************************************************************
*   Function   : Copy256
*   Description: This function copies the contents of one 256 bit block of
*                RAM to another.  This is function uses memcpy, so it is
*                crucial that the machine implementation of arrays of
*                unsigned char be compatible with memcpy.
*   Parameters : dest - pointer to destination array of 32 unsigned char
*                       (256 bits).
*                src - pointer to source array of 32 unsigned char
*                      (256 bits).
*   Effects    : dest will contain a copy of src.  If dest and src overlap,
*                the effects are undefined.
*   Returned   : NONE
****************************************************************************/
void Copy256(unsigned char *dest, const unsigned char *src)
{
    memcpy((void *)dest, (void *)src, NUM_BYTES);
}

/****************************************************************************
*   Function   : And256
*   Description: This function performs a bitwise AND between two arrays of
*                32 unsigned characters, storing the results in a third
*                array of 32 unsigned characters.
*   Parameters : dest - pointer to destination array of 32 unsigned char
*                       (256 bits).  dest may be identical to src1 or src2.
*                src1 - pointer to first source array of 32 unsigned char
*                       (256 bits).
*                src1 - pointer to second source array of 32 unsigned char
*                       (256 bits).
*   Effects    : dest will contain the results of a bitwise AND of src1 and
*                src2.
*   Returned   : NONE
****************************************************************************/
void And256(unsigned char *dest,
            const unsigned char *src1,
            const unsigned char *src2)
{
    int byte;

    for(byte = 0; byte < NUM_BYTES; byte++)
    {
        dest[byte] = src1[byte] & src2[byte];
    }
}

/****************************************************************************
*   Function   : Or256
*   Description: This function performs a bitwise OR between two arrays of
*                32 unsigned characters, storing the results in a third
*                array of 32 unsigned characters.
*   Parameters : dest - pointer to destination array of 32 unsigned char
*                       (256 bits).  dest may be identical to src1 or src2.
*                src1 - pointer to first source array of 32 unsigned char
*                       (256 bits).
*                src1 - pointer to second source array of 32 unsigned char
*                       (256 bits).
*   Effects    : dest will contain the results of a bitwise OR of src1 and
*                src2.
*   Returned   : NONE
****************************************************************************/
void Or256(unsigned char *dest,
           const unsigned char *src1,
           const unsigned char *src2)
{
    int byte;

    for(byte = 0; byte < NUM_BYTES; byte++)
    {
        dest[byte] = src1[byte] | src2[byte];
    }
}

/****************************************************************************
*   Function   : Xor256
*   Description: This function performs a bitwise Xor between two arrays of
*                32 unsigned characters, storing the results in a third
*                array of 32 unsigned characters.
*   Parameters : dest - pointer to destination array of 32 unsigned char
*                       (256 bits).  dest may be identical to src1 or src2.
*                src1 - pointer to first source array of 32 unsigned char
*                       (256 bits).
*                src1 - pointer to second source array of 32 unsigned char
*                       (256 bits).
*   Effects    : dest will contain the results of a bitwise Xor of src1 and
*                src2.
*   Returned   : NONE
****************************************************************************/
void Xor256(unsigned char *dest,
            const unsigned char *src1,
            const unsigned char *src2)
{
    int byte;

    for(byte = 0; byte < NUM_BYTES; byte++)
    {
        dest[byte] = src1[byte] ^ src2[byte];
    }
}

/****************************************************************************
*   Function   : Not256
*   Description: This function performs a bitwise Not on an array of
*                32 unsigned characters.
*   Parameters : bits - pointer to array of 32 unsigned char (256 bits).
*                       This array is both the source an destination of
*                       the NOT operation.
*   Effects    : 'bits' will contain the results of a NOT operation performed
*                on itself.
*   Returned   : NONE
****************************************************************************/
void Not256(unsigned char *bits)
{
    int byte;

    for(byte = 0; byte < NUM_BYTES; byte++)
    {
        bits[byte] = ~(bits[byte]);
    }
}


/****************************************************************************
*   Function   : LeftShift256
*   Description: This function left shifts the bits in an array of 32 bytes
*                by the amount of positions specified.
*   Parameters : bits - pointer to array of 32 bytes (unsigned char)
*                       bits[0] is the leftmost byte.
*                shifts - number of bits to shift by.
*   Effects    : The data pointed to by 'bits' is shifted to the left.
*   Returned   : None
****************************************************************************/
void LeftShift256(unsigned char *bits, int shifts)
{
    int i;
    unsigned int overflow;
    int bytes = shifts / 8;     /* number of whole byte shifts */
    shifts = shifts % 8;        /* number of bit shifts remaining */

    /* first handle big jumps of bytes */
    if (bytes > 0)
    {
        for (i = 0; (i + bytes) < NUM_BYTES; i++)
        {
            bits[i] = bits[i + bytes];
        }

        /* now zero out new bytes on the right */
        for (i = NUM_BYTES; bytes > 0; bytes--)
        {
            bits[i - bytes] = 0;
        }
    }

    /* now handle the remaining shifts (no more than 7 bits) */
    if (shifts > 0)
    {
        bits[0] <<= shifts;

        for (i = 1; i < NUM_BYTES; i++)
        {
            overflow = bits[i];
            overflow <<= shifts;
            bits[i] = (unsigned char)overflow;

            /* handle shifts across byte bounds */
            if (overflow & 0xFF00)
            {
                bits[i - 1] |= (unsigned char)(overflow >> 8);
            }
        }
    }
}

/****************************************************************************
*   Function   : RightShift256
*   Description: This function right shifts the bits in an array of 32 bytes
*                by the amount of positions specified.
*   Parameters : bits - pointer to array of 32 bytes (unsigned char)
*                       bits[31] is the rightmost byte.
*                shifts - number of bits to shift by.
*   Effects    : The data pointed to by 'bits' is shifted to the right.
*   Returned   : None
****************************************************************************/
void RightShift256(unsigned char *bits, int shifts)
{
    int i;
    unsigned int overflow;
    int bytes = shifts / 8;     /* number of whole byte shifts */
    shifts = shifts % 8;        /* number of bit shifts remaining */

    /* first handle big jumps of bytes */
    if (bytes > 0)
    {
        for (i = LAST_BYTE; (i - bytes) >= 0; i--)
        {
            bits[i] = bits[i - bytes];
        }

        /* now zero out new bytes on the right */
        for (; bytes > 0; bytes--)
        {
            bits[bytes - 1] = 0;
        }
    }

    /* now handle the remaining shifts (no more than 7 bits) */
    if (shifts > 0)
    {
        bits[LAST_BYTE] >>= shifts;

        for (i = LAST_BYTE - 1; i >= 0; i--)
        {
            overflow = bits[i];
            overflow <<= (8 - shifts);
            bits[i] = (unsigned char)(overflow >> 8);

            /* handle shifts across byte bounds */
            if (overflow & 0xFF)
            {
                bits[i + 1] |= (unsigned char)overflow;
            }
        }
    }
}

/****************************************************************************
*   Function   : Increment256
*   Description: This function increments on an array of 32 unsigned
*                characters as if it was a single 256 bit unsigned value.
*   Parameters : bits - pointer to array of 32 unsigned char (256 bits).
*                       This array is both the source an destination of
*                       the increment operation.
*   Effects    : 'bits' will contain the results of a increment operation
*                performed on itself.
*   Returned   : 1 if overflow, otherwise 0
****************************************************************************/
int Increment256(unsigned char *bits)
{
    int i;

    for (i = LAST_BYTE; i >= 0; i--)
    {
        if (bits[i] != 0xFF)
        {
            bits[i] = bits[i] + 1;
            break;
        }
        else
        {
            /* need to carry to next byte */
            bits[i] = 0;

            if (i == 0)
            {
                /* we had an overflow */
                return 1;
            }
        }
    }

    return 0;
}

/****************************************************************************
*   Function   : Decrement256
*   Description: This function increments on an array of 32 unsigned
*                characters as if it was a single 256 bit unsigned value.
*   Parameters : bits - pointer to array of 32 unsigned char (256 bits).
*                       This array is both the source an destination of
*                       the increment operation.
*   Effects    : 'bits' will contain the results of a increment operation
*                performed on itself.
*   Returned   : 1 if underflow, otherwise 0
****************************************************************************/
int Decrement256(unsigned char *bits)
{
    int i;

    for (i = LAST_BYTE; i >= 0; i--)
    {
        if (bits[i] != 0x00)
        {
            bits[i] = bits[i] - 1;
            break;
        }
        else
        {
            /* need to borrow from the next byte */
            bits[i] = 0xFF;

            if (i == 0)
            {
                /* we had an underflow */
                return 1;
            }
        }
    }

    return 0;
}

/****************************************************************************
*   Function   : Add256
*   Description: This function performs addition on two arrays of 32
*                unsigned characters, storing the results in a third array
*                of 32 unsigned characters.
*   Parameters : dest - pointer to destination array of 32 unsigned char
*                       (256 bits).  dest may be identical to src1 or src2.
*                src1 - pointer to first source array of 32 unsigned char
*                       (256 bits).
*                src1 - pointer to second source array of 32 unsigned char
*                       (256 bits).
*   Effects    : dest will contain the sum of src1 and src2.
*   Returned   : 1 if overflow, otherwise 0
****************************************************************************/
int Add256(unsigned char *dest,
           const unsigned char *src1,
           const unsigned char *src2)
{
    int i;
    unsigned int tempReg;

    tempReg = 0;

    /* do bytewise addition with carry */
    for (i = LAST_BYTE; i >= 0; i--)
    {
        tempReg >>= 8;  /* shift carried bits from previous bytes */
        tempReg += (unsigned int)src1[i] + (unsigned int)src2[i];
        dest[i] = (unsigned char)tempReg & 0xFF;
    }

    /* did we carry on our last addition? */
    return ((tempReg >> 8) != 0);
}

/****************************************************************************
*   Function   : Subtract256
*   Description: This function performs subtraction on two arrays of 32
*                unsigned characters, storing the results in a third array
*                of 32 unsigned characters.
*   Parameters : dest - pointer to destination array of 32 unsigned char
*                       (256 bits).  dest may be identical to src1 or src2.
*                src1 - pointer to first source array of 32 unsigned char
*                       (256 bits).
*                src1 - pointer to second source array of 32 unsigned char
*                       (256 bits).
*   Effects    : dest will contain the difference between src1 and src2.
*   Returned   : 1 if underflow, otherwise 0
****************************************************************************/
int Subtract256(unsigned char *dest,
                const unsigned char *src1,
                const unsigned char *src2)
{
    int i;
    unsigned int tempReg;

    /* start with something to borrow from */
    tempReg = 0x100;

    /* do bytewise subtraction with carry */
    for (i = LAST_BYTE; i >= 0; i--)
    {
        tempReg &= 0x100;   /* clear out old value */
        if (tempReg == 0)
        {
            /***************************************************************
            * A borrow occurred, set to 0xFF (-1).  When the current byte
            * is added to this, it will decrement it by one and put back
            * a borrow bit.
            ***************************************************************/
            tempReg = 0xFF;
        }

        tempReg += (unsigned int)src1[i] - (unsigned int)src2[i];
        dest[i] = (unsigned char)tempReg & 0xFF;
    }

    /* did we borrow on our last subtraction? */
    return ((tempReg >> 8) == 0);
}

/****************************************************************************
*   Function   : Compare256
*   Description: This function compares two arrays of 32 unsigned
*                characters.
*   Parameters : bits1 - pointer to array of 32 unsigned char (256 bits).
*                bits2 - pointer to array of 32 unsigned char (256 bits).
*   Effects    : 'bits' will contain the results of a increment operation
*                performed on itself.
*   Returned   : < 0 if 'bits1' < 'bits2'
*                0 if 'bits1' == 'bits2'
*                > 0 if 'bits1' > 'bits2'
****************************************************************************/
int Compare256(const unsigned char *bits1, const unsigned char *bits2)
{
    int i;

    for(i = 0; i < NUM_BYTES; i++)
    {
        if (bits1[i] != bits2[i])
        {
            return(bits1[i] - bits2[i]);
        }
    }

    return 0;
}
