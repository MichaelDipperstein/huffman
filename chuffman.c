/***************************************************************************
*                  Canonical Huffman Encoding and Decoding
*
*   File    : chuffman.c
*   Purpose : Use canonical huffman coding to compress/decompress files
*   Author  : Michael Dipperstein
*   Date    : November 20, 2002
*
****************************************************************************
*   UPDATES
*
*   Date        Change
*   10/21/03    Fixed one symbol file bug discovered by David A. Scott
*   10/21/03    Dynamically allocate storage for canonical list.
*   11/20/03    Correcly handle codes up to 256 bits (the theoretical
*               max).  With symbol counts being limited to 32 bits, 31
*               bits will be the maximum code length.
*
*   $Id: chuffman.c,v 1.9 2007/09/20 03:30:06 michael Exp $
*   $Log: chuffman.c,v $
*   Revision 1.9  2007/09/20 03:30:06  michael
*   Changes required for LGPL v3.
*
*   Revision 1.8  2005/05/23 03:18:04  michael
*   Moved internal routines and definitions common to both canonical and
*   traditional Huffman coding so that they are only declared once.
*
*   Revision 1.7  2004/06/15 13:37:10  michael
*   Change function names and make static functions to allow linkage with huffman.
*
*   Revision 1.6  2004/02/26 04:55:36  michael
*   Remove main(), allowing code to be generate linkable object file.
*
*   Revision 1.4  2004/01/13 15:49:41  michael
*   Beautify header
*
*   Revision 1.3  2004/01/13 05:55:02  michael
*   Use bit stream library.
*
*   Revision 1.2  2004/01/05 05:03:18  michael
*   Use encoded EOF instead of counting characters.
*
*
*
****************************************************************************
*
* Huffman: An ANSI C Canonical Huffman Encoding/Decoding Routine
* Copyright (C) 2002-2005, 2007 by
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

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "huflocal.h"
#include "bitarray.h"
#include "bitfile.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
typedef struct canonical_list_t
{
    short value;        /* characacter represented */
    byte_t codeLen;     /* number of bits used in code (1 - 255) */
    bit_array_t *code;  /* code used for symbol (left justified) */
} canonical_list_t;

/***************************************************************************
*                                CONSTANTS
***************************************************************************/

/***************************************************************************
*                                 MACROS
***************************************************************************/

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
canonical_list_t canonicalList[NUM_CHARS];      /* list of canonical codes */

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
/* creating canonical codes */
static int BuildCanonicalCode(huffman_node_t *ht, canonical_list_t *cl);
static int AssignCanonicalCodes(canonical_list_t *cl);
static int CompareByCodeLen(const void *item1, const void *item2);

/* reading/writing code to file */
static void WriteHeader(canonical_list_t *cl, bit_file_t *bfp);
static int ReadHeader(canonical_list_t *cl,  bit_file_t *bfp);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : CHuffmanEncodeFile
*   Description: This routine genrates a huffman tree optimized for a file
*                and writes out an encoded version of that file.
*   Parameters : inFile - Name of file to encode
*                outFile - Name of file to write a tree to
*   Effects    : File is Huffman encoded
*   Returned   : TRUE for success, otherwise FALSE.
****************************************************************************/
int CHuffmanEncodeFile(char *inFile, char *outFile)
{
    FILE *fpIn;
    bit_file_t *bfpOut;
    huffman_node_t *huffmanTree;        /* root of huffman tree */
    int c;


    /* open binary input file and bitfile output file */
    if ((fpIn = fopen(inFile, "rb")) == NULL)
    {
        perror(inFile);
        return FALSE;
    }

    if (outFile == NULL)
    {
        bfpOut = MakeBitFile(stdout, BF_WRITE);
    }
    else
    {
        if ((bfpOut = BitFileOpen(outFile, BF_WRITE)) == NULL)
        {
            perror(outFile);
            fclose(fpIn);
            return FALSE;
        }
    }

    /* build tree */
    if ((huffmanTree = GenerateTreeFromFile(fpIn)) == NULL)
    {
        fclose(fpIn);
        BitFileClose(bfpOut);
        return FALSE;
    }

    /* use tree to generate a canonical code */
    if (!BuildCanonicalCode(huffmanTree, canonicalList))
    {
        fclose(fpIn);
        BitFileClose(bfpOut);
        FreeHuffmanTree(huffmanTree);     /* free allocated memory */
        return FALSE;
    }

    /* write out encoded file */

    /* write header for rebuilding of code */
    WriteHeader(canonicalList, bfpOut);

    /* read characters from file and write them to encoded file */
    rewind(fpIn);               /* start another pass on the input file */

    while((c = fgetc(fpIn)) != EOF)
    {
        /* write encoded symbols */
        BitFilePutBits(bfpOut,
            BitArrayGetBits(canonicalList[c].code),
            canonicalList[c].codeLen);
    }

    /* now write EOF */
    BitFilePutBits(bfpOut,
        BitArrayGetBits(canonicalList[EOF_CHAR].code),
        canonicalList[EOF_CHAR].codeLen);

    /* clean up */
    fclose(fpIn);
    BitFileClose(bfpOut);

    return TRUE;
}

