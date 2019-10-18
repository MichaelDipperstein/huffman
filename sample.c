/***************************************************************************
*                       Bit Array Library Usage Sample
*
*   File    : sample.c
*   Purpose : Demonstrates usage of bit array library.
*   Author  : Michael Dipperstein
*   Date    : January 30, 2004
*
****************************************************************************
*
* Sample: A bit array library sample usage program
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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "bitarray.h"

/***************************************************************************
*                                 MACROS
***************************************************************************/
#define NUM_BITS 128        /* size of bit array */

/* macro for error checking when NDEBUG isn't defined */
#ifdef  NDEBUG
#define EVAL_RESULT(R)  (void)(R)
#else
#define EVAL_RESULT(R)  if (-1 == (R)) {perror(NULL); exit(EXIT_FAILURE);};
#endif

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : ShowArray
*   Description: This is a wrapper for BitArrayDump that shows the array
*                array name as well as its value.
*   Parameters : name - name of array
*   Parameters : ba - pointer to bit array
*   Effects    : Writes array to stdout.
*   Returned   : None
***************************************************************************/
void ShowArray(char *name, bit_array_t *ba)
{
    printf("%s: ", name);
    BitArrayDump(ba, stdout);
    printf("\n");
}

/***************************************************************************
*   Function   : main
*   Description: This function demonstrates the usage of each of the bit
*                array functions.
*   Parameters : None
*   Effects    : Writes results of bit operations to stdout.
*   Returned   : EXIT_SUCCESS
***************************************************************************/
int main()
{
    bit_array_t *ba1, *ba2;
    int i;
    int result;

    ba1 = BitArrayCreate(NUM_BITS);

    printf("set all bits in ba1\n");
    BitArraySetAll(ba1);
    ShowArray("ba1", ba1);

    printf("\nclear all bits in ba1\n");
    BitArrayClearAll(ba1);
    ShowArray("ba1", ba1);

    printf("\nset 8 bits on each end of ba1 from the outside in\n");
    for (i = 0; i < 8; i++)
    {
        result = BitArraySetBit(ba1, i);
        EVAL_RESULT(result);
        result = BitArraySetBit(ba1, NUM_BITS - i - 1);
        EVAL_RESULT(result);
        ShowArray("ba1", ba1);
    }

    printf("\nduplicate ba1 with ba2\n");
    ba2 = BitArrayDuplicate(ba1);
    if (NULL == ba2)
    {
        EVAL_RESULT(-1);
    }
    ShowArray("ba2", ba2);

    printf("\nba2 = ~(ba2)\n");
    result = BitArrayNot(ba2, ba2);
    EVAL_RESULT(result);
    ShowArray("ba2", ba2);

    printf("\nba2 = ba2 | ba1\n");
    result = BitArrayOr(ba2, ba1, ba2);
    EVAL_RESULT(result);
    ShowArray("ba2", ba2);

    printf("\nba2 = ba2 ^ ba1\n");
    result = BitArrayXor(ba2, ba1, ba2);
    EVAL_RESULT(result);
    ShowArray("ba2", ba2);

    printf("\nba2 = ba2 & ba1\n");
    result = BitArrayAnd(ba2, ba1, ba2);
    EVAL_RESULT(result);
    ShowArray("ba2", ba2);

    printf("\ntesting some bits in ba1\n");
    for (; i > 6; i--)
    {
        if(BitArrayTestBit(ba1, i))
        {
            printf("ba1 bit %d is set.\n", i);
        }
        else
        {
            printf("ba1 bit %d is clear.\n", i);
        }

        if(BitArrayTestBit(ba1, NUM_BITS - i - 1))
        {
            printf("ba1 bit %d is set.\n", NUM_BITS - i - 1);
        }
        else
        {
            printf("ba1 bit %d is clear.\n", NUM_BITS - i - 1);
        }
    }

    printf("\nclear 8 bits on each end of ba1 from the outside in\n");
    for (i = 0; i < 8; i++)
    {
        result = BitArrayClearBit(ba1, i);
        EVAL_RESULT(result);
        result = BitArrayClearBit(ba1, NUM_BITS - i - 1);
        EVAL_RESULT(result);
        ShowArray("ba1", ba1);
    }

    printf("\nset all bits in ba1 and shift right by 20\n");
    BitArraySetAll(ba1);
    result = BitArrayShiftRight(ba1, 20);
    EVAL_RESULT(result);
    ShowArray("ba1", ba1);

    printf("\nshift ba1 left by 20\n");
    result = BitArrayShiftLeft(ba1, 20);
    EVAL_RESULT(result);
    ShowArray("ba1", ba1);

    printf("\nset all bits in ba1 and increment\n");
    BitArraySetAll(ba1);
    result = BitArrayIncrement(ba1);
    EVAL_RESULT(result);
    ShowArray("ba1", ba1);

    printf("\nincrement ba1\n");
    result = BitArrayIncrement(ba1);
    EVAL_RESULT(result);
    ShowArray("ba1", ba1);

    printf("\nincrement ba1\n");
    result = BitArrayIncrement(ba1);
    EVAL_RESULT(result);
    ShowArray("ba1", ba1);

    printf("\ndecrement ba1\n");
    result = BitArrayDecrement(ba1);
    EVAL_RESULT(result);
    ShowArray("ba1", ba1);

    printf("\ndecrement ba1\n");
    result = BitArrayDecrement(ba1);
    EVAL_RESULT(result);
    ShowArray("ba1", ba1);

    printf("\ndecrement ba1\n");
    result = BitArrayDecrement(ba1);
    EVAL_RESULT(result);
    ShowArray("ba1", ba1);

    printf("\ncompare ba1 with ba1\n");
    if (BitArrayCompare(ba1, ba1) == 0)
    {
        printf("ba1 == ba1\n");
    }
    else
    {
        printf("Comparison error.\n");
    }

    printf("\ncompare ba1 with ba2\n");
    if (BitArrayCompare(ba1, ba2) > 0)
    {
        printf("ba1 > ba2\n");
    }
    else
    {
        printf("Comparison error.\n");
    }

    printf("\ncompare ba2 with ba1\n");
    if (BitArrayCompare(ba2, ba1) < 0)
    {
        printf("ba2 < ba1\n");
    }
    else
    {
        printf("Comparison error.\n");
    }

    BitArrayDestroy(ba1);
    BitArrayDestroy(ba2);
    return EXIT_SUCCESS;
}
