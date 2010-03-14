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
#include "bitop256.h"
#include "getopt.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
/* system dependent types */
typedef unsigned char byte_t;       /* unsigned 8 bit */
typedef unsigned char code_t;      /* unsigned 8 bit for character codes */
typedef unsigned int count_t;       /* unsigned 32 bit for character counts */

/* use preprocessor to verify type lengths */
#if (UCHAR_MAX != 255)
#error This program expects unsigned char to be 1 byte
#endif

#if (USHRT_MAX != 65535)
#error This program expects unsigned short to be 2 bytes
#endif

/* breaks count_t into array of byte_t */
typedef union count_byte_t
{
    count_t count;
    byte_t byte[sizeof(count_t)];
} count_byte_t;

typedef struct huffman_node_t
{
    short value;        /* character(s) represented by this entry */
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

typedef struct canonical_list_t
{
    short value;        /* characacter represented */
    byte_t codeLen;     /* number of bits used in code (1 - 255) */
    code_t code[32];    /* code used for symbol (left justified) */
} canonical_list_t;

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
#define NUM_CHARS           256     /* 256 1 byte symbols */

#define max(a, b) ((a)>(b)?(a):(b))

/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
huffman_node_t *huffmanArray[NUM_CHARS];        /* array of all leaves */
count_t totalCount = 0;                         /* total number of chars */

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
canonical_list_t *BuildCanonicalCode(huffman_node_t *ht);
void AssignCanonicalCodes(canonical_list_t *cl);
void PrintCode(canonical_list_t *cl, char *outFile);

void EncodeFile(canonical_list_t *cl, char *inFile, char *outFile);
void DecodeFile(char *inFile, char *outFile);

/* reading/writing tree to file */
void WriteHeader(canonical_list_t *cl, FILE *fp);
void ReadHeader(canonical_list_t *cl, FILE *fp);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : Main
*   Description: This is the main function for this program, it validates
*                the command line input and, if valid, it will build a
*                canonical Huffman code for the input file, huffman encode a
*                file, or decode a huffman encoded file.
*   Parameters : argc - number of parameters
*                argv - parameter list
*   Effects    : Builds a huffman tree for specified file and outputs
*                resulting code to stdout.
*   Returned   : EXIT_SUCCESS for success, otherwise EXIT_FAILURE.
****************************************************************************/
int main (int argc, char *argv[])
{
    huffman_node_t *huffmanTree;        /* root of huffman tree */
    canonical_list_t *canonicalList;    /* list of canonical codes */
    int opt;
    char *inFile, *outFile;
    MODES mode;

    /* initialize variables */
    canonicalList = NULL;
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
                printf("Usage: chuffman <options>\n\n");
                printf("options:\n");
                printf("  -c : Encode input file to output file.\n");
                printf("  -d : Decode input file to output file.\n");
                printf("  -t : Generate code tree for input file to output file.\n");
                printf("  -i<filename> : Name of input file.\n");
                printf("  -o<filename> : Name of output file.\n");
                printf("  -h|?  : Print out command line options.\n\n");
                printf("Default: chuffman -t -ostdout\n");
                return(EXIT_SUCCESS);
        }
    }

    /* validate command line */
    if (inFile == NULL)
    {
        fprintf(stderr, "Input file must be provided\n");
        fprintf(stderr, "Enter \"chuffman -?\" for help.\n");
        exit (EXIT_FAILURE);
    }

    if ((mode == COMPRESS) || (mode == BUILD_TREE))
    {
        /* build huffman tree and use code lengths for canonical code */
        huffmanTree = GenerateTreeFromFile(inFile);
        canonicalList = BuildCanonicalCode(huffmanTree);
        FreeHuffmanTree(huffmanTree);     /* free allocated memory */

        if (canonicalList == NULL)
        {
            free(inFile);
            perror("Memory allocation");
            exit (EXIT_FAILURE);
        }

        /* determine what to do with code */
        if (mode == COMPRESS)
        {
            /* write encoded file */
            EncodeFile(canonicalList, inFile, outFile);
        }
        else
        {
            /* just output code */
            PrintCode(canonicalList, outFile);
        }
    }
    else if (mode == DECOMPRESS)
    {
        DecodeFile(inFile, outFile);
    }

    /* clean up*/
    free(inFile);
    if (outFile != NULL)
    {
        free(outFile);
    }

    if (canonicalList != NULL)
    {
        free(canonicalList);
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

    /* count occurrence of each character */
    while ((c = fgetc(fp)) != EOF)
    {
        if (totalCount < COUNT_T_MAX)
        {
            totalCount++;

            /* increment count for character and include in tree */
            huffmanArray[c]->count++;       /* check for overflow */
            huffmanArray[c]->ignore = FALSE;
        }
        else
        {
            fprintf(stderr,
                "Number of characters in file is too large to count.\n");
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
int CompareByCodeLen(const void *item1, const void *item2)
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
            return 1;;
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
int CompareBySymbolValue(const void *item1, const void *item2)
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
*                cl -
*   Effects    : cl stores a list of canonical codes sorted by the value
*                of the charcter to be encode.
*   Returned   : Pointer to allocated array that stores the list of
*                canonical codes.  NULL is returned on failure.
****************************************************************************/
canonical_list_t *BuildCanonicalCode(huffman_node_t *ht)
{
    int i;
    byte_t depth = 0;
    canonical_list_t *cl;

    /* allocate canonical code list */
    cl = (canonical_list_t *)malloc(NUM_CHARS * sizeof(canonical_list_t));
    if (cl == NULL)
    {
        return(cl);
    }

    /* initialize list */
    for(i = 0; i < NUM_CHARS; i++)
    {
        cl[i].value = i;
        cl[i].codeLen = 0;
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

    AssignCanonicalCodes(cl);

    /* re-sort list in lexical order for use by encode algorithm */
    qsort(cl, NUM_CHARS, sizeof(canonical_list_t), CompareBySymbolValue);

    return(cl);
}

/****************************************************************************
*   Function   : AssignCanonicalCode
*   Description: This function accepts a list of symbols sorted by their
*                code lengths, and assigns a canonical Huffman code to each
*                symbol.
*   Parameters : cl - sorted list of symbols to have code values assigned
*   Effects    : cl stores a list of canonical codes sorted by the length
*                of the code used to encode the symbol.
*   Returned   : None
****************************************************************************/
void AssignCanonicalCodes(canonical_list_t *cl)
{
    int i;
    byte_t length;
    code_t code[32];

    /* assign the new codes */
    ClearAll256(code);
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
            RightShift256(code, (length - cl[i].codeLen));
            length = cl[i].codeLen;
        }

        /* assign left justified code */
        Copy256(cl[i].code, code);
        LeftShift256(cl[i].code, 256 - length);

        Increment256(code);
    }
}

/****************************************************************************
*   Function   : PrintCode
*   Description: This function prints the codes for each symbol in a
*                canonical list with a non-zero code length.
*   Parameters : cl - canonical list of codes
*                outFile - where to output results (NULL -> stdout)
*   Effects    : The code for the characters contained in the list is
*                printed to outFile.
*   Returned   : None
****************************************************************************/
void PrintCode(canonical_list_t *cl, char *outFile)
{
    int i;
    int length = 0;
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
        }
    }

    /* print heading to make things look pretty (int is 10 char max) */
    fprintf(fp, "Char  CodeLen  Encoding\n");
    fprintf(fp, "----- -------- ----------------\n");

    for(i = 0; i < NUM_CHARS; i++)
    {
        if(cl[i].codeLen > 0)
        {
            printf("0x%02X  %02d       ", cl[i].value, cl[i].codeLen);

            /* now write out the code bits */
            for(length = 0; length < cl[i].codeLen; length++)
            {
                if (TestBit256(cl[i].code, length))
                {
                    putchar('1');
                }
                else
                {
                    putchar('0');
                }
            }

            putchar('\n');
        }
    }

    if (outFile != NULL)
    {
        fclose(fp);
    }
}

