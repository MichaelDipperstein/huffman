/***************************************************************************
*                    256 bit Array Manipulation Header
*
*   File    : bitop256.h
*   Purpose : Header file for a library which provides for bit operations
*             for an array of 32 8 bit characters (256 bits).
*   Author  : Michael Dipperstein
*   Date    : November 20, 2003
*
****************************************************************************
*   UPDATES
*
*   Date        Change
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
*                               PROTOTYPES
***************************************************************************/

/* set/clear functions */
void SetAll256(unsigned char *bits);
void ClearAll256(unsigned char *bits);

void SetBit256(unsigned char *bits, unsigned char bit);
void ClearBit256(unsigned char *bits, unsigned char bit);

/* bit test function */
int TestBit256(const unsigned char *bits, unsigned char bit);

/* copy function */
void Copy256(unsigned char *dest, const unsigned char *src);

/* logical operations */
void And256(unsigned char *dest,
            const unsigned char *src1,
            const unsigned char *src2);

void Or256(unsigned char *dest,
           const unsigned char *src1,
           const unsigned char *src2);

void Xor256(unsigned char *dest,
            const unsigned char *src1,
            const unsigned char *src2);

void Not256(unsigned char *bits);

/* bit shift functions */
void LeftShift256(unsigned char *bits, int shifts);
void RightShift256(unsigned char *bits, int shifts);

/* increment/decrement (retun true for over/underflow) */
int Increment256(unsigned char *bits);
int Decrement256(unsigned char *bits);

int Add256(unsigned char *dest,
           const unsigned char *src1,
           const unsigned char *src2);

int Subtract256(unsigned char *dest,
                const unsigned char *src1,
                const unsigned char *src2);

/* comparison */
int Compare256(const unsigned char *bits1, const unsigned char *bits2);
