/*******************************************************************************
 *  cnvseg2sth.c STABLE BRANCH		
 *  Copyright 2013, 2012 Westheimer Energy Consulting Ltd. All rights reserved.
 ******************************************************************************/

/* This file last updated 20130603:1015 */

/*
 *   This program's parameter are all keyword, not positional, parameters.
 *   Some of them are mandatory. There are defaults for the opional parameters.
 *
 *   -i  <input file name>      Mandatory  The filename of the input file:
 *                                         NOTE that stdin is a special case
 *   -o  <output file name>     Mandatory  The filename of the primary output 
 *                                         file:
 *                                         NOTE that stdout is a special case
 *   -D  <directory name>       Optional   The name of the directory for the 
 *                                         output file(s).
 *                                         Default is NULL, so the directory is 
 *                                         relative to the current working 
 *                                         directory. This value is used only 
 *                                         for OUTline output.
 *   -s  <file sequence number> Optional   File sequence number for the first 
 *                                         outline trace file. Default is zero.
 *   -n  <IN or OUT>            Optional   Output is INline or OUTline. 
 *                                         Default is IN.
 *                                         NOT NOW SUPPORTED
 *   -l  <S or M or L>          Optional   LOB filename is Short (16 chars.) 
 *                                         or Medium (256) or Long (4096). 
 *                                         Default is Long (4096).
 *   -j                         Optional   Jetison extra header records if they
 *                                         are read. Default is that extra 
 *                                         headers will flag an error.
 *   -p  <parameters>           Optional   The dispacements of the extra, 
 *                                         leading, copy fields.
 *                                         Up to 6 (MAXP) of these may be 
 *                                         specified
 *   -L  <length>               Optional   The length of the previously 
 *                                         specified -p field, in chars.
 *                                         The default is 4.
 *   -m  <Y or N>               Optional   Include the max number of samples 
 *                                         as part of the extra, leading fields. 
 *                                         Default is Y - it *will* be included.
 *   -c  <brick size>           Optional   Brick size. Default is 0 - all the 
 *                                         LOB goes into one row.
 *                                         This is the number of (assumed 
 *                                         4-byte) samples per brick - the 
 *                                         length of a single brick (in bytes) 
 *                                         is four times this number specified, 
 *                                         plus a small amount.
 *                                         NOTE: old "brick" is new "chunk" for
 *                                         external purposes only.
 *   -F <format>                Optional   Format is either i or I (for IBM) or 
 *                                         e or E (for ieee), and indicates the
 *                                         output format for floating point
 *                                         numbers in the traces. Conversion is
 *                                         applied only when necessary.
 *   -R <YES or NO>             Optional   Existing files may / may not be over-
 *                                         written. Default is YES - existing
 *                                         files may be overwritten.
 *   -v                         Optional   Print version number and update date
 *                                         of this program.
 *   -d  <2 or 3>               Optional   2D or 3D - Default is 2D
 *                                         NOT YET SUPPORTED
 *   -t  <trace debug level>    Optional   Set debug print level. 
 *                                         Default is 0 (no debug).
 *                                         NOT FOR PRODUCTION USE
 *   -0, -1, -2, -3             Optional   Split output files for testing:
 *                                         0 - production, all in one file
 *                                         1 - file header only, then stop
 *                                         2 - file header and data headers
 *                                             only - no data
 *                                         3 - file header, trace header, and
 *                                             trace data all in separate files
 */

/*
 *  The output from this program is in one of two forms: INline or OUTline.
 *  INline format:
 *      Type F	SEG Y File Header record + Binary Header record
 *		Type S	SEG Y Optional additional Header record - NOT YET SUPPORTED
 *		Type H	SEG Y Trace Header
 *		Type D	SEG Y Trace Data
 *
 *  OUTline format: - NO LONGER SUPPORTED
 *      Type F	SEG Y File Header record + Binary Header record
 *		Type S	SEG Y Optional additional Header record - NOT YET SUPPORTED
 *		Type H	SEG Y Trace Header + LOB filename
 *		Type D	SEG Y Trace Data, out of line, in a file of the name given
 *                    in the Type H record
 *
 *  The input to this program is a SEGY file.
 */

/* TO DO:
 *   >>>  get out - and display? - the SEGY revision number (near the end of 
 *        the Binary Header)
 */

/*
 *  Modifications version 0.954
 *     Assume this machine is LITTLEendian (but general test also included)
 *     Generated values are endian corrected (this has still to be checked 
 *         for completeness)
 *     "Closed input" (etc.) message now only appears when tracing
 *     File types are F / H / D
 *     -m is default YES, with 4-byte value 0 inserted
 *
 *  Modifications version 0.957
 *     Name justification is LEFT
 *     Brick size is in trace header
 *     All parameters are listed in both whinge and option listing
 *     Some character arrays lengthened to allow for temination null
 *     Use 16 bit not 32 bit swapping for shorts (duh!)
 *     Keep nearly all of the C code in 80 columns
 *
 *  Modifications version 0.958
 *     IN PROGRESS allow .fh, .th and .dt output - complete in 0.96 onwards
 *
 *  Modifications version 0.959
 *     Complete the Name justification LEFT for ALL record types
 *
 *  Modifications version 0.965
 *     Every file close checks that the FILE pointer is not NULL 
 *     Remove INline/OUTline visibility
 *
 *  Modifications version 0.966
 *     Regular counting of input data length
 *     Data (type 'D') records contain file id and trace header id.
 *     Data-type is used in computing expected input data length
 *     Data type is displayed on reading File Header record
 *
 *  Modifications version 0.967
 *     Number of samples is displayed for first data record only
 *     Dimensionality commented out
 *
 *  Modifications version 0.968
 *     Trace data id (two byte brick count) now in the data (type 'D') record
 *
 *  Modifications version 0.970
 *     Identification markers should NOT have termination null
 *     Ensure all leading record counts are correctly endian swapped
 *     Ensure trace header records length is correct
 *
 *  Modifications version 0.975
 *     File sequence number correctly flipped the same in all record types
 *
 *  Modifications version 0.976
 *     All preceding lengths do NOT include themselves in the value
 *
 *  Modifications version 0.978/9
 *     No functional changes, but code commented
 *     First version of packing together output to reduce calls on fwrite
 *
 *  Modifications version 0.980
 *     Code now uses the data type when computing data length
 *     We now insert a TWO-byte count in front of the data field in the .td
 *
 *  Modifications version 0.981/2
 *     Brick size is now in .fh output
 *     Checking of data lengths being read in, and error exits
 *     Output file types are .ifh, .ith and .itd
 *     Debug statements removed, and comments tidied
 *     Parameter -b is number of BYTES for output data in a brick
 *     Display number of Bricks written, as well as Traces written
 *
 *  Modifications version 0.983/4
 *     Optimisation of calculations, to reduce their repetition
 *     Verify number of bricks matches number of traces, and warn if not
 *     Remove all INline/OUTline code
 *     Revert -b to be number of observations, but with more checking
 *     String-terminal added to end of file name
 *
 *  Modifications version 0.985
 *     Bug fix - use sample count, not bytes, in written brick size
 *
 *  Modifications version 0.986
 *     "Brick" (-b) is changed to "Chunk" (-c)
 *     First (not yet active) version of floating-point conversion included
 *
 *  Modifications version 0.987
 *     Floating-point conversion is now active, excluding the "both" option
 *     Conversion parameter is -F (not -f)
 *
 *  Modifications version 0.991
 *     Bug fix for -b 0
 *     Interchange -D and -d
 *
 */

/*
 *  ASSUMPTIONS:
 *     By default the input file is
 *           big-endian
 *           in the floating point format specified in its header
 *     By default the output file is
 *           big-endian
 *           in IEEE floating point format
 *     NO assumption is made about the edian-ness of *this* machine (i.e. the
 *           machine upon which this code is running): that is dynamically
 *           tested at the start of the run.
 */

/* TO VERIFY:
 *
 *    1   All parameters are correctly parsed and remembered within the program:
 *        -b   blocking OK
 *        -d   directory name for output OK
 *        -i   input file name OK
 *        -j   jetison extra headers, if present OK
 *        -l   length (S/M/L) of generated filenames
 *        -m   include "maxsamp" in output
 *        -n   INline or OUTline output INLINE ONLY
 *        -o   output file name OK
 *        -p   extra indexing field(s) to be included OK
 *        -s   file sequence number OK
 *        -t   trace level NOT PRODUCTION
 *        -v   display version number OK
 *        -D   Dimensionality NOT PRODUCTION
 *        -L   length of previous -p operand
 *
 *    2   Verify that valid SEGY files are read correctly, including:
 *        2 Dimensions - NOT YET
 *        3 Dimensions - NOT YET
 *        4 Dimensions - NOT YET
 *        without extra header records
 *        with extra header records
 *        "short" records - i.e. fewer than 8100 traces per point
 *        "long" records - i.e. more than 8100 traces per point
 *
 *    3   Verify blocking works
 *        -b 0 (default) no blocking - all data in a single block OK
 *        -b 1 (extreme case) only one sample per block OK
 *        -b <other> splitting of "short" records OK
 *        -b <other> splitting of "long" records OK
 *
 *    4   Verify that -d (directory) is handled correctly: OK
 *        -d not specified - local (cwd) output for both INlin and OUTline OK
 *        -d specified absolutely - both IN and OUT OK
 *        -d specified relative to cwd - both IN and OUT OK
 *        The primry output file is NOT in that directory:
 *           this only for the secondary (OUTline) output files.
 *
 *    5   Verify that file name and directory name lengths are handled
 *        correctly in the Short, Medium and Long cases. There are a number
 *        of possibilities (at least 20):
 *          File name | Directory name | -l option
 *            <17     |     <17        |  S, M, L    (three separate tests)
 *          >16 <256  |     <17        |  S, M, L
 *          >16 <256  |   >16 <256     |  S, M, L
 *          >255 <4096|     <17        |  S, M, L
 *          >256 <4096|   >16 <256     |  S, M, L
 *          >256 <4096|  >256 < 4096   |  S, M, L
 *            >4096   |     <any>      |   <any>
 *            <any>   |     >4096      |   <any>
 * 
 *    6   Verify that input file names may:
 *        be "ordinary" (not any of the following special cases) OK
 *        can contain more than one "."
 *        can contain special characters (e.g. %, &, _ etc.)
 *        may be "stdin" (special case for opening/closing) OK
 *
 *    7   Verify that output file names may:
 *        be "ordinary" OK
 *        can contain more than one ","
 *        can contain special characters
 *        may be "stdout" (special case for opening/closing) OK
 *
 *    8   Verify that invalid SEGY files are rejected
 *
 *    9   Verify that errors of output disk space and permissions are correctly
 *        signaled to the user
 *
 *   10   Verify that the -p / -L fields are correctly used - both when
 *        present and absent OK
 *
 *   11   Check that the -m field:
 *        contains the correct value
 *        is put in the correct place in the output OK
 *        is NOT included when not requested OK
 *
 *   12   Verify file sequence number (-s) is correct:
 *        when not specified OK
 *        when specified OK
 *
 *   13   Verify trace counter is correct for:
 *        conversions with/without -p/-L
 *        conversions with/without -m
 *        INline and OUTline conversions
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>       /* for getopts()         */
#include <sys/types.h>
#include <sys/stat.h>   
#include <stdint.h>
#include <fcntl.h>
#include <math.h>         /* for sqrt()            */
#include <netinet/in.h>   /* for htonl() ntohl()   */