/****************************************************************************
*   Function   : EncodeFile
*   Description: This function uses the provide Huffman code to encode
*                the file passed as a parameter.
*   Parameters : cl - pointer to list of canonical Huffman codes
*                inFile - file to encode
*                outFile - where to output results (NULL -> stdout)
*   Effects    : inFile is encoded and the code plus the results are
*                written to outFile.
*   Returned   : None
****************************************************************************/
void EncodeFile(canonical_list_t *cl, char *inFile, char *outFile)
{
    FILE *fpIn, *fpOut;
    int c, i, bitCount;
    char bitBuffer;

    /* open binary input and output files */
    if ((fpIn = fopen(inFile, "rb")) == NULL)
    {
        perror(inFile);
        exit(EXIT_FAILURE);
    }

    if (outFile == NULL)
    {
        fpOut = stdout;
    }
    else
    {
        if ((fpOut = fopen(outFile, "wb")) == NULL)
        {
            perror(outFile);
            exit(EXIT_FAILURE);
        }
    }

    WriteHeader(cl, fpOut);         /* write header for rebuilding of tree */

    /* write encoded file 1 byte at a time */
    bitBuffer = 0;
    bitCount = 0;

    while((c = fgetc(fpIn)) != EOF)
    {

        /* shift in bits */
        for(i = 0; i < cl[c].codeLen; i++)
        {
            bitCount++;
            bitBuffer = (bitBuffer << 1) |
                (TestBit256(cl[c].code, i) != 0);

            if (bitCount == 8)
            {
                /* we have a byte in the buffer */
                fputc(bitBuffer, fpOut);
                bitCount = 0;
            }
        }
    }

    /* now handle spare bits */
    if (bitCount != 0)
    {
        bitBuffer <<= 8 - bitCount;
        fputc(bitBuffer, fpOut);
    }

    fclose(fpIn);
    fclose(fpOut);
}

