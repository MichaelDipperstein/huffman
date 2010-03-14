/***************************************************************************
*                       Huffman Encoding and Decoding
*
*   File    : huffman.c
*   Purpose : Use huffman coding to compress/decompress files
*   Author  : Michael Dipperstein
*   Date    : November 20, 2002
*
****************************************************************************
*   UPDATES
*
*   Date        Change
*   10/21/03    Fixed one symbol file bug discovered by David A. Scott
*   10/22/03    Corrected fix above to handle decodes of file containing
*               multiples of just one symbol.
*   11/20/03    Correcly handle codes up to 256 bits (the theoretical
*               max).  With symbol counts being limited to 32 bits, 31
*               bits will be the maximum code length.
*
*   $Id: huffman.c,v 1.9 2007/09/20 03:30:06 michael Exp $
*   $Log: huffman.c,v $
*   Revision 1.9  2007/09/20 03:30:06  michael
*   Changes required for LGPL v3.
*
*   Revision 1.8  2005/05/23 03:18:04  michael
*   Moved internal routines and definitions common to both canonical and
*   traditional Huffman coding so that they are only declared once.
*
*   Revision 1.7  2004/06/15 13:37:29  michael
*   Change function names and make static functions to allow linkage with chuffman.
*
*   Revision 1.6  2004/02/26 04:55:04  michael
*   Remove main(), allowing code to be generate linkable object file.
*
*   Revision 1.4  2004/01/13 15:49:45  michael
*   Beautify header
*
*   Revision 1.3  2004/01/13 05:45:17  michael
*   Use bit stream library.
*
*   Revision 1.2  2004/01/05 04:04:58  michael
*   Use encoded EOF instead of counting characters.
*
****************************************************************************
*
* Huffman: An ANSI C Huffman Encoding/Decoding Routine
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
typedef struct code_list_t
{
    byte_t codeLen;     /* number of bits used in code (1 - 255) */
    bit_array_t *code;  /* code used for symbol (left justified) */
} code_list_t;

/***************************************************************************
*                                CONSTANTS
***************************************************************************/

/***************************************************************************
*                                 MACROS
***************************************************************************/

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
/* creates look up table from tree */
static int MakeCodeList(huffman_node_t *ht, code_list_t *codeList);

/* reading/writing tree to file */
static void WriteHeader(huffman_node_t *ht, bit_file_t *bfp);
static int ReadHeader(huffman_node_t **ht, bit_file_t *bfp);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : HuffmanEncodeFile
*   Description: This routine genrates a huffman tree optimized for a file
*                and writes out an encoded version of that file.
*   Parameters : inFile - Name of file to encode
*                outFile - Name of file to write a tree to
*   Effects    : File is Huffman encoded
*   Returned   : TRUE for success, otherwise FALSE.
****************************************************************************/
int HuffmanEncodeFile(char *inFile, char *outFile)
{
    huffman_node_t *huffmanTree;        /* root of huffman tree */
    code_list_t codeList[NUM_CHARS];    /* table for quick encode */
    FILE *fpIn;
    bit_file_t *bfpOut;
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
        return FALSE;
    }

    /* build a list of codes for each symbol */

    /* initialize code list */
    for (c = 0; c < NUM_CHARS; c++)
    {
        codeList[c].code = NULL;
        codeList[c].codeLen = 0;
    }

    if (!MakeCodeList(huffmanTree, codeList))
    {
        return FALSE;
    }

    /* write out encoded file */

    /* write header for rebuilding of tree */
    WriteHeader(huffmanTree, bfpOut);

    /* read characters from file and write them to encoded file */
    rewind(fpIn);               /* start another pass on the input file */

    while((c = fgetc(fpIn)) != EOF)
    {
        BitFilePutBits(bfpOut,
            BitArrayGetBits(codeList[c].code),
            codeList[c].codeLen);
    }

    /* now write EOF */
    BitFilePutBits(bfpOut,
        BitArrayGetBits(codeList[EOF_CHAR].code),
        codeList[EOF_CHAR].codeLen);

    /* free the code list */
    for (c = 0; c < NUM_CHARS; c++)
    {
        if (codeList[c].code != NULL)
        {
            BitArrayDestroy(codeList[c].code);
        }
    }


    /* clean up */
    fclose(fpIn);
    BitFileClose(bfpOut);
    FreeHuffmanTree(huffmanTree);     /* free allocated memory */

    return TRUE;
}