#include "cnvseg2sth.h"

#define DEBUG_LEV 0

int main (int argc, char *argv[])
{ 
	
    extern char *optarg;
    extern int optind, optopt;

    int  c;                        /* work variable for getopts   */
	int  cmderr = 0;               /* Initialised: No errors yet  */
	int  tprint = 0;               /* Trace print level           */
    
    char *ifile = (char *)NULL;    /* Input file name             */
    char *ofile = (char *)NULL;    /* Output file name            */
	char *pdir  = (char *)NULL;    /* Output directory name       */
    char *rewrite = (char *)NULL;  /* -R parameter value          */
    
	char *LongShort = (char *)NULL; /* LONGname or SHORTname      */
	char *MaxSampct = (char *)NULL; /* Include maxsamp count Y/N  */
    char *oformat = (char *)NULL;  /* Output floating-point format*/
	
    FILE *ipf   = (FILE *)NULL;    /* The input file              */
    FILE *opf   = (FILE *)NULL;    /* The primary output file     */
    FILE *opffh = (FILE *)NULL;    /* Primary output - File hdr.  */
    FILE *opfth = (FILE *)NULL;    /* Primary output - Trace hdr. */
    FILE *opftd = (FILE *)NULL;    /* Primary output - Trace data */
	
	char fnamein  [NAMEMAX+1];     /* Name of primary input file  */
	char fnameout [NAMEMAX+1];     /* Name of primary output file */
	char fname    [NAMEMAX+1];     /* Work area for filenames     */
	char outdir   [NAMEMAX+1];     /* Name of output directory    */
	char temp     [NAMEMAX+1];     /* Temporary work area         */
	char progname [NAMEMAX+1];     /* Name of *this* program      */
	
	char slash = '/';
	char dot   = '.';
	
	short fseq;                    /* File sequence                        */
	short wfseq;                   /* Endian correct vn. of fseq           */
    short Wbsize = 0;              /* Brick length to record in trace hdr. */
    short headerbrick = 0;         /* Value to write into the chunk in hdrs*/
	short recsize;                 /* Record length in bytes               */
    short wrecsize;                /* Endian-correct version of recsize    */
    short opbrick = 0;             /* Brick size for .fh record            */
    short wopbrick = 0;            /* Endian-correct version os opbrick    */
    short sampcnt, sampsize;       /* Sample count and byte size           */
    short sampunread;              /* So far unread sample len. this trace */
	short maxsamp=MAXSAMP;         /* Maximum possible no. of samples      */
//    short haveshowncount = 0;      /* Flag indicating set after first data */
    short tracedataid = 0;         /* Brick count within a single sample   */
    short wtdid;                   /* Writable version of tracedataid      */
    short wthpre = 0;              /* Writable vn. of Trace header prefix  */
    short wtdpre = 0;              /* Writable vn. of Trace data prefix    */
    short floatconvert = NOCONVERT;/* Floating point conversion flag       */
    /*  The values in floatconvert mean:                                   */
    /*    0   No conversion requested or no conversion to be performed     */
    /*        NOCONVERT                                                    */
    /*   -1   Conversion to IBM32 format requested                         */
    /*        CIBM                                                         */
    /*    1   Conversion to IBM32 format to be done from IEEE32 format     */
    /*        CIEEE2IBM                                                    */
    /*   -2   Conversion to IEEE32 format requested                        */
    /*        CIEEE                                                        */
    /*    2   Conversion to IEEE32 format to be done from IBM32 format     */
    /*        CIBM2IEEE                                                    */
    /*   -3   Both formats of output (IEEE32 and IBM32) requested          */
    /*        CBOTH                                                        */
    /*    3   Conversion to both IEEE32 and IBM32 from IBM32 input to do   */
    /*        CIBM2BOTH                                                    */
    /*    4   Conversion to both IEEE32 and IBM32 from IEEE32 input to do  */
    /*        CIEEE2BOTH                                                   */
	unsigned short b2;
    
	char F='F';                    /* Output Rec. Type - File Header       */
//	char B='B';                    /* Output Rec. Type - Binary File Header*/
//	char S='S';                    /* Output Rec. Type - 2ndary File Header*/
	char D='D';                    /* Output Rec. Type - Trace Data        */
//	char T='T';
	char H='H';                    /* Output rec. Type - Trace Header      */
//	char U='U';
    
    char FHtype[] = ".ifh";        /* File type for File Header file       */
    char THtype[] = ".ith";        /* File type for Trace Header file      */
    char TDtype[] = ".itd";        /* File type for Trace Data file        */
	
	int  i, j, k, l;               /* Work variables                       */
    long samplecount = 0;          /* Count of samples in all of data      */
    int  OutType = ALLTOGETHER;    /* Type of splitting of output files    */
	int  Tseq=0;                   /* The trace sequence number - auto.inc.*/
	int  WTseq=0;                  /* Endian correct version of Tseq       */
	int  readl=0;                  /* When non-zero, input is exhausted    */
    int  brickcount = 0;           /* Number of bricks written             */
	int  bricksize = 0;            /* Number of bytes in a single brick    */
    int  mbb = MAXBRICK;           /* Maximum bytes chosen for a brick     */
    int  mbs = MAXSAMP;            /* Maximum sample chosen for a brick    */
	int  bricklen = 0;             /* Length of sample section of a brick  */
//    long long  brickfirst = 0;     /* 64-bit version of bricklen           */
//    long long  wbricklen = 0;      /* Writable version of brickfirst       */
//	int  thisbrick = 0;            /* Length in bytes of the current brick */
	int  jetison = 0;              /* Jetison (or not) auxilliary headers  */
    int  rew = 0;                  /* Rewrite permission for existing files*/
	int  p[MAXP];                  /* Displacements of additional fields   */
	int  plen[MAXP];               /* Lengths of each additional field     */
	int  nump = 0;                 /* Number of "p" fields in use          */
	int  Fhsize = FHSIZE;          /* SEGY File Header length in bytes     */
	int  Thsize = THSIZE;          /* SEGY Trace Header length in bytes    */
//	int  Bhsize = BHSIZE;          /* SEGY Binary File Header length       */
	int  FBhsize = FHSIZE + BHSIZE;  /* Length of SEGY File+Binary Header  */
	int  longname    = SHORTNAME;  /* DSname: Long (4096), Medium (256), Short (16) */
	int  fnamelen    = DSNAMEL;    /* File name length                     */
	int  exttrace    = 0;          /* Number of auxilliary header records  */
	int  incmaxsamp  = YESMAXSAMP; /* Include extra count of samples       */
	int  maxsampval  = 0;          /* Value of extra sample count          */
	int  wmaxsampval = 0;          /* Endian correct version of maxsampval */
	int  dimensions  = TWODIM;     /* Two dimensions by default            */
	int  thisendian  = ISLITTLE;   /* Endianicty of *this* machine         */
    int  datatype    = 0;          /* Type of this SEGY data, from file hdr*/
    int  savedatatype= 0;          /* Saved copy of datatype for comparison*/
    int  typelength  = 4;          /* Length of *this* data type, in bytes */
    /*
     * NOTE: the first element (subscript 0) of the typelens array is for
     *       type 1, the second element (subscript 1) for type 2, and so on.
     *       This is the length, in bytes, of one sample of the given type:
     */
    int  typelens[MAXTYPE] = {4, 4, 2, 4, 4, 4, 4, 1 };
    int  charinopb   = 0;          /* Number of characters in buffer opbuf */
    int  savecharinopb = 0;        /* Temporary saved copy of charinopb    */
	
	/* Debug Trace flags: */
//	short t1=0, t2=0, t4=0, t8=0, t16=0, t32=0, t64=0, t128=0, t256=0, t512=0;
//	short t1024=0, t2048=0, t4096=0, t8192=0, t16384=0, t32768=0;
//#define t3 t32
//#define t5 t64
//#define t6 t128
//#define t7 t256
//#define t9 t512
//#define t10 t1024
//#define t11 t2048
//#define t12 t4096
//#define t13 t8192
//#define t14 t16384
//#define t15 t32768
	
	char input[32760];   /* Input buffer                   */
//    /* unsigned */ char output[65400];  /* Output buffer, when converting */
    char opbuf[8192];    /* Output prefix buffer           */
    
    fprintf(stderr, "This program is strictly Copyright (c) 2013 "
            "Westheimer Energy Consultants Ltd.\n"
            "All Rights Reserved\n\n");
	
	/*
	 *  Set to default debug level:
	 */
//	tprint = DEBUG_LEV;
	
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
                "ERROR: ENDIAN format of this machine not supported (%d / %x).\n",
                endianness,endianness);
		fprintf(stderr, "[endianness first byte is %x]\n",
                *(const char *)&endianness);
		cmderr++;
	}
    
    /*  PROGRAM NAME  */
	
	/*
	 *  Remember the program name - the name of this program - for
	 *  debug and message printout. We edit out, from the name of
	 *  the program as invoked, any prior directory name specified by looking
     *  for the right-most slash:
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
	 * rightmost slash; if there was no slash, then skip this: 
     */
	if ( temp[j] == slash ) 
	{
		j++;
	}
	k = strlen(temp); 
	strncpy(progname, &temp[j], k-j);
	/* Add a terminating string end character: */
	progname[k-j] = '\0';
	
    
    /*  COMMAND LINE ARGUMENTS  */
    
	/*
	 ** Read the Command Line Arguments
	 */
	
	/*
	 * Initially there are no "p" arguments, and the default length of each -
	 * if requested - is 4:
	 */
	for (i=0;i<MAXP;i++)
	{
		p[i] = 0;
		plen[i] = 4;
	}
    
	/*
     *  Read in and initially process each command line parameter:
     */
    while ((c = getopt(argc, argv, ":i:o:s:d:l:m:p:L:t:T:b:c:F:D:R:vj0123")) != -1)
    {
        switch(c)
        {
            /* Input file name: */    
			case 'i':
				ifile = optarg;
				strcpy(fnamein, ifile);
				break;
			
            /* Output file name: */
			case 'o':
				ofile = optarg;
				strcpy(fnameout, ofile);
				break;
				
            /* File sequence number: */
			case 's':
				fseq = atoi(optarg);
				break;
				
            /* Length of filename to be included in the output: */
			case 'l':
				LongShort = optarg;
				break;

            /* Include (or not) the maximum sample count (Cristina's field): */
			case 'm':
				MaxSampct = optarg;
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
                else if ((strncmp(rewrite,"n",1)==0)||(strncmp(rewrite, "N", 1)==0))
                {
                    fprintf(stderr, "Existing files CAN be overwritten\n");
                    rew = 1;
                }
                else 
                {
                    fprintf(stderr, "ERROR: -R may be Y or N not %s\n",rewrite);
                    cmderr++;
                }                
                break;
				
            /* Get maximum chunk (brick) size: */
            /* Note that 'b' is being retained for compatibility - the new 'c'
             * is the one that is recommended, and which will be supported.
             */
			case 'b':
            case 'c':    
				bricksize = atoi(optarg);
                mbs = bricksize;
                headerbrick = mbs;
                /*
                 *  The field headerbrick is used as the displayed chunk size in
                 *  both the File Header record, and each of the Trace Header
                 *  records. It must be in big-endian format, so swap it here,
                 *  if necessary:
                 */
                if (thisendian == ISLITTLE)
                {
                    headerbrick = swap_uint16(headerbrick);
                }
				break;
				
            /* Floating-point format of the output: */
            case 'F':
                oformat = optarg;
                break;
                
            /* Jetison auxilliary header records: */
			case 'j':
				jetison = 1;
				break;
				
            /* Name of output directory: */
			case 'D':
				outdir[0]='\0';
				pdir = optarg;
				strcpy(outdir, pdir);
				i=strlen(outdir);
				if(outdir[i-1] != slash) 
				{
					outdir[i]=slash; 
					outdir[i+1]='\0';
				}
				break;
				
            /* Dimensionality of input: */
			case 'd':
				dimensions = atoi(optarg);
				fprintf(stderr, "WARNING: Dimensions (-d) NOT YET SUPPORTED\n");
				break;
				
            /* Trace flags: Note that 't' will be withdrawn; 'T' is correct */
			case 't':
            case 'T':    
				tprint = atoi(optarg);
                fprintf(stderr,
                        "WARNING: trace flags (-t) not for production\n");
				/*
				 * Set the trace flags:
				 */
//				i		= tprint;
//				t1		= i%2; i = i/2;
//				t2		= i%2; i = i/2;
//				t4		= i%2; i = i/2;
//				t8		= i%2; i = i/2;
//				t16		= i%2; i = i/2;
//				t32		= i%2; i = i/2;
//				t64		= i%2; i = i/2;
//				t128	= i%2; i = i/2;
//				t256	= i%2; i = i/2;
//				t512	= i%2; i = i/2;
//				t1024	= i%2; i = i/2;
//				t2048	= i%2; i = i/2;
//				t4096	= i%2; i = i/2;
//				t8192	= i%2; i = i/2;
//				t16384	= i%2; i = i/2;
//				t32768	= i%2; i = i/2;
				break;
			
            /* Extra 'field to copy' position: */
			case 'p':
				if (nump >= MAXP) 
				{
					fprintf(stderr, 
                            "ERROR: Too many -p arguments - maximum of %d\n",
                            MAXP);
					cmderr++;
				}
				else 
				{
					p[nump] = atoi(optarg);
					nump++;
				}
				break;
				
            /* Extra 'field to copy' length, associated with previous '-p' */
			case 'L':
				if (nump == 0)
				{
					fprintf(stderr, 
                            "ERROR: Length -L specified for unknown field\n");
					cmderr++;
				}
				else 
				{
					plen[nump-1] = atoi(optarg);
					if (plen[nump-1]<=0)
					{
						fprintf(stderr, 
                                "ERROR: Invalid length specified - must be positive\n");
						cmderr++;
					}
				}

				break;
				
            /* Show program version number: */
			case 'v':
				fprintf(stderr, "Program Version %s %s\n", SEGVERSION, SEGDATE);
				fprintf(stderr, "Program Name:   %s : %s\n", progname, argv[0]);
				break;
            
            /* Error condition - missing parameter: */
			case ':':       /* -i or -o (etc.) without operand */
				fprintf(stderr, "ERROR: Option -%c requires an operand\n", 
                        optopt);
				cmderr++;
				break;
                
            /* Output file groupings - all together, or only the File Header,
             * or only the File Header and Trace Header (with no data) or as
             * three separate files: 
             */
            case '0':
                OutType = ALLTOGETHER;
                break;
				
            case '1':
                OutType = FHONLY;
                break;
				
            case '2':
                OutType = FHTHONLY;
                break;
				
            case '3':
                OutType = THREEAPART;
                break;
				
            /* Error condition - unrecognised parameter: */
			case '?':
				fprintf(stderr, "ERROR: Unrecognized option: -%c\n", optopt);
				cmderr++;
        }
        /* Copy previous argument for any future error messages: */
        optopt = c;
//        fprintf(stderr, "DEBUG: optopt = %c\n",optopt);
    }
	
    /*
	 ** We must have been passed the input Filename
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
//        if (t1 !=0 ) fprintf(stderr, "Opening Input File  [%s]\n", ifile);
		if ( strcmp(ifile,"stdin") ==0 )
		{
			ipf = stdin;
		}
		else
		{
			if ((ipf = fopen(ifile, "r" )) == (FILE *)NULL)
			{
				fprintf(stderr, "ERROR: Unable to open input file (-i) [%s]\n", 
                        ifile);
				cmderr++;
			}
			/* Leave the input file open for the run */
		}
    }
	
    /*
	 ** We must have been passed the output (Intermediate) Filename
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
//        if (t1 != 0) fprintf(stderr, "Opening Output File [%s]\n", ofile);
		if ( strcmp(ofile, "stdout") ==0 )
		{
			opf = stdout;
		}
		else
		{
            /* Check whether overwriting is permitted, and if not whether this
             * file already exists:
             */
            if (rew != 0)
            {
                opf = fopen(ofile, "r");
                if (opf != (FILE *)NULL)
                {
                    /* This output file already exists, but we may not overwrite
                     * it - Issue an error message, and terminate the run:
                     */
                    fprintf(stderr, "ERROR: File %s already exists - no overwrite permitted\n",
                            ofile);
                    cmderr++;
                }
            }
			if ((opf = fopen(ofile, "w" )) == (FILE *)NULL)
			{
				fprintf(stderr, "ERROR: Unable to open output file (-o) [%s]\n",
                        ofile);
				cmderr++;
			}
			/* Leave the primary output file open for the run */
            /* 
             * Remember the primary output file pointer in the three (optional)
             * file pointers, for the possible split output:
             */
            opffh = opf;
            opfth = opf;
            opftd = opf;
		}
    }
	
    
	/*
	 *  Is the user asking for LONGname, SHORTname or Midname (Medium) output?
	 *  Allow both upper and lower case:
	 */
	if (LongShort == (char *)NULL)
	{
		longname = SHORTNAME;
		fnamelen = DSNAMEL;
	}
	else if ((*LongShort == 'L') || (*LongShort == 'l'))
	{
		longname = LONGNAME;
        fnamelen = NAMEMAX;
	}
	else if ((*LongShort == 'S') || (*LongShort == 's'))
	{
		longname = SHORTNAME;
		fnamelen = DSNAMEL;
	}
	else if ((*LongShort == 'M') || (*LongShort == 'm'))
	{
		longname = MIDNAME;
		fnamelen = NAMEMID;
	}
	else
	{
		/* The -l parameter is wrongly specified: */
		fprintf(stderr, 
                "ERROR: The -l parameter must be either L (long) or M (medium) "
                "\n       or S (short), not %s\n", LongShort);
		cmderr++;
	}
	
	/*
	 * Is the dimensionality allowable?
	 */
	
	if ((dimensions<1) || (dimensions>MAXDIM))
	{
		fprintf(stderr, "ERROR: Dimensions (-D) must be 2, 3 or 4, not %d\n",
                dimensions);
        fprintf(stderr, "NOTE the -D parameter is not now supported\n");
		cmderr++;
	}
	
	/*
	 * Is the user asking for maximum sample count to be included?
	 */
	if (MaxSampct ==(char *)NULL)
	{
		incmaxsamp = YESMAXSAMP;
	}
	else if ((*MaxSampct == 'Y') || (*MaxSampct == 'y'))
	{
        /* Note: Any string starting with 'Y' or 'y' is taken as 'yes' */
		incmaxsamp = YESMAXSAMP;
		fprintf(stderr, "WARNING: -m (include maximum sample count)"
                " NOT FULLY SUPPORTED\n");
	}
	else if ((*MaxSampct == 'N') || (*MaxSampct == 'n'))
	{
        /* Note: Any string starting wit 'N' or 'n' is taken as 'no' */
		incmaxsamp = NOMAXSAMP;
		fprintf(stderr, "WARNING: -m (exclude maximum sample count)"
                " NOT FULLY SUPPORTED\n");
	}
	else 
	{
		fprintf(stderr, 
                "ERROR: The -m parameter must be Y (Yes) or N (No), not %s\n",
                MaxSampct);
		cmderr++;
	}

    /*
     * Check the stated brick size against the maximum allowed:
     */
	/* Is this an allowable size? */
    /*
     * NOTE: in version 0.981 we assume that -b indicates the maximum number
     *       of BYTES for the data in an output brick. We set mbb and mbs to
     *       their initial values, also assuming that a single sample is four
     *       bytes long. These values may be changed later, when the datatype
     *       has been read.
     *
     * MODIFICATION: in version 0.983 we take -b to be in terms of observations,
     *       not bytes. This requires a little more checking - especially when
     *       we have read the File Header record, and determined the actual size
     *       of a sample.
     *
     * NOTE: in version 0.986 onwards it is -c (chunks) not -b (bricks) for
     *       external purposes - but the code has not changed (it still talks
     *       about bricks)
     */
    mbb = 4 * mbs;
    bricksize = mbb;
    bricklen = bricksize;
	if (mbs > MAXBRICK)
	{
		fprintf(stderr, 
                "ERROR: The chunk size (%d) exceeds the maximum allowed (%d)\n",
                mbs, MAXBRICK);
		cmderr++;
	}
    if (mbs > MAXSAMP)
    {
        if (mbs > MAXSAMP2)
        {
            fprintf(stderr, 
                    "WARNING: chunk size %d will work only for 1-byte samples\n"
                    "         (Type 8 samples)\n", mbs);
        }
        else 
        {
            fprintf(stderr, 
                    "WARNING: chunk size %d will work for 1- or 2-byte samples\n"
                    "         (Type 3 or 8 samples)\n", mbs);
        }

    }
    if (bricklen != 0)
    {
//        fprintf(stderr,"DEBUG: bricklen = %d maxsamp = %d mbs = %d\n",
//                bricklen, maxsamp, mbs);
        maxsamp = mbs;
        if (maxsamp == 0) maxsamp = MAXSAMP;
    }

    /*
     * Construct the canonical form of bricksize to write in the trace headers:
     */
    Wbsize = mbs;
    if (thisendian == ISLITTLE)
    {
	    Wbsize = swap_uint16(Wbsize);
    }
    if (headerbrick != 0)
    {
        headerbrick = Wbsize;
    }

    /*
     *  At this point:
     *    bricklen and bricksize   contain the maximum number of bytes *or zero*
     *    mbb                      contains the maximum number of bytes, and
     *    mbs                      contains the maximum number of 4-byte samples
     */
			
    /*
     *  Check the floating-point conversion, if requested.
     *  The values of floatconvert are:
     *    0   No conversion requested / no conversion required        NOCONVERT
     *    -1  Conversion to IBM format requested                      CIBM
     *    -2  Conversion to IEEE format requested                     CIEEE
     *    -3  Conversion to both is requested                         CBOTH
     *    +1  Conversion from IEEE to IBM is required                 CIEEE2IBM
     *    +2  Conversion from IBM to IEEE is required                 CIBM2IEEE
     *    +3  Conversion from IEEE to IBM is required + original IEEE CIEEE2BOTH
     *    +4  Conversion from IBM to IEEE is required + original IBM  CIBM2BOTH
     */
    if (oformat != (char *)NULL)
    {
        if ((*oformat == 'i') || (*oformat == 'I'))
        {
            fprintf(stderr, "Output will contain IBM floating-point format\n");
            floatconvert = CIBM;
        }
        else if ((*oformat == 'e') || (*oformat == 'E'))
        {
            fprintf(stderr, "Output will contain IEEE floating-point format\n");
            floatconvert = CIEEE;
        }
        else if ((*oformat == 'b') || (*oformat == 'B'))
        {
            /* In this case BOTH IGM and IEEE have been requested */
            fprintf(stderr, "Output will contain both IBM and IEEE formats\n");
            floatconvert = CBOTH;
            fprintf(stderr, "ERROR: -F b not yet supported.\n");
            cmderr++;
        }
        else {
            fprintf(stderr, "ERROR: -F parameter may be i or e and not %c\n",
                    *oformat);
            cmderr++;
        }

    }
    
    /*
	 ** If something went wrong, whinge and die ...
     ** NOTE that we do NOT tell the user about the unsupported and debug
     **      parameters -n, -t and -D
	 */
    if (cmderr)
    {
        fprintf(stderr, "Command Error (%d)\n", cmderr);
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage: %s <options>\n", argv[0]);
        fprintf(stderr, "  Options:-\n");
        fprintf(stderr, "   -i <filename>   Input SEG File     [No Default]\n");
        fprintf(stderr, "   -o <filename>   Output Trace File  [No Default]\n");
		fprintf(stderr, "   -s <fileseq>    File sequence no.  [Default 0]\n");
		fprintf(stderr, "   -d <directory>  Output Directory   [No Default]\n");
		fprintf(stderr, "   -l <L/M/S>      Lng/Medm/Shrt name [Default S(hort)]\n");
		fprintf(stderr, "   -m <Y/N>        Include maxsamp    [Default Y (Yes)]\n");
		fprintf(stderr, "   -p <position>   Extra index field  [Default none - up to %d allowed]\n",MAXP);
		fprintf(stderr, "   -L <length>     Extra field length [Default length 4]\n");
//		fprintf(stderr, "   -T <number>     Trace print level  [Def 0]\n");
		fprintf(stderr, "   -c <chunksize>  Data Size in Chunk [Default 0 - all data]\n");
        fprintf(stderr, "   -F <format>     Float output i / e [Default no conversion]\n");
//		fprintf(stderr, "   -D <dimensions> Dimensions 2/3/4   [Default 2]\n");
        fprintf(stderr, "   -0 -1 -2 -3     Output Splitting   [Default -0]\n");
		fprintf(stderr, "   -j              Jetison extra hdrs.[Default hdrs. not allowed]\n");
		fprintf(stderr, "   -v              Show version       \n\n");
		fprintf(stderr, "Program Version %s %s\n", SEGVERSION, SEGDATE);
        fprintf(stderr, "\n");
		
        /*
		 ** Close any files that are still open - should be none
		 */
        if ((ipf != (FILE *)NULL) && ( strcmp(ifile, "stdin") != 0 ))
        {
            tfclose(ipf);
//			if (t1 != 0) fprintf(stderr, "Closed input.\n");
        }
        if ((opf != (FILE *)NULL) && ( strcmp(ofile, "stdout") != 0 ))
        {
            tfclose(opf);
//			if (t1 != 0) fprintf(stderr, "Closed output.\n");	
        }
        exit(PARAM_ERROR);
    }
	
    
    /*  ANNOUNCE PROGRAM PARAMETERS REQUESTED */
    
	/*
	 *  Tell the user what the values are of the parameters, as read and as
	 *  taken by default:
	 */
	fprintf(stderr, "\nProgram %s being run with following options:\n\n"
            ,progname);
	fprintf(stderr, "Program Version %s %s\n", SEGVERSION, SEGDATE);
 	fprintf(stderr, "Input file name:      %s\n", fnamein);
	fprintf(stderr, "Output file name:     %s\n", fnameout);
	fprintf(stderr, "File sequence number: %d\n", fseq);
	fprintf(stderr, "Directory name:       %s\n", outdir);
    fprintf(stderr, "Name length:          %d (%s)\n", fnamelen, LongShort);
    if (incmaxsamp == YESMAXSAMP)
    {
        fprintf(stderr, "Include max. sample count\n");
    }
    else 
    {
        fprintf(stderr, "Do not include max sample count\n");
    }
	fprintf(stderr, "Prefixes are at:      %d : %d : %d : %d : %d : %d\n",
			p[0], p[1], p[2], p[3], p[4], p[5]);
	fprintf(stderr, "Prefix field lengths: %d : %d : %d : %d : %d : %d\n",
			plen[0], plen[1], plen[2], plen[3], plen[4], plen[5]);
	fprintf(stderr, "Chunk size:           %d\n", mbs);
    if (oformat != (char *)NULL)
    {
        if (floatconvert == CIBM)
        {
            fprintf(stderr, "Floating point output is in IBM format\n");
        }
        else if (floatconvert == CIEEE)
        {
            fprintf(stderr, "Floating point output is in IEEE format\n");
        }
    }
    /*
     *  Do not tell user about -D, as this is not supported:
     */
