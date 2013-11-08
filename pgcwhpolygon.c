/*
 ** Copyright (c) 2012-2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 **
 ** pgcwhpolygon.c (part of pgcwhpolygon)
 ** 
 ** This program reads a *.shp file derived *.csv file and produces a file
 ** containing a suitable WHERE clause
 **
 ** Use this program to convert the .shp part of a Shape File into the .csv
 **
 ** e.g. pgcwhpolygon -i xxx.csv -o where.sql
 **
 ** NOTE: This copy has been [IS BEING] modified by IDKK to deal with 
 **       (a) sorting rings, and 
 **       (b) determining the nesting of rings, and 
 **       (c) scanning rings in the order required for generating the 
 **       correct SQL "WHERE" clause. 
 */

/* This file last updated 20130815:1200 */

/*
 **  Return (exit) conditions:
 **      0    Normal return
 **      1    Passed in parameters incorrect
 **      2    Failed to read rings
 **      3    Failed to build Convex Hull
 **      4    Could not get memory for nesting calculation
 **      5    Failed to build WHERE
 */

/*
 **  The text (human readable) output from this program is on both stdout and
 **  stderr. The stdout output records the information useful to the user, and
 **  the stderr output (if any) records the error indications, and any trace
 **  information. Thus stdout can NOT be used as the output data (WHERE clause)
 **  file.
 */

/*
 **      A "box" is rectangular, with its sides parallel to the axes.
 **      We are assuming, in this discussion, that all rings are convex.
 **      A box surrounds a ring. If something is not in the box, then it is not
 **      in the ring - though it IS possible for something to be in the box but
 **      not in the ring.
 **      A diamond is a quadrilateral joining the northern-most (top), eastern-
 **      most, southern-most (bottom), and western-most points on the ring. 
 **      The points (corners) of a diamond touch the sides of the box, and the 
 **      diamond is entirely inside the box. If something is in the diamond, 
 **      then it is in the ring - though it is also possible for something 
 **      *not* in the diamond to be inside the ring.
 **      So there are two very quick tests for any point, before we have to 
 **      consider all the edges of a ring, to determine whether that point is 
 **      inside the ring:
 **      1) is the point outside the box? If so, then the point is OUTside the 
 **         ring.
 **      2) is the point inside the diamond (four tests against the four sides 
 **         of the diamond)? If so then the point is INside the the ring.
 **      Only for points *in* the box and *not in* the diamond do we have to 
 **      check all the edges of the ring - the most computationally expensive 
 **      part of the decision.
 **      
 **      Nesting: A ring may be inside another ring. There are no partially 
 **      overlapping rings - such things are in error: ring A is either entirely
 **      inside or entirely outside ring B (though touching is permitted).
 **      For any INCLUDE ring, we are interested only in the immediately 
 **      contained EXCLUDE rings: we are NOT interested in anything more deeply 
 **      nested.
 **      Of course, we are interested in *every* INCLUDE ring, but each INCLUDE 
 **      ring may be considered separately, in conjunction with (and only with) 
 **      its immediately subordinate EXCLUDE ring(s). This allows us to build up 
 **      the WHERE clause in all cases. 
 **      The ordering of the WHERE clause, though, is another question, and 
 **      (I think) we should start from the innermost INCLUDE ring(s) - that is,
 **      the INCLUDE ring(s) that contain no deeper INCLUDE rings, and move
 **      outwards, using the most inclusive INCLUDE rings last of all - this is 
 **      for efficiency of database recovery. In the first instance, though, it 
 **      does not matter in which order we take the INCLUDE rings - all orders 
 **      will work.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>       /* for getopts()   */
#include <sys/types.h>
#include <sys/stat.h>   
#include <stdint.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>         /* for sqrt()      */

/*
 **		NOTE that the following definitions must be specified BEFORE the 
 **		inclusion of pgcwhpolygon.h
 */
#define XCOL "XCOLNAME"  /* X Coord Column Name */
#define YCOL "YCOLNAME"  /* Y Coord Column Name */
char *xcol = XCOL;
char *ycol = YCOL;
int  tprint = 0;
int  maxcorners = 0;
int  maxmaxcorners = 0;
int  corners = 0;
int  strict = 0;
int  tolerance = 1;

#include "cnvswap.h"
#include "cnvendian.h"

#include "pgcwhpolygon.h"


/*
 ** Function : xxx()
 ** Purpose  : 
 ** Arguments: 
 ** Returns  : void
 ** Notes    :
 */
