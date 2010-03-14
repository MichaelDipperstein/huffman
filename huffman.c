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
*   $Id: huffman.c,v 1.5 2004/02/04 15:30:44 michael Exp $
*   $Log: huffman.c,v $
*   Revision 1.5  2004/02/04 15:30:44  michael
*   replace bitop with bitarry library.
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
* Copyright (C) 2002 by Michael Dipperstein (mdipper@cs.ucsb.edu)
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
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "bitarray.h"
#include "bitfile.h"
#include "getopt.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
/* use preprocessor to verify type lengths */
#if (UCHAR_MAX != 0xFF)
#error This program expects unsigned char to be 1 byte
#endif

#if (USHRT_MAX != 0xFFFF)
#error This program expects unsigned short to be 2 bytes
#endif

#if (UINT_MAX != 0xFFFFFFFF)
#error This program expects unsigned int to be 4 bytes
#endif

#if (ULONG_MAX != 0xFFFFFFFF)
#error This program expects unsigned long to be 4 bytes
#endif

/* system dependent types */
typedef unsigned char byte_t;       /* unsigned 8 bit */
typedef unsigned int count_t;       /* unsigned 32 bit for character counts */

typedef struct huffman_node_t
{
    int value;          /* character(s) represented by this entry */
    count_t count;      /* number of occurrences of value (probability) */

    char ignore;        /* TRUE -> already handled or no need to handle */
    int level;          /* depth in tree (root is 0) */

    /***********************************************************************
    *  pointer to children and parent.
    *  NOTE: parent is only useful if non-recursive methods are used to
    *        search the huffman tree.
    ***********************************************************************/
    struct huffman_node_t *left, *right, *parent;
} huffman_node_t;

typedef struct code_list_t
{
    byte_t codeLen;     /* number of bits used in code (1 - 255) */
    bit_array_t *code;  /* code used for symbol (left justified) */
} code_list_t;

typedef enum
{
    BUILD_TREE,
    COMPRESS,
    DECOMPRESS
} MODES;

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define FALSE   0
#define TRUE    1
#define NONE    -1

#define COUNT_T_MAX     UINT_MAX    /* based on count_t being unsigned int */

#define COMPOSITE_NODE      -1      /* node represents multiple characters */
#define NUM_CHARS           257     /* 256 bytes + EOF */
#define EOF_CHAR    (NUM_CHARS - 1) /* index used for EOF */

#define max(a, b) ((a)>(b)?(a):(b))

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
huffman_node_t *huffmanArray[NUM_CHARS];        /* array of all leaves */

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/* allocation/deallocation routines */
huffman_node_t *AllocHuffmanNode(int value);
huffman_node_t *AllocHuffmanCompositeNode(huffman_node_t *left,
    huffman_node_t *right);
void FreeHuffmanTree(huffman_node_t *ht);

/* build and display tree */
huffman_node_t *GenerateTreeFromFile(char *inFile);
huffman_node_t *BuildHuffmanTree(huffman_node_t **ht, int elements);
void PrintCode(huffman_node_t *ht, char *outFile);

void EncodeFile(huffman_node_t *ht, char *inFile, char *outFile);
void DecodeFile(huffman_node_t **ht, char *inFile, char *outFile);
int MakeCodeList(huffman_node_t *ht, code_list_t *codeList);