//	fprintf(stderr, "Dimensions:           %d\n", dimensions);
	fprintf(stderr, "Filename length is    %d\n", fnamelen);
	if (jetison==1)
	{
		fprintf(stderr, "Jetison any extra header records\n");
	}
	else 
	{
		fprintf(stderr, "Do not permit extra header records\n");
	}
    if (OutType == ALLTOGETHER)
    {
        fprintf(stderr, "Production format output\n");
    }
    else if (OutType == FHONLY)
    {
        fprintf(stderr, "File Header ONLY read and output\n");
    }
    else if (OutType == FHTHONLY)
    {
        fprintf(stderr, "File and Trace Headers ONLY read and output\n");
    }
    else 
    {
        fprintf(stderr, "Testing format output - headers and data separate\n");
    }


	fprintf(stderr, "Endianness for this machine is ");
	if (thisendian == ISLITTLE)
	{
		fprintf (stderr, "LITTLE\n");
	}
	else 
	{
		fprintf (stderr, "BIG\n");
	}

    /*
     *  Do NOT tell the user about debug trace level - this is for
     *  developers only:
     */
//	fprintf(stderr, "Trace print level:    %d\n\n", tprint);
	
	
	/*
	 *   MAIN CODE
	 */
	
    /*  INPUT & OUPUT FILE NAMES  */
	
	/*      The input file name may contain directory information - or may not.
	 *      We scan the name, as given, to get the 'nucleus' of the file name.
	 *      That is, we (a) remove any prior directory information, and
	 *      (b) we remove the trailing ".segy", if they exist:
	 */
	
	/* Scan backwards to find the rightmost slash, if any: */
	j = strlen(fnamein); 
	i = 0;
	while((j>i) && (fnamein[j] != slash)) 
	{
		j--;
	}
	/* If there actually was a slash in the filename, we are now pointing at
	 * it - so move forwards to point to the first character after the
	 * rightmost slash: */
	if ( fnamein[j] == slash ) 
	{
		j++;
	}
	
	/* Now scan backwards from the end of the string to only as far as the 
	 * character we have just found - the one following the rightmost slash.
	 * We are looking for the rightmost dot (which may, or may not, be
	 * followed by "segy"):
	 */
	k = strlen(fnamein); 
	while( (k>j) && ( fnamein[k] != dot ) ) 
	{
		k--;
	}
	if ( k==j ) 
	{
		k=strlen(fnamein);
	}
	
	/*
	 * This code looks at the input filetype ... in fact, whatever it 
	 * is, and no matter how long that filetype is, we drop it, and extract just 
	 * the nucleus of the filename, which runs from the first character after 
	 * the rightmost slash (if any) to the first character before the rightmost 
	 * dot (if any). If there is no slash, we start from the beginning of the 
	 * string, and if there is no dot then we go to the end of the string. We 
	 * also add a terminating 'nul' to the extracted filename, so that we can be 
	 * sure that it is a well-formed string:
	 */
	strncpy(fname, &fnamein[j], k-j);
	fname[k-j] = '\0';
	
	/*
	 * The input and output files are already open....
     * well, they are if all the output is going in to one file - but if there
     * is a splitting of the output, then we need to open the other files:
 	 */
    if (OutType != ALLTOGETHER)
    {
        /* 
         *  Close the primary output file, and open instead a file of the
         *  same name, but with ".ifh" appended to it:
         */
        tfclose(opf);
        sprintf(temp, "%s%s", ofile, FHtype);
        if (rew != 0)
        {
            opffh = fopen(temp, "r");
            if (opffh != (FILE *)NULL)
            {
                fprintf(stderr, "ERROR: File Header file %s already exists -\n"
                                "       Overwrite not permitted\n"
                        "       Run terminates\n", temp);
                exit (BAD_FH_EXISTS);
            }
        }
        if ((opffh = fopen(temp, "w" )) == (FILE *)NULL)
        {
            fprintf(stderr, 
                    "ERROR: Unable to open output file header file (-o) [%s]\n", 
                    temp);
            exit (BAD_FILE_HEADER);
        }
        
        if ((OutType == THREEAPART) || (OutType == FHTHONLY))
        {
            sprintf(temp,"%s%s",ofile,THtype);
            if (rew != 0)
            {
                opfth = fopen(temp, "r");
                if (opfth != (FILE *)NULL)
                {
                    fprintf(stderr, "ERROR: Trace Header file %s already exists -\n"
                            "       Overwriting not permitted\n"
                            "       Run terminates.\n", temp);
                    exit(BAD_TH_EXISTS);
                }
            }
            if ((opfth = fopen(temp, "w" )) == (FILE *)NULL)
            {
                fprintf(stderr, 
                        "ERROR: Unable to open output trace header file (-o) [%s]\n", 
                        temp);
                exit (BAD_TRACE_HEADER);
            }
        }
        if (OutType == THREEAPART)
        {
            sprintf(temp, "%s%s", ofile, TDtype);
            if (rew != 0)
            {
                opftd = fopen(temp, "r");
                if (opftd != (FILE *)NULL)
                {
                    fprintf(stderr, "ERROR: Trace Data file %s already exists -\n"
                            "       Overwriting not permitted\n"
                            "       Run terminates.\n", temp);
                    exit(BAD_TD_EXISTS);
                }
            }
            if ((opftd = fopen(temp, "w" )) == (FILE *)NULL)
            {
                fprintf(stderr, 
                        "ERROR: Unable to open output trace data file (-o) [%s]\n", 
                        temp);
                exit (BAD_TRACE_DATA);
            }
        }
    }
    else 
    {
        opffh = opf;
        opfth = opf;
        opftd = opf;
    }

    if (OutType == FHONLY)
    {
        opfth = (FILE *)NULL;
        opftd = (FILE *)NULL;
    }
    
    if (OutType == FHTHONLY)
    {
        opftd = (FILE *)NULL;
    }
	
	/*
	 * The input file has the following format:
	 *   3200 byte SEGY file header
	 *    400 byte Binary file header
	 *   3200 byte (OPTIONAL) series of extended file headers
	 *    240 byte Trace header
	 *    ??? byte Trace data
	 *    240 byte Trace header
	 *    ??? byte Trace data
	 *         etc.
	 */
   
	/* Read in SEGY FILE HEADER */
	
	/*
	 * Read in the SEGY File Header. If there is a problem in reading this, 
	 * then raise an error message and exit.
	 * We read both the SEGY File Header *and* the Binary File header at
	 * the same time, as these both occur at the start of the SEGY file:
	 */
	l = fread(input, 1, FBhsize, ipf);
	if( l != FBhsize ) 
	{	fprintf(stderr,
                "ERROR: Reading File HDR of %d Bytes: Only %d Bytes were read.\n", 
                FBhsize, l);
		exit(FILE_HEADER_ERROR); 
	} 
	
	
	/* Write out FILE Header and Binary file header info:  */ 
    
    /*
     *  NOTE: the way in which we write the header record is to construct it,
     *        field by field, into a buffer, and only when it is all complete
     *        to write it in a single go.
     */
	
	/* calc var size */
	/*
	 *  The size is the sum of:
	 *  (1) the base header size of the SEGY file header, and
	 *  (2) the base header size of the SEGY binary header, and
	 *  (3) the length of the letter 'F' (one!), and
	 *  (4) the length of the file sequence number (depends on variable
	 *      type, but is likely to be 2), and
	 *  (5) the length of the DSNAME - currently 16, but this could alter OR
	 *      the length of the LONG DSNAME - currently 4096 (or, now, 256)
     *  (6) the lenght of the bricklength field
	 */
	i = 0;
	i = i+FBhsize;                /* base + binary length  */
	i = i+1;                      /* size of 'F'           */
	i = i+sizeof(wfseq);          /* file sequence number  */
    i = i + 2;                    /* bricklength field     */
	if (longname == SHORTNAME) 
	{
		i = i+DSNAMEL;            /* size of DSNAME        */
	}
	else if (longname == MIDNAME)
	{
		i = i+NAMEMID;            /* size of medium DSNAME */
	}
	else 
	{
		i = i+NAMEMAX;	          /* size of LONG DSNAME   */
	}
	
	/* Calculate VAR size, and put it in buffer  */
	
	/*
	 *  This writes to the output file one element of the size of recsize (which
	 *  depends upon the type declaration, but is typically 2). The value
	 *  written is the contents of recsize. That is - we write out recsize
	 *  in binary (with endian correction, if needed):
	 */
    recsize = i;
    wrecsize = recsize;
	if (thisendian == ISLITTLE)
	{
		wrecsize = swap_uint16 (recsize);
	}

    charinopb = 0;
    memcpy(&opbuf[charinopb], &wrecsize, sizeof(wrecsize));
    charinopb = charinopb + sizeof(wrecsize);
    /*
     * above is encapsulation of:
     * l = fwrite (&wrecsize, 1, sizeof(wrecsize), opffh);
     */
	
	/* Write out 'F' - to identify FileHeader record type */
    memcpy(&opbuf[charinopb], &F, 1);
    charinopb++;
    /*
     * above is encapsulation of:
     * l = fwrite (&F, 1, 1 , opffh);
     */
	
	/* Write out 2 byte file_seq - with endian correction */
    wfseq = fseq;
	if (thisendian == ISLITTLE)
	{
		wfseq = swap_uint16(fseq);
	}
    
    memcpy(&opbuf[charinopb], &wfseq, sizeof(wfseq));
    charinopb = charinopb + sizeof(wfseq);
    /*
     *  above is encapsulation of:
     *  l = fwrite (&wfseq, 1, sizeof(wfseq), opffh);
     */
	
	/* Write out 16 or 256 or 4096 byte DSN - LEFT-adjusted */
	if (longname==SHORTNAME)
	{
        sprintf(&opbuf[charinopb], "%-" DSNAMELS "s", fname);
        charinopb = charinopb + DSNAMEL;
        /*
         *  above is encapsulation of:
         *  fprintf(opf,"%-" DSNAMELS "s",fname);
         */
	}
	else if (longname==MIDNAME) 
	{
        sprintf(&opbuf[charinopb], "%-" NAMEMIDS "s", fname);
        charinopb = charinopb + NAMEMID;
        /*
         *  above is encapsulation of:
         *  fprintf(opf,"%-" NAMEMIDS "s",fname);
         */
	}
	else 
	{
        sprintf(&opbuf[charinopb], "%-" NAMEMAXS "s", fname);
        charinopb = charinopb + NAMEMAX;
        /*
         *  above is encapsulation of:
         *  fprintf(opf,"%-" NAMEMAXS "s",fname);
         */
	}
	
    /*
     *  Put Brick Length (bytes) into file header, endian corrected:
     *  Version 0.985: brick size written is in *samples*, not *bytes* -
     *  but we do not yet know what the maximum size of a brick really is.
     *  We start by using mbs, but remember where we are in the output buffer
     *  and come back later to fix this field:
     */
    /* >>>>>> MODIFY THIS >>>>>> */
    opbrick = bricklen;
    wopbrick = mbs;
    if (thisendian == ISLITTLE)
    {
        wopbrick = swap_uint16(opbrick);
    }