/****************************************************************************
*   Function   : WriteHeader
*   Description: This function writes the code size for each symbol and the
*                total number of characters in the original file to the
*                specified output file.  If the same algorithm that produced
*                produced the original canonical code is used with these code
*                lengths, an exact copy of the code will be produced.
*   Parameters : cl - pointer to list of canonical Huffman codes
*                fp - pointer to open binary file to write to.
*   Effects    : Symbol code lengths and symbol count are written to the
*                output file.
*   Returned   : None
****************************************************************************/
void WriteHeader(canonical_list_t *cl, FILE *fp)
{
    int i;
    count_byte_t byteUnion;

    /* write out code size for each symbol */
    for (i = 0; i < NUM_CHARS; i++)
    {
        fputc(cl[i].codeLen, fp);
    }

    /* now write out number of symbols encoded */
    byteUnion.count = totalCount;
    for(i = 0; i < sizeof(count_t); i++)
    {
        fputc(byteUnion.byte[i], fp);
    }
}

/****************************************************************************
*   Function   : GetBit
*   Description: This function returns the next unread bit from a file.
*                Bits of each byte are returned msb to lsb, while bytes are
*                read from start of file to end of file.  Passing a different
*                file pointer will flush any buffered bits.
*   Parameters : fp - pointer file to read bits from.
*   Effects    : If there are no buffered bits to return, the next byte of
*                the file passed as a parameter is read.
*   Returned   : EOF if there are no more bits in the file
*                1 if the next bit is a 1
*                0 if the next bit is a 0
****************************************************************************/
int GetBit(FILE *fp)
{
    static char bitsBuffered = 0;
    static int buffer = 0;
    static FILE *lastFp = NULL;
    int retVal;

    if (lastFp != fp)
    {
        /* we just started a new file */
        bitsBuffered = 0;
        buffer = 0;
        lastFp = fp;
    }

    if (bitsBuffered == 0)
    {
        /* we need to read another byte */
        buffer = fgetc(fp);

        if (buffer == EOF)
        {
            return EOF;
        }

        bitsBuffered = 8;
    }

    /* determine value of msb and shift it out of the buffer */
    retVal = ((0x80 & (char)buffer) != 0);
    bitsBuffered--;
    buffer = (char)buffer << 1;
    return retVal;
}

