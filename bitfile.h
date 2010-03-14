/***************************************************************************
*                            Bit Stream File Header
*
*   File    : bitfile.h
*   Purpose : Providea definitions and prototypes for a simple library of
*             I/O functions for files that contain data in sizes that aren't
*             integral bytes.
*             An attempt was made to make the functions in this library
*             analogous to functions provided to manipulate byte streams.
*             The functions contained in this library were created with
*             compression algorithms in mind, but may be suited to other
*             applications.
*   Author  : Michael Dipperstein
*   Date    : January 9, 2004
*
****************************************************************************
*   UPDATES
*
*   $Id: bitfile.h,v 1.2 2004/01/13 15:43:14 michael Exp $
*   $Log: bitfile.h,v $
*   Revision 1.2  2004/01/13 15:43:14  michael
*   Beautify header
*
*   Revision 1.1  2004/01/13 05:57:49  michael
*   Initial release
*
*
****************************************************************************
*
* Bitfile: Bit stream File I/O Routines
* Copyright (C) 2004 by Michael Dipperstein (mdipper@cs.ucsb.edu)
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

#ifndef _BITFILE_H_
#define _BITFILE_H_

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
typedef enum
{
    BF_READ = 0,
    BF_WRITE = 1,
    BF_APPEND= 2,
    BF_NO_MODE
} BF_MODES;

typedef struct
{
    FILE *fp;                   /* file pointer used by stdio functions */
    unsigned char bitBuffer;    /* bits waiting to be read/written */
    unsigned char bitCount;     /* number of bits in bitBuffer */
    BF_MODES mode;              /* open for read, write, or append */
} bit_file_t;

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/* open/close file */
bit_file_t *BitFileOpen(const char *fileName, const BF_MODES mode);
bit_file_t *MakeBitFile(FILE *stream, const BF_MODES mode);
int BitFileClose(bit_file_t *stream);

/* get/put character */
int BitFileGetChar(bit_file_t *stream);
int BitFilePutChar(const int c, bit_file_t *stream);

/* get/put single bit */
int BitFileGetBit(bit_file_t *stream);
int BitFilePutBit(const int c, bit_file_t *stream);

/* get/put number of bits */
int BitFileGetBits(bit_file_t *stream, void *bits, const unsigned int count);
int BitFilePutBits(bit_file_t *stream, void *bits, const unsigned int count);

#endif /* _BITFILE_H_ */