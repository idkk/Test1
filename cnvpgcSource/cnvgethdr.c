/*******************************************************************************
 *  cnvgethdr.c        
 *  Copyright 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 ******************************************************************************/

/* This file last updated 20131104:10.50 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>       /* for getopts()   */
#include <sys/types.h>
#include <sys/stat.h>   
#include <stdint.h>
#include <fcntl.h>
#include <math.h>         /* for sqrt()      */

#include "cnvswap.h"
#include "cnvendian.h"
#include "cnvfloat.h"
#include "cnvsegyfmt.h"
#include "cnvgethdr.h"

#define DEBUG_LEV 0

#define NEVERSWAP 0

int main (int argc, char* argv[])
{
    /*
     *  External routines for handling input parameters:
     */
    extern char *optarg;
    extern int optind, optopt;
    // int   tprint = 0;
    int   thisendian = LITTLE;
    int   cmderr = 0;
    char  temp[4096];
    char  progname[4096];
    char  slash = '/';
    int   i, j, k, l;
    char  c;
    char *ifile = (char *)NULL;
    FILE *fifile = (FILE *)NULL;
    char fnamein [NAMELEN+1];
    char *ofile = (char *)NULL;
    FILE *fofile = (FILE *)NULL;
    char fnameout [NAMELEN+1];
    unsigned char buffer[120];

    /* The array e2a is the complete list of ASCII characters in EBCDIC order: */
    unsigned char e2a[256] =
          "....\x9c\x09\x86\x7f\x97\x8d\x8e\x0b\x0c\x0d\x0e\x0f"  /* 00 */
          "\x10\x11\x12\x13\x9d\x85\x08\x87\x18\x19\x92\x8f\x1c\x1d\x1e\x1f"  /* 10 */
          "\x80\x81,\x83\x84\x0a\x17\x1b^\x89\x8a\x8b\x8c\x05\x06\x07"  /* 20 */
          "\x90\x91\x16\x93\x94\x95\x96\x04\x98\x99\x9a\x9b\x14\x15\x9e\x1a"  /* 30 */
          " \xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8[.<(+!"  /* 40 */
          "&\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1]$*);^"  /* 50 */
          "-/\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9|,%_>?"  /* 60 */
          "\xba\xba\xbc\xbd\xbe\xbf\xc0\xc1\xc2.:#@'=\x22"  /* 70 */
          "\xc3"  "abcdefghi\xc4\xc5\xc6\xc7\xc8\xc9"  /* 80 */
          "\xcajklmnopqr\xcb\xcc\xcd\xce\xcf\xd0"  /* 90 */
          "\xd1~stuvwxyz\xd2\xd3\xd4\xd5\xd6\xd7"  /* A0 */
          "\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7"  /* B0 */
          "{ABCDEFGHI\xe8\xe9\xea\xeb\xec\xed"  /* C0 */
          "}JKLMNOPQR\xee\xef\xf0\xf1\xf2\xf3"  /* D0 */
          "\\\x9fSTUVWXYZ\xf4\xf5\xf6\xf7\xf8\xf9"  /* E0 */
          "0123456789\xfa\xfb\xfc\xfd\xfe\xff"  /* F0 */
          ;


    /*  COPYRIGHT  */
    fprintf(stderr, "This program is strictly Copyright (c) 2013 Westheimer "
            "Energy Consultants Inc.\n"
            "All rights reserved\n\n");


    /*  ENDIANNESS  */

    /*
     *  Test and remember the endianness of *this* machine:
     */
    if (ENDIANNESS == BIG)
    {
        thisendian = BIG;
        fprintf(stderr, "INFO: This machine is BIG-endian.\n");
    }
    else if (ENDIANNESS == LITTLE)
    {
        thisendian = LITTLE;
        fprintf(stderr, "INFO: This machine is LITTLE-endian.\n");
    }
    else
    {
        fprintf(stderr,
                "ERROR: ENDIAN format of this machine not supported (%d / %x).\n",
                endianness,endianness);
                fprintf(stderr, "[endianness first byte is %x]\n",
                *(const char *)&endianness);
                cmderr++;
    }

    /*  Remember the program name - the name of this program - for
     *  debug and message printout. We edit out, from the name of
     *  the program as invoked, any prior directory name specified:
     */
    strcpy(temp, argv[0]);
    j = strlen(temp);
    i = 0;
    while((j>i) && (temp[j] != slash))
    {
        j--;
    }
    /* If there actually was a slash in the program name, we are now pointing at
     * it - so move forwards to point to the first character after the
     * rightmost slash: */
    if ( temp[j] == slash )
    {
        j++;
    }
    k = strlen(temp);
    strncpy(progname, &temp[j], k-j);
    /* Add a terminating string end character: */
    progname[k-j] = '\0';


    /*  COMMAND LINE PROCESSING  */

    /*
     *  Read in and initially process each command line parameter:
     */


    /* Get command line arguments: */
    while ((c = getopt(argc, argv, ":i:o:v")) != -1)
    {
        switch(c)
        {
            case 'i':
                ifile = optarg;
                strcpy (fnamein, ifile);
                break;

            case 'o':
                ofile = optarg;
                strcpy (fnameout, ofile);
                break;

            case 'v':
               fprintf(stderr, "Program Version %s %s\n", STHVERSION, STHDATE);
                                fprintf(stderr, "Program Name:   %s : %s\n", progname, argv[0]);
                break;

            case '?':
                fprintf(stderr, "ERROR: Unrecognized option: [-%c]\n", c);
                cmderr++;
        }
    }
    
    /*  Check that we have an input file, and that it can be opened: */
    if (ifile == (char *)NULL)
    {
        fprintf(stderr, "ERROR: no input file specified [-i]\n"
                        "       Program terminates.\n\n");
        return 1;
    }

    fifile = fopen(fnamein,"r");
    if (fifile == (FILE *)NULL)
    {
        fprintf(stderr, "ERROR: Input file [%s] cannot be opened\n"
                        "       Program terminates.\n\n",fnamein);
        return 2;
    }

    if (ofile == (char *)NULL)
    {
        strcpy (fnameout,"stdout");
        fofile = stdout;
        fprintf(stderr, "INFO:  Output is to stdout.\n");
    }
    else
    {
        fofile = fopen(fnameout,"w");
        if (fofile == (FILE *)NULL)
        {
            fprintf(stderr, "ERROR: Output file [%s] could not be opened.\n"
                            "       Program terminates.\n",fnameout);
            return 3;
        }
        fprintf(stderr, "INFO:  Output is to file [%s]\n",fnameout);
    }

   /* OK - we know that we have an open input file, and an open output
    * file - so start by copying the first 40 "cards" (80-byte chunks) from
    * the input to the output:
    */

    for (i=0; (i<40); i++)
    {
        l = fread(buffer,sizeof(char),80,fifile);
        if (l != 80)
        {
            fprintf(stderr, "ERROR: Incomplete read, length [%d] line [%d]\n"
                            "       Program terminates.\n",l,i);
            return 4;
        }
        // buffer[80] = '\0';
        for (j=0;(j<80);j++)
        {
            buffer[j] = e2a[buffer[j]];
        }
        buffer[80] = '\n';
        buffer[81] = '\0';
        l = fwrite(buffer,sizeof(char),81,fofile);
        // j = fwrite("\n",sizeof(char),1,fofile);
        /*
         * At this point we can write any extra fields from the input,
         * which will follow the listing of the header cards.
         * TO BE DONE
         */
    }
    
    tfclose(fifile);
    tfclose(fofile);

    return 0;
    
}  /* End of main sth2seg */


    /*
    * Conditionally close a file, if the pointer is not NULL
    */
int tfclose (FILE *filep)
{
    int ret = 0;
    
    if (filep == (FILE *) NULL)
    {
        return (0);
    }
    else 
    {
        ret = fclose(filep);
        /* NOTE: Do ***NOT*** un-comment the following statement, as the file
         *       being closed *might* be stdin or stdout or stderr
         */
//        filep = (FILE *)NULL;
        return ret;
    }
}


/*******************************************************************************
 *  cnvgethdr.c        
 *  Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 ******************************************************************************/

