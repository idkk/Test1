/******************************************************************************
 Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 cnvshp2csv.c
 (deshape4.c)
 
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
 
 *****************************************************************************/

#define PROGRAM_VERSION   "0.31"
#define PROGRAM_EDIT_DATE "20130617:15.35"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <fcntl.h>

/*
 ** Forward Declarations
 */
int ReadPolygon(FILE *ipf, FILE *opf, int rounding, long double denom, 
                int strict, int tolerance);
int DbltoInt(double dval);
int32_t swap_int32(int32_t val);

/*
 ** Definitions for Shape File Type
 */
#define SHP_NULL           0
#define SHP_POINT          1
#define SHP_POLYLINE       3
#define SHP_POLYGON        5
#define SHP_MULTIPOINT     8
#define SHP_POINTZ         1
#define SHP_POLYLINEZ      13
#define SHP_POLYGONZ       15
#define SHP_MULTIPOINTZ    18
#define SHP_POINTM         21
#define SHP_POLYLINEM      23
#define SHP_POLYGONM       25
#define SHP_MULTIPOINTM    28
#define SHP_MULTIPATCH     31

#define MAXCORNERS      100
#define MAXMAXCORNERS 32000

#define ISBIG    0
#define ISLITTLE 1

#define NAMELEN  4096
#define MAXDENOM 1000000
#define MINDENOM 0.000001
#define MAXTOL   1000000

#define END_OK         0
#define FAIL_COMMAND   1
#define FAIL_INPUT     2
#define FAIL_OUTPUT    3
#define FAIL_VERSION   4
#define FAIL_POLYGON   5
#define FAIL_SHAPE     6
#define FAIL_PARTS     7
#define FAIL_MALLOC    8
#define FAIL_ARRAY     9
#define FAIL_POINTS   10
#define FAIL_CORNERS  11
#define FAIL_NOINPUT  12
#define FAIL_NOOUTPUT 13
#define FAIL_STRICT   14
#define FAIL_STRICT2  15

/* 32-bit Endian definition of *this* machine: */
/*
 * Note that this definition can be
 * (a) extended to cover other formats, such as PDP (0xad), and
 * (b) altered to err at compile time if the format is not
 *     recognised (remove UNDEFINED and uncomment the assert)
 */
static uint32_t endianness = 0xdeadbeef; 
enum ENDIANNESS { BIG, LITTLE, UNDEFINED };

#define ENDIANNESS ( *(const char *)&endianness == (char) 0xef ? LITTLE \
: *(const char *)&endianness == (char) 0xde ? BIG \
: UNDEFINED )
// : assert(0))
/*
 * This is tested by (for example):
 *   if (ENDIANNESS == BIG)...
 */