//    memcpy(&opbuf[charinopb], &wopbrick, 2);
    memcpy(&opbuf[charinopb], &headerbrick, 2);
    savecharinopb = charinopb;
    charinopb = charinopb + 2;
    /*
     *  above is encapsulation of:
     *  l = fwrite (wopbrick, 1, 2, opffh);
     */
//    if (t2 != 0)
//    {
//        fprintf(stderr,"Chunk size stated is %d [%d] (bytes)\n", 
//                opbrick, wopbrick);
//    }
	
		
	/*
	 *  Get number of samples per trace:
	 */
	memcpy(&b2, &input[Fhsize+DSAMPCNT], 2);
    sampcnt = b2;
    if (thisendian == ISLITTLE)
    {
        sampcnt = swap_uint16(sampcnt);
    }
    fprintf(stderr, "Expected number of samples per trace: %d\n", 
            sampcnt);
    
	/*
	 *  Get the number of auxiliary trace (header) records:
	 */
	memcpy(&b2, &input[Fhsize+304], 2);
    exttrace = b2;
    if (thisendian == ISLITTLE)
    {
        exttrace = swap_uint16(b2);
    }

    /*
     *  Get the data type, and calculate the number of bytes per sample:
     */
	memcpy(&b2, &input[Fhsize+DDATAFMT], 2); 
    datatype = b2;
    if (thisendian == ISLITTLE)
    {
        datatype = swap_uint16(b2);
    }
    savedatatype = datatype;
    typelength = 4;
    if ((datatype>0) && (datatype<=MAXTYPE))
    {
        typelength = typelens[datatype-1];
    }

    fprintf(stderr, "Input data type              = %d\n", datatype);
    fprintf(stderr, "Size in bytes per sample     = %d\n", typelength);
    fprintf(stderr, "Expected data size per trace = %d\n", typelength*sampcnt);
    if ((datatype<=0) || (datatype>MAXTYPE))
    {
        fprintf(stderr, "WARNING: Input data type is %d [%x],\n"
                        "         Byte size per sample is assumed to be 4\n",
                datatype, datatype);
    }
    
    /*
     *  Now we know the real datatype, as stated in the File Header, we are
     *  able to determine whether the value in mbs is acceptable.
     */
        
	/*
     *  If the number of data bytes per sample is anything other then four
     *  then we need to recompute mbb and check its validity:
     */
    if ((typelength != 4) && (typelength != 0))
    {
        mbb = mbs * typelength;
//        fprintf(stderr,"DEBUG: typlength = %d maxsamp = %d mbs = %d mbb = %d\n",
//                typelength, maxsamp, mbs, mbb);
        maxsamp = mbs;
        if (maxsamp == 0) maxsamp = MAXSAMP;
    }
    /*
     *  If mbb is now too big, then tell the user, raise an error and exit:
     */
    if (mbb > MAXBRICK)
    {
        fprintf(stderr, "ERROR: This data is type %d and chunk is %d bytes\n"
                "       This exceeds the maximum of %d bytes for %d samples\n"
                "       Program %s terminating\n", datatype, mbb,
                MAXBRICK, mbs, progname );
        exit(BAD_BRICKING_ERR);
    }
    
    /*
     *  We now go back and correct the initial brick size that we put in the
     *  buffer, as now - and only now! - we know the real number of samples
     *  that we shall be writing into each full brick. The number of samples
     *  is the smallest, non-zero out of mbs and sampcnt:
     */
    
    opbrick = mbs;
    if (opbrick == 0)
    {
        opbrick = sampcnt;
    }
    if (thisendian == ISLITTLE)
    {
        wopbrick = swap_uint16(opbrick);
    }
    memcpy(&opbuf[savecharinopb], &wopbrick, 2);
    
    /*
     *  Now that we have read the header record we are able to determine, from
     *  the declared data type, whether or not we shall have to apply conversion
     *  the the floating-point trace fields.
     *  We do NOT have to apply conversion if:
     *    the trace input is not floating-point or
     *    the trace input is in the same format as requested for output.
     */
    if (floatconvert != NOCONVERT)
    {
        /*
         *  We perform these tests only if some kind of floating point
         *  conversion has been requested. There are six final posibilities:
         *    a) conversion to IBM has been requested and input is IBM
         *          - no conversion
         *    b) conversion to IBM requested and input is IEEE
         *          - IBM conversion
         *    c) conversion to IEEE requested and input is IEEE
         *          - no conversion
         *    d) conversion to IEEE requested and input is IBM
         *          - IEEE conversion
         *    e) conversion to both requested and input is IBM
         *          - IEEE conversion and then IBM copy
         *    f) conversion to both requested and input is IEEE
         *          - IEEE copy and then IBM conversion
         *  Note that in cases e) and f) we have to use the larger buffer for
         *  generating the output.
         *  All other possibilities are errors.
         */
    switch (datatype)
        {
            case TYPEIBM32:
                /* Input is IBM, 4 bytes */
                if (floatconvert == CIBM)
                {
                    /* Data is already in IBM format - no convertion needed */
                    floatconvert = NOCONVERT;
                }
                else if (floatconvert == CIEEE)
                {
                    /* Data is in IEEE format - convert to IBM */
                    floatconvert = CIEEE2IBM;
                    /* We *could* record the old datatype as well as the new -
                     * but we just (for the moment - 2013.01.08) record the new */
//                  datatype = IBM32_WAS_IEEE32;
                    datatype = IBM32;
                }
                else if (floatconvert == CBOTH)
                {
                    /* Data is in IBM format, both are required */
                    floatconvert = CIBM2BOTH;
                    datatype = WAS_IBM32_BOTH;
                }
                else 
                {
                    /* Internal program error - stop now! */
                    fprintf(stderr, 
                            "ERROR: Internal error: datatype=%d convert=%d\n",
                            datatype, floatconvert);
                    fprintf(stderr, "Run terminating (%d)\n",BAD_FLOAT_INT1);
                    exit(BAD_FLOAT_INT1);
                }

                break;
                
            case TYPEIEEE32:
                /* Input is IEEE, 4 bytes */
                if (floatconvert == CIBM)
                {
                    /* Data is in IBM, IEEE is required */
                    floatconvert = CIBM2IEEE;
                    /* We *could* record the old datatype as well as the new -
                     * but we just (for the moment - 2013.01.08) record the new */
//                  datatype = IEEE32_WAS_IBM32;
                    datatype = IEEE32;
                }
                else if (floatconvert == CIEEE)
                {
                    /* Data is already in IEEE - no conversion needed */
                    floatconvert = NOCONVERT;
                }
                else if (floatconvert == CBOTH)
                {
                    /* Data is in IEEE, both are required */
                    floatconvert = CIEEE2BOTH;
                    datatype = WAS_IEEE32_BOTH;
                }
                else 
                {
                    /* Internal program error - stop now! */
                    fprintf(stderr, 
                            "ERROR: Internal error: datatype=%d convert=%d\n",
                            datatype, floatconvert);
                    fprintf(stderr, "Run terminating (%d)\n",BAD_FLOAT_INT2);
                    exit(BAD_FLOAT_INT2);
                }
                break;
                
            default:
                /*
                 *  This is an error situation: some floating point format
                 *  conversion has been requested, BUT the input has not been
                 *  declared (in the input file) as being in floating point.
                 */
                fprintf(stderr, 
                        "ERROR: Floating point conversion requested,\n");
                fprintf(stderr, 
                        "but input format is type %d (not floating point)\n", 
                        datatype);
                fprintf(stderr, "This conversion is terminated. (%d)\n",
                        BAD_FLOAT_RQST);
                exit (BAD_FLOAT_RQST);
                break;
        }
        
        /*
         *  In case we have altered the datatype, rewrite this back into the
         *  input/output buffer: READ ALL THE COMMENTS BELOW BEFORE UNCOMMENTING
         */
//        memcpy(&input[Fhsize+DDATAFMT], &datatype,2);
        /* BEWARE!!!!!!
           In the above we have ALTERED the input, by specifying the output data
           type generated by this program in lieu of the original data type
           declared in the input. The original contents of that datatype are
           NOT preserved encoded in the new datatype.
           BEWARE!!!!!!
        */
        /*
         *  As you can see, the instruction altering the datatype has been
         *  commented out - the data *as saved* indicates the INCOMING data
         *  type... the potential alterations of the recorded data type are for
         *  FUTURE USE ONLY
         */
    }
    
    /*
     *  Check that the type length and the conversion parameters are consistent.
     *  We check that IF there is any floating-point conversion requested THEN
     *  the input sample data length of each item is 4. If there is a problem
     *  we terminate this run:
     */
    if ((floatconvert != NOCONVERT) && (typelength != 4))
    {
        fprintf(stderr,"ERROR: Data typelength is %d, and "
                "cannot be converted in format\n",typelength);
        fprintf(stderr,"Conversion run terminated (%d)\n",
                BAD_CONVERT_FMT);
        exit (BAD_CONVERT_FMT);                
    }
    
    /*  WRITE FILE HEADER RECORD:    */
    
    /*
     *  Now write out FBhsize characters - the contents of the SEGY file header
	 *  plus the Binary file header - flush our own output buffer first:
     */
    l = fwrite (opbuf, 1, charinopb, opffh); /* Flush our output buffer */
