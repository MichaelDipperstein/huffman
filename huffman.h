/***************************************************************************
*                        Huffman Library Header File
*
*   File    : huffman.h
*   Purpose : Provide header file for programs linking to Huffman library
*             functions.
*   Author  : Michael Dipperstein
*   Date    : February 25, 2004
*
****************************************************************************
*   UPDATES
*
*   $Id: huffman.h,v 1.3 2007/09/20 03:30:06 michael Exp $
*   $Log: huffman.h,v $
*   Revision 1.3  2007/09/20 03:30:06  michael
*   Changes required for LGPL v3.
*
*   Revision 1.2  2004/06/15 13:37:59  michael
*   Incorporate changes in chuffman.c.
*
*   Revision 1.1  2004/02/26 04:58:22  michael
*   Initial revision.  Headers for encode/decode functions.
*
*
****************************************************************************
*
* Huffman: An ANSI C Huffman Encoding/Decoding Routine
* Copyright (C) 2004, 2007 by
* Michael Dipperstein (mdipper@alumni.engr.ucsb.edu)
*
* This file is part of the Huffman library.
*
* The Huffman library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* The Huffman library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
* General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************/

#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/* traditional codes */
int HuffmanShowTree(char *inFile, char *outFile);       /* dump codes */
int HuffmanEncodeFile(char *inFile, char *outFile);     /* encode file */
int HuffmanDecodeFile(char *inFile, char *outFile);     /* decode file */

/* canonical code */
int CHuffmanShowTree(char *inFile, char *outFile);       /* dump codes */
int CHuffmanEncodeFile(char *inFile, char *outFile);     /* encode file */
int CHuffmanDecodeFile(char *inFile, char *outFile);     /* decode file */

#endif /* _HUFFMAN_H_ */