/* reading/writing tree to file */
void WriteHeader(huffman_node_t *ht, bit_file_t *bfp);
void ReadHeader(huffman_node_t **ht, bit_file_t *bfp);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : Main
*   Description: This is the main function for this program, it validates
*                the command line input and, if valid, it will build a
*                huffman tree for the input file, huffman encode a file, or
*                decode a huffman encoded file.
*   Parameters : argc - number of parameters
*                argv - parameter list
*   Effects    : Builds a huffman tree for specified file and outputs
*                resulting code to stdout.
*   Returned   : EXIT_SUCCESS for success, otherwise EXIT_FAILURE.
****************************************************************************/
int main (int argc, char *argv[])
{
    huffman_node_t *huffmanTree;        /* root of huffman tree */
    int opt;
    char *inFile, *outFile;
    MODES mode;

    /* initialize variables */
    inFile = NULL;
    outFile = NULL;
    mode = BUILD_TREE;

    /* parse command line */
    while ((opt = getopt(argc, argv, "cdtni:o:h?")) != -1)
    {
        switch(opt)
        {
            case 'c':       /* compression mode */
                mode = COMPRESS;
                break;

            case 'd':       /* decompression mode */
                mode = DECOMPRESS;
                break;

            case 't':       /* just build and display tree */
                mode = BUILD_TREE;
                break;

            case 'i':       /* input file name */
                if (inFile != NULL)
                {
                    fprintf(stderr, "Multiple input files not allowed.\n");
                    free(inFile);

                    if (outFile != NULL)
                    {
                        free(outFile);
                    }

                    exit(EXIT_FAILURE);
                }
                else if ((inFile = (char *)malloc(strlen(optarg) + 1)) == NULL)
                {
                    perror("Memory allocation");

                    if (outFile != NULL)
                    {
                        free(outFile);
                    }

                    exit(EXIT_FAILURE);
                }

                strcpy(inFile, optarg);
                break;

            case 'o':       /* output file name */
                if (outFile != NULL)
                {
                    fprintf(stderr, "Multiple output files not allowed.\n");
                    free(outFile);

                    if (inFile != NULL)
                    {
                        free(inFile);
                    }

                    exit(EXIT_FAILURE);
                }
                else if ((outFile = (char *)malloc(strlen(optarg) + 1)) == NULL)
                {
                    perror("Memory allocation");

                    if (inFile != NULL)
                    {
                        free(inFile);
                    }

                    exit(EXIT_FAILURE);
                }

                strcpy(outFile, optarg);
                break;

            case 'h':
            case '?':
                printf("Usage: huffman <options>\n\n");
                printf("options:\n");
                printf("  -c : Encode input file to output file.\n");
                printf("  -d : Decode input file to output file.\n");
                printf("  -t : Generate code tree for input file to output file.\n");
                printf("  -i<filename> : Name of input file.\n");
                printf("  -o<filename> : Name of output file.\n");
                printf("  -h|?  : Print out command line options.\n\n");
                printf("Default: huffman -t -ostdout\n");
                return(EXIT_SUCCESS);
        }
    }

    /* validate command line */
    if (inFile == NULL)
    {
        fprintf(stderr, "Input file must be provided\n");
        fprintf(stderr, "Enter \"huffman -?\" for help.\n");
        exit (EXIT_FAILURE);
    }

    if ((mode == COMPRESS) || (mode == BUILD_TREE))
    {
        huffmanTree = GenerateTreeFromFile(inFile);

        /* determine what to do with tree */
        if (mode == COMPRESS)
        {
            /* write encoded file */
            EncodeFile(huffmanTree, inFile, outFile);
        }
        else
        {
            /* just output code */
            PrintCode(huffmanTree, outFile);
        }

        FreeHuffmanTree(huffmanTree);     /* free allocated memory */
    }
    else if (mode == DECOMPRESS)
    {
        DecodeFile(huffmanArray, inFile, outFile);
    }

    /* clean up*/
    free(inFile);
    if (outFile != NULL)
    {
        free(outFile);
    }
    return(EXIT_SUCCESS);
}

/****************************************************************************
*   Function   : GenerateTreeFromFile
*   Description: This routine creates a huffman tree optimized for encoding
*                the file passed as a parameter.
*   Parameters : inFile - Name of file to create tree for
*   Effects    : Huffman tree is built for file.
*   Returned   : Pointer to resulting tree
****************************************************************************/
huffman_node_t *GenerateTreeFromFile(char *inFile)
{
    huffman_node_t *huffmanTree;              /* root of huffman tree */
    int c;
    FILE *fp;

    /* open file as binary */
    if ((fp = fopen(inFile, "rb")) == NULL)
    {
        perror(inFile);
        exit(EXIT_FAILURE);
    }

    /* allocate array of leaves for all possible characters */
    for (c = 0; c < NUM_CHARS; c++)
    {
        huffmanArray[c] = AllocHuffmanNode(c);
    }

    /* assume that there will be exactly 1 EOF */
    huffmanArray[EOF_CHAR]->count = 1;
    huffmanArray[EOF_CHAR]->ignore = FALSE;

    /* count occurrence of each character */
    while ((c = fgetc(fp)) != EOF)
    {
        if (huffmanArray[c]->count < COUNT_T_MAX)
        {
            /* increment count for character and include in tree */
            huffmanArray[c]->count++;
            huffmanArray[c]->ignore = FALSE;
        }
        else
        {
            fprintf(stderr,
                "This file contains too many 0x%02X to count.\n", c);
            exit(EXIT_FAILURE);
        }
    }

    fclose(fp);

    /* put array of leaves into a huffman tree */
    huffmanTree = BuildHuffmanTree(huffmanArray, NUM_CHARS);

    return(huffmanTree);
}