int 
main (int   argc,
      char *argv[])
{  
    int c;
    int cmderr = 0;                /* Initialised: No errors yet           */
    extern char *optarg;
    extern int optind, optopt;
    char *ifile = (char *)NULL;    /* Input file name                      */
    char *ofile = (char *)NULL;    /* Output file name                     */
    char *rewrite = (char *)NULL;  /* -R parameter value                   */
    char *strictstr = (char *)NULL; /* -S parameter value                  */
    FILE *ipf = (FILE *)NULL;      /* Input file pointer                   */
    FILE *opf = (FILE *)NULL;      /* Output file pointer                  */
    struct box   Box;              /* Min and Max X and Y coords           */
	struct diamond Diamond;        /* Interior, contained quadrilateral    */
    struct ring *rings;            /* Ptr to list of Read In rings         */
    int defboxflag = 0;
    int boxflag    = defboxflag;
    int ringCnt    = 0;            /* Count of number of rings found       */
    int ret        = 0;            /* Return code                          */
    int rew        = 0;            /* Overwrite permission - default YES   */
	int i, j, k;                   /* Work variables                       */
    int  thisendian  = LITTLE;     /* Default Endianicty of *this* machine */
    char temp     [NAMEMAX+1];     /* Temporary work area                  */
	char progname [NAMEMAX+1];     /* Name of *this* program               */
	
	char slash = '/';
//	char dot   = '.';
	
    
	
	int *nesting = (int *)NULL; /* Ptr to Matrix for nesting tests */
    //	int outer;  /* IDKK - nesting */
    //	int inner;  /* IDKK - nesting */
	
    fprintf(stderr, "This program is strictly Copyright (c) 2013 "
            "Westheimer Energy Consultants Ltd.\n"
            "All Rights Reserved\n\n");
    
    maxcorners = MAXCORNERS;
    maxmaxcorners = UPPERMAXCORN;
    corners = 0;
    strict = 0;
    tolerance = 1;
    
    /*  ENDIANNESS  */
    
	/*
	 *  Test and remember the endianness of *this* machine:
	 */
	if (ENDIANNESS == BIG)
	{
		thisendian = BIG;
        fprintf(stderr, "INFO: This machine is BIG-endian\n");
	}
	else if (ENDIANNESS == LITTLE)
	{
		thisendian = LITTLE;
        fprintf(stderr, "INFO: This machine is LITTLE-endian\n");
	}
	else 
	{
		fprintf(stderr,
                "ERROR: ENDIAN format of this machine not supported (%d / %x).\n",
                endianness, endianness);
		fprintf(stderr,"[endianness first byte is %x]\n",
                *(const char *)&endianness);
		cmderr++;
	}
    
    /*  PROGRAM NAME  */
	
	/*
	 *  Remember the program name - the name of this program - for
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
	
    	
    /*
	 ** Read the Command Line Arguments
	 */
    while ((c = getopt(argc, argv, ":i:o:b:c:C:x:y:S:t:R:v")) != -1)
    {
        switch(c)
        {
            /* Input file: */
			case 'i':
				ifile = optarg;
				break;
                
            /* Output file: */
			case 'o':
				ofile = optarg;
				break;
				
            /* Bounding box (Y or N): */
			case 'b':
				boxflag = atoi(optarg);
				break;
               
            /* Max number of corners: */
            case 'c':
                maxcorners = atoi(optarg);
                if (maxcorners > maxmaxcorners)
                {
                    fprintf(stderr,"ERROR: Corners (-c) must be less than %d\n",
                            UPPERMAXCORN);
                    cmderr++;
                }
                break;
				
            /* Tolerance of corner matching: */
            case 'C':
                tolerance = atoi(optarg);
                if ((tolerance < 1) || (tolerance > MAXTOL))
                {
                    fprintf(stderr, 
                            "ERROR: Tolerance (-C) must be between 1 and %d, not %d\n",
                            MAXTOL, tolerance);
                    cmderr++;
                }
                break;
               
            /* Displacement of X column: */
			case 'x':
				xcol = optarg;
				break;
                
            /* Displacement of Y column: */
			case 'y':
				ycol = optarg;
				break;
				
            /* Trace/Debug flags: */
			case 't':
				tprint = atoi(optarg);
				break;
                
            /* Permit (or not) overwriting pre-existing files: */
            case 'R':
                rewrite = optarg;
                /* Check argument against y/Y/n/N */
                if ((strncmp(rewrite,"n",1)==0)||(strncmp(rewrite, "N", 1)==0))
                {
                    fprintf(stderr, "Existing files may NOT be overwritten\n");
                    rew = 0;
                }
                else if ((strncmp(rewrite,"n",1)==0)||
                         (strncmp(rewrite, "N", 1)==0))
                {
                    fprintf(stderr, "Existing files CAN be overwritten\n");
                    rew = 1;
                }
                else 
                {
                    fprintf(stderr, "ERROR: -R may be Y or N not %s\n",
                            rewrite);
                    cmderr++;
                }                
                break;
                
            /* Strictness (Y or N): */
            case 'S':
                strictstr = optarg;
                if ((strncmp(strictstr, "Y", 1) == 0) ||
                    (strncmp(strictstr, "y", 1) == 0))
                {
                    strict = 1;
                }
                else if ((strncmp(strictstr, "N", 1) == 0) ||
                         (strncmp(strictstr, "n", 1) == 0))
                {
                    strict = 0;
                }
                else 
                {
                    fprintf(stderr, 
                            "ERROR: -S (strict) may be Y or N, not %s\n",
                            strictstr);
                    cmderr++;
                }

                break;
				       
            /* Program version display: */
            case 'v':
                fprintf(stderr, "\n");
                fprintf(stderr, "Program %s version: %s / %s\n",
                        progname, PROGRAM_VERSION, PROGRAM_EDIT_DATE);
                fprintf(stderr, "(%s)\n", argv[0]);
                fprintf(stderr, "\n");
                break;
				
            /* Parameter specification error: */
			case ':':       /* -i or -o without operand */
				fprintf(stderr, "ERROR: Option -%c requires an operand\n", 
                        optopt);
				cmderr++;
				break;
                
            /* Unrecognized parameter - error: */
			case '?':
				fprintf(stderr, "ERROR: Unrecognized option: -%c\n", optopt);
				cmderr++;
        }
    }

#ifdef LONGPOINT
    fprintf(stderr,"INFO: Point type is long\n");
#else
    fprintf(stderr,"INFO: Point type is int\n");
#endif
	
    /*
	 ** We must have been passed the input (CSV) Filename
	 */
    if (ifile == (char *)NULL)
    {
        fprintf(stderr, "ERROR: An input file (-i) must be provided\n");
        cmderr++;
    }
    else
    {
        /*
		 ** Does the Input file exist
		 */
        fprintf(stderr,"Opening [%s]\n", ifile);
        if ((ipf = fopen(ifile, "r" )) == (FILE *)NULL)
        {
            fprintf(stderr, "ERROR: Unable to open input file (-i) [%s]\n", 
                    ifile);
            cmderr++;
        }
    }
	
    /*
	 ** We must have been passed the output (SQL) Filename
	 **
	 */
    if (ofile == (char *)NULL)
    {
        fprintf(stderr, "ERROR: An output file (-o) must be provided\n");
        cmderr++;
    }
    else
    {
        /*
		 ** We could see if the Output File already exists - overwrite flag?
		 ** Open the output file
		 */
        if (rew != 0)
        {
            opf = fopen(ofile, "r");
            if (opf != (FILE *)NULL)
            {
                fprintf(stderr, "ERROR: Output file %s already exists -\n"
                                "       Overwriting not permitted\n"
                        "       Run terminates.\n", ofile);
                exit(FAIL_OUTPUT_EXISTS);
            }
        }
        fprintf(stderr,"Opening [%s]\n", ofile);
        if ((opf = fopen(ofile, "w" )) == (FILE *)NULL)
        {
            fprintf(stderr, "ERROR: Unable to open output file (-o) [%s]\n", 
                    ofile);
            cmderr++;
        }
    }
	
    /*
	 ** the Box Flag must be zero or one
	 */
    if (boxflag != 0 && boxflag != 1)
    {
        fprintf(stderr, "ERROR: The Box Flag (-b) must be 0 or 1 (%d)\n", 
                boxflag);
        cmderr++;
    }
	
    /*
	 ** If something went wrong, whinge and die ...
	 */
    if (cmderr)
    {
        fprintf(stderr, "Command Error (%d)\n", cmderr);
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage: %s <options>\n", argv[0]);
        fprintf(stderr, "  Options:-\n");
        fprintf(stderr, "   -i <filename>  Input CSV File     [No Default]\n");
        fprintf(stderr, "   -o <filename>  Output SQL File    [No Default]\n");
        fprintf(stderr, "   -b <0|1>       Build 'box' WHERE  [Def %d]\n",
				defboxflag);
        fprintf(stderr, "   -x <Col Name>  X Coord Col name   [Def '%s']\n",
				XCOL);
        fprintf(stderr, "   -y <Col Name>  Y Coord Col name   [Def '%s']\n",
				YCOL);
        fprintf(stderr, "   -c <corners>   Maximum corners    [Def %d]\n",
                MAXCORNERS);
		fprintf(stderr, "   -t <number>    Trace print level  [Def 0]\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Program %s version: %s / %s\n",
                progname, PROGRAM_VERSION, PROGRAM_EDIT_DATE);
        fprintf(stderr, "(%s)\n", argv[0]);
        fprintf(stderr, "\n");
		
        /*
		 ** Close any files that have been opened
		 */
        tfclose(ipf);
        tfclose(opf);
		exit(ERROR_PARAMETERS);
    }
	
    /*
	 ** Here We Go >>>
	 */
    fprintf(stderr,"\n");
    /*
     ** Set the outermost Box and Diamond to impossible values:
     */
    
    Box.min.x = XYMAX;
    Box.min.y = XYMAX;
    Box.max.x = XYMIN;
    Box.max.y = XYMIN;
    Diamond.top.x = XYMIN;
    Diamond.top.y = XYMIN;
    Diamond.bottom.x = XYMAX;
    Diamond.bottom.y = XYMAX;
    Diamond.eastmost.x = XYMIN;
    Diamond.eastmost.y = XYMIN;
    Diamond.westmost.x = XYMAX;
    Diamond.westmost.y = XYMAX;
    	
    /*
	 ** Build the Rings (by reading the Coords - etc)
	 */
    if ((rings = GetRings(&ipf, &Box, &Diamond)) == (struct ring *)NULL)
    {
        fprintf(stderr, "ERROR: Failed to read Rings (%d)\n", ret);
        
        exit(FAIL_READ_RINGS);
    }
    
	    
    /*
	 ** Done with the Input File now, so we can close it, but check it is open!
	 */
    tfclose(ipf);
    
    /*
	 ** Sort the Rings so 'rings-within-rings' are suitably concentric ordered
	 */
	/*
	 ** IDKK: I *think* we need to find the convex hulls and *then* order the 
     **       rings
	 */
    ringCnt = rings->prev->pos + 1;
    /*
	 ** Build the Convex Hull Rings
	 */
    if (ConvexHullRings(rings) == 0)
    {
        fprintf(stderr, "ERROR: Failed to build Convex Hull\n");
        
        exit(FAIL_BUILD_CONVEX_HULL);
    }
	/*
	 ** Now determine the nesting of the rings, if there is more than one ring:
	 ** NOTE that this is incompatible with the warning issued just above of our
	 ** not supporting Concentric Rings... remove the previous warnings, though, 
	 ** only when this nesting code is complete (NOT YET as of 20121017).
	 */
	if (ringCnt > 1)
	{
		/*
         **  Ensure that each ring has a good box and diamond - thisis achieved
         **  within routine setBoxDiamond, which runs round every ring.
         **  Then call the nesting routine, which returns a pointer to an array
         **  indicating the nesting of the rings. Note that ringNesting allocates
         **  that array, but returning NULL in the event of error.
		 */
       
//        AllDirections(ringCnt, rings);
        setBoxDiamond(rings);
  		nesting = ringNesting( ringCnt, rings );
		/*
		 ** If the "nesting" pointer is still NULL, then we have had an error:
		 */
		if (nesting == (int *)NULL)
		{
			fprintf(stderr, "ERROR: No memory for nesting calculation.\n");
			exit(FAIL_GET_MEMORY_1);		
		}
	}
	else 
	{
	}

    /*
	 ** Build the Box Where Clause
	 **   and remember we did it in the first ring
	 */
    if (boxflag == 0)
    {
        ShowBox(&Box);
        BuildWhereBox(&opf, &Box);
        rings->opand = OPANDAND;
    }
	
    /*
	 ** Build the WHERE Clause(s) from the Rings
	 */
    
	/* !!!! TODO This can be altered to use the nesting information */
    if ((ret = BuildWhereRing(&opf, rings)) != 0)
    {
        fprintf(stderr, "ERROR: Failed to build WHERE Clause(s) (%d)\n", ret);
        
        exit(FAIL_BUILD_WHERE);
    }
	
    /*
	 ** We have to close the braces if we use the Box
	 */
    if (boxflag == 0)
    {
			fprintf(opf, "      -- End WHERE\n");
    }
	
    /*
	 ** Close the files
	 */
    tfclose(opf);
	
    exit(NORMAL_RETURN);
}     /* endof main */      


/*  UTILITY FUNCTIONS  */

/*
 * Conditionally close a file, if the pointer is not NULL
 */
int tfclose (FILE *filep)
{
    if (filep == (FILE *) NULL)
    {
        return (0);
    }
    else 
    {
        return (fclose(filep));
    }
}


/*
 ** End of File
 ** Copyright (c) 2012-2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 */