//    if (t3 != 0)
//    {
//        fprintf(stderr,"File Header displacement %d [%o]\n",
//                charinopb,charinopb);
//    }
	l = fwrite (input, 1, FBhsize, opffh);   /* Copy real file header   */
    
    
    /*  SKIP AUXILLIARY HEADER RECORDS, IF ANY  */
    
	/*
	 *  If there are auxilliary header records AND the user has asked to 
	 *  jetison them then read in, and ignore, that number of records.
	 *  If there are auxilliary header records but the user has not asked for 
	 *  them to be jetisoned, then issue an error message and stop:
	 */
	if (exttrace != 0)
	{
		if (jetison)
		{
			fprintf(stderr, "About to jetison %d auxilliary header records\n",
                    exttrace);
			for (i=0;i<exttrace;i++)
			{
				l = fread(input, 1, Fhsize, ipf);
                fprintf(stderr, "Jetisoned auxilliary header record %d\n", i+1);
			}
		}
		else 
		{
		    fprintf(stderr,
                "ERROR: There are %d extra header records - not supported\n",
                exttrace);
		    exit(EXTRA_HEADERS);
		}
	}
	
    
	/*
     * If we are putting all the output into a single dump - as we would be
     * for production - then copy the file header FILE pointer into the
     * trace header FILE pointer. If, however, we are not, then do NOT do this:
     */
    if (OutType == FHONLY)
    {
        fprintf(stderr, 
                "Finished writing File Header Record - program stops\n");
        tfclose(opffh);
        exit(FINISHED_HEADER);
    }
	
    
    /*  MAIN DATA READ/WRITE LOOP  */
    