/****************************************************************************
*   Function   : AllocHuffmanNode
*   Description: This routine allocates and initializes memory for a node
*                (tree entry for a single character) in a Huffman tree.
*   Parameters : value - character value represented by this node
*   Effects    : Memory for a huffman_node_t is allocated from the heap
*   Returned   : Pointer to allocated node.  NULL on failure to allocate.
****************************************************************************/
huffman_node_t *AllocHuffmanNode(int value)
{
    huffman_node_t *ht;

    ht = (huffman_node_t *)(malloc(sizeof(huffman_node_t)));

    if (ht != NULL)
    {
        ht->value = value;
        ht->ignore = TRUE;      /* will be FALSE if one is found */

        /* at this point, the node is not part of a tree */
        ht->count = 0;
        ht->level = 0;
        ht->left = NULL;
        ht->right = NULL;
        ht->parent = NULL;
    }
    else
    {
        perror("Allocate Node");
        exit(EXIT_FAILURE);
    }

    return ht;
}

/****************************************************************************
*   Function   : AllocHuffmanCompositeNode
*   Description: This routine allocates and initializes memory for a
*                composite node (tree entry for multiple characters) in a
*                Huffman tree.  The number of occurrences for a composite is
*                the sum of occurrences of its children.
*   Parameters : left - left child in tree
*                right - right child in tree
*   Effects    : Memory for a huffman_node_t is allocated from the heap
*   Returned   : Pointer to allocated node
****************************************************************************/
huffman_node_t *AllocHuffmanCompositeNode(huffman_node_t *left,
    huffman_node_t *right)
{
    huffman_node_t *ht;

    ht = (huffman_node_t *)(malloc(sizeof(huffman_node_t)));

    if (ht != NULL)
    {
        ht->value = COMPOSITE_NODE;     /* represents multiple chars */
        ht->ignore = FALSE;
        ht->count = left->count + right->count;     /* sum of children */
        ht->level = max(left->level, right->level) + 1;

        /* attach children */
        ht->left = left;
        ht->left->parent = ht;
        ht->right = right;
        ht->right->parent = ht;
        ht->parent = NULL;
    }
    else
    {
        perror("Allocate Composite");
        exit(EXIT_FAILURE);
    }

    return ht;
}

/****************************************************************************
*   Function   : FreeHuffmanTree
*   Description: This is a recursive routine for freeing the memory
*                allocated for a node and all of its descendants.
*   Parameters : ht - structure to delete along with its children.
*   Effects    : Memory for a huffman_node_t and its children is returned to
*                the heap.
*   Returned   : None
****************************************************************************/
void FreeHuffmanTree(huffman_node_t *ht)
{
    if (ht->left != NULL)
    {
        FreeHuffmanTree(ht->left);
    }

    if (ht->right != NULL)
    {
        FreeHuffmanTree(ht->right);
    }

    free(ht);
}

