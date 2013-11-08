/*******************************************************************************
 *  Start of cnvgethdr.h        
 *  Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 ******************************************************************************/

#ifndef CNVGETHDR
#define CNVGETHDR


#define STHVERSION "0.20"
#define STHDATE  "25 October 2013:10.40"


#define NAMELEN  4096

#define FHSIZE   3200
#define BHSIZE   400
#define THSIZE   240

#define MAXTYPE  8

#define MAXOUTFILES 10
#define MAXSAMPLES  8250

#define MAXBUFFER 65536

/*
 *  Maxima for the number of samples in each direction -
 *  for the moment (October 25 2013) these are fixed maxima:
 */
#define MAXTIMESLICE  99999
#define MAXCROSSSLICE 99999
#define MAXINSLICE    99999

/*
 *  Possible error exit conditions:
 */
#define PARAM_ERROR       1
#define BAD_FILE_HEADER   2
#define BAD_AUX_FHEAD     3
#define BAD_EXTRA_FH1     4
#define BAD_EXTRA_FH2     5
#define BAD_FH_WRITE1     6
#define BAD_FH_WRITE2     7
#define BAD_AUX_FHWRITE   8
#define BAD_EOF_ITH       9
#define BAD_MISSING_DATA 10
#define BAD_INITIAL_DATA 11
#define BAD_DATA_READ    12
#define BAD_DATA_READ2   13
#define BAD_DATA_WRITE   14
#define BAD_DATA_LENGTH  15
#define BAD_INPUT_END    16
#define BAD_OUTPUT_END   17
#define BAD_OUTPUT_OPEN  18
#define BAD_P_PARAM      19
#define BAD_SLICE_FORM   20
#define BAD_TIMELINE     21
#define BAD_INLINE       22
#define BAD_CROSSLINE    23
#define BAD_IFILE_EXISTS 24
#define BAD_XFILE_EXISTS 25
#define BAD_NFILE_EXISTS 26
#define BAD_TFILE_EXISTS 27

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
 *  The following constants may be used by the IEEE-IBM conversion settings:
 *  Note that (Version 0.57) some of these are commented out, and are not
 *  actually required by the current conversion code, but these comments
 *  should still remain for reference.
 */
#define NOCONVERT       0
#define CIBM            -1
#define CIEEE           -2
#define CBOTH           -3


/* Forward declaration of procedures: */
int         tfclose     (FILE  *a);

#endif

/*******************************************************************************
 *  End of cnvgethdr.h        
 *  Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 ******************************************************************************/
