/*******************************************************************************
 *  Start of cnvsth2seg.h		
 *  Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 ******************************************************************************/

#ifndef CNVSTH2SEG
#define CNVSTH2SEG

//#define STHVERSION  "0.49"  /* Base version of cnvsth2seg for this program */
#define STHVERSION "0.67"
#define STHDATE  "16 September 2013:11.45"


/*
 *  This is the set of type definitions, and record definitions for
 *
 *                cnvsth2segT
 *                (based upon program cnvsth2seg version 0.49)
 *
 *  It contains:
 *     the lengths of the various records processed
 *     error exit conditions for the program
 *     definitions of the record field lengths
 *     definitions of the record field displacements
 *     global type definitions
 *     forward declarations of procedures
 */

/*
 *  DEBUG (Trace) flags (with mnemonics for a possible enclosing shell script)
 *
 *  1      t1    a Show File Header lengths & displacements
 *  2      t2    b Show Trace Header lengths & displacements
 *  4      t4    c Show Trace Data lengths & displacements
 *  8      t8    d FH and TH record lengths displayed, & samples at FH
 *  16     t3    e Details of EOF detection
 *  32     t5    f Record count at TH and TD reading
 *  64     t6    g Lengths at TH read, TH write
 *  128    t7    h Lengths at TD read, TD write
 *  256    t9    i Initial data display of TD
 *  512    t10   j Program loop length displays
 *  1024   t11   k TH field contents
 *  2048   t12   l TH field contents details
 *  4096   t13   m FH record contents
 *  8192   t14   n FH record contents details
 *  16384  t15   o Show per-record data lengths
 *
 *  If any trace flag is on, then some Trace Header and file opening 
 *  information and samples counts will be shown
 *
 */

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
 *  for the moment (Release 0.57) these are fixed maxima:
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
int         FHwrite     (unsigned char *a, int b, FILE *c, char *d);
int         DbgShowFlds (int t1, int t2, int t4);
int         DumpRec      (unsigned char *Rec, int len);
int         ShowTHRecord (unsigned char *THRec, int t12);
int         ShowFHRecord (unsigned char *FHRec, int t14);

#endif

/*******************************************************************************
 *  End of cnvsth2seg.h		
 *  Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 ******************************************************************************/
