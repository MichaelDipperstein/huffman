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
*   $Id: huffman.h,v 1.1 2004/02/26 04:58:22 michael Exp $
*   $Log: huffman.h,v $
*   Revision 1.1  2004/02/26 04:58:22  michael
*   Initial revision.  Headers for encode/decode functions.
*
*
****************************************************************************
*
* Huffman: An ANSI C Huffman Encoding/Decoding Routine
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

#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
int HuffmanShowTree(char *inFile, char *outFile);       /* dump codes */
int HuffmanEncodeFile(char *inFile, char *outFile);     /* encode file */
int HuffmanDecodeFile(char *inFile, char *outFile);     /* decode file */

#endif /* _HUFFMAN_H_ */
