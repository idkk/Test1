/******************************************************************************
 Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 cnvshp2csv.c
 (deshape4.c)
 
 Last updated 20131002:10.32
 
 This program reads a *.shp file of types point from the input file, and
 outputs to the output file a csv file of x,y pairs.
 The default input is stdin, but can be specified by parameter -i
 The default output is stdout, but can be specified by parameter -o
 
 Parameter -r (whose default value is 0) indicates the kind of output:
 0  x,y as whole numbers
 2  x,y as decimal numbers (option 0 times 1000 in value)
 1  x,y in both "0" and "2" format - four values per line
 
 Parameter -d (whose default value is 1000) is the scaling factor for the
 format -r 2
 
 Parameter -t (whose default value is 0) indicates whether there is to be
 any trace output
 
 e.g. pgcdeshape -r 0 <myshp.shp  >mypoints.csv
 pgcdeshape -i myshp.shp -o mypoints.csv -r 1
 pgcdeshape -t 1 -r 2 <myshp.shp -o mypoints.csv 2>trace.txt
 
 UPDATES
 Version 0.26
    -i and -o are now mandatory options - even if stdin and stdout are required
 Version 0.27
    -t is now spelled as -T, with -t still available pro tem.
    Minor spelling fixes in messages
 Version 0.28
    -t/-T trace flags all removed
    debug messages all commented out
 Version 0.29
    debug messages and dead code removed
 Version 0.30
    data type truncations and overflows removed
 Version 0.31
    rigorous/lax modes introduced
    detection of valid point pairs
    better float-to-integer conversion
    specifiable rounding allowed
 Version 0.32
    support for polyline
    suppression, and errors, for unsupported shp types
 Version 0.33
    minor efficiency corrections
 Version 0.34
    additions to support other shp types, but non-active
 Version 0.35
    removal of unused code, preparation for other type support
    removal of -r parameter and corresponging format
 
 *****************************************************************************/

#define PROGRAM_VERSION   "0.41"
#define PROGRAM_EDIT_DATE "20131002:10.32"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>

#include "cnvswap.h"
#include "cnvendian.h"
#include "cnvshp2csv.h"

