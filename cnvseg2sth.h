/*
 *  cnvseg2sth.h
 *  (seg2sth.h
 *   SEG2STH)
 *
 *  Created by Ian Kelly on 21/05/2012.
 *  Copyright 2012-2013 Westheimer Energy Consulting Ltd. All rights reserved.
 * 
 */

#ifndef CNVSEG2STH
#define CNVSEG2STH

#define SEGVERSION "1.000"
#define SEGDATE  "9 October 2013:13.00"

/* 
 *  The following definition for TEST_FLOAT_OVERFLOW is required only if we
 *  wish the IBM/IEEE conversion code to test for overflow in the conversion.
 *  This slightly reduces the (speed) efficiency of the code, and by default we
 *  leave this as undefined (commented out):
 */
//#define TEST_FLOAT_OVERFLOW 1

/* MAXSAMP is the maximum number of 4-byte samples:   */
#define MAXSAMP  8175
/* MAXSAMP2 is the maximum numnber of 2-byte samples: */
#define MAXSAMP2 16350
/* MAXBRICK is the maximum bricklength in bytes:      */
#define MAXBRICK 32700
/* Note that 32700 = 4 * 8175 and 16350 = 2 * 8175    */

#define FHSIZE   3200
#define BHSIZE   400
#define THSIZE   240
#define RDSIZE   4000
#define OFFSET   114
#define NAMEMAX  4096
#define NAMEMAXS "4096"
#define DSNAMELS "16"
#define DSNAMEL  16
#define NAMEMID  256
#define NAMEMIDS "256"

#define LONGNAME  2
#define MIDNAME   1
#define SHORTNAME 0

#define YESMAXSAMP 1
#define NOMAXSAMP  0

#define MAXTYPE  8

#define TWODIM   2
#define THREEDIM 3
#define FOURDIM  4
#define MAXDIM   4

#define ISBIG    0
#define ISLITTLE 1

#define ALLTOGETHER 0
#define FHONLY      1
#define FHTHONLY    2
#define THREEAPART  3

/* Data types when floating conversion requested */
#define MAX_SEGY_TYPE     8
#define IBM32             1
#define IEEE32            5
#define BOTH              2
#define IEEE32_WAS_IBM32 ((IBM32*256)+IEEE32)
#define IBM32_WAS_IEEE32 ((IEEE32*256)+IBM32)
#define WAS_IBM32_BOTH   ((IBM32*256)+(BOTH*256))
#define WAS_IEEE32_BOTH  ((IEEE32*256)+(BOTH*256))
// #define IEEE64_WAS_IBM64 21
// #define IBM64_WAS_IEEE64 22
// #define WAS_IBM64_BOTH   23
// #define WAS_IEEE64_BOTH  24
/* This the new datatype maps to the original (input datatype) as follows: 
 *
 *      old   new
 *       1     17
 *       1     19
 *       5     18
 *       5     20
 */

/*
 *  Raw data types, as defined in the SEG-Y standard:
 */
#define TYPEUNDEFINED 0
#define TYPEIBM32     1
#define TYPEINT32     2
#define TYPEINT16     3
#define TYPEINTGAIN   4
#define TYPEIEEE32    5
#define TYPEINT8      8

/*
 *  The following constants may be used by the IEEE-IBM conversions:
 *  Note that some of these constants are not used and are commented out -
 *  Please do not remove these comments, as these values may be used in later
 *  versions of this code (Version 0.994)
 */
#define NOCONVERT       0
#define CIBM2IEEE       1
#define CIEEE2IBM       2
#define CIEEE2BOTH      3
#define CIBM2BOTH       4
//#define CIBMINPUT       -1
//#define CIEEEINPUT      -2
//#define CIBMINPUTB      -3
#define CIBM            -1
#define CIEEE           -2
#define CBOTH           -3
//#define IBM_SIGN        0x80000000U
//#define IBM_EXPONENT    0x7f000000U
//#define IBM_MANTISSA    0x00ffffffU
//#define IEEE_SIGN       0x80000000U
//#define IEEE_EXPONENT   0x7f800000U
//#define IEEE_MANTISSA   0x007fffffU
//#define IEEE_HIDDEN_BIT 0x00800000U
//#define IEEE32_BIAS     127
//#define IBM_32_BIAS     64

#define MAXP     6

/*
 *  Exit codes for the program:
 */
#define GOOD_EXIT         0
#define PARAM_ERROR       1
#define FILE_EXIST_ERROR  2
#define FILE_HEADER_ERROR 3
#define TOO_MANY_ERROR    4
#define EXTRA_HEADERS     5
#define BAD_FILE_HEADER   6
#define BAD_TRACE_HEADER  7
#define BAD_TRACE_DATA    8
#define FINISHED_HEADER   9
#define FINISHED_TRACE   10
#define FINISHED_OUTLINE 11
#define EXTRA_INPUT_ERR  12
#define SHORT_INPUT_ERR  13
#define SHORT_INPUT_ERR2 14
#define BAD_BRICKING_ERR 15
#define BAD_FLOAT_RQST   16
#define BAD_FLOAT_CNVT   17
#define BAD_FLOAT_INT1   18
#define BAD_FLOAT_INT2   19
#define BAD_FLOAT_RQST2  20
#define BAD_FLOAT_RQST3  21
#define BAD_CONVERT_FMT  22
#define BAD_FH_EXISTS    23
#define BAD_TH_EXISTS    24
#define BAD_TD_EXISTS    25

/*
 * Displacement of Binary Trace Header fields. These are
 * the numbers given in the SEGY Standard definition,
 * minus 3201, so that the first field (job id.) has
 * displacement zero. Each is defined in terms of the
 * *previous* field, plus the length of that *previous*
 * field:
 */

#define DJOBID		0
#define DLINENO		DJOBID+4
#define DREELNO		DLINENO+4
#define DNOTRACE	DREELNO+4
#define DNOATRACE	DNOTRACE+2
#define DSAMPINT	DNOATRACE+2
#define DSAMPINTO	DSAMPINT+2
#define DSAMPCNT	DSAMPINTO+2
#define DSAMPCNTO	DSAMPCNT+2
#define DDATAFMT	DSAMPCNTO+2
#define DENSMBFLD	DDATAFMT+2
#define DTRSRTCD	DENSMBFLD+2
#define DVARSCNCD	DTRSRTCD+2
#define DSWEEPFST	DVARSCNCD+2
#define	DSWEEPFEN	DSWEEPFST+2
#define DTAPERTP	DSWEEPFEN+2
#define DCORRDAT	DTAPERTP+2
#define DBINGNRCV	DCORRDAT+2
#define	DAMPRCV		DBINGNRCV+2
#define DMEASSYS	DAMPRCV+2
#define DIMPSGPOL	DMEASSYS+2
#define DVIBRPOLC	DIMPSGPOL+2


/* Forward declaration of procedures: */
int         tfclose     (FILE  *a);

#endif


/*******************************************************************************
 *  End of cnvseg2sth.h		
 *  Copyright 2013, 2012 Westheimer Energy Consulting Ltd. All rights reserved.
 ******************************************************************************/