/****************************************************************************
*   Function   : FindMinimumCount
*   Description: This function searches an array of HUFFMAN_STRCUT to find
*                the active (ignore == FALSE) element with the smallest
*                frequency count.  In order to keep the tree shallow, if two
*                nodes have the same count, the node with the lower level
*                selected.
*   Parameters : ht - pointer to array of structures to be searched
*                elements - number of elements in the array
*   Effects    : None
*   Returned   : Index of the active element with the smallest count.
*                NONE is returned if no minimum is found.
****************************************************************************/
int FindMinimumCount(huffman_node_t **ht, int elements)
{
    int i;                          /* array index */
    int currentIndex = NONE;        /* index with lowest count seen so far */
    int currentCount = INT_MAX;     /* lowest count seen so far */
    int currentLevel = INT_MAX;     /* level of lowest count seen so far */

    /* sequentially search array */
    for (i = 0; i < elements; i++)
    {
        /* check for lowest count (or equally as low, but not as deep) */
        if ((ht[i] != NULL) && (!ht[i]->ignore) &&
            (ht[i]->count < currentCount ||
                (ht[i]->count == currentCount && ht[i]->level < currentLevel)))
        {
            currentIndex = i;
            currentCount = ht[i]->count;
            currentLevel = ht[i]->level;
        }
    }

    return currentIndex;
}

/****************************************************************************
*   Function   : BuildHuffmanTree
*   Description: This function builds a huffman tree from an array of
*                HUFFMAN_STRCUT.
*   Parameters : ht - pointer to array of structures to be searched
*                elements - number of elements in the array
*   Effects    : Array of huffman_node_t is built into a huffman tree.
*   Returned   : Pointer to the root of a Huffman Tree
****************************************************************************/
huffman_node_t *BuildHuffmanTree(huffman_node_t **ht, int elements)
{
    int min1, min2;     /* two nodes with the lowest count */

    /* keep looking until no more nodes can be found */
    for (;;)
    {
        /* find node with lowest count */
        min1 = FindMinimumCount(ht, elements);

        if (min1 == NONE)
        {
            /* no more nodes to combine */
            break;
        }

        ht[min1]->ignore = TRUE;    /* remove from consideration */

        /* find node with second lowest count */
        min2 = FindMinimumCount(ht, elements);

        if (min2 == NONE)
        {
            /* no more nodes to combine */
            break;
        }

        ht[min2]->ignore = TRUE;    /* remove from consideration */

        /* combine nodes into a tree */
        ht[min1] = AllocHuffmanCompositeNode(ht[min1], ht[min2]);
        ht[min2] = NULL;
    }

    return ht[min1];
}


/****************************************************************************
*   Function   : PrintCode
*   Description: This function does a depth first traversal of huffman tree
*                printing out the code for each character node it reaches.
*   Parameters : ht - pointer to root of tree
*                outFile - where to output results (NULL -> stdout)
*   Effects    : The code for the characters contained in the tree is
*                printed to outFile.
*   Returned   : None
****************************************************************************/
void PrintCode(huffman_node_t *ht, char *outFile)
{
    char code[NUM_CHARS - 1];
    int depth = 0;
    FILE *fp;

    if (outFile == NULL)
    {
        fp = stdout;
    }
    else
    {
        if ((fp = fopen(outFile, "w")) == NULL)
        {
            perror(outFile);
            FreeHuffmanTree(ht);
            exit(EXIT_FAILURE);
        }
    }

    /* print heading to make things look pretty (int is 10 char max) */
    fprintf(fp, "Char  Count      Encoding\n");
    fprintf(fp, "----- ---------- ----------------\n");

    for(;;)
    {
        /* follow this branch all the way left */
        while (ht->left != NULL)
        {
            code[depth] = '0';
            ht = ht->left;
            depth++;
        }

        if (ht->value != COMPOSITE_NODE)
        {
            /* handle the case of a single symbol code */
            if (depth == 0)
            {
                code[depth] = '0';
                depth++;
            }

            /* we hit a character node, print its code */
            code[depth] = '\0';

            if (ht->value != EOF_CHAR)
            {
                fprintf(fp, "0x%02X  %10d %s\n", ht->value, ht->count, code);
            }
            else
            {
                fprintf(fp, "EOF   %10d %s\n", ht->count, code);
            }
        }

        while (ht->parent != NULL)
        {
            if (ht != ht->parent->right)
            {
                /* try the parent's right */
                code[depth - 1] = '1';
                ht = ht->parent->right;
                break;
            }
            else
            {
                /* parent's right tried, go up one level yet */
                depth--;
                ht = ht->parent;
                code[depth] = '\0';
            }
        }

        if (ht->parent == NULL)
        {
            /* we're at the top with nowhere to go */
            break;
        }
    }

    if (outFile != NULL)
    {
        fclose(fp);
    }
}