int main (int argc, char *argv[])
{
    FILE *opf     = (FILE *)NULL;    /* The Output file pointer              */
	char fnameout   [NAMELEN+1];     /* Name of the Output file              */
    char *offile  = (char *)NULL;    /* Pointer to output file name          */
    FILE *ipf     = (FILE *)NULL;    /* The Input file pointer               */
    char fnamein    [NAMELEN+1];     /* Name of the Input file               */
    char *iffile  = (char *)NULL;    /* Pointer to input file name           */
    char *rndstr  = (char *)NULL;    /* Pointer to Rounding request          */
    char *strictstr = (char *)NULL;  /* Pointer to Strictness request        */
    char  slash = '/';
    char  c;                         /* Work area, parameter unpacking       */
    int   thisendian  = LITTLE;      /* Default Endianicty of *this* machine */
    int   cmderr = 0;                /* Command error - default OK           */
    long double   denom = 1;         /* Conversion factor for rounding       */
    long double   scaling = 1;       /* Scaling factor                       */
    int   iscaling = 0;              /* Scaling indicator                    */
    int   rounding = 0;              /* Indicate whether rounding or not     */
    int   strict = 0;                /* Indicate whether strict or not       */
    int   tprint = 0;                /* Trace flags - collective             */
    int   maxcorners = MAXCORNERS;   /* Maximum corners permitted            */
    
    char  progname [4096];           /* Name of *this* program               */
    char  temp[4096];                /* Work area                            */
    
    /*
     char outline[128]; 
     */
    int indat[25];                   /* for 1st 100 bytes BIG endian         */
    int flipdat[25];                 /* for 1st 100 bytes FLIPPED            */
    int i, j, k;                     /* Work variables                       */
    int tolerance = 1;               /* Comparison tolerance if strict       */
    int fsize;                       /* File Size                            */
    int rsize;                       /* Record Size                          */
    int versn;                       /* Version                              */
    int stype;                       /* Shape Type                           */
    int ret = 0;
    
    /*  COPYRIGHT  */
    fprintf(stderr,"This program is strictly Copyright (c) 2013 "
            "Westheimer Energy Consultants Ltd.\n"
            "All rights reserved\n\n");
    
    /*  ENDIANNESS  */
    
	/*
	 *  Test and remember the endianness of *this* machine:
	 */
	if (ENDIANNESS == BIG)
	{
		thisendian = BIG;
	}
	else if (ENDIANNESS == LITTLE)
	{
		thisendian = LITTLE;
	}
	else 
	{
		fprintf(stderr,
                "ERROR: ENDIAN format of this machine not supported (%d / %x).\n\n",
                endianness, endianness);
		fprintf(stderr, "[endianness first byte is %x]\n",
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
	/* If there actually was a slash in the program name, we are now pointing 
	 * at it - so move forwards to point to the first character after the
	 * rightmost slash: */
	if ( temp[j] == slash ) 
	{
		j++;
	}
	k = strlen(temp); 
	strncpy(progname, &temp[j], k-j);
	/* Add a terminating string end character: */
	progname[k-j] = '\0';
    
    fprintf(stderr, "Program %s [%s %s] running\n",
            progname, PROGRAM_VERSION, PROGRAM_EDIT_DATE); 
    
    
    /*  COMMAND LINE PROCESSING  */
    
	/*
     *  Read in and initially process each command line parameter:
     */
    
    while ((c = getopt(argc, argv, ":i:o:c:C:d:m:R:S:t:T:v")) != -1)
    {
        switch(c)
        {
            /* Input file: */
            case 'i':
				iffile = optarg;
                strcpy(fnamein, iffile);
                break;
                
            /* Output file: */
            case 'o':
				offile = optarg;
				strcpy(fnameout, offile);
                break;

            case 'm':
                iscaling = atoi(optarg);
                if (iscaling > 0)
                {
                    // scaling = pow(10,iscaling);
                    scaling = iscaling;
                }
                else if (iscaling < 0)
                {
                    // scaling = -1.0 / pow(10,iscaling);
                    /* Scaling factor is *always* positive! */
                    scaling = -1.0 / iscaling;
                }
                else
                {
                    scaling = 1.0;
                }
                /* Scaling factor is *always* positive! */
                /* if (scaling < 0) scaling = -scaling; */
                break;
                
            case 'R':
                rndstr = optarg;
                if ((strncmp(rndstr, "Y", 1)==0) ||
                    (strncmp(rndstr, "y", 1)==0))
                {
                    rounding = 1;
                }
                else if ((strncmp(rndstr, "N", 1)==0) ||
                         (strncmp(rndstr, "n", 1)==0))
                {
                    rounding = 0;
                }
                else 
                {
                    fprintf(stderr, "ERROR: -R may be Yes or No, not \"%s\"\n",rndstr);
                    cmderr++;
                }

                break;
                
            case 'S':
                strictstr = optarg;
                if ((strncmp(strictstr, "Y", 1)==0) ||
                    (strncmp(strictstr, "y", 1)==0))
                {
                    strict = 1;
                }
                else if ((strncmp(strictstr, "N", 1)==0) ||
                         (strncmp(strictstr, "n", 1)==0))
                {
                    strict = 0;
                }
                else 
                {
                    fprintf(stderr, "ERROR: -S may be Yes or No, not \"%s\"\n",
                            strictstr);
                    cmderr++;
                }
                
                break;
                
            case 'C':
                tolerance = atoi(optarg);
                if ((tolerance < 1) || (tolerance > MAXTOL))
                {
                    fprintf(stderr, "ERROR: -C must be between 1 and %d, not %d\n",
                            MAXTOL, tolerance);
                }
                break;
                                
            case 'd':
                denom = atoi(optarg);
                if ((denom < MINDENOM) || (denom > MAXDENOM) || (denom == 0))
                {
                    fprintf(stderr, 
                            "ERROR: valid range for -d is non-zero %f to %d, not %Lf\n", 
                            MINDENOM, MAXDENOM, denom);
                    cmderr++;
                }
                break;
                
            case 'c':
                maxcorners = atoi(optarg);
                if ((maxcorners<1) || (maxcorners>MAXMAXCORNERS))
                {
                    fprintf(stderr, 
                            "ERROR: Maximum corners requested too great - above %d\n",
                            MAXMAXCORNERS);
                    cmderr++;
                }
                break;
                
            case 't':
            case 'T':    
                tprint = atoi(optarg);
                break;
                
            case 'v':
                fprintf(stderr, "Program Version %s %s\n", 
                        PROGRAM_VERSION, PROGRAM_EDIT_DATE);
				fprintf(stderr, "Program Name:   %s : %s\n", progname, argv[0]);
                break;
                
            case '?':
				fprintf(stderr, "ERROR: Unrecognized option: -%c\n", optopt);
				cmderr++;
                
        }
    }
    
    if (iffile == (char *)NULL)
    {
        fprintf(stderr, "ERROR: -i input file not specified\n");
        fprintf(stderr, "       Conversion run terminated (%d)\n",
                FAIL_NOINPUT);
        cmderr++;
    }
    if (offile == (char *)NULL)
    {
        fprintf(stderr, "ERROR: -o output file not specified\n");
        fprintf(stderr, "       Conversion run terminated (%d)\n",
                FAIL_NOOUTPUT);
        cmderr++;
    }
    
    /*
     **  If there is an error in the parameters, tell the user how to call this
     **  program, and exit:
     */
    if (cmderr != 0)
    {
        fprintf(stderr, "ERROR: Command line error\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage: %s <options>\n", argv[0]);
        fprintf(stderr, "  Options:-\n");
        fprintf(stderr, "  -i <filename>   Input  File name   [Default stdin]\n");
        fprintf(stderr, "  -o <filename>   Output File name   [Default stdout]\n");
        fprintf(stderr, "  -d <rounding>   Rounding factor    [Default 1]\n");
        fprintf(stderr, "  -m <scaling>    Scaling multiplier [Default 1]\n");
        fprintf(stderr, "  -R <Y/N>        Apply rounding     [Default No]\n");
        fprintf(stderr, "  -S <Y/N>        Strict input rules [Default No]\n");
        fprintf(stderr, "  -C <tolerance>  Strict tolerance   [Default 1]\n");
        fprintf(stderr, "  -c <maxcorners> Maximum corners    [Default %d]\n",
                MAXCORNERS);
        fprintf(stderr, "  -T <trace>      Trace level        [Default 0]\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Program %s [%s]\n", progname, argv[0]);
        fprintf(stderr, "Version %s %s\n", PROGRAM_VERSION, PROGRAM_EDIT_DATE);
        fprintf(stderr, "\n");
        
        exit (FAIL_COMMAND);
    }
 
    if (iffile == (char *)NULL)
    {
        fprintf(stderr, "ERROR: -i input file not specified\n");
        fprintf(stderr, "       Conversion run terminated (%d)\n",
                FAIL_NOINPUT);
        exit(FAIL_NOINPUT);
    }
    if (offile == (char *)NULL)
    {
        fprintf(stderr, "ERROR: -o output file not specified\n");
        fprintf(stderr, "       Conversion run terminated (%d)\n",
                FAIL_NOOUTPUT);
        exit(FAIL_NOOUTPUT);
    }
    
    /*
     **  If there is an indication of an input file, point to it and open it;
     **  otherwise set the input file to be stdin
     */
    if (strcmp(fnamein,"stdin") == 0)
    {
        ipf = stdin;
    }
    else 
    {
        ipf = fopen(fnamein, "r");
        if (ipf == (FILE *)NULL)
        {
            fprintf(stderr, "ERROR: Cannot open input file %s\n\n", fnamein);
            fprintf(stderr, "       Conversion run terminated (%d)\n",
                    FAIL_INPUT);
            exit (FAIL_INPUT);
        }
    }
    
    /*
     **  Same process for the output file:
     */
    if (strcmp(fnameout,"stdout") == 0)
    {
        opf = stdout;
    }
    else 
    {
        opf = fopen(fnameout, "w");
        if (opf == (FILE *)NULL)
        {
            fprintf(stderr, "ERROR: Cannot open output file %s\n\n", fnameout);
            fprintf(stderr, "       Conversion run terminated (%d)\n",
                    FAIL_OUTPUT);
            exit (FAIL_OUTPUT);
        }
    }
    
    /*
     **  Indicate the parameters being used by this run:
     */
    fprintf(stderr, "Program %s [%s] version %s %s\n", argv[0], progname, 
            PROGRAM_VERSION, PROGRAM_EDIT_DATE);
    fprintf(stderr, "Input  file :    %s\n", fnamein);
    fprintf(stderr, "Output file :    %s\n", fnameout);
    if (rounding == 0)
    {
        fprintf(stderr, "No rounding\n");
    }
    else 
    {
        fprintf(stderr, "Rounding factor: %Lf\n", denom);
    }
    if (iscaling == 0)
    {
        fprintf(stderr, "No scaling\n");
    }
    else
    {
        fprintf(stderr, "Scaling multiply by %Lf\n",scaling);
    }
    if (strict == 0)
    {
        fprintf(stderr, "Strict input rules NOT applied\n");
    }
    else 
    {
        fprintf(stderr, "Strict input rules ARE applied...\n");
        fprintf(stderr, "with tolerance:  %d\n", tolerance);
    }
    fprintf(stderr, "Maximum corners: %d\n", maxcorners);
    if (tprint != 0)
    {
        fprintf(stderr, "Trace flags :  %d\n",tprint);
    }
    /*
     ** read first 100 bytes to get FILESIZE
     **   Position Field        Value       Type    Order
     **   Byte 0   File Code    9994        Integer Big
     **   Byte 4   Unused          0        Integer Big
     **   Byte 8   Unused          0        Integer Big
     **   Byte 12  Unused          0        Integer Big
     **   Byte 16  Unused          0        Integer Big
     **   Byte 20  Unused          0        Integer Big
     **   Byte 24  File Length  File Length Integer Big
     **   Byte 28  Version      1000        Integer Little
     **   Byte 32  Shape Type   Shape Type  Integer Little
     **   Byte 36  Bounding Box Xmin        Double  Little
     **   Byte 44  Bounding Box Ymin        Double  Little
     **   Byte 52  Bounding Box Xmax        Double  Little
     **   Byte 60  Bounding Box Ymax        Double  Little
     **   Byte 68* Bounding Box Zmin        Double  Little
     **   Byte 76* Bounding Box Zmax        Double  Little
     **   Byte 84* Bounding Box Mmin        Double  Little
     **   Byte 92* Bounding Box Mmax        Double  Little
     */
    i = fread(indat, 4, 25, ipf);
    
    /* AGMW
     ** The first 7 are BIG ENDIAN integers and now need to be converted to
     ** Little Endian so we can use them ...
     */
    for (j = 0; j < 7; j++)
    {
        indat[j] = swap_int32(indat[j]);
    }
    for (j = 0; j < 25; j++)
    {
        flipdat[j] = swap_int32(indat[j]);
    }
    
    /*
     ** Pick out The File Size, version, and Shape Type
     **   Value Shape Type    Value Shape Type
     **      0  Null Shape      15  PolygonZ
     **      1  Point           18  MultiPointZ
     **      3  PolyLine        21  PointM
     **      5  Polygon         23  PolyLineM
     **      8  MultiPoint      25  PolygonM
     **     11  PointZ          28  MultiPointM
     **     13  PolyLineZ       31  MultiPatch
     */
    fsize = indat[6] * 2;    /* for 16 bit words */
    versn = indat[7];
    stype = indat[8];
    fprintf(stderr, "Ver %d file size = %d type = %d\n", versn, fsize, stype); 
    fsize = fsize - 100;   
        
    switch (stype)
    {
        /*
         *  Call the procedure that deals with this type of input file. Note
         *  that we have commented out, here, all those types that are not yet
         *  fully supported - though we have inserted the procedure calls to
         *  the processing routines.
         */
//        case SHP_NULL:
//            ret = ReadNullshape(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
//            break;
            
        case SHP_POINT:
            rsize = POINT_RECORD_LENGTH;
            ret = ReadPoint  (ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance, 
                              fsize, rsize, maxcorners);
            break;
            
        case SHP_POLYLINE:
            ret = ReadPolyline(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
            break;
            
        case SHP_POLYGON:
            ret =  ReadPolygon(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
            break;

//        case SHP_MULTIPOINT:
//            ret = ReadMultipoint(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
//            break;
            
//        case SHP_POINTZ:
//            rsize = POINTZ_RECORD_LENGTH;
//            ret = ReadPointZ(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance,
//                             fsize, rsize, maxcorners);
//            break;
            
//        case SHP_POLYLINEZ:
//            ret = ReadPolylineZ(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
//            break;
            
//        case SHP_POLYGONZ:
//            ret = ReadPolygonZ(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
//            break;
            
//        case SHP_MULTIPOINTZ:
//            ret = ReadMultipointZ(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
//            break;
            
//        case SHP_POINTM:
//            rsize = POINTM_RECORD_LENGTH;
//            ret = ReadPointM(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
//            break;
                        
//        case SHP_POLYLINEM:
//            ret = ReadPolylineM(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
//            break;
            
//        case SHP_POLYGONM:
//            ret = ReadPolygonM(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
//            break;
            
//        case SHP_MULTIPOINTM:
//            ret = ReadMultipointM(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
//            break;
            
//        case SHP_MULTIPATCH:
//            ret = ReadMultipatch(ipf, opf, rounding, denom, iscaling, scaling, strict, tolerance);
//            break;
            
        default:
            fprintf(stderr, "ERROR: Version type %d not supported\n\n", stype);
            /*
             *  Indicate the type of the file - if we can recignise it
             */
        {
            ret = FAIL_VERSION;
            if (stype == SHP_NULL)
            {
                fprintf(stderr, "       Shape file: Null Shape\n");
                ret = FAIL_NULLSHAPE;
            } 
            else if (stype == SHP_POINT)
            {
                fprintf(stderr, "       Shape file: Point File\n");
                ret = FAIL_POINT;
            }
            else if (stype == SHP_MULTIPOINT)
            {
                fprintf(stderr, "       Shape file: Multipoint File\n");
                ret = FAIL_MULTIPOINT;
            }
            else if (stype == SHP_POINTZ)
            {
                fprintf(stderr, "       Shape file: PointZ File\n");  
                ret = FAIL_POINTZ;
            }
            else if (stype == SHP_POLYLINEZ)
            {
                fprintf(stderr, "       Shape file: PolylineZ File\n");
                ret = FAIL_POLYLINEZ;
            }
            else if (stype == SHP_POLYGONZ)
            {
                fprintf(stderr, "       Shape file: PolygonZ File\n");
                ret = FAIL_POLYGONZ;
            }
            else if (stype == SHP_MULTIPOINTZ)
            {
                fprintf(stderr, "       Shape file: MultipointZ File\n");
                ret = FAIL_MULTIPOINTZ;
            }
            else if (stype == SHP_POINTM)
            {
                fprintf(stderr, "       Shape file: PointM File\n");
                ret = FAIL_POINTM;
            }
            else if (stype == SHP_POLYLINEM)
            {
                fprintf(stderr, "       Shape file: PolylineM File\n");
                ret = FAIL_POLYLINEM;
            }
            else if (stype == SHP_POLYGONM)
            {
                fprintf(stderr, "       Shape file: PolygonM File\n");
                ret = FAIL_POLYGONM;
            }
            else if (stype == SHP_MULTIPOINTM)
            {
                fprintf(stderr, "       Shape file: MultipointM File\n");
                ret = FAIL_MULTIPOINTM;
            }
            else if (stype == SHP_MULTIPATCH)
            {
                fprintf(stderr, "       Shape file: Multipatch File\n");
                ret = FAIL_MULTIPATCH;
            }
        }
            fprintf(stderr, "       Conversion run terminated (%d)\n", ret);
            exit(ret);
    }
    
    
    exit(ret);
}     /* endof main */    

/*
 *  Deal with a Null Shape - this is an empty entity:
 */
int ReadNullshape  (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance)
{
    return (0);
}

/*
 *  Deal with a Points file. Each of the points is read, and its X, Y is
 *  sent to the output, with the rounding that has been requested.
 */
int ReadPoint  (FILE *ipf, FILE *opf, int rounding, long double denom, 
                int iscaling, long double scaling, int strict, int tolerance, int fsize, int rsize,
                int maxcorners)
{
    double xval;
    double yval;
    long double dxval;
    long double dyval;
    int x, y;               /* Work variables                  */
    long int lx, ly;
    int reccnt = 0;
    int   corners = 0;               /* Number of corners found         */
    unsigned char readin[POINT_RECORD_LENGTH];   /* for shape records LITTLE Endian */
    int xprev = 0, yprev = 0;        /* Previous x, y values            */

    
        fprintf(stderr, "Filesize now %d, expect %d records \n", 
                fsize, (fsize / rsize));    
        
        /* read & write x,y csv file */
        while (fsize >= rsize)
        {
            /* read all the bytes for the 'next' record into unsigned */ 
            fread(readin, rsize, 1, ipf);     
            
            /*
             **  Count the number of points, and check that this is not greater
             **  than is permitted for this run. If it is, then error exit:
             */
            corners++;
            if (corners>maxcorners)
            {
                fprintf(stderr, 
                        "ERROR: Number of points %d is greater than permitted %d\n", 
                        corners, maxcorners);
                fprintf(stderr, "       Conversion run terminated (%d)\n",
                        FAIL_CORNERS);
                exit (FAIL_CORNERS);
            }
                        
            /* 
             ** Move the 8 bytes into the X Value - assume we are a LITTLE-
             ** endian machine
             */
            memcpy(&xval, &readin[12],  8);
            
            /* IDKK
             * Perform rounding, if requested, and always convert to int:
             */
            if (rounding == 0)
            {
                if (iscaling == 0)
                {
                    x = (int) xval;
                }
                else
                {
                    x = (int) (xval * scaling);
                }
            }
            else 
            {
                dxval = xval * denom * scaling;
                lx = (long int)dxval;
                x = lx / denom;
            }
            
            /* AGMW
             ** Ditto for 'y'
             */
            memcpy(&yval, &readin[20],  8);
            
            /* IDKK
             * Round if required, and always convert to type int:
             */
            
            if (rounding == 0)
            {
                if (iscaling == 0)
                {
                    y = (int) yval;
                }
                else
                {
                    y = (int) (yval * scaling);
                }
            }
            else 
            {
                dyval = yval * denom * scaling;
                ly = (long int)dyval;
                y = ly / denom;
            }
            
            /*
             *  If we are aplying strict rules, check whether x or y are within
             *  tolerance of the previous x and y:
             */
            if (strict != 0)
            {
                if ((abs(x - xprev) < tolerance) ||
                    (abs(y - yprev) < tolerance))
                {
                    fprintf(stderr, 
                            "ERROR: this point (%d, %d) is within tolerance (%d)\n"
                            "       of previous point (%d, %d).\n"
                            "       Run terminates.\n", x, y, tolerance, 
                            xprev, yprev);
                    exit (FAIL_STRICT);
                }
            }
            xprev = x;
            yprev = y;
            
            /* write to output */  
            fprintf(opf, "%d, %d\n", x, y);
            
            /* reduce file size */
            fsize = fsize - POINT_RECORD_LENGTH;
            
            reccnt++;
        }
        
        fprintf(stderr, "Number of records (Points) = %d (fsize=%d)\n\n", reccnt, fsize);
    
    
    return (0);
}

/*
 ** Function: ReadPolyline()
 ** Args: Pointer to input file; pointer to output file
 ** Returns : 0 if OK, 1 for problems
 **
 ** In version 0.35 this outputs only the bounding box, with the first point
 ** repeated at the end, so that this output looks a bit like the output from
 ** a Polygon type file, but with just a single polygon. This, in general, is
 ** not good enough, but in particualr it IS good enough - and probably will be
 ** so for all real-world input that we encounter
 */
int ReadPolyline(FILE *ipf, FILE *opf, int rounding, long double denom,
                int iscaling, long double scaling, int strict, int tolerance)
{
    struct recordheader {
        int    recordNo;          /* Big Endian    */
        int    contentLen;        /* Big Endian    */
    };
    struct polylineheader {
        struct recordheader rh;
        int    shapeType;         /* Little Endian */
        char   boxXMin[8];        /* Little Endian */
        char   boxXMax[8];        /* Little Endian */
        char   boxYMin[8];        /* Little Endian */
        char   boxYMax[8];        /* Little Endian */
        int    numParts;          /* Little Endian */
        int    numPoints;         /* Little Endian */
    } pgh;
    int reccnt = 0;
    int r;
    int xval, yval;
    int xprev, yprev;
    double dblX;
    double dblY;
    int showXmin, showXmax, showYmin, showYmax = 0;
    
    xprev = 0;
    yprev = 0;
    /*
     ** Read in the Polyline Header Info
     */
    r = fread(&pgh, sizeof(struct polylineheader), 1, ipf);
    if (r != 1)
    {
        fprintf(stderr, "ERROR: Failed to read Polyline Header Info (%d)\n\n", 
                r);
        fprintf(stderr, "       Conversion run terminated (%d)\n",
                FAIL_POLYLINE);       
        return(FAIL_POLYLINE);
    }
    
    fprintf(stderr, "INFO:  Polyline Number of Parts:  %d\n",pgh.numParts);
    fprintf(stderr, "INFO:  Polyline Number of Points: %d\n",pgh.numPoints);

    /*
     ** Endian Flip the Record Header integers so we can display 'em
     */
    pgh.rh.recordNo   = swap_int32(pgh.rh.recordNo);
    pgh.rh.contentLen = swap_int32(pgh.rh.contentLen);
    
    /*
     ** Check it all seems reasonable so far
     */
    if (pgh.shapeType != SHP_POLYLINE)
    {
        fprintf(stderr, "ERROR: Incorrect Shape Type found (%d)\n\n", 
                pgh.shapeType);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_SHAPE);       
        return(FAIL_SHAPE);
    }
    
    if (pgh.numParts < 1)
    {
        fprintf(stderr, "ERROR: Must have at least 1 'Part' (%d)\n\n", 
                pgh.numParts);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_PARTS);        
        return(FAIL_PARTS);
    }
    
  
    memcpy(&dblX, pgh.boxXMin, 8);
    memcpy(&dblY, pgh.boxYMin, 8);
    xval = (int)dblX;
    yval = (int)dblY;
    fprintf(stderr, "INFO:  XMin : XMax = %d : %d\n", xval, yval);
    showXmin = xval;
    showXmax = yval;
    /*
     *  At this time, xval and yval contain the box minima ...
     *  well, that's what it SHOULD be according to the ESRI dpocumentation,
     *  but actually these are the Xmin and Xmax. Similary for the (following)
     *  pair...
     */
    memcpy(&dblX, pgh.boxXMax, 8);
    memcpy(&dblY, pgh.boxYMax, 8);
    xval = (int)dblX;
    yval = (int)dblY;
    fprintf(stderr, "INFO:  YMin : YMax = %d : %d\n", xval, yval);
    showYmin = xval;
    showYmax = yval;
    /*
     *  And now xval and yval contain the box maxima
     */
    
    /*
     *  For this kind of input file we output only five lines:
     *    Xmin Ymin
     *    Xmin Ymax
     *    Xmax Ymax
     *    Xmax Ymin
     *    Xmin Ymin (a repeat of the first line)
     */
    if (rounding != 0)
    {
        showXmin = showXmin * denom * scaling;
        showXmin = (int) (showXmin / denom);
        showXmax = showXmax * denom * scaling;
        showXmax = (int) (showXmax / denom);
        showYmin = showYmin * denom * scaling;
        showYmin = (int) (showYmin / denom);
        showYmax = showYmax * denom * scaling;
        showYmax = (int) (showYmax / denom);
    }
    else
    {
        if (iscaling != 0)
        {
            showXmin = (int) (showXmin * scaling);
            showXmax = (int) (showXmax * scaling);
            showYmin = (int) (showYmin * scaling);
            showYmax = (int) (showYmax * scaling);
        }
        else
        {
            showXmin = (int) showXmin;
            showXmax = (int) showXmax;
            showYmin = (int) showYmin;
            showYmin = (int) showYmax;
        }
    }
    fprintf(opf, "%d, %d\n", showXmin, showYmin);
    reccnt++;
    fprintf(opf, "%d, %d\n", showXmin, showYmax);
    reccnt++;
    fprintf(opf, "%d, %d\n", showXmax, showYmax);
    reccnt++;
    fprintf(opf, "%d, %d\n", showXmax, showYmin);
    reccnt++;
    fprintf(opf, "%d, %d\n", showXmin, showYmin);
    reccnt++;
    fprintf(stderr, "INFO:  Number of output records (Polyline) = %d\n", reccnt);
    return(0);
}

/*
 ** Function: ReadPolygon()
 ** Args: Pointer to input file; pointer to output file
 ** Returns : 0 if OK, 1 for problems
 ** Notes:
 **   Position Field           Value           Type     Number     Endian
 **   Byte 0   Record Number   Record Number   Integer      1      Big
 **   Byte 4   Content Length  Content Length  Integer      1      Big
 **
 **   Byte 0   Shape Type          5           Integer      1      Little
 **   Byte 4   Box             Box             Double       4      Little
 **   Byte 36  NumParts        NumParts        Integer      1      Little
 **   Byte 40  NumPoints       NumPoints       Integer      1      Little
 **   Byte 44  Parts           Parts           Integer  NumParts   Little
 **   Byte X   Points          Points          Point    NumPoints  Little
 **
 ** For every input polygon we output each of the points, including the repeat
 ** of the first point in the polygon. The ordering of the points in the output
 ** is the same as in the input, to retain the clockwise/anticlockwise signal of
 ** which polygons are 'include' and which 'exclude'.
 */
int ReadPolygon(FILE *ipf, FILE *opf, int rounding, long double denom,
                int iscaling, long double scaling, int strict, int tolerance)
{
    struct points {
        double pointX;            /* Little Endian */
        double pointY;            /* Little Endian */
    } pnts;
    struct recordheader {
        int    recordNo;          /* Big Endian    */
        int    contentLen;        /* Big Endian    */
    };
    struct polygonheader {
        struct recordheader rh;
        int    shapeType;         /* Little Endian */
        char   boxXMin[8];        /* Little Endian */
        char   boxXMax[8];        /* Little Endian */
        char   boxYMin[8];        /* Little Endian */
        char   boxYMax[8];        /* Little Endian */
        int    numParts;          /* Little Endian */
        int    numPoints;         /* Little Endian */
    } pgh;
    int reccnt = 0;
    int r;
    int i;
    int xval, yval;
    int xprev, yprev;
    int *parts = (int *)NULL;           /* ptr to Array of Integers */
    double dblX;
    double dblY;
    
    xprev = 0;
    yprev = 0;
    /*
     ** Read in the Polygon Header Info
     */
    r = fread(&pgh, sizeof(struct polygonheader), 1, ipf);
    if (r != 1)
    {
        fprintf(stderr, "ERROR: Failed to read Polygon Header Info (%d)\n\n", 
                r);
        fprintf(stderr, "       Conversion run terminated (%d)\n",
                FAIL_POLYGON);       
        return(FAIL_POLYGON);
    }
    
    fprintf(stderr, "INFO:  Polygon Number of Parts:  %d\n",pgh.numParts);
    fprintf(stderr, "INFO:  Polygon Number of Points: %d\n",pgh.numPoints);
        
    /*
     ** Endian Flip the Record Header integers so we can display 'em
     */
    pgh.rh.recordNo   = swap_int32(pgh.rh.recordNo);
    pgh.rh.contentLen = swap_int32(pgh.rh.contentLen);
    
    /*
     ** Check it all seems reasonable so far
     */
    if ((pgh.shapeType != SHP_POLYGON) & (pgh.shapeType != SHP_POLYGONM) &
        (pgh.shapeType != SHP_POLYGONZ))
    {
        fprintf(stderr, "ERROR: Incorrect Shape Type found (%d)\n\n", 
                pgh.shapeType);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_SHAPE);       
        return(FAIL_SHAPE);
    }
    
    if (pgh.numParts < 1)
    {
        fprintf(stderr, "ERROR: Must have at least 1 'Part' (%d)\n\n", 
                pgh.numParts);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_PARTS);        
        return(FAIL_PARTS);
    }
    
    /*
     ** Grab some space, then Read the parts[numParts] integer array into it
     */
    parts = (int *)malloc(pgh.numParts * sizeof(int));
    if (parts == NULL)
    {
        fprintf(stderr, "ERROR: Unable to allocate space for Parts (%d)\n\n",
                pgh.numParts);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_MALLOC);       
        return(FAIL_MALLOC);
    }
    r = fread(parts, pgh.numParts * sizeof(int), 1, ipf);
    if (r != 1)
    {
        fprintf(stderr, "ERROR: Failed to read Polygon No Parts array (%d)\n\n", 
                r);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_ARRAY);       
        free(parts);
        return(FAIL_ARRAY);
    }
    
    /*
     ** Read the Points
     */
    for (i = 0; i < pgh.numPoints; i++)
    {
        r = fread(&pnts, sizeof(struct points), 1, ipf);
        if (r != 1)
        {
            fprintf(stderr, "ERROR: Failed to read Polygon Points %d (%d)\n", 
                    i, r);
            fprintf(stderr, "       Conversion run terminated (%d)\n",
                    FAIL_POINTS);           
            free(parts);
            return(FAIL_POINTS);
        }
        
        /*
         ** Convert them to the output size/format and print 'em
         */
        /*
         *  Convert to type int, and if Rounding has been requested,
         *  apply that rounding:
         */
        if (rounding == 0)
        {
            if (iscaling == 0)
            {
               xval = (int) pnts.pointX;
               yval = (int) pnts.pointY;
            }
            else
            {
               xval = (int) (pnts.pointX * scaling);
               yval = (int) (pnts.pointY * scaling);
            }
        }
        else 
        {
            dblX = pnts.pointX * denom * scaling;
            xval = (int) (dblX / denom);
            dblY = pnts.pointY * denom * scaling;
            yval = (int) (dblY / denom);
        }
        
        /*
         *  If we are applying strict rules, check whether these x, y values
         *  are within tolerance of the previous x, y values:
         */
        
        if (strict != 0)
        {
            if ((abs(xprev - xval) < tolerance) ||
                (abs(yprev - yval) < tolerance))
            {
                fprintf(stderr, "ERROR: Previous polygon point (%d, %d) is within\n"
                        "       tolerance (%d) of previous point (%d, %d)\n"
                        "       Run terminates.\n", xval, yval, tolerance,
                        xprev, yprev);
                exit (FAIL_STRICT2);
            }
        }
        xprev = xval;
        yprev = yval;
        
        fprintf(opf, "%d, %d\n", xval, yval);
        reccnt++;
    }
    
    fprintf(stderr, "INFO:  Number of output records (Polygon) = %d\n", reccnt);
    
    free(parts);
    return(0);
}