/****************************************************************************
*   Function   : CHuffmanDecodeFile
*   Description: This routine reads a Huffman coded file and writes out a
*                decoded version of that file.
*   Parameters : inFile - Name of file to decode
*                outFile - Name of file to write a tree to
*   Effects    : Huffman encoded file is decoded
*   Returned   : TRUE for success, otherwise FALSE.
****************************************************************************/
int CHuffmanDecodeFile(char *inFile, char *outFile)
{
    bit_file_t *bfpIn;
    FILE *fpOut;
    bit_array_t *code;
    byte_t length;
    char decodedEOF;
    int i, newBit;
    int lenIndex[NUM_CHARS];

    /* open binary output file and bitfile input file */
    if ((bfpIn = BitFileOpen(inFile, BF_READ)) == NULL)
    {
        perror(inFile);
        return FALSE;
    }

    if (outFile == NULL)
    {
        fpOut = stdout;
    }
    else
    {
        if ((fpOut = fopen(outFile, "wb")) == NULL)
        {
            BitFileClose(bfpIn);
            perror(outFile);
            return FALSE;
        }
    }

    /* allocate canonical code list */
    code = BitArrayCreate(256);
    if (code == NULL)
    {
        perror("Bit array allocation");
        BitFileClose(bfpIn);
        fclose(fpOut);
        return FALSE;
    }

    /* initialize canonical list */
    for (i = 0; i < NUM_CHARS; i++)
    {
        canonicalList[i].codeLen = 0;
        canonicalList[i].code = NULL;
    }

    /* populate list with code length from file header */
    if (!ReadHeader(canonicalList, bfpIn))
    {
        BitArrayDestroy(code);
        BitFileClose(bfpIn);
        fclose(fpOut);
        return FALSE;
    }

    /* sort the header by code length */
    qsort(canonicalList, NUM_CHARS, sizeof(canonical_list_t),
        CompareByCodeLen);

    /* assign the codes using same rule as encode */
    if (AssignCanonicalCodes(canonicalList) == 0)
    {
        /* failed to assign codes */
        BitFileClose(bfpIn);
        fclose(fpOut);

        for (i = 0; i < NUM_CHARS; i++)
        {
            if(canonicalList[i].code != NULL)
            {
                BitArrayDestroy(canonicalList[i].code);
            }
        }

        return FALSE;
    }

    /* now we have a huffman code that matches the code used on the encode */

    /* create an index of first code at each possible length */
    for (i = 0; i < NUM_CHARS; i++)
    {
        lenIndex[i] = NUM_CHARS;
    }

    for (i = 0; i < NUM_CHARS; i++)
    {
        if (lenIndex[canonicalList[i].codeLen] > i)
        {
            /* first occurance of this code length */
            lenIndex[canonicalList[i].codeLen] = i;
        }
    }

    /* decode input file */
    length = 0;
    BitArrayClearAll(code);
    decodedEOF = FALSE;

    while(((newBit = BitFileGetBit(bfpIn)) != EOF) && (!decodedEOF))
    {
        if (newBit != 0)
        {
            BitArraySetBit(code, length);
        }

        length++;

        if (lenIndex[length] != NUM_CHARS)
        {
            /* there are code of this length */
            for(i = lenIndex[length];
                (i < NUM_CHARS) && (canonicalList[i].codeLen == length);
                i++)
            {
                if ((BitArrayCompare(canonicalList[i].code, code) == 0) &&
                    (canonicalList[i].codeLen == length))
                {
                    /* we just read a symbol output decoded value */
                    if (canonicalList[i].value != EOF_CHAR)
                    {
                        fputc(canonicalList[i].value, fpOut);
                    }
                    else
                    {
                        decodedEOF = TRUE;
                    }
                    BitArrayClearAll(code);
                    length = 0;

                    break;
                }
            }
        }
    }

    /* close all files */
    BitFileClose(bfpIn);
    fclose(fpOut);

    return TRUE;
}