/****************************************************************************
*   Function   : HuffmanDecodeFile
*   Description: This routine reads a Huffman coded file and writes out a
*                decoded version of that file.
*   Parameters : inFile - Name of file to decode
*                outFile - Name of file to write a tree to
*   Effects    : Huffman encoded file is decoded
*   Returned   : TRUE for success, otherwise FALSE.
****************************************************************************/
int HuffmanDecodeFile(char *inFile, char *outFile)
{
    huffman_node_t *huffmanTree, *currentNode;
    int i, c;
    bit_file_t *bfpIn;
    FILE *fpOut;

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

    /* allocate array of leaves for all possible characters */
    for (i = 0; i < NUM_CHARS; i++)
    {
        if ((huffmanArray[i] = AllocHuffmanNode(i)) == NULL)
        {
            /* allocation failed clear existing allocations */
            for (i--; i >= 0; i--)
            {
                free(huffmanArray[i]);
            }

            BitFileClose(bfpIn);
            fclose(fpOut);
            return FALSE;
        }
    }

    /* populate leaves with frequency information from file header */
    if (!ReadHeader(huffmanArray, bfpIn))
    {
        for (i = 0; i < NUM_CHARS; i++)
        {
            free(huffmanArray[i]);
        }

        BitFileClose(bfpIn);
        fclose(fpOut);
        return FALSE;
    }

    /* put array of leaves into a huffman tree */
    if ((huffmanTree = BuildHuffmanTree(huffmanArray, NUM_CHARS)) == NULL)
    {
        FreeHuffmanTree(huffmanTree);

        BitFileClose(bfpIn);
        fclose(fpOut);
        return FALSE;
    }

    /* now we should have a tree that matches the tree used on the encode */
    currentNode = huffmanTree;

    while ((c = BitFileGetBit(bfpIn)) != EOF)
    {
        /* traverse the tree finding matches for our characters */
        if (c != 0)
        {
            currentNode = currentNode->right;
        }
        else
        {
            currentNode = currentNode->left;
        }

        if (currentNode->value != COMPOSITE_NODE)
        {
            /* we've found a character */
            if (currentNode->value == EOF_CHAR)
            {
                /* we've just read the EOF */
                break;
            }

            fputc(currentNode->value, fpOut);   /* write out character */
            currentNode = huffmanTree;          /* back to top of tree */
        }
    }

    /* close all files */
    BitFileClose(bfpIn);
    fclose(fpOut);
    FreeHuffmanTree(huffmanTree);     /* free allocated memory */

    return TRUE;
}

/****************************************************************************
*   Function   : HuffmanShowTree
*   Description: This routine genrates a huffman tree optimized for a file
*                and writes out an ASCII representation of the code
*                represented by the tree.
*   Parameters : inFile - Name of file to create tree for
*                outFile - Name of file to write a tree to
*   Effects    : Huffman tree is written out to a file
*   Returned   : TRUE for success, otherwise FALSE.
****************************************************************************/
int HuffmanShowTree(char *inFile, char *outFile)
{
    FILE *fpIn, *fpOut;
    huffman_node_t *huffmanTree;        /* root of huffman tree */
    huffman_node_t *htp;                /* pointer into tree */
    char code[NUM_CHARS - 1];           /* 1s and 0s in character's code */
    int depth = 0;                      /* depth of tree */

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

    /* write out tree */
    /* print heading to make things look pretty (int is 10 char max) */
    fprintf(fpOut, "Char  Count      Encoding\n");
    fprintf(fpOut, "----- ---------- ----------------\n");

    htp = huffmanTree;
    for(;;)
    {
        /* follow this branch all the way left */
        while (htp->left != NULL)
        {
            code[depth] = '0';
            htp = htp->left;
            depth++;
        }

        if (htp->value != COMPOSITE_NODE)
        {
            /* handle the case of a single symbol code */
            if (depth == 0)
            {
                code[depth] = '0';
                depth++;
            }

            /* we hit a character node, print its code */
            code[depth] = '\0';

            if (htp->value != EOF_CHAR)
            {
                fprintf(fpOut, "0x%02X  %10d %s\n",
                    htp->value, htp->count, code);
            }
            else
            {
                fprintf(fpOut, "EOF   %10d %s\n", htp->count, code);
            }
        }

        while (htp->parent != NULL)
        {
            if (htp != htp->parent->right)
            {
                /* try the parent's right */
                code[depth - 1] = '1';
                htp = htp->parent->right;
                break;
            }
            else
            {
                /* parent's right tried, go up one level yet */
                depth--;
                htp = htp->parent;
                code[depth] = '\0';
            }
        }

        if (htp->parent == NULL)
        {
            /* we're at the top with nowhere to go */
            break;
        }
    }

    /* clean up */
    fclose(fpIn);
    fclose(fpOut);
    FreeHuffmanTree(huffmanTree);     /* free allocated memory */

    return TRUE;
}