/*
 *  Process a MultiPoint type file. This (in version 0.35) is treated the same
 *  as for a Polyline, in that only the bounding box is written to the output,
 *  with the first point repeated.
 *  ALTERNATE interpretation is to view this is being the same as a Point file,
 *  but allowing for the extra Measure field.
 */
int ReadMultipoint(FILE *ipf, FILE *opf, int rounding, long double denom, 
                   int iscaling, long double scaling, int strict, int tolerance)
{    
    struct points {
            double pointX;            /* Little Endian */
            double pointY;            /* Little Endian */
        } pnts;
    struct recordheader {
        int    recordNo;          /* Big Endian    */
        int    contentLen;        /* Big Endian    */
    };
    struct multipointheader {
        struct recordheader rh;
        int    shapeType;         /* Little Endian */
        char   boxXMin[8];        /* Little Endian */
        char   boxXMax[8];        /* Little Endian */
        char   boxYMin[8];        /* Little Endian */
        char   boxYMax[8];        /* Little Endian */
        int    numPoints;         /* Little Endian */
    } pgh;
    int reccnt = 0;
    int r;
    int i;
    int xval, yval;
    int xprev, yprev;
    double dblX;
    double dblY;
    int showXmin, showXmax, showYmin, showYmax = 0;
    
    xprev = 0;
    yprev = 0;
    /*
     ** Read in the Multipoint Header Info
     */
    r = fread(&pgh, sizeof(struct multipointheader), 1, ipf);
    if (r != 1)
    {
        fprintf(stderr, "ERROR: Failed to read Multipoint Header Info (%d)\n\n", 
                r);
        fprintf(stderr, "       Conversion run terminated (%d)\n",
                FAIL_MULTIPOINT);       
        return(FAIL_MULTIPOINT);
    }
    
    fprintf(stderr, "INFO:  Multipoint Number of Points: %d\n",pgh.numPoints);
    
    /*
     ** Endian Flip the Record Header integers so we can display 'em
     */
    pgh.rh.recordNo   = swap_int32(pgh.rh.recordNo);
    pgh.rh.contentLen = swap_int32(pgh.rh.contentLen);
    
    /*
     ** Check it all seems reasonable so far
     */
    if (pgh.shapeType != SHP_MULTIPOINT)
    {
        fprintf(stderr, "ERROR: Incorrect Shape Type found (%d)\n\n", 
                pgh.shapeType);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_SHAPE);       
        return(FAIL_SHAPE);
    }
    
    
    memcpy(&dblX, pgh.boxXMin, 8);
    memcpy(&dblY, pgh.boxYMin, 8);
    xval = (int)dblX;
    yval = (int)dblY;
    fprintf(stderr, "INFO:  XMin : XMax = %d : %d\n", xval, yval);
    showXmin = xval;
    showXmax = yval;
    /*
     *  At this time, xval and yval contain the box minima ...
     *  well, that's what it SHOULD be according to the ESRI dpocumentation,
     *  but actually these are the Xmin and Xmax. Similary for the (following)
     *  pair...
     */
    memcpy(&dblX, pgh.boxXMax, 8);
    memcpy(&dblY, pgh.boxYMax, 8);
    xval = (int)dblX;
    yval = (int)dblY;
    fprintf(stderr, "INFO:  YMin : YMax = %d : %d\n", xval, yval);
    showYmin = xval;
    showYmax = yval;
    /*
     *  And now xval and yval contain the box maxima
     */
    