int main (int argc, char *argv[])
{
    int  reccnt=0; 
    FILE *opf     = (FILE *)NULL;    /* The Output file pointer         */
	char fnameout   [NAMELEN+1];     /* Name of the Output file         */
    char *offile  = (char *)NULL;    /* Pointer to output file name     */
    FILE *ipf     = (FILE *)NULL;    /* The Input file pointer          */
    char fnamein    [NAMELEN+1];     /* Name of the Input file          */
    char *iffile  = (char *)NULL;    /* Pointer to input file name      */
    char *rndstr  = (char *)NULL;    /* Pointer to Rounding request     */
    char *strictstr = (char *)NULL;  /* Pointer to Strictness request   */
   	char  slash = '/';
    char  c;                         /* Work area, parameter unpacking  */
    int   thisendian  = ISLITTLE;    /* Endianicty of *this* machine    */
    int   cmderr = 0;                /* Command error - default OK      */
    long double   denom = 1;         /* Conversion factor for rounding  */
    int   rounding = 0;              /* Indicate whether rounding or not*/
    int   strict = 0;                /* Indicate whether strict or not  */
    int   tprint = 0;                /* Trace flags - collective        */
    int   corners = 0;               /* Number of corners found         */
    int   maxcorners = MAXCORNERS;   /* Maximum corners permitted       */
    
    char  progname [4096];           /* Name of *this* program          */
    char  temp[4096];                /* Work area                       */
    
    /*
     char outline[128]; 
     */
    int indat[25];                   /* for 1st 100 bytes BIG endian    */
    unsigned char readin[28];        /* for shape records LITTLE Endian */
    int x, y, i, j, k;               /* Work variables                  */
    int xprev = 0, yprev = 0;        /* Previous x, y values            */
    int tolerance = 1;               /* Comparison tolerance if strict  */
    long int lx, ly;
    int fsize;                       /* File Size                       */
    int rsize;                       /* Record Size                     */
    int versn;                       /* Version                         */
    int stype;                       /* Shape Type                      */
    int ret = 0;
    
    double xval;
    double yval;
    long double dxval;
    long double dyval;
    int rptlvl;                      /* Report level                    */
    
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
		thisendian = ISBIG;
	}
	else if (ENDIANNESS == LITTLE)
	{
		thisendian = ISLITTLE;
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
    
    while ((c = getopt(argc, argv, ":i:o:c:C:d:r:R:S:t:T:v")) != -1)
    {
        switch(c)
        {
            case 'i':
				iffile = optarg;
                strcpy(fnamein, iffile);
                break;
                
            case 'o':
				offile = optarg;
				strcpy(fnameout, offile);
                break;
                
            case 'r':
                rptlvl = atoi(optarg);
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
                if ((denom < 1) || (denom > MAXDENOM))
                {
                    fprintf(stderr, 
                            "ERROR: valid range for -d is %f to %d, not %Lf\n", 
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
        fprintf(stderr, "  -r <r-level>    Report level 0/1/2 [Default 0]\n");
        fprintf(stderr, "  -d <rounding>   Rounding factor    [Default 1]\n");
        fprintf(stderr, "  -R <Y/N>        Apply rounding     [Default No]\n");
        fprintf(stderr, "  -S <Y/N>        Strict input rules [Default No]\n");
        fprintf(stderr, "  -C <tolerance>  Strict tolerance   [Default 1]\n");
        fprintf(stderr, "  -c <maxcorners> Maximum corners    [Default %d]\n", MAXCORNERS);
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
    fprintf(stderr, "Report level:    %d\n", rptlvl);
    if (rounding == 0)
    {
        fprintf(stderr, "No rounding/scaling\n");
    }
    else 
    {
        fprintf(stderr, "Rounding factor: %Lf\n", denom);
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
    for (x = 0; x < 7; x++)
    {
        indat[x] = swap_int32(indat[x]);
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
        case SHP_POINT:
            rsize = 28;
            break;
            
        case SHP_POLYGON:
            ret =  ReadPolygon(ipf, opf, rounding, denom, strict, tolerance);
            break;
            
        default:
            fprintf(stderr, "ERROR: Version type %d not supported\n\n", stype);
            fprintf(stderr, "       Conversion run terminated (%d)\n",
                    FAIL_VERSION);           
            exit(FAIL_VERSION);
    }
    
    if (stype == SHP_POINT)
    {
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
            
            /* do LITTLE Endian to BIG Endian conversion for x & y first x */
            /* AGMW
             ** Don't Swap 'cos we're on a Little ENDIAN machine now
             **                            vv
             ** 10|11|12|13|14|15|16|17|18|19|20
             ** --|--|07|06|05|04|03|02|01|00|--
             **
             i = 0; j = 7; k = 19;
             while (i <= j)
             {
             memcpy(&hold[i], &readin[k-i], 1);
             i++;
             }
             memcpy(&xval, &hold,  8);
             **
             */
            
            /* AGMW
             ** Instead, we just move the 8 bytes into the X Value
             */
            memcpy(&xval, &readin[12],  8);
            
            /* IDKK
             * Perform rounding, if requested, and always convert to int:
             */
            if (rounding == 0)
            {
                x = (int) xval;
            }
            else 
            {
                dxval = xval * denom;
                lx = (long int)dxval;
                x = lx / denom;
            }

            /* AGMW
             ** Ditto for 'y'
             **                            vv
             ** 18|19|20|21|22|23|24|25|26|27|28
             ** --|--|07|06|05|04|03|02|01|00|--
             **
             i = 0; j = 7; k = 27;
             while (i <= j)
             {
             memcpy(&hold[i], &readin[k-i], 1);
             i++;
             }
             memcpy(&yval, &hold,  8);
             **
             */
            
            /* AGMW
             ** Instead, we just move the 8 bytes into the Y Value
             */
            memcpy(&yval, &readin[20],  8);
            
            /* IDKK
             * Round if required, and alwas convert to type int:
             */
            
            if (rounding == 0)
            {
                y = (int) yval;
            }
            else 
            {
                dyval = yval * denom;
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
            /* IDKK
             What we write depends upon the value of rptlvl
             Value   what we write
             0     x, y
             1     x, y, xval, yval
             2     xval, yval
             */
            if (rptlvl < 2 )
            {
                fprintf(opf, "%d, %d", x, y);
                /*        fprintf(stderr, "%e, %e\n", xval, yval);   */
            }
            
            if (rptlvl == 1 )
            {
                fprintf(opf, ", ");
            }
            
            if (rptlvl > 0 )
            {
                fprintf(opf, "%f, %f", xval, yval);
            } 
            fprintf(opf, "\n");        
            
            /* reduce file size */
            fsize = fsize - 28;
            
            reccnt++;
        }
        
        fprintf(stderr, "Number of records = %d (fsize=%d)\n\n", reccnt, fsize);
    }
    
    exit(ret);
}     /* endof main */      


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
 */
int ReadPolygon(FILE *ipf, FILE *opf, int rounding, long double denom,
                int strict, int tolerance)
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
    
    /*
     ** Endian Flip the Record Header integers so we can display 'em
     */
    pgh.rh.recordNo   = swap_int32(pgh.rh.recordNo);
    pgh.rh.contentLen = swap_int32(pgh.rh.contentLen);
    
    /*
     ** Check it all seems reasonable so far
     */
    if (pgh.shapeType != SHP_POLYGON)
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
    
#if (0==1)    
    memcpy(&dblX, pgh.boxXMin, 8);
    memcpy(&dblY, pgh.boxYMin, 8);
    xval = DbltoInt(dblX);
    yval = DbltoInt(dblY);
    /*
     *  At this time, xval and yval contain the box minima
     */
    memcpy(&dblX, pgh.boxXMax, 8);
    memcpy(&dblY, pgh.boxYMax, 8);
    xval = DbltoInt(dblX);
    yval = DbltoInt(dblY);
    /*
     *  And now xval and yval contain the box maxima
     */
#endif
    
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
         **  SEGY data is stored as INTEGERs, so we need to do the same
         fprintf(stdout, "%f, %f\n", pnts.pointX, pnts.pointY);
         */
        /*
         *  Convert to type int, and if Rounding has been requested,
         *  apply that rounding:
         */
        if (rounding == 0)
        {
            xval = (int) pnts.pointX;
            yval = (int) pnts.pointY;
        }
        else 
        {
            dblX = pnts.pointX * denom;
            xval = (int) (dblX / denom);
            dblY = pnts.pointY * denom;
            yval = (int) (dblY / denom);
        }

#if (0==1)
        xval = DbltoInt(pnts.pointX);
        yval = DbltoInt(pnts.pointY);
#endif
        
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
    }
    
    free(parts);
    return(0);
}

/*
 ** Function : DbltoInt()
 ** Arguments: Double value
 ** Returns  : Integer value
 */
int DbltoInt(double dval)
{
    int ival;
    long int lival;
    long double ldval;
    
    ldval = dval * 1000;
    lival = (long int)ldval;
    ival  = lival / 1000;

#if (0==1)    
    dval = dval * 1000;
    ival = (int)dval;
    ival = ival / 1000;
#endif
    
    return(ival);
}


/*
 **! Byte swap int
 */
int32_t swap_int32(int32_t val)
{
    int32_t locval;
    
    locval = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (locval << 16) | ((locval >> 16) & 0xFFFF); 
}


/******************************************************************************
 Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
******************************************************************************/