/****************************************************************************
*   Function   : MakeCodeList
*   Description: This function uses a huffman tree to build a list of codes
*                and their length for each encoded symbol.  This simplifies
*                the encoding process.  Instead of traversing a tree to
*                in search of the code for any symbol, the code maybe found
*                by accessing cl[symbol].code.
*   Parameters : ht - pointer to root of huffman tree
*                codeList - code list to populate.
*   Effects    : Code values are filled in for symbols in a code list.
*   Returned   : TRUE for success, FALSE for failure
****************************************************************************/
static int MakeCodeList(huffman_node_t *ht, code_list_t *codeList)
{
    bit_array_t *code;
    byte_t depth = 0;

    if((code = BitArrayCreate(256)) == NULL)
    {
        perror("Unable to allocate bit array");
        return (FALSE);
    }

    BitArrayClearAll(code);

    for(;;)
    {
        /* follow this branch all the way left */
        while (ht->left != NULL)
        {
            BitArrayShiftLeft(code, 1);
            ht = ht->left;
            depth++;
        }

        if (ht->value != COMPOSITE_NODE)
        {
            /* enter results in list */
            codeList[ht->value].codeLen = depth;
            codeList[ht->value].code = BitArrayDuplicate(code);
            if (codeList[ht->value].code == NULL)
            {
                perror("Unable to allocate bit array");
                BitArrayDestroy(code);
                return (FALSE);
            }

            /* now left justify code */
            BitArrayShiftLeft(codeList[ht->value].code, 256 - depth);
        }

        while (ht->parent != NULL)
        {
            if (ht != ht->parent->right)
            {
                /* try the parent's right */
                BitArraySetBit(code, 255);
                ht = ht->parent->right;
                break;
            }
            else
            {
                /* parent's right tried, go up one level yet */
                depth--;
                BitArrayShiftRight(code, 1);
                ht = ht->parent;
            }
        }

        if (ht->parent == NULL)
        {
            /* we're at the top with nowhere to go */
            break;
        }
    }

    BitArrayDestroy(code);
    return (TRUE);
}

/****************************************************************************
*   Function   : WriteHeader
*   Description: This function writes the each symbol contained in a tree
*                as well as its number of occurrences in the original file
*                to the specified output file.  If the same algorithm that
*                produced the original tree is used with these counts, an
*                exact copy of the tree will be produced.
*   Parameters : ht - pointer to root of huffman tree
*                bfp - pointer to open binary file to write to.
*   Effects    : Symbol values and symbol counts are written to a file.
*   Returned   : None
****************************************************************************/
static void WriteHeader(huffman_node_t *ht, bit_file_t *bfp)
{
    int i;

    for(;;)
    {
        /* follow this branch all the way left */
        while (ht->left != NULL)
        {
            ht = ht->left;
        }

        if ((ht->value != COMPOSITE_NODE) &&
            (ht->value != EOF_CHAR))
        {
            /* write symbol and count to header */
            BitFilePutChar(ht->value, bfp);
            BitFilePutBits(bfp, (void *)&(ht->count), 8 * sizeof(count_t));
        }

        while (ht->parent != NULL)
        {
            if (ht != ht->parent->right)
            {
                ht = ht->parent->right;
                break;
            }
            else
            {
                /* parent's right tried, go up one level yet */
                ht = ht->parent;
            }
        }

        if (ht->parent == NULL)
        {
            /* we're at the top with nowhere to go */
            break;
        }
    }

    /* now write end of table char 0 count 0 */
    BitFilePutChar(0, bfp);
    for(i = 0; i < sizeof(count_t); i++)
    {
        BitFilePutChar(0, bfp);
    }
}

/****************************************************************************
*   Function   : ReadHeader
*   Description: This function reads the header information stored by
*                WriteHeader.  If the same algorithm that produced the
*                original tree is used with these counts, an exact copy of
*                the tree will be produced.
*   Parameters : ht - pointer to array of pointers to tree leaves
*                inFile - file to read from
*   Effects    : Frequency information is read into the node of ht
*   Returned   : TRUE for success, otherwise FALSE
****************************************************************************/
static int ReadHeader(huffman_node_t **ht, bit_file_t *bfp)
{
    count_t count;
    int c;
    int status = FALSE;     /* in case of premature EOF */

    while ((c = BitFileGetChar(bfp)) != EOF)
    {
        BitFileGetBits(bfp, (void *)(&count), 8 * sizeof(count_t));

        if ((count == 0) && (c == 0))
        {
            /* we just read end of table marker */
            status = TRUE;
            break;
        }

        ht[c]->count = count;
        ht[c]->ignore = FALSE;
    }

    /* add assumed EOF */
    ht[EOF_CHAR]->count = 1;
    ht[EOF_CHAR]->ignore = FALSE;

    if (status == FALSE)
    {
        /* we hit EOF before we read a full header */
        fprintf(stderr, "error: malformed file header.\n");
    }

    return status;
}