#if (0==1)
    /*
     *  For this kind of input file we output only five lines:
     *    Xmin Ymin
     *    Xmin Ymax
     *    Xmax Ymax
     *    Xmax Ymin
     *    Xmin Ymin (a repeat of the first line)
     */
    if (rounding != 0)
    {
        showXmin = showXmin * denom * scaling;
        showXmin = (int) (showXmin / denom);
        showXmax = showXmax * denom * scaling;
        showXmax = (int) (showXmax / denom);
        showYmin = showYmin * denom * scaling;
        showYmin = (int) (showYmin / denom);
        showYmax = showYmax * denom * scaling;
        showYmax = (int) (showYmax / denom);
    }
    else
    {
        if (iscaling != 0)
        {
            showXmin = (int) (showXmin * scaling);
            showXmax = (int) (showXmax * scaling);
            showYmin = (int) (showYmin * scaling);
            showYmax = (int) (showYmax * scaling);
        }
        else
        {
            showXmin = (int) showXmin;
            showXmax = (int) showXmax;
            showYmin = (int) showYmin;
            showYmax = (int) showYmax;
        }
    }
    fprintf(opf, "%d, %d\n", showXmin, showYmin);
    reccnt++;
    fprintf(opf, "%d, %d\n", showXmin, showYmax);
    reccnt++;
    fprintf(opf, "%d, %d\n", showXmax, showYmax);
    reccnt++;
    fprintf(opf, "%d, %d\n", showXmax, showYmin);
    reccnt++;
    fprintf(opf, "%d, %d\n", showXmin, showYmin);
    reccnt++;
    fprintf(stderr, "INFO:  Number of output records (Multipoint) = %d\n", reccnt);