/****************************************************************************
*   Function   : CHuffmanShowTree
*   Description: This routine genrates a huffman tree optimized for a file
*                and writes out an ASCII representation of the code
*                represented by the tree.
*   Parameters : inFile - Name of file to create tree for
*                outFile - Name of file to write a tree to
*   Effects    : Huffman tree is written out to a file
*   Returned   : TRUE for success, otherwise FALSE.
****************************************************************************/
int CHuffmanShowTree(char *inFile, char *outFile)
{
    FILE *fpIn, *fpOut;
    huffman_node_t *huffmanTree;        /* root of huffman tree */
    int i, length;

    /* open binary input and output files */
    if ((fpIn = fopen(inFile, "rb")) == NULL)
    {
        perror(inFile);
        return FALSE;
    }

    if (outFile == NULL)
    {
        fpOut = stdout;
    }
    else
    {
        if ((fpOut = fopen(outFile, "w")) == NULL)
        {
            perror(outFile);
            fclose(fpIn);
            return FALSE;
        }
    }

    /* build tree */
    if ((huffmanTree = GenerateTreeFromFile(fpIn)) == NULL)
    {
        fclose(fpIn);
        fclose(fpOut);
        return FALSE;
    }

    /* use tree to generate a canonical code */
    if (!BuildCanonicalCode(huffmanTree, canonicalList))
    {
        fclose(fpIn);
        fclose(fpOut);
        FreeHuffmanTree(huffmanTree);     /* free allocated memory */
        return FALSE;
    }

    FreeHuffmanTree(huffmanTree);     /* free allocated memory */

    /* write out canonical code */
    /* print heading to make things look pretty (int is 10 char max) */
    fprintf(fpOut, "Char  CodeLen  Encoding\n");
    fprintf(fpOut, "----- -------- ----------------\n");

    for(i = 0; i < NUM_CHARS; i++)
    {
        if(canonicalList[i].codeLen > 0)
        {
            if (canonicalList[i].value != EOF_CHAR)
            {
                fprintf(fpOut,
                        "0x%02X  %02d       ",
                        canonicalList[i].value, canonicalList[i].codeLen);
            }
            else
            {
                fprintf(fpOut,
                        "EOF   %02d       ", canonicalList[i].codeLen);
            }

            /* now write out the code bits */
            for(length = 0; length < canonicalList[i].codeLen; length++)
            {
                if (BitArrayTestBit(canonicalList[i].code, length))
                {
                    fputc('1', fpOut);
                }
                else
                {
                    fputc('0', fpOut);
                }
            }

            fputc('\n', fpOut);
        }
    }

    /* clean up */
    fclose(fpIn);
    fclose(fpOut);

    return TRUE;
}

/****************************************************************************
*   Function   : CompareByCodeLen
*   Description: Compare function to be used by qsort for sorting canonical
*                list items by code length.  In the event of equal lengths,
*                the symbol value will be used.
*   Parameters : item1 - pointer canonical list item
*                item2 - pointer canonical list item
*   Effects    : None
*   Returned   : 1 if item1 > item2
*                -1 if item1 < item 2
*                0 if something went wrong (means item1 == item2)
****************************************************************************/
static int CompareByCodeLen(const void *item1, const void *item2)
{
    if (((canonical_list_t *)item1)->codeLen >
        ((canonical_list_t *)item2)->codeLen)
    {
        /* item1 > item2 */
        return 1;
    }
    else if (((canonical_list_t *)item1)->codeLen <
        ((canonical_list_t *)item2)->codeLen)
    {
        /* item1 < item2 */
        return -1;
    }
    else
    {
        /* both have equal code lengths break the tie using value */
        if (((canonical_list_t *)item1)->value >
            ((canonical_list_t *)item2)->value)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }

    return 0;   /* we should never get here */
}

/****************************************************************************
*   Function   : CompareBySymbolValue
*   Description: Compare function to be used by qsort for sorting canonical
*                list items by symbol value.
*   Parameters : item1 - pointer canonical list item
*                item2 - pointer canonical list item
*   Effects    : None
*   Returned   : 1 if item1 > item2
*                -1 if item1 < item 2
****************************************************************************/
static int CompareBySymbolValue(const void *item1, const void *item2)
{
    if (((canonical_list_t *)item1)->value >
        ((canonical_list_t *)item2)->value)
    {
        /* item1 > item2 */
        return 1;
    }

    /* it must be the case that item1 < item2 */
    return -1;
}