//LOOP:
	/* Now loop through the trace records */
	while(readl == 0) {
		
        /*  READ TRACE HEADER  */
        
		/* Read in trace header:   */
		l=fread(input, 1, Thsize, ipf);
		/*
		 *  If we have not read in one complete item, then we have finished
		 *  reading the input, so exit from this "forever" loop:
		 */
		if( l != Thsize ) 
		{
			/*
			 *  Indicate that we have reached the end of the input.
			 *  And despite what Xcode says, this value *is* read -
			 *  at the top of the enclosing "while" loop:
			 */
			readl = 1; 
            fprintf(stderr, "Finished reading input\n");
            /*
             *  If the amount of data read is anything other than zero or
             *  end-of-file, then we need to raise an error, and warn the
             *  user:
             */
            if ((l != 0) && (l != EOF))
            {
                /*
                 *  We have read some real characters - but cannot process
                 *  them - tell the user:
                 */
                fprintf(stderr,
                        "ERROR: The last %d characters input not processed\n",
                        l);
                exit (EXTRA_INPUT_ERR);
            }
			goto LOOPX;
		}
		
		
		/*
		 *  Increment the Trace Sequence number. That is, Tseq contains
		 *  the Trace Sequence number for this trace. Since the initial
		 *  value for Tseq is zero, the first trace record will have
		 *  a Tseq of 1 (one):
		 */
		Tseq++;
		
		
		/* get number of SAMPLES per trace */
		i = sampcnt;
		
		/*
		 *  Indicate, on stderr, the number of samples in this particular
		 *  trace record. Note that for a large SEGY file, with many traces,
		 *  then this could be a very large amount of output! ... hence the
         *  flag to prevent its being shown more than once:
		 */
//		if ((t4 != 0) && (haveshowncount == 0))
//        {
//            fprintf(stderr, "There are %d samples per trace \n", i);
//            haveshowncount = 1;
//        }
		
		/*  Make decision about samples here and calc size for the Trace rec. */
		
		/*
		 *  We have to decide whether to write the trace contents inline with 
		 *  the trace header, or outline. That is, we have to decide whether to 
		 *  write the trace contents in the same file as the trace header, or in 
		 *  a different file.
		 *  In the original version of this program, this decision was made 
		 *  record by record , so that some could be inline, and some outline. 
		 *  In this version of the program (version 0.85 / 20120528 onwards) 
		 *  this decision is made by the user in the -n parameter, and it covers 
		 *  every record - either ALL are inline, or ALL are outline:
		 */
		
        /*
         *  The size of the sample section of the input is the number of
         *  expected samples times the length of each sample:
         */
		sampsize = sampcnt*typelength;           
//		if ((t5 != 0) && (haveshowncount == 1))
//        {
//            fprintf(stderr,
//                    "Data length expected per input record is %d bytes \n", 
//                    sampsize);
//            haveshowncount = 2;
//        }
		
		
		/*
		 *   If the sample size is greater than what can be held 
		 *   in a two-byte integer (32767), then we have to write the trace data  
		 *   out of line. 32767/4 = just over 8191 ... hence we use 8100 as a 
         *   (safe) upper bound.
		 *   If the user has specified INline (or not specified - and INline is 
		 *   the default), and the number of samples is greater than 8100 then 
		 *   we have have a problem - an error, which must be signalled, and 
         *   this program terminated:
		 *
		 *   There is, however, an exception. If the user has specified 
		 *   bricking, in the -b parameter, then we have to split this input 
		 *   record into bricks, each of which contains the specified number of 
         *   samples (plus, of course, the header). The last brick in the set 
         *   contains what is left over:
		 */
		
		if ((bricksize == 0) && ( sampcnt > maxsamp ))
		{
			fprintf(stderr, 
                    "ERROR: Too many samples per trace: %d\n",
                    sampcnt);
			exit(TOO_MANY_ERROR);
		}
		
        /* Calculate and display VAR size, only once */
        
        /*
         *  The calculation of this Trace Header size word is done only once.
         *  We save the result in wthpre - which will then be non-zero.
         */
        if (wthpre == 0)
        {
            i = 0;
            i = i+Thsize;              /* Base Trace Header size   */
            i = i+1;                   /* size of 'H'              */
            i = i+sizeof(wfseq);       /* file sequence count      */
            if (longname == SHORTNAME)
            {
                i = i+DSNAMEL;          /* size of DSNAME - Short  */
            }
            else if (longname == MIDNAME)
            {
                i = i+NAMEMID;          /* size of DSNAME - Medium */
            }
            else
            {
                i = i+NAMEMAX;          /* size of DSNAME - Long   */
            }
            i = i+sizeof(WTseq);        /* trace sequence count    */
            i = i+sizeof(sampsize);     /* sample size             */
            /*
            *  Now increase the computed record length by the length of each  
            *  extra field we are adding in to the record:
            */
            for (j=0;j<MAXP;j++)
            {
                if (p[j] != 0)
                    i = i+plen[j];       /* size of this -p field  */
            }
            /*
            * And the length of the sample count, if required:
            */
            if (incmaxsamp == YESMAXSAMP)
            {
                i = i + 4;               /* size of sample count   */
            }
		
            /* compute VAR size - with endian correction */
            recsize = i;
            wthpre = recsize;
            if (thisendian == ISLITTLE)
            {
                wthpre = swap_uint16 (recsize);
            }
        }
        
        
        /*  PREPARE TRACE HEADER PREFIX  */
        
        /* Write out VAR size: */
        charinopb = 0;
        memcpy(&opbuf[charinopb], &wthpre, sizeof(wthpre));
        charinopb = charinopb + sizeof(wthpre);
        /*
         *  above is encapsulation of:
         *  l = fwrite (&wthpre, 1, sizeof(wthpre), opfth);
         */
		
		/* write out 'H' - to identify DataHeader record type */
        memcpy(&opbuf[charinopb], &H, 1);
        charinopb++;
        /*
         *  above is encapsulation of:
         *  l = fwrite (&H, 1, 1, opfth);
         */
		
		/* write out 2 byte file_seq - with endian correction */
        wfseq = fseq;
        if (thisendian == ISLITTLE)
        {
            wfseq = swap_uint16(fseq);
        }
        
        memcpy(&opbuf[charinopb], &wfseq, sizeof(wfseq));
        charinopb = charinopb + sizeof(wfseq);
        /*
         *  above is encapsulation of:
         *  l = fwrite (&wfseq, 1, sizeof(wfseq), opfth);
         */
		
		/* write out 16 or 256 or 4096 (Short, Medium or Long) byte DSN */
        /* Write out 16 or 256 or 4096 byte DSN - LEFT-adjusted */
        if (longname==SHORTNAME)
        {
            sprintf(&opbuf[charinopb], "%-" DSNAMELS "s", fname);
            charinopb = charinopb + DSNAMEL;
            /*
             *  above is encapsulatiopn of:
             *  fprintf(opfth,"%-" DSNAMELS "s",fname);
             */
        }
        else if (longname==MIDNAME) 
        {
            sprintf(&opbuf[charinopb], "%-" NAMEMIDS "s", fname);
            charinopb = charinopb + NAMEMID;
            /*
             *  above is encapsulation of:
             *  fprintf(opfth,"%-" NAMEMIDS "s",fname);
             */
        }
        else 
        {
            sprintf(&opbuf[charinopb], "%-" NAMEMAXS "s", fname);
            charinopb = charinopb + NAMEMAX;
            /*
             *  above is encapsulation of:
             *  fprintf(opfth,"%-" NAMEMAXS "s",fname);
             */
        }
		
		/*
		 *   Now for each "extra" field being added to the front of the record,
		 *   write out that field. NOTE the p array is column number - hence for
         *   displacement we have to lower it by one.
         *   NOTE also that this "extra" copy is being made in the original
         *   format of the field, ***without*** any floating point format
         *   conversion being applied:
		 */
		for (j=0;j<MAXP;j++)
		{
			if (p[j] != 0)
			{
                memcpy(&opbuf[charinopb], &input[p[j]-1], plen[j]);
                charinopb = charinopb + plen[j];
                /*
                 *  above is encapsulation of:
                 *  l = fwrite(&input[p[j]-1], 1, plen[j], opfth);
                 */
			}
		}
		
		/* Write the -m (max samp count) field, if required: */
		if (incmaxsamp == YESMAXSAMP)
		{
			if (maxsampval == 0)
			{
				wmaxsampval = 0;
			}
			else if (thisendian == ISLITTLE)
			{
				wmaxsampval = swap_uint32(maxsampval);
			}
			else
			{
				wmaxsampval = maxsampval;
			}
            memcpy(&opbuf[charinopb], &wmaxsampval, 4);
            charinopb = charinopb + 4;
            /*
             *  above is encapsulation of:
             *  l = fwrite(&wmaxsampval, 1, 4, opfth);
             */
		}
        
        /*
         * Write out the canonical bricksize, in two bytes:
         */
        /* >>>>>>> MODIFY HERE >>>>>> */
//        memcpy(&opbuf[charinopb], &Wbsize, 2);
        memcpy(&opbuf[charinopb], &headerbrick, 2);
        charinopb = charinopb + 2;
        /*
         *  above is encapsulation of:
         *  l = fwrite (&Wbsize, 1, 2, opfth);
         */
        
		/* write out TRACE SEQUENCE - with endian correction 
         * (also known as Trace Header ID) */
        WTseq = Tseq;
		if (thisendian == ISLITTLE)
		{
			WTseq = swap_uint32(Tseq);
		}

        memcpy(&opbuf[charinopb], &WTseq, sizeof(WTseq));
        charinopb = charinopb + sizeof(WTseq);
        /*
         *  above is encapsulation of:
         *  l = fwrite (&WTseq, 1, sizeof(WTseq), opfth);
         */
        
        /*  WRITE TRACE HEADER RECORD  */
				
		/* now write out Trace Header contents */
        memcpy(&opbuf[charinopb], input, Thsize);
        charinopb = charinopb + Thsize;
        /*
         *  above copies the input buffer to the end of our constructed
         *  buffer - see the original previous code below
         */  

        l = fwrite(opbuf, 1, charinopb, opfth);
        /*
         *  Previous code here was:
         *  l = fwrite (input, 1, Thsize, opfth);
         */
		
		/* now either write out the data contents OR the lob filename */
        
        /*
         * Start by setting the brick count (within this one sample) to zero,
         * noting that it will the augmented at each brick:
         */
        tracedataid = 0;
                    
        /*
         * If this is ALLTOGETHER or THREEAPART then we have something to do
         * here - otherwise we completely skip the writing of the data -
         * but we do have to read it, to skip past it:
         */
			
        /* Read inline data & write it inline */

        /*
         * Since this is INline, we already know that the amount to be read
         * is not larger than our largest chunk - so read it all in one go,
         * and write it in one go...
         * OR we know that we can read and write in bricks, each brick
         * containing the specified number of samples, other than the
         * last brick, which contains the residue:
         * Note that this output does NOT go to a LOB file, but direct to
         * the inline output:
         */
        sampunread = sampsize;
        while (sampunread > 0)
        {
            /*  READ IN A BRICK'S WORTH OF DATA  */
            
            if (bricksize == 0)
            {
                i = sampsize;
                bricklen = sampsize;
            }
            else 
            {
                if (sampunread <= bricksize)
                {
                    bricklen = sampunread;
                }
                else 
                {
                    bricklen = bricksize;
                }
            }
                
            sampsize = sampsize - bricklen;
            sampunread = sampunread - bricklen;
            
            l = fread (input, 1, bricklen, ipf);
            if (l != bricklen)
            {
                /*
                 *  The amount of input read may not be sufficient: we
                 *  have read a trace header, but there is not enough data
                 *  to follow. Tell the user, and stop:
                 */
                if ((l == 0) || (l == EOF))
                {
                    fprintf(stderr, "ERROR: Data input terminates (%d) "
                            "too early", l);
                    exit (SHORT_INPUT_ERR);
                }
                else 
                {
                    fprintf(stderr, "ERROR: data input too short. Last "
                            "%d characters not processed", l);
                    exit (SHORT_INPUT_ERR2);
                }
            }
                
            /*  CREATE DATA BRICK PREFIX  */
            
            /*
                 *  If bricksize is zero, then all the data is going in to one
                 *  brick. Since (for the moment) we are assuming that every
                 *  data record will have the same number of samples that means
                 *  that the length of every brick will be the same, and can
                 *  be calculated only once. The result of that calculation
                 *  is stored in wtdpre - which is then non-zero.
                 *  If, however, bricksize is *not* zero, then we still have to
                 *  recompute the length of the brick for every brick, as they
                 *  may of of different lengths.
                 */
//                wtdpre = 0;  /* Un-comment this if regular data can length vary */
            if ((bricksize != 0) || (wtdpre == 0))
            {
                i = 0;
                i = i+1;         /* type 'D' */
                i = i+bricklen;  /* The actual data itself */
                i = i + sizeof(wfseq) + sizeof(WTseq) + sizeof(wtdid);
////            i = i + sizeof(wbricklen); /* 64 bit Counter for LOB */
                i = i + 2;      /* Two-byte counter in front of LOB */
                
                /* write out VAR size - endian corrected */
                recsize = i;
                wtdpre = recsize;
                if (thisendian == ISLITTLE)
                {
                    wtdpre = swap_uint16(recsize);
                }
            }
            if (OutType == ALLTOGETHER)
            {
                opftd = opfth;
            }
            
            if ((OutType == ALLTOGETHER) || (OutType == THREEAPART))
            {
                charinopb = 0;
                memcpy(&opbuf[charinopb], &wtdpre, sizeof(wtdpre));
                charinopb = charinopb + sizeof(wtdpre);
                /*
                 *  above is encapsulation of:
                 *  l = fwrite (&wtdpre, 1, sizeof(wtdpre), opftd);
                 */
			
                /* write out 'D' - to identify Data record type: */
                memcpy(&opbuf[charinopb], &D, 1);
                charinopb++;
                /*
                 *  above is encapsulation of:
                 *  l = fwrite (&D, 1, 1, opftd);
                 */
                    
                /*
                 *  Write out the three fields:
                 *    File sequence number (File id)
                 *    Trace header count (Trace header id)
                 *    Brick count within single sample (Trace data id)
                 */
                /* File id.: */
                wfseq = fseq;
                if (thisendian == ISLITTLE)
                {
                    wfseq = swap_uint16(fseq);
                }
                memcpy(&opbuf[charinopb], &wfseq, sizeof(wfseq));
                charinopb = charinopb + sizeof(wfseq);
                /*
                 *  above is encapsulation of:
                 *  l = fwrite (&wfseq, 1, sizeof(wfseq), opftd);
                 */
                    
                /* Trace Header id.: */
                memcpy(&opbuf[charinopb], &WTseq, sizeof(WTseq));
                charinopb = charinopb + sizeof(WTseq);
                /*
                 *  above is encapsulation of:
                 *  l = fwrite (&WTseq, 1, sizeof(WTseq), opftd); 
                 */
                    
                /* Trace Data id: */
                /*
                 *  We are assuming that the trace data id. is the brick
                 *  count within this single sample (starting at one)
                 */
                tracedataid++;
                wtdid = tracedataid;
                if (thisendian == ISLITTLE)
                {
                    wtdid = swap_uint16(tracedataid);
                }
                memcpy(&opbuf[charinopb], &wtdid, sizeof(wtdid));
                charinopb = charinopb + sizeof(wtdid);
                /*
                 *  above is encapsulation of:
                 *  l = fwrite(&wtdid, 1, sizeof(wtdid), opftd);
                 */
                    
                /* 
                 *  Construct the eight-byte length prefix for the LOB.
                 *  NOTE that this is not yet used (version 0.981), as
                 *  we use only the two-byte prefix (see below):
                 */
////                    brickfirst = bricklen;
////                    wbricklen = brickfirst;
////                    if (thisendian == LITTLE)
////                    {
////                        wbricklen = swap_double(brickfirst);
////                    }
////                    memcpy(&opbuf[charinopb], &wbricklen, 8);
////                    charinopb = charinopb + 8;
////                    /* l = fwrite(&wbricklen, 1, sizeof(wbricklen), opftd); */
                    
                /* Construct and write two-byte length of LOB field */
                b2 = bricklen;
                if (thisendian == LITTLE)
                {
                    b2 = swap_uint16(b2);
                }
                memcpy(&opbuf[charinopb], &b2, 2);
                charinopb = charinopb + 2;
                /*
                 *  above is encapsulation of:
                 *  l = fwrite(b2, 1, 2, opftd);
                 */

                
                /*  WRITE OUT THE DATA BRICK  */
                    
                /*
                 *  Flush our prefix buffer:
                 */
                l = fwrite(opbuf, 1, charinopb, opftd); 
                
                /*
                 *  Construct and write the brick of data:
                 *  If there is no floating-point format conversion taking
                 *  place, then the data to be written comes from the input
                 *  buffer - it is a simple copy. If, however, we do have to
                 *  convert the format, then we scan down the input buffer
                 *  replacing each entry by its converted equivalent, and only
                 *  then write the output - but, again, from the input buffer:
                 */
                j = bricklen>>2;
                samplecount += j;
                if (floatconvert != NOCONVERT)
                {
                    /*
                     *  The number of words to be converted is the bricklen
                     *  divided by four (hence the above shift right by two). 
                     *  We could also check here that the typelength is equal to
                     *  four, as if it is not, then we have a real conversion
                     *  problem! For the moment (2013.01.11) that check IS done,
                     *  even though it is a little expensive - if we can do this
                     *  check earlier in the process (e.g. whilst processing the
                     *  file header) it would be computationally cheaper than
                     *  doing it every brick.
                     */
                    if (typelength != 4)
                    {
                        fprintf(stderr,"ERROR: Data typelength is %d, and "
                                "cannot be converted in format\n",typelength);
                        fprintf(stderr,"Conversion run terminated (%d)\n",
                                BAD_CONVERT_FMT);
                        exit (BAD_CONVERT_FMT);        
                    }
                    if (floatconvert == CIEEE2IBM)
                    {
                        /* To IBM from IEEE */
                        ieee2ibm((char *)input, (char *)input, j);
                    }
                    else if (floatconvert == CIBM2IEEE)
                    {
                        /* To IEEE from IBM */
                        ibm2ieee((char *)input, (char *)input, j);
                    }
                    else if (floatconvert == CIEEE2BOTH)
                    {
                        /* To both from IEEE */
                        fprintf(stderr, "ERROR: -f b not yet supported\n");
                        fprintf(stderr, "Conversion run terminated (%d)\n",
                                BAD_FLOAT_RQST2);
                        exit (BAD_FLOAT_RQST2);
                    }
                    else if (floatconvert == CIBM2BOTH)
                    {
                        /* To both from IBM */
                        fprintf(stderr, "ERROR: -f b not yet supported\n");
                        fprintf(stderr, "Conversion run terminated (%d)\n",
                                BAD_FLOAT_RQST3);
                        exit (BAD_FLOAT_RQST3);
                    }
                    else
                    {
                        /* Invalid value for conversion flag - error */
                        fprintf(stderr, 
                                "ERROR: Conversion flag invalid (%d)\n",
                                floatconvert);
                        fprintf(stderr, "Conversion run terminated (%d)\n",
                                BAD_FLOAT_CNVT);
                        exit (BAD_FLOAT_CNVT);
                    }
                }
                /*
                 *  Finally the data portion is ready to be written out:
                 */
                l = fwrite(input, 1, bricklen,  opftd); 
                /*
                 * ... and count the bricks written:
                 * (this should be related to tracedataid)
                 */
                brickcount++;
            }
            /*
             *  Since in the ALLTOGETHER case we are actually using only one
             *  file, we copy back the file pointer that we have just used, 
             *  ready for the next trace header (or data) record:
             */
            if (OutType == ALLTOGETHER)
            {
                opfth = opftd;
            }
        }
//LOOPE:
//        if (t14 != 0) fprintf(stderr, "End of LOOP E\n");
	} /* endof read loop */
	