#else    
    
    /*
     ** Read the Points
     */
    for (i = 0; i < pgh.numPoints; i++)
    {
        r = fread(&pnts, sizeof(struct points), 1, ipf);
        if (r != 1)
        {
            fprintf(stderr, "ERROR: Failed to read MultiPoint Points %d (%d)\n", 
                    i, r);
            fprintf(stderr, "       Conversion run terminated (%d)\n",
                    FAIL_POINTS);           
            return(FAIL_POINTS);
        }
        
        /*
         ** Convert them to the output size/format and print 'em
         **  SEGY data is stored as INTEGERs, so we need to do the same
         fprintf(stdout, "%f, %f\n", pnts.pointX, pnts.pointY);
         */
        /*
         *  Convert to type int, and if Rounding has been requested,
         *  apply that rounding:
         */
        if (rounding == 0)
        {
            if (iscaling == 0)
            {
               xval = (int) pnts.pointX;
               yval = (int) pnts.pointY;
            }
            else
            {
               xval = (int) (pnts.pointX * scaling);
               yval = (int) (pnts.pointY * scaling);
            }
        }
        else 
        {
            dblX = pnts.pointX * denom * scaling;
            xval = (int) (dblX / denom);
            dblY = pnts.pointY * denom * scaling;
            yval = (int) (dblY / denom);
        }
        
        /*
         *  If we are applying strict rules, check whether these x, y values
         *  are within tolerance of the previous x, y values:
         */
        
        if (strict != 0)
        {
            if ((abs(xprev - xval) < tolerance) ||
                (abs(yprev - yval) < tolerance))
            {
                fprintf(stderr, "ERROR: Previous Multipoint point (%d, %d) is within\n"
                        "       tolerance (%d) of previous point (%d, %d)\n"
                        "       Run terminates.\n", xval, yval, tolerance,
                        xprev, yprev);
                exit (FAIL_STRICT2);
            }
        }
        xprev = xval;
        yprev = yval;
        
        fprintf(opf, "%d, %d\n", xval, yval);
        reccnt++;
    }
    
    fprintf(stderr, "INFO:  Number of output records (Multipoint) = %d\n", reccnt);