/****************************************************************************
*   Function   : DecodeFile
*   Description: This function decodes a huffman encode file, writing the
*                results to the specified output file.
*   Parameters : inFile - file to decode
*                outFile - where to output results (NULL -> stdout)
*   Effects    : inFile is decode and the results are written to outFile.
*   Returned   : None
****************************************************************************/
void DecodeFile(char *inFile, char *outFile)
{
    canonical_list_t *cl;
    code_t code[32];
    byte_t length;
    int i, newBit;
    int lenIndex[257];
    FILE *fpIn, *fpOut;

    if ((fpIn = fopen(inFile, "rb")) == NULL)
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
            perror(outFile);
            exit(EXIT_FAILURE);
        }
    }

    /* allocate canonical code list */
    cl = (canonical_list_t *)malloc(NUM_CHARS * sizeof(canonical_list_t));
    if (cl == NULL)
    {
        perror("Memory allocation");
        fclose(fpIn);
        fclose(fpOut);
        exit(EXIT_FAILURE);
    }

    /* populate list with code length from file header */
    ReadHeader(cl, fpIn);

    /* sort the header by code length */
    qsort(cl, NUM_CHARS, sizeof(canonical_list_t), CompareByCodeLen);

    /* assign the codes using same rule as encode */
    AssignCanonicalCodes(cl);

    /* now we have a huffman code that matches the code used on the encode */

    /* create an index of first code at each possible length */
    for (i = 0; i < 256; i++)
    {
        lenIndex[i] = NUM_CHARS;
    }

    for (i = 0; i < NUM_CHARS; i++)
    {
        if (lenIndex[cl[i].codeLen] > i)
        {
            /* first occurance of this code length */
            lenIndex[cl[i].codeLen] = i;
        }
    }

    /* decode input file */
    length = 0;
    ClearAll256(code);

    while((newBit = GetBit(fpIn)) != EOF)
    {
        if (newBit != 0)
        {
            SetBit256(code, length);
        }

        length++;

        if (lenIndex[length] != NUM_CHARS)
        {
            /* there are code of this length */
            for(i = lenIndex[length];
                (i < NUM_CHARS) && (cl[i].codeLen == length);
                i++)
            {
                if ((Compare256(cl[i].code, code) == 0) &&
                    (cl[i].codeLen == length))
                {
                    /* we just read a symbol output decoded value */
                    if (totalCount > 0)
                    {
                        fputc(cl[i].value, fpOut);
                        totalCount--;
                    }
                    ClearAll256(code);
                    length = 0;

                    break;
                }
            }
        }
    }

    /* close all files */
    fclose(fpIn);
    fclose(fpOut);

    /* free allocated memory */
    free(cl);
}

/****************************************************************************
*   Function   : ReadHeader
*   Description: This function reads the header information stored by
*                WriteHeader.  If the same algorithm that produced the
*                original tree is used with these counts, an exact copy of
*                the tree will be produced.
*   Parameters : cl - pointer to list of canonical Huffman codes
*                inFile - file to read from
*   Effects    : Code lengths and symbols are read into the canonical list.
*                Total number of symbols encoded is store in totalCount
*   Returned   : None
****************************************************************************/
void ReadHeader(canonical_list_t *cl, FILE *fp)
{
    int c;
    int i;
    count_byte_t byteUnion;

    /* read the code length */
    for (i = 0; i < NUM_CHARS; i++)
    {
        c = fgetc(fp);

        if (c != EOF)
        {
            cl[i].value = i;
            cl[i].codeLen = (byte_t)c;
            ClearAll256(cl[i].code);
        }
        else
        {
            /* should probably handle short header */
        }
    }

    /* now read the number of symbols eoncoded */
    for(i = 0; i < sizeof(count_t); i++)
    {
        byteUnion.byte[i] = fgetc(fp);
    }
    totalCount = byteUnion.count;
}