/****************************************************************************
*   Function   : BuildCanonicalCode
*   Description: This function builds a canonical Huffman code from a
*                Huffman tree.
*   Parameters : ht - pointer to root of tree
*                cl - pointer to canonical list
*   Effects    : cl is filled with the canonical codes sorted by the value
*                of the charcter to be encode.
*   Returned   : TRUE for success, FALSE for failure
****************************************************************************/
static int BuildCanonicalCode(huffman_node_t *ht, canonical_list_t *cl)
{
    int i;
    byte_t depth = 0;

    /* initialize list */
    for(i = 0; i < NUM_CHARS; i++)
    {
        cl[i].value = i;
        cl[i].codeLen = 0;
        cl[i].code = NULL;
    }

    /* fill list with code lengths (depth) from tree */
    for(;;)
    {
        /* follow this branch all the way left */
        while (ht->left != NULL)
        {
            ht = ht->left;
            depth++;
        }

        if (ht->value != COMPOSITE_NODE)
        {
            /* handle one symbol trees */
            if (depth == 0)
            {
                depth++;
            }

            /* enter results in list */
            cl[ht->value].codeLen = depth;
        }

        while (ht->parent != NULL)
        {
            if (ht != ht->parent->right)
            {
                /* try the parent's right */
                ht = ht->parent->right;
                break;
            }
            else
            {
                /* parent's right tried, go up one level yet */
                depth--;
                ht = ht->parent;
            }
        }

        if (ht->parent == NULL)
        {
            /* we're at the top with nowhere to go */
            break;
        }
    }

    /* sort by code length */
    qsort(cl, NUM_CHARS, sizeof(canonical_list_t), CompareByCodeLen);

    if (AssignCanonicalCodes(cl))
    {
        /* re-sort list in lexical order for use by encode algorithm */
        qsort(cl, NUM_CHARS, sizeof(canonical_list_t), CompareBySymbolValue);
        return TRUE;    /* success */
    }

    perror("Code assignment failed");
    return FALSE;       /* assignment failed */
}

/****************************************************************************
*   Function   : AssignCanonicalCode
*   Description: This function accepts a list of symbols sorted by their
*                code lengths, and assigns a canonical Huffman code to each
*                symbol.
*   Parameters : cl - sorted list of symbols to have code values assigned
*   Effects    : cl stores a list of canonical codes sorted by the length
*                of the code used to encode the symbol.
*   Returned   : TRUE for success, FALSE for failure
****************************************************************************/
static int AssignCanonicalCodes(canonical_list_t *cl)
{
    int i;
    byte_t length;
    bit_array_t *code;

    /* assign the new codes */
    code = BitArrayCreate(256);
    BitArrayClearAll(code);

    length = cl[(NUM_CHARS - 1)].codeLen;

    for(i = (NUM_CHARS - 1); i >= 0; i--)
    {
        /* bail if we hit a zero len code */
        if (cl[i].codeLen == 0)
        {
            break;
        }

        /* adjust code if this length is shorter than the previous */
        if (cl[i].codeLen < length)
        {
            BitArrayShiftRight(code, (length - cl[i].codeLen));
            length = cl[i].codeLen;
        }

        /* assign left justified code */
        if ((cl[i].code = BitArrayDuplicate(code)) == NULL)
        {
            perror("Duplicating code");
            BitArrayDestroy(code);
            return FALSE;
        }

        BitArrayShiftLeft(cl[i].code, 256 - length);

        BitArrayIncrement(code);
    }

    BitArrayDestroy(code);
    return TRUE;
}

/****************************************************************************
*   Function   : WriteHeader
*   Description: This function writes the code size for each symbol and the
*                total number of characters in the original file to the
*                specified output file.  If the same algorithm that produced
*                produced the original canonical code is used with these code
*                lengths, an exact copy of the code will be produced.
*   Parameters : cl - pointer to list of canonical Huffman codes
*                bfp - pointer to open binary file to write to.
*   Effects    : Symbol code lengths and symbol count are written to the
*                output file.
*   Returned   : None
****************************************************************************/
static void WriteHeader(canonical_list_t *cl, bit_file_t *bfp)
{
    int i;

    /* write out code size for each symbol */
    for (i = 0; i < NUM_CHARS; i++)
    {
        BitFilePutChar(cl[i].codeLen, bfp);
    }
}

/****************************************************************************
*   Function   : ReadHeader
*   Description: This function reads the header information stored by
*                WriteHeader.  If the same algorithm that produced the
*                original tree is used with these counts, an exact copy of
*                the tree will be produced.
*   Parameters : cl - pointer to list of canonical Huffman codes
*                bfp - file to read from
*   Effects    : Code lengths and symbols are read into the canonical list.
*                Total number of symbols encoded is store in totalCount
*   Returned   : TRUE on success, otherwise FALSE.
****************************************************************************/
static int ReadHeader(canonical_list_t *cl, bit_file_t *bfp)
{
    int c;
    int i;

    /* read the code length */
    for (i = 0; i < NUM_CHARS; i++)
    {
        c = BitFileGetChar(bfp);

        if (c != EOF)
        {
            cl[i].value = i;
            cl[i].codeLen = (byte_t)c;
        }
        else
        {
            fprintf(stderr, "error: malformed file header.\n");
            return FALSE;
        }
    }

    return TRUE;
}