#endif
    
    return (0);
}

int ReadPointZ(FILE *ipf, FILE *opf, int rounding, long double denom, 
               int iscaling, long double scaling, int strict, int tolerance, int fsize, int rsize, int maxcorners)
{
    double xval;
    double yval;
    long double dxval;
    long double dyval;
    int x, y;               /* Work variables                  */
    long int lx, ly;
    int reccnt = 0;
    int   corners = 0;               /* Number of corners found         */
    unsigned char readin[POINTZ_RECORD_LENGTH];   /* for shape records LITTLE Endian */
    int xprev = 0, yprev = 0;        /* Previous x, y values            */
    
    
    fprintf(stderr, "Filesize (PointZ) now %d, expect %d records \n", 
            fsize, (fsize / rsize));    
    
    /* read & write x,y csv file */
    while (fsize >= rsize)
    {
        /* read all the bytes for the 'next' record into unsigned */ 
        fread(readin, rsize, 1, ipf);     
        
        /*
         **  Count the number of points, and check that this is not greater
         **  than is permitted for this run. If it is, then error exit:
         */
        corners++;
        if (corners>maxcorners)
        {
            fprintf(stderr, 
                    "ERROR: Number of points %d is greater than permitted %d\n", 
                    corners, maxcorners);
            fprintf(stderr, "       Conversion run terminated (%d)\n",
                    FAIL_CORNERS);
            exit (FAIL_CORNERS);
        }
        
        /* 
         ** Move the 8 bytes into the X Value - assume we are a LITTLE-
         ** endian machine
         */
        memcpy(&xval, &readin[12],  8);
        
        /* IDKK
         * Perform rounding, if requested, and always convert to int:
         */
        if (rounding == 0)
        {
            if (iscaling == 0)
            {
                x = (int) xval;
            }
            else
            {
                x = (int) (xval * scaling);
            }
        }
        else 
        {
            dxval = xval * denom * scaling;
            lx = (long int)dxval;
            x = lx / denom;
        }
        
        /* AGMW
         ** Ditto for 'y'
         */
        memcpy(&yval, &readin[20],  8);
        
        /* IDKK
         * Round if required, and always convert to type int:
         */
        
        if (rounding == 0)
        {
            if (iscaling == 0)
            {
               y = (int) yval;
            }
            else
            {
               y = (int) (yval * scaling);
            }
        }
        else 
        {
            dyval = yval * denom * scaling;
            ly = (long int)dyval;
            y = ly / denom;
        }
        
        /*
         *  If we are aplying strict rules, check whether x or y are within
         *  tolerance of the previous x and y:
         */
        if (strict != 0)
        {
            if ((abs(x - xprev) < tolerance) ||
                (abs(y - yprev) < tolerance))
            {
                fprintf(stderr, 
                        "ERROR: this point (%d, %d) is within tolerance (%d)\n"
                        "       of previous point (%d, %d) [PointZ].\n"
                        "       Run terminates.\n", x, y, tolerance, 
                        xprev, yprev);
                exit (FAIL_STRICT);
            }
        }
        xprev = x;
        yprev = y;
        
        /* write to output */  
        fprintf(opf, "%d, %d\n", x, y);
        
        /* reduce file size */
        fsize = fsize - POINTZ_RECORD_LENGTH;
        
        reccnt++;
    }
    
    fprintf(stderr, "Number of records (PointsZ) = %d (fsize=%d)\n\n", reccnt, fsize);
    
    
    return (0);
}

int ReadPolylineZ(FILE *ipf, FILE *opf, int rounding, long double denom, 
                  int iscaling, long double scaling, int strict, int tolerance)
{
    return (0);
}

