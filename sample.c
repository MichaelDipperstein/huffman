/***************************************************************************
*                       Huffman Library Usage Sample
*
*   File    : sample.c
*   Purpose : Demonstrates the usage of Huffman library encoding and
*             decoding routines.
*   Author  : Michael Dipperstein
*   Date    : February 25, 2004
*
****************************************************************************
*   UPDATES
*
*   $Id: sample.c,v 1.1 2004/02/26 04:57:32 michael Exp $
*   $Log: sample.c,v $
*   Revision 1.1  2004/02/26 04:57:32  michael
*   Initial revision.  Library usage sample.
*
****************************************************************************
*
* sample: An ANSI C Huffman Encoding/Decoding Library Examples
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

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "getopt.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/

typedef enum
{
    SHOW_TREE,
    COMPRESS,
    DECOMPRESS
} MODES;

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define FALSE   0
#define TRUE    1
#define NONE    -1

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
    int opt, status;
    char *inFile, *outFile;
    MODES mode;

    /* initialize variables */
    inFile = NULL;
    outFile = NULL;
    mode = SHOW_TREE;

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

            case 't':       /* just display tree */
                mode = SHOW_TREE;
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
                printf("Usage: huffsample <options>\n\n");
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
        fprintf(stderr, "Enter \"huffsample -?\" for help.\n");
        exit (EXIT_FAILURE);
    }

    /* execute selected function */
    switch (mode)
    {
        case SHOW_TREE:
            status = HuffmanShowTree(inFile, outFile);
            break;

        case COMPRESS:
            status = HuffmanEncodeFile(inFile, outFile);
            break;

        case DECOMPRESS:
            status = HuffmanDecodeFile(inFile, outFile);
            break;

        default:        /* error case */
            status = 0;
            break;
    }

    /* clean up*/
    free(inFile);
    if (outFile != NULL)
    {
        free(outFile);
    }

    if (status)
    {
        return (EXIT_SUCCESS);
    }
    else
    {
        return (EXIT_FAILURE);
    }
}