LOOPX:
    
    /*  TIDY UP AND FINISH EXECUTION  */
    
	/*
	 *  Tell the user how many Traces and Bricks, close files, and exit:
	 */
	fprintf(stderr, "Number of Traces written: %d\n", Tseq);
    fprintf(stderr, "Number of Chunks written: %d (%d)\n", 
            brickcount, tracedataid);
    fprintf(stderr, "Number of sample words  : %ld\n",samplecount);
    /*
     *  The number of bricks written should be the same as the product of
     *  the number of traces written times the number of bricks per trace.
     *  If this is not the case, raise a warning message to the user:
     */
    if ( brickcount != (Tseq * tracedataid))
    {
        fprintf(stderr, "WARNING: If all traces are the same size, then there\n"
                "         should have been %d chunks\n", (Tseq * tracedataid));
    }
	
	/* Close input file if it's not stdin: */
	if ( strcmp(ifile, "stdin") != 0 )
	{
		tfclose(ipf);
            fprintf(stderr, "Input closed.\n");
	}
	
	/* Close output file if it's not stdout: */
	if ( strcmp(ofile, "stdout") != 0 )
	{
        fprintf(stderr, "Closing output file\n");
        tfclose ( opffh );
        if ((OutType == FHTHONLY) || (OutType == THREEAPART))
        {
            fprintf(stderr, "Closing output trace header\n");
            tfclose ( opfth );
            if (OutType == THREEAPART)
            {
                fprintf(stderr, "Closing output trace data\n");
                tfclose ( opftd );
            }
        }
//		if (t1 != 0) fprintf(stderr, "Output file(s) closed\n");
	}
	
    fprintf(stderr, "End of run for %s\n", progname);
	exit (GOOD_EXIT);
	
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
 **! Byte swap unsigned short
 */
uint16_t swap_uint16( uint16_t val ) 
{
	return (val << 8) | (val >> 8 );
}


/*
 **! Byte swap short
 */
int16_t swap_int16( int16_t val ) 
{
	return (val << 8) | ((val >> 8) & 0xFF);
}

/*
 **! Byte swap unsigned int
 */
uint32_t swap_uint32( uint32_t val )
{
	val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
	return (val << 16) | (val >> 16);
}


/*
 **! Byte swap int
 */
int32_t swap_int32( int32_t val )
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

/*
 ** unswap using char pointers
 */
double swap_double(unsigned long long a) 
{
	
    double d;
    unsigned char *src = (unsigned char *)&a;
    unsigned char *dst = (unsigned char *)&d;
	
    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];
	
    return d;
}


/*******************************************************************************
 *  End of seg2sth.c		
 *  Copyright 2013, 2012 Westheimer Energy Consulting Ltd. All rights reserved.
 ******************************************************************************/