/****************************************************************************
*   Function   : EncodeFile
*   Description: This function uses the provide Huffman tree to encode
*                the file passed as a parameter.
*   Parameters : ht - pointer to root of tree
*                inFile - file to encode
*                outFile - where to output results (NULL -> stdout)
*   Effects    : inFile is encoded and the code plus the results are
*                written to outFile.
*   Returned   : None
****************************************************************************/
void EncodeFile(huffman_node_t *ht, char *inFile, char *outFile)
{
    code_list_t codeList[NUM_CHARS];    /* table for quick encode */
    FILE *fpIn;
    bit_file_t *bfpOut;
    int c;

    /* open binary input and output files */
    if ((fpIn = fopen(inFile, "rb")) == NULL)
    {
        perror(inFile);
        exit(EXIT_FAILURE);
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
            FreeHuffmanTree(ht);
            fclose(fpIn);
            exit(EXIT_FAILURE);
        }
    }

    /* initialize the code list */
    for (c = 0; c < NUM_CHARS; c++)
    {
        codeList[c].code = NULL;
        codeList[c].codeLen = 0;
    }

    if (MakeCodeList(ht, codeList))
    {
        /* converted code to easy to use list */
        WriteHeader(ht, bfpOut);        /* write header for rebuilding of tree */

        /* read character and write it to encoded file */
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
    }

    /* free the code list */
    for (c = 0; c < NUM_CHARS; c++)
    {
        if (codeList[c].code != NULL)
        {
            BitArrayDestroy(codeList[c].code);
        }
    }

    fclose(fpIn);
    BitFileClose(bfpOut);
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
int MakeCodeList(huffman_node_t *ht, code_list_t *codeList)
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
void WriteHeader(huffman_node_t *ht, bit_file_t *bfp)
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
*   Function   : DecodeFile
*   Description: This function decodes a huffman encode file, writing the
*                results to the specified output file.
*   Parameters : ht - pointer to array of tree node pointers
*                inFile - file to decode
*                outFile - where to output results (NULL -> stdout)
*   Effects    : inFile is decode and the results are written to outFile.
*   Returned   : None
****************************************************************************/
void DecodeFile(huffman_node_t **ht, char *inFile, char *outFile)
{
    huffman_node_t *huffmanTree, *currentNode;
    int i, c;
    bit_file_t *bfpIn;
    FILE *fpOut;

    if ((bfpIn = BitFileOpen(inFile, BF_READ)) == NULL)
    {
        perror(inFile);
        exit(EXIT_FAILURE);
        return;
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
            exit(EXIT_FAILURE);
        }
    }

    /* allocate array of leaves for all possible characters */
    for (i = 0; i < NUM_CHARS; i++)
    {
        ht[i] = AllocHuffmanNode(i);
    }

    /* populate leaves with frequency information from file header */
    ReadHeader(ht, bfpIn);

    /* put array of leaves into a huffman tree */
    huffmanTree = BuildHuffmanTree(ht, NUM_CHARS);

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

            fputc(currentNode->value, fpOut);
            currentNode = huffmanTree;
        }
    }

    /* close all files */
    BitFileClose(bfpIn);
    fclose(fpOut);
    FreeHuffmanTree(huffmanTree);     /* free allocated memory */
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
*   Returned   : None
****************************************************************************/
void ReadHeader(huffman_node_t **ht, bit_file_t *bfp)
{
    count_t count;
    int c;

    while ((c = BitFileGetChar(bfp)) != EOF)
    {
        BitFileGetBits(bfp, (void *)(&count), 8 * sizeof(count_t));

        if ((count == 0) && (c == 0))
        {
            /* we just read end of table marker */
            break;
        }

        ht[c]->count = count;
        ht[c]->ignore = FALSE;
    }

    /* add assumed EOF */
    ht[EOF_CHAR]->count = 1;
    ht[EOF_CHAR]->ignore = FALSE;
}