int ReadPolygonZ(FILE *ipf, FILE *opf, int rounding, long double denom, 
                 int iscaling, long double scaling, int strict, int tolerance)
{
    int ret = 0;
    
    struct points {
        double pointX;            /* Little Endian */
        double pointY;            /* Little Endian */
    } pnts;
    struct recordheader {
        int    recordNo;          /* Big Endian    */
        int    contentLen;        /* Big Endian    */
    };
    struct polygonzheader {
        struct recordheader rh;
        int    shapeType;         /* Little Endian */
        char   boxXMin[8];        /* Little Endian */
        char   boxXMax[8];        /* Little Endian */
        char   boxYMin[8];        /* Little Endian */
        char   boxYMax[8];        /* Little Endian */
        int    numParts;          /* Little Endian */
        int    numPoints;         /* Little Endian */
    } pgh;
#if (0==1)
    struct polygonzpair {
        double Qmin;
        double Qmax;
    } pgzp;
#endif
    int reccnt = 0;
    int r;
    int i;
    int xval, yval;
    int xprev, yprev;
    int *parts = (int *)NULL;           /* ptr to Array of Integers */
#if (0==1)
    double *headers = (double *)NULL;   /* ptr to Array of pgzp     */
#endif
    double dblX;
    double dblY;
    
    xprev = 0;
    yprev = 0;
    /*
     ** Read in the Polygon Header Info
     */
    r = fread(&pgh, sizeof(struct polygonzheader), 1, ipf);
    if (r != 1)
    {
        fprintf(stderr, "ERROR: Failed to read PolygonZ Header Info (%d)\n\n", 
                r);
        fprintf(stderr, "       Conversion run terminated (%d)\n",
                FAIL_POLYGON);       
        return(FAIL_POLYGON);
    }
    
    fprintf(stderr, "INFO:  PolygonZ Number of Parts:  %d\n",pgh.numParts);
    fprintf(stderr, "INFO:  PolygonZ Number of Points: %d\n",pgh.numPoints);
    
    /*
     ** Endian Flip the Record Header integers so we can display 'em
     */
    pgh.rh.recordNo   = swap_int32(pgh.rh.recordNo);
    pgh.rh.contentLen = swap_int32(pgh.rh.contentLen);
    
    /*
     ** Check it all seems reasonable so far
     */
    if (pgh.shapeType != SHP_POLYGONZ)
    {
        fprintf(stderr, "ERROR: Incorrect Shape Type found (%d)\n\n", 
                pgh.shapeType);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_SHAPE);       
        return(FAIL_SHAPE);
    }
    
    if (pgh.numParts < 1)
    {
        fprintf(stderr, "ERROR: Must have at least 1 'Part' (%d)\n\n", 
                pgh.numParts);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_PARTS);        
        return(FAIL_PARTS);
    }
    
    /*
     ** Grab some space, then Read the parts[numParts] integer array into it
     */
    parts = (int *)malloc(pgh.numParts * sizeof(int));
    if (parts == NULL)
    {
        fprintf(stderr, "ERROR: Unable to allocate space for Parts (%d)\n\n",
                pgh.numParts);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_MALLOC);       
        return(FAIL_MALLOC);
    }
    r = fread(parts, pgh.numParts * sizeof(int), 1, ipf);
    if (r != 1)
    {
        fprintf(stderr, 
                "ERROR: Failed to read PolygonZ No Parts array (%d)\n\n", r);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_ARRAY);       
        free(parts);
        return(FAIL_ARRAY);
    }
       
    /*
     ** Read the Points
     */
    for (i = 0; i < pgh.numPoints; i++)
    {
        r = fread(&pnts, sizeof(struct points), 1, ipf);
        if (r != 1)
        {
            fprintf(stderr, "ERROR: Failed to read PolygonZ Points %d (%d)\n", 
                    i, r);
            fprintf(stderr, "       Conversion run terminated (%d)\n",
                    FAIL_POINTS);           
            free(parts);
            return(FAIL_POINTS);
        }
        
        /*
         ** Convert them to the output size/format and print 'em
         */
        /*
         *  Convert to type int, and if Rounding has been requested,
         *  apply that rounding:
         */
        if (rounding == 0)
        {
            if (iscaling == 0)
            {
                xval = (int) pnts.pointX;
                yval = (int) pnts.pointY;
            }
            else
            {
                xval = (int) (pnts.pointX * scaling);
                yval = (int) (pnts.pointY * scaling);
            }
        }
        else 
        {
            dblX = pnts.pointX * denom * scaling;
            xval = (int) (dblX / denom);
            dblY = pnts.pointY * denom * scaling;
            yval = (int) (dblY / denom);
        }
        
        /*
         *  If we are applying strict rules, check whether these x, y values
         *  are within tolerance of the previous x, y values:
         */
        
        if (strict != 0)
        {
            if ((abs(xprev - xval) < tolerance) ||
                (abs(yprev - yval) < tolerance))
            {
                fprintf(stderr, "ERROR: Previous PolygonZ point (%d, %d) is within\n"
                        "       tolerance (%d) of previous point (%d, %d)\n"
                        "       Run terminates.\n", xval, yval, tolerance,
                        xprev, yprev);
                exit (FAIL_STRICT2);
            }
        }
        xprev = xval;
        yprev = yval;
        
        fprintf(opf, "%d, %d\n", xval, yval);
        reccnt++;
    }
    
#if (0==1)    
    /*
     *  Get more core, and read in the Zmin, Zmax and Zarray, and then the
     *  Mmin, Mmax and Marray:
     */
    headers = malloc(pgh.numPoints * sizeof(double));
    if (headers == NULL)
    {
        fprintf(stderr, 
                "ERROR: Unable to allocate space for PolygonZ headers (%d)\n\n",
                pgh.numParts);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_MALLOC);       
        return(FAIL_MALLOC);
    }
        
    /* Zmin, Zmax, Zarray: */
    r = fread(&pgzp, sizeof(pgzp), 1, ipf);
    r = fread(headers, sizeof(double), pgh.numPoints, ipf);
    /* Mmin, Mmax, Marray: */
    r = fread(&pgzp, sizeof(pgzp), 1, ipf);
    r = fread(headers, sizeof(double), pgh.numPoints, ipf);
    /*
     *  ... and release the gotten memory:
     */
    free(headers);
#endif
    
    fprintf(stderr, "INFO:  Number of output records (PolygonZ) = %d\n", reccnt);
    
    free(parts);
    return(ret);
    
}

int ReadMultipointZ(FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance)
{
    return (0);
}

int ReadPointM(FILE *ipf, FILE *opf, int rounding, long double denom, 
               int iscaling, long double scaling, int strict, int tolerance, int fsize, int rsize,
               int maxcorners)
{
    double xval;
    double yval;
    long double dxval;
    long double dyval;
    int x, y;               /* Work variables                  */
    long int lx, ly;
    int reccnt = 0;
    int   corners = 0;               /* Number of corners found         */
    unsigned char readin[POINTM_RECORD_LENGTH];   /* for shape records LITTLE Endian */
    int xprev = 0, yprev = 0;        /* Previous x, y values            */
    
    
    fprintf(stderr, "Filesize now %d, expect %d records \n", 
            fsize, (fsize / rsize));    
    
    /* read & write x,y csv file */
    while (fsize >= rsize)
    {
        /* read all the bytes for the 'next' record into unsigned */ 
        fread(readin, rsize, 1, ipf);     
        
        /*
         **  Count the number of points, and check that this is not greater
         **  than is permitted for this run. If it is, then error exit:
         */
        corners++;
        if (corners>maxcorners)
        {
            fprintf(stderr, 
                    "ERROR: Number of points %d [PointM] is greater than permitted %d\n", 
                    corners, maxcorners);
            fprintf(stderr, "       Conversion run terminated (%d)\n",
                    FAIL_CORNERS);
            exit (FAIL_CORNERS);
        }
        
        /* 
         ** Move the 8 bytes into the X Value - assume we are a LITTLE-
         ** endian machine
         */
        memcpy(&xval, &readin[12],  8);
        
        /* IDKK
         * Perform rounding, if requested, and always convert to int:
         */
        if (rounding == 0)
        {
            if (iscaling == 0)
            {
                x = (int) xval;
            }
            else
            {
                x = (int) (xval * scaling);
            }
        }
        else 
        {
            dxval = xval * denom * scaling;
            lx = (long int)dxval;
            x = lx / denom;
        }
        
        /* AGMW
         ** Ditto for 'y'
         */
        memcpy(&yval, &readin[20],  8);
        
        /* IDKK
         * Round if required, and always convert to type int:
         */
        
        if (rounding == 0)
        {
            if (iscaling == 0)
            {
                y = (int) yval;
            }
            else
            {
                y = (int) (yval * scaling);
            }
        }
        else 
        {
            dyval = yval * denom * scaling;
            ly = (long int)dyval;
            y = ly / denom;
        }
        
        /*
         *  If we are aplying strict rules, check whether x or y are within
         *  tolerance of the previous x and y:
         */
        if (strict != 0)
        {
            if ((abs(x - xprev) < tolerance) ||
                (abs(y - yprev) < tolerance))
            {
                fprintf(stderr, 
                        "ERROR: this point (%d, %d) is within tolerance (%d)\n"
                        "       of previous point (%d, %d) [PointM].\n"
                        "       Run terminates.\n", x, y, tolerance, 
                        xprev, yprev);
                exit (FAIL_STRICT);
            }
        }
        xprev = x;
        yprev = y;
        
        /* write to output */  
        fprintf(opf, "%d, %d\n", x, y);
        
        /* reduce file size */
        fsize = fsize - POINTM_RECORD_LENGTH;
        
        reccnt++;
    }
    
    fprintf(stderr, "Number of records (PointM) = %d (fsize=%d)\n\n", reccnt, fsize);
    
    
    return (0);
}

int ReadPolylineM(FILE *ipf, FILE *opf, int rounding, long double denom, 
                  int iscaling, long double scaling, int strict, int tolerance)
{
    return (0);
}

int ReadPolygonM(FILE *ipf, FILE *opf, int rounding, long double denom, 
                 int iscaling, long double scaling, int strict, int tolerance)
{
    int ret = 0;
    
    struct points {
        double pointX;            /* Little Endian */
        double pointY;            /* Little Endian */
    } pnts;
    struct recordheader {
        int    recordNo;          /* Big Endian    */
        int    contentLen;        /* Big Endian    */
    };
    struct polygonmheader {
        struct recordheader rh;
        int    shapeType;         /* Little Endian */
        char   boxXMin[8];        /* Little Endian */
        char   boxXMax[8];        /* Little Endian */
        char   boxYMin[8];        /* Little Endian */
        char   boxYMax[8];        /* Little Endian */
        int    numParts;          /* Little Endian */
        int    numPoints;         /* Little Endian */
    } pgh;
#if (0==1)
    struct polygonmpair {
        double Qmin;
        double Qmax;
    } pgmp;
#endif
    int reccnt = 0;
    int r;
    int i;
    int xval, yval;
    int xprev, yprev;
    int *parts = (int *)NULL;           /* ptr to Array of Integers */
#if (0==1)
    double *headers = (double *)NULL;   /* ptr to Array of pgzp     */
#endif
    double dblX;
    double dblY;
    
    xprev = 0;
    yprev = 0;
    /*
     ** Read in the Polygon Header Info
     */
    r = fread(&pgh, sizeof(struct polygonmheader), 1, ipf);
    if (r != 1)
    {
        fprintf(stderr, "ERROR: Failed to read PolygonM Header Info (%d)\n\n", 
                r);
        fprintf(stderr, "       Conversion run terminated (%d)\n",
                FAIL_POLYGON);       
        return(FAIL_POLYGON);
    }
    
    fprintf(stderr, "INFO:  PolygonM Number of Parts:  %d\n",pgh.numParts);
    fprintf(stderr, "INFO:  PolygonM Number of Points: %d\n",pgh.numPoints);
    
    /*
     ** Endian Flip the Record Header integers so we can display 'em
     */
    pgh.rh.recordNo   = swap_int32(pgh.rh.recordNo);
    pgh.rh.contentLen = swap_int32(pgh.rh.contentLen);
    
    /*
     ** Check it all seems reasonable so far
     */
    if (pgh.shapeType != SHP_POLYGONM)
    {
        fprintf(stderr, "ERROR: Incorrect Shape Type found (%d)\n\n", 
                pgh.shapeType);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_SHAPE);       
        return(FAIL_SHAPE);
    }
    
    if (pgh.numParts < 1)
    {
        fprintf(stderr, "ERROR: Must have at least 1 'Part' (%d)\n\n", 
                pgh.numParts);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_PARTS);        
        return(FAIL_PARTS);
    }
    
    /*
     ** Grab some space, then Read the parts[numParts] integer array into it
     */
    parts = (int *)malloc(pgh.numParts * sizeof(int));
    if (parts == NULL)
    {
        fprintf(stderr, "ERROR: Unable to allocate space for Parts (%d)\n\n",
                pgh.numParts);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_MALLOC);       
        return(FAIL_MALLOC);
    }
    r = fread(parts, pgh.numParts * sizeof(int), 1, ipf);
    if (r != 1)
    {
        fprintf(stderr, 
                "ERROR: Failed to read PolygonM No Parts array (%d)\n\n", r);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_ARRAY);       
        free(parts);
        return(FAIL_ARRAY);
    }
        
    /*
     ** Read the Points
     */
    for (i = 0; i < pgh.numPoints; i++)
    {
        r = fread(&pnts, sizeof(struct points), 1, ipf);
        if (r != 1)
        {
            fprintf(stderr, 
                    "ERROR: Failed to read PolygonM Points %d (%d)\n", 
                    i, r);
            fprintf(stderr, "       Conversion run terminated (%d)\n",
                    FAIL_POINTS);           
            free(parts);
            return(FAIL_POINTS);
        }
        
        /*
         ** Convert them to the output size/format and print 'em
         */
        if (rounding == 0)
        {
            if (iscaling == 0)
            {
                xval = (int) pnts.pointX;
                yval = (int) pnts.pointY;
            }
            else
            {
                xval = (int) (pnts.pointX * scaling);
                yval = (int) (pnts.pointY * scaling);
            }
        }
        else 
        {
            dblX = pnts.pointX * denom * scaling;
            xval = (int) (dblX / denom);
            dblY = pnts.pointY * denom * scaling;
            yval = (int) (dblY / denom);
        }
        
        /*
         *  If we are applying strict rules, check whether these x, y values
         *  are within tolerance of the previous x, y values:
         */
        
        if (strict != 0)
        {
            if ((abs(xprev - xval) < tolerance) ||
                (abs(yprev - yval) < tolerance))
            {
                fprintf(stderr, "ERROR: Previous PolygonM point (%d, %d) is within\n"
                        "       tolerance (%d) of previous point (%d, %d)\n"
                        "       Run terminates.\n", xval, yval, tolerance,
                        xprev, yprev);
                exit (FAIL_STRICT2);
            }
        }
        xprev = xval;
        yprev = yval;
        
        fprintf(opf, "%d, %d\n", xval, yval);
        reccnt++;
    }
    
#if (0==1)  
    /*
     *  Get more core, and read in the Zmin, Zmax and Zarray, and then the
     *  Mmin, Mmax and Marray:
     */
    headers = malloc(pgh.numPoints * sizeof(double));
    if (headers == NULL)
    {
        fprintf(stderr, 
                "ERROR: Unable to allocate space for PolygonM headers (%d)\n\n",
                pgh.numParts);
        fprintf(stderr, "       Conversion run terminated (%d)\n", FAIL_MALLOC);       
        return(FAIL_MALLOC);
    }
    
    /* Zmin, Zmax, Zarray: */
    r = fread(&pgzp, sizeof(pgzp), 1, ipf);
    r = fread(headers, sizeof(double), pgh.numPoints, ipf);
    /* Mmin, Mmax, Marray: */
    r = fread(&pgzp, sizeof(pgzp), 1, ipf);
    r = fread(headers, sizeof(double), pgh.numPoints, ipf);
    /*
     *  ... and release the gotten memory:
     */
    free(headers);
#endif
    
    fprintf(stderr, "INFO:  Number of output records (PolygonM) = %d\n", reccnt);
    
    free(parts);
    return(ret);
    
}

int ReadMultipointM(FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance)
{
    return (0);
}

int ReadMultipatch(FILE *ipf, FILE *opf, int rounding, long double denom, 
                   int iscaling, long double scaling, int strict, int tolerance)
{
    return (0);
}


/******************************************************************************
 Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
******************************************************************************/
