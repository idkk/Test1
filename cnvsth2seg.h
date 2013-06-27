/*******************************************************************************
 *  Start of cnvsth2seg.h		
 *  Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 ******************************************************************************/

//#define STHVERSION  "0.49"  /* Base version of cnvsth2seg for this program */
#define STHVERSION "0.55"
#define STHDATE  "8 June 2013:11.45"


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

#define ISBIG    0
#define ISLITTLE 1

#define NAMELEN  4096

#define FHSIZE   3200
#define BHSIZE   400
#define THSIZE   240

#define MAXTYPE  8

#define MAXOUTFILES 10
#define MAXSAMPLES  8250

#define MAXBUFFER 65536

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
 */
#define NOCONVERT       0
#define CIBM2IEEE       1
#define CIEEE2IBM       2
#define CIEEE2BOTH      3
#define CIBM2BOTH       4
#define CIBMINPUT       -1
#define CIEEEINPUT      -2
#define CIBMINPUTB      -3
#define CIBM            -1
#define CIEEE           -2
#define CBOTH           -3
#define IBM_SIGN        0x80000000U
#define IBM_EXPONENT    0x7f000000U
#define IBM_MANTISSA    0x00ffffffU
#define IEEE_SIGN       0x80000000U
#define IEEE_EXPONENT   0x7f800000U
#define IEEE_MANTISSA   0x007fffffU
#define IEEE_HIDDEN_BIT 0x00800000U
#define IEEE32_BIAS     127
#define IBM_32_BIAS     64

/*
 *  The lengths and positions of the various fields in the input records.
 *  Each position is defined in terms of the position of the *previous*
 *  field, plus the length of the *previous* field. All lengths are in bytes.
 *  The lengths/positions of the File Header record are LFH_/PFH_, for
 *  Trace Header it is LTH_/PTH_, and for the Trace Data LTD_/PTD_.
 */

/* File Header Lengths: */
#define  LFH_RECLEN         2   /* Record length (exclusive of this field) */
#define  LFH_RECID          1   /* File Header record ID ('H')             */
#define  LFH_FILEID         2   /* File Header file id.                    */
#define  LFH_FILENAME      16   /* Filename                                */
#define  LFH_CHUNKSIZE      2   /* Chunksize, in samples                   */
/* The above is the SEG2STH prefix; SEG2 record starts here: */
#define  LFH_CARD01        80
#define  LFH_CARD02        80
#define  LFH_CARD03        80
#define  LFH_CARD04        80
#define  LFH_CARD05        80
#define  LFH_CARD06        80
#define  LFH_CARD07        80
#define  LFH_CARD08        80
#define  LFH_CARD09        80
#define  LFH_CARD10        80
#define  LFH_CARD11        80
#define  LFH_CARD12        80
#define  LFH_CARD13        80
#define  LFH_CARD14        80
#define  LFH_CARD15        80
#define  LFH_CARD16        80
#define  LFH_CARD17        80
#define  LFH_CARD18        80
#define  LFH_CARD19        80
#define  LFH_CARD20        80
#define  LFH_CARD21        80
#define  LFH_CARD22        80
#define  LFH_CARD23        80
#define  LFH_CARD24        80
#define  LFH_CARD25        80
#define  LFH_CARD26        80
#define  LFH_CARD27        80
#define  LFH_CARD28        80
#define  LFH_CARD29        80
#define  LFH_CARD30        80
#define  LFH_CARD31        80
#define  LFH_CARD32        80
#define  LFH_CARD33        80
#define  LFH_CARD34        80
#define  LFH_CARD35        80
#define  LFH_CARD36        80
#define  LFH_CARD37        80
#define  LFH_CARD38        80
#define  LFH_CARD39        80
#define  LFH_CARD40        80
/*  The above is the base File Header record;
 *  what follows is the Binary File Header, which is contiguous
 *  with the first base (textual) File Header record: */
#define  LFH_JOBIDNO        4   /* Job Identification Number                  */
#define  LFH_LINENO         4   /* Line Number                                */
#define  LFH_REELNO         4   /* Reel Number                                */
#define  LFH_NTREC          2   /* Number of data traces per ensemble         */
#define  LFH_NAUXREC        2   /* Number of auxilliary traces per ensemble   */
#define  LFH_INTREELDA      2   /* Sample interval in microseconds            */
#define  LFH_INTORGFLD      2   /* Samp. int. in mic.secs., org. fld. recding */
#define  LFH_SAMPTHISR      2   /* No. samples per data trace                 */
#define  LFH_SAMPORGFLD     2   /* No. samples per data trace, org. fld. rec. */
#define  LFH_DATSAMPFC      2   /* Data sample format code                    */
#define  LFH_CDPFLD         2   /* Ensemble fold                              */
#define  LFH_TRACESRTCD     2   /* Trace sorting code (type of ensemble)      */
#define  LFH_VERTSUMCD      2   /* Vertical sum code                          */
#define  LFH_SWFRSTRT       2   /* Sweep frequency at start (Hz)              */
#define  LFH_SWFREND        2   /* Sweep frequency at end (Hz)                */
#define  LFH_SWLEN          2   /* Sweep length (ms)                          */
#define  LFH_SWTYPCD        2   /* Sweep type code                            */
#define  LFH_TRNOSWCH       2   /* Trace number of sweep channel              */
#define  LFH_SWTRTLST       2   /* Sweep trace taper length at start (ms)     */
#define  LFH_SWTRTLEND      2   /* Sweep trace taper length at end (ms)       */
#define  LFH_TAPTYPCD       2   /* Taper type                                 */
#define  LFH_CORRDATTR      2   /* Correlated data traces                     */
#define  LFH_BINGAINREC     2   /* Binary gain recovered                      */
#define  LFH_AMPRECMCD      2   /* Amplitude recovery method                  */
#define  LFH_MSRMSYS        2   /* Measurement system                         */
#define  LFH_IMPSIGPOL      2   /* Impulse signal polarity                    */
#define  LFH_VIBPOLCD       2   /* Vibratory polarity code                    */
#define  LFH_HFILL1       170
#define  LFH_HFILL2       170
/* Fields which lie within HFILL1 and HFILL2: */
#define  LFH_UNDEF1       240   /* Undefined                                  */
#define  LFH_SEGYREVNO      2   /* SEGY Format Revision Number                */
#define  LFH_FLTRFLG        2   /* Fixed length trace flag                    */
#define  LFH_NETFHREC       2   /* Number of extended file header records     */
#define  LFH_UNDEF2        94   /* Undefined                                  */

#define  LEN_FH \
40*(LFH_CARD01)+LFH_JOBIDNO+LFH_LINENO+LFH_REELNO+LFH_NTREC+LFH_NAUXREC+\
LFH_INTREELDA+LFH_INTORGFLD+LFH_SAMPTHISR+LFH_SAMPORGFLD+LFH_DATSAMPFC+\
LFH_CDPFLD+LFH_TRACESRTCD+LFH_VERTSUMCD+LFH_SWFRSTRT+LFH_SWFREND+LFH_SWLEN+\
LFH_SWTYPCD+LFH_TRNOSWCH+LFH_SWTRTLST+LFH_SWTRTLEND+LFH_TAPTYPCD+\
LFH_CORRDATTR+LFH_BINGAINREC+LFH_AMPRECMCD+LFH_MSRMSYS+LFH_IMPSIGPOL+\
LFH_VIBPOLCD+LFH_HFILL1+LFH_HFILL2
#define   LEN_FH_4  LEN_FH / 4

#define  LEN_FH_PREFIX FH_RECLEN+LFH_RECID+LFH_FILEID+LFH_FILENAME+LFH_CHUNKSIZE

#define  LEN_FH_FULL LEN_FH+LEN_FH_PREFIX

/* Trace Header Lengths: */
#define  LTH_RECLEN         2   /* Record length (exclusive of this field)    */
#define  LTH_RECID          1   /* Trace Header record ID ('T')               */
#define  LTH_FILEID         2   /* Trace Header record file id.               */
#define  LTH_FILENAME      16   /* Filename                                   */
#define  LTH_X              4   /* (repeated) X co-ordinate                   */
#define  LTH_Y              4   /* (repeated) Y co-ordinate                   */
#define  LTH_XL             4   /* (repeated) Cross Line                      */
#define  LTH_IL             4   /* (repeated) Inline                          */
#define  LTH_MAXSAMP        4   /* (computed) Maximum sample ?count?          */
#define  LTH_CHUNKSIZE      2   /* Chunksize, in samples per chunk            */
#define  LTH_TRHDRID        4   /* Trace header record id.                    */
/* The above is the SEG2STH prefix; SEG2 Record starts here: */
#define  LTH_TRSEQNOL       4   /* Trace sequence number within line          */
#define  LTH_TRSEQNOR       4   /* Trace sequence number within SEGY file     */
#define  LTH_ORGFLDRNO      4   /* Original field record number               */
#define  LTH_TRSEQNO        4   /* Trace number within original field record  */
#define  LTH_ERGSRCPT       4   /* Energy source point number                 */
#define  LTH_CDPENSNO       4   /* Ensemble number                            */
#define  LTH_TRSEQNOCDP     4   /* Trace number within ensemble               */
#define  LTH_TRIDCD         2   /* Trace identification code                  */
#define  LTH_NOVERTSUM      2   /* No. vertically summed traces yielding this */
#define  LTH_NOHORST        2   /* No. horiz. stacked traces yielding this tr.*/
#define  LTH_DATAUSE        2   /* Data use                                   */
#define  LTH_DISTSTOR       4   /* Distance source centr grp. to receiver grp.*/
#define  LTH_RECGRPELE      4   /* Receiver group elevation                   */
#define  LTH_SURFELEAS      4   /* Surface elevation at source                */
#define  LTH_SORDBS         4   /* Source depth below surface                 */
#define  LTH_DATELERECGP    4   /* Datum elevation at receiver group          */
#define  LTH_DATELEAS       4   /* Datum elevation at source                  */
#define  LTH_WDEPASRC       4   /* Water depth at source                      */
#define  LTH_WDEPAGRP       4   /* Water depth at group                       */
#define  LTH_SCALELEAD      2   /* Scalar to apply to give real depth values  */
#define  LTH_SCALCORD       2   /* Scalar to apply to co-ordinates            */
#define  LTH_XSRCCORD       4   /* Source co-ordinate X                       */
#define  LTH_YSRCCORD       4   /* Source co-ordinate Y                       */
#define  LTH_XRECGRPCORD    4   /* Group co-ordinate X                        */
#define  LTH_YRECGRPCORD    4   /* Group co-ordinate Y                        */
#define  LTH_CORDUS         2   /* Co-ordinate units                          */
#define  LTH_WEATHVEL       2   /* Weathering velocity                        */
#define  LTH_SUBWEATHVEL    2   /* Subweathering velocity                     */
#define  LTH_UPHTAS         2   /* Uphole time at source in ms.               */
#define  LTH_UPHTARECGRP    2   /* Uphole time at group in ms.                */
#define  LTH_SRCSTCORR      2   /* Source static correction in ms.            */
#define  LTH_RECGRSTCORR    2   /* Group static correction in ms.             */
#define  LTH_TOTSTAP        2   /* Total static applied in ms.                */
#define  LTH_LAGTEOH        2   /* Lag time A - ms.                           */
#define  LTH_LAGTBAS        2   /* Lag time B - ms.                           */
#define  LTH_LAGTSARS       2   /* Delay recording time ms.                   */
#define  LTH_SRTMT          2   /* Mute time start - ms.                      */
#define  LTH_ENDMT          2   /* Mute time end - ms.                        */
#define  LTH_NOSAMPTR       2   /* Number of samples in this trace            */
#define  LTH_SAMPINTTR      2   /* Sample interval in microseconds this trace */
#define  LTH_FLDINSTGC      2   /* Gain type of field instruments             */
#define  LTH_INSTGC         2   /* Instrument gain constant (dB)              */
#define  LTH_INSTEG         2   /* Instrument early or initial gain (dB)      */
#define  LTH_CORRLTD        2   /* Correlated                                 */
#define  LTH_SWFRSTRT       2   /* Sweep frequency at start (Hz)              */
#define  LTH_SWFREND        2   /* Sweep frequency at end (Hz)                */
#define  LTH_SWLEN          2   /* Sweep length (ms)                          */
#define  LTH_SWTYPCD        2   /* Sweep type                                 */
#define  LTH_SWTAPTLSTRT    2   /* Sweep trace taper length at start (ms)     */
#define  LTH_SWTAPTLEND     2   /* Sweep trace taper length at end (ms)       */ 
#define  LTH_TAPTYPCD       2   /* Taper type                                 */
#define  LTH_ALFILTFREQ     2   /* Alias filter frequency (Hz)                */
#define  LTH_ALFILTSLP      2   /* Alias filter slope (dB/octave)             */
#define  LTH_NTCHFILTFREQ   2   /* Notch filter frequency (Hz)                */
#define  LTH_NTCHFILTSLP    2   /* Notch filter slope (dB/octave)             */
#define  LTH_LOWCFREW       2   /* Low-cut frequency (Hz)                     */
#define  LTH_HICFREQ        2   /* High-cut frequency (Hz)                    */
#define  LTH_LOWCSLP        2   /* Low-cut slope (dB/octave)                  */
#define  LTH_HICSLP         2   /* High-cut slope (dB/octave)                 */
#define  LTH_YRDATREC       2   /* Year data recorded                         */
#define  LTH_DAYOFYEAR      2   /* Day of year                                */
#define  LTH_HOUROFDAY      2   /* Hour of day (24-hour clock)                */
#define  LTH_MINOFHOUR      2   /* Minute of hour                             */
#define  LTH_SECOFMIN       2   /* Second of minute                           */
#define  LTH_TIMEBASIS      2   /* Time basis code                            */
#define  LTH_TRWTFAC        2   /* Trace weighting factor                     */
#define  LTH_ROLLSWPOS1     2   /* Geophone group number roll switch pos. one */
#define  LTH_FSTTRORG       2   /* Geo. grp. no. of trace no. 1 in org fld rec*/
#define  LTH_LASTTRORG      2   /* Geo. grp. no. last trace within org record */
#define  LTH_GAPSIZE        2   /* Gap size (total number of groups dropped)  */
#define  LTH_OVERAST        2   /* Overtravel associated with taper           */
#define  LTH_BYTE181        4   /* X co-ordinate of ensemble (CDP)            */
#define  LTH_BYTE185        4   /* Y co-ordinate of ensemble (CDP)            */
#define  LTH_BYTE189        4   /* Crossline number                           */
#define  LTH_BYTE193        4   /* Shotpoint number                           */
#define  LTH_BYTE197        4
#define  LTH_BYTE201        4
#define  LTH_BYTE205        4
#define  LTH_BYTE209        4
#define  LTH_BYTE213        4
#define  LTH_BYTE217        4
#define  LTH_BYTE221        4
#define  LTH_BYTE225        4
#define  LTH_BYTE229        4
#define  LTH_BYTE233        4
#define  LTH_BYTE237        4
/* Redefinition of fields in the byte181-byte237 range, where needed: */
#define  LTH_XCOORDENS      4   /* X co-ordinate of ensemble (CDP)            */
#define  LTH_YCOORDENS      4   /* Y co-ordinate of ensemble (CDP)            */
#define  LTH_XLNUMBER       4   /* Crossline number                           */
#define  LTH_SHOTPNO        4   /* Shotpoint number                           */
#define  LTH_SCALSPNO       2   /* Scalar to apply to shotpoint number        */
#define  LTH_TRVALMU        2   /* Trace value measurement unit               */
#define  LTH_TRANSCNST      6   /* Transduction constant                      */
#define  LTH_TRANSUNIT      2   /* Transduction units                         */
#define  LTH_DEVTRID        2   /* Device Trace Identifier                    */
#define  LTH_TIMSCALAR      2   /* Time scalar to apply                       */
#define  LTH_SRCTORIENT     2   /* Source type orientation                    */
#define  LTH_SRCENGDIR      6   /* Source energy direction                    */
#define  LTH_SRCMEASURE     6   /* Source measurement                         */
#define  LTH_SRCMEASUN      2   /* Source measurement unit                    */

#define  LEN_TH  LTH_TRSEQNOL+\
LTH_TRSEQNOR+LTH_ORGFLDRNO+LTH_TRSEQNO+LTH_ERGSRCPT+LTH_CDPENSNO+\
LTH_TRSEQNOCDP+LTH_TRIDCD+LTH_NOVERTSUM+LTH_NOHORST+LTH_DATAUSE+LTH_DISTSTOR+\
LTH_RECGRPELE+LTH_SURFELEAS+LTH_SORDBS+LTH_DATELERECGP+LTH_DATELEAS+\
LTH_WDEPASRC+LTH_WDEPAGRP+LTH_SCALELEAD+LTH_SCALCORD+LTH_XSRCCORD+LTH_YSRCCORD+\
LTH_XRECGRPCORD+LTH_YRECGRPCORD+LTH_CORDUS+LTH_WEATHVEL+LTH_SUBWEATHVEL+\
LTH_UPHTAS+LTH_UPHTARECGRP+LTH_SRCSTCORR+LTH_RECGRSTCORR+LTH_TOTSTAP+\
LTH_LAGTEOH+LTH_LAGTBAS+LTH_LAGTSARS+LTH_SRTMT+LTH_ENDMT+LTH_NOSAMPTR+\
LTH_SAMPINTTR+LTH_FLDINSTGC+LTH_INSTGC+LTH_INSTEG+LTH_CORRLTD+LTH_SWFRSTRT+\
LTH_SWFREND+LTH_SWLEN+LTH_SWTYPCD+LTH_SWTAPTLSTRT+LTH_SWTAPTLEND+\
LTH_TAPTYPCD+LTH_ALFILTFREQ+LTH_ALFILTSLP+LTH_NTCHFILTFREQ+LTH_NTCHFILTSLP+\
LTH_LOWCFREW+LTH_HICFREQ+LTH_LOWCSLP+LTH_HICSLP+LTH_YRDATREC+LTH_DAYOFYEAR+\
LTH_HOUROFDAY+LTH_MINOFHOUR+LTH_SECOFMIN+LTH_TIMEBASIS+LTH_TRWTFAC+\
LTH_ROLLSWPOS1+LTH_FSTTRORG+LTH_LASTTRORG+LTH_GAPSIZE+LTH_OVERAST+LTH_BYTE181+\
LTH_BYTE185+LTH_BYTE189+LTH_BYTE193+LTH_BYTE197+LTH_BYTE201+LTH_BYTE205+\
LTH_BYTE209+LTH_BYTE213+LTH_BYTE217+LTH_BYTE221+LTH_BYTE225+LTH_BYTE229+\
LTH_BYTE233+LTH_BYTE237

#define  LEN_TH_4  LEN_TH / 4

#define  LEN_TH_PREFIX LTH_RECLEN+LTH_RECID+LTH_FILEID+LTH_FILENAME+LTH_X+\
LTH_Y+LTH_XL+LTH_IL+LTH_MAXSAMP+LTH_CHUNKSIZE+LTH_TRHDRID

#define  LEN_TH_FULL  LEN_TH_PREFIX+LEN_TH

/* Trace Data Lengths: */
#define  LTD_RECLEN         2   /* Data record length excluding this field    */
#define  LTD_RECID          1   /* Trace data record id ('D')                 */
#define  LTD_FILEID         2   /* Trace data file id.                        */
#define  LTD_TRHDRID        4   /* Trace data header id.                      */
#define  LTD_TRDATAID       2   /* Trace data id.                             */
#define  LTD_LOBLEN         8   /* LOB field length - 64 bits                 */

#define  LEN_TD_PREFIX  LTD_RECLEN+LTD_RECID+LTD_FILEID+LTD_TRHDRID+\
LTD_TRDATAID+LTD_LOBLEN

/* File Header Displacements: */
#define  PFH_RECLEN         0
#define  PFH_RECID          PFH_RECLEN+LFH_RECLEN
#define  PFH_FILEID         PFH_RECID+LFH_RECID
#define  PFH_FILENAME       PFH_FILEID+LFH_FILEID
#define  PFH_CHUNKSIZE      PFH_FILENAME+LFH_FILENAME
/* The above is the SEG2STH prefix area; SEG2 record starts here: */
#define  PFH_CARD01         PFH_CHUNKSIZE+LFH_CHUNKSIZE
#define  PFH_START          PFH_CARD01
#define  PFH_CARD02         PFH_CARD01+LFH_CARD01
#define  PFH_CARD03         PFH_CARD02+LFH_CARD02
#define  PFH_CARD04         PFH_CARD03+LFH_CARD03
#define  PFH_CARD05         PFH_CARD04+LFH_CARD04
#define  PFH_CARD06         PFH_CARD05+LFH_CARD05
#define  PFH_CARD07         PFH_CARD06+LFH_CARD06
#define  PFH_CARD08         PFH_CARD07+LFH_CARD07
#define  PFH_CARD09         PFH_CARD08+LFH_CARD08
#define  PFH_CARD10         PFH_CARD09+LFH_CARD09
#define  PFH_CARD11         PFH_CARD10+LFH_CARD10
#define  PFH_CARD12         PFH_CARD11+LFH_CARD11
#define  PFH_CARD13         PFH_CARD12+LFH_CARD12
#define  PFH_CARD14         PFH_CARD13+LFH_CARD13
#define  PFH_CARD15         PFH_CARD14+LFH_CARD14
#define  PFH_CARD16         PFH_CARD15+LFH_CARD15
#define  PFH_CARD17         PFH_CARD16+LFH_CARD16
#define  PFH_CARD18         PFH_CARD17+LFH_CARD17
#define  PFH_CARD19         PFH_CARD18+LFH_CARD18
#define  PFH_CARD20         PFH_CARD19+LFH_CARD19
#define  PFH_CARD21         PFH_CARD20+LFH_CARD20
#define  PFH_CARD22         PFH_CARD21+LFH_CARD21
#define  PFH_CARD23         PFH_CARD22+LFH_CARD22
#define  PFH_CARD24         PFH_CARD23+LFH_CARD23
#define  PFH_CARD25         PFH_CARD24+LFH_CARD24
#define  PFH_CARD26         PFH_CARD25+LFH_CARD25
#define  PFH_CARD27         PFH_CARD26+LFH_CARD26
#define  PFH_CARD28         PFH_CARD27+LFH_CARD27
#define  PFH_CARD29         PFH_CARD28+LFH_CARD28
#define  PFH_CARD30         PFH_CARD29+LFH_CARD29
#define  PFH_CARD31         PFH_CARD30+LFH_CARD30
#define  PFH_CARD32         PFH_CARD31+LFH_CARD31
#define  PFH_CARD33         PFH_CARD32+LFH_CARD32
#define  PFH_CARD34         PFH_CARD33+LFH_CARD33
#define  PFH_CARD35         PFH_CARD34+LFH_CARD34
#define  PFH_CARD36         PFH_CARD35+LFH_CARD35
#define  PFH_CARD37         PFH_CARD36+LFH_CARD36
#define  PFH_CARD38         PFH_CARD37+LFH_CARD37
#define  PFH_CARD39         PFH_CARD38+LFH_CARD38
#define  PFH_CARD40         PFH_CARD39+LFH_CARD39
#define  PFH_JOBIDNO        PFH_CARD40+LFH_CARD40  
#define  PFH_BINSTART       PFH_JOBIDNO
#define  PFH_LINENO         PFH_JOBIDNO+LFH_JOBIDNO
#define  PFH_REELNO         PFH_LINENO+LFH_LINENO
#define  PFH_NTREC          PFH_REELNO+LFH_REELNO
#define  PFH_NAUXREC        PFH_NTREC+LFH_NTREC
#define  PFH_INTREELDA      PFH_NAUXREC+LFH_NAUXREC
#define  PFH_INTORGFLD      PFH_INTREELDA+LFH_INTREELDA
#define  PFH_SAMPTHISR      PFH_INTORGFLD+LFH_INTORGFLD
#define  PFH_SAMPORGFLD     PFH_SAMPTHISR+LFH_SAMPTHISR
#define  PFH_DATSAMPFC      PFH_SAMPORGFLD+LFH_SAMPORGFLD
#define  PFH_CDPFLD         PFH_DATSAMPFC+LFH_DATSAMPFC
#define  PFH_TRACESRTCD     PFH_CDPFLD+LFH_CDPFLD
#define  PFH_VERTSUMCD      PFH_TRACESRTCD+LFH_TRACESRTCD
#define  PFH_SWFRSTRT       PFH_VERTSUMCD+LFH_VERTSUMCD
#define  PFH_SWFREND        PFH_SWFRSTRT+LFH_SWFRSTRT
#define  PFH_SWLEN          PFH_SWFREND+LFH_SWFREND
#define  PFH_SWTYPCD        PFH_SWLEN+LFH_SWLEN
#define  PFH_TRNOSWCH       PFH_SWTYPCD+LFH_SWTYPCD
#define  PFH_SWTRTLST       PFH_TRNOSWCH+LFH_TRNOSWCH
#define  PFH_SWTRTLEND      PFH_SWTRTLST+LFH_SWTRTLST
#define  PFH_TAPTYPCD       PFH_SWTRTLEND+LFH_SWTRTLEND
#define  PFH_CORRDATTR      PFH_TAPTYPCD+LFH_TAPTYPCD
#define  PFH_BINGAINREC     PFH_CORRDATTR+LFH_CORRDATTR
#define  PFH_AMPRECMCD      PFH_BINGAINREC+LFH_BINGAINREC
#define  PFH_MSRMSYS        PFH_AMPRECMCD+LFH_AMPRECMCD
#define  PFH_IMPSIGPOL      PFH_MSRMSYS+LFH_MSRMSYS
#define  PFH_VIBPOLCD       PFH_IMPSIGPOL+LFH_IMPSIGPOL
#define  PFH_HFILL1         PFH_VIBPOLCD+LFH_VIBPOLCD
#define  PFH_HFILL2         PFH_HFILL1+LFH_HFILL1
#define  PFH_E_O_R          PFH_HFILL2+LFH_HFILL2
/* Also indicate some of the Binary fieds which are within HFILL2 and HFILL2: */
#define  PFH_UNDEF1         PFH_VIBPOLCD+LFH_VIBPOLCD
#define  PFH_SEGYREVNO      PFH_UNDEF1+LFH_UNDEF1
#define  PFH_FLTRFLG        PFH_SEGYREVNO+LFH_SEGYREVNO
#define  PFH_NETFHREC       PFH_FLTRFLG+LFH_FLTRFLG
#define  PFH_UNDEF2         PFH_NETFHREC+LFH_NETFHREC

/* Trace Header Displacements: */
#define  PTH_RECLEN         0
#define  PTH_RECID          PTH_RECLEN+LTH_RECLEN
#define  PTH_FILEID         PTH_RECID+LTH_RECID
#define  PTH_FILENAME       PTH_FILEID+LTH_FILEID
#define  PTH_X              PTH_FILENAME+LTH_FILENAME
#define  PTH_Y              PTH_X+LTH_X
#define  PTH_XL             PTH_Y+LTH_Y
#define  PTH_IL             PTH_XL+LTH_XL
#define  PTH_MAXSAMP        PTH_IL+LTH_IL
#define  PTH_CHUNKSIZE      PTH_MAXSAMP+LTH_MAXSAMP
#define  PTH_TRHDRID        PTH_CHUNKSIZE+LTH_CHUNKSIZE
/* The above is the SEG2STH prefix area; SEG2 record starts here: */
#define  PTH_TRSEQNOL       PTH_TRHDRID+LTH_TRHDRID
#define  PTH_START          PTH_TRSEQNOL
#define  PTH_TRSEQNOR       PTH_TRSEQNOL+LTH_TRSEQNOL
#define  PTH_ORGFLDRNO      PTH_TRSEQNOR+LTH_TRSEQNOR
#define  PTH_TRSEQNO        PTH_ORGFLDRNO+LTH_ORGFLDRNO
#define  PTH_ERGSRCPT       PTH_TRSEQNO+LTH_TRSEQNO
#define  PTH_CDPENSNO       PTH_ERGSRCPT+LTH_ERGSRCPT
#define  PTH_TRSEQNOCDP     PTH_CDPENSNO+LTH_CDPENSNO
#define  PTH_TRIDCD         PTH_TRSEQNOCDP+LTH_TRSEQNOCDP
#define  PTH_NOVERTSUM      PTH_TRIDCD+LTH_TRIDCD
#define  PTH_NOHORST        PTH_NOVERTSUM+LTH_NOVERTSUM
#define  PTH_DATAUSE        PTH_NOHORST+LTH_NOHORST
#define  PTH_DISTSTOR       PTH_DATAUSE+LTH_DATAUSE
#define  PTH_RECGRPELE      PTH_DISTSTOR+LTH_DISTSTOR
#define  PTH_SURFELEAS      PTH_RECGRPELE+LTH_RECGRPELE
#define  PTH_SORDBS         PTH_SURFELEAS+LTH_SURFELEAS
#define  PTH_DATELERECGP    PTH_SORDBS+LTH_SORDBS
#define  PTH_DATELEAS       PTH_DATELERECGP+LTH_DATELERECGP
#define  PTH_WDEPASRC       PTH_DATELEAS+LTH_DATELEAS
#define  PTH_WDEPAGRP       PTH_WDEPASRC+LTH_WDEPASRC
#define  PTH_SCALELEAD      PTH_WDEPAGRP+LTH_WDEPAGRP
#define  PTH_SCALCORD       PTH_SCALELEAD+LTH_SCALELEAD
#define  PTH_XSRCCORD       PTH_SCALCORD+LTH_SCALCORD
#define  PTH_YSRCCORD       PTH_XSRCCORD+LTH_XSRCCORD
#define  PTH_XRECGRPCORD    PTH_YSRCCORD+LTH_YSRCCORD
#define  PTH_YRECGRPCORD    PTH_XRECGRPCORD+LTH_XRECGRPCORD
#define  PTH_CORDUS         PTH_YRECGRPCORD+LTH_YRECGRPCORD
#define  PTH_WEATHVEL       PTH_CORDUS+LTH_CORDUS
#define  PTH_SUBWEATHVEL    PTH_WEATHVEL+LTH_WEATHVEL
#define  PTH_UPHTAS         PTH_SUBWEATHVEL+LTH_SUBWEATHVEL
#define  PTH_UPHTARECGRP    PTH_UPHTAS+LTH_UPHTAS
#define  PTH_SRCSTCORR      PTH_UPHTARECGRP+LTH_UPHTARECGRP
#define  PTH_RECGRSTCORR    PTH_SRCSTCORR+LTH_SRCSTCORR
#define  PTH_TOTSTAP        PTH_RECGRSTCORR+LTH_RECGRSTCORR
#define  PTH_LAGTEOH        PTH_TOTSTAP+LTH_TOTSTAP
#define  PTH_LAGTBAS        PTH_LAGTEOH+LTH_LAGTEOH
#define  PTH_LAGTSARS       PTH_LAGTBAS+LTH_LAGTBAS
#define  PTH_SRTMT          PTH_LAGTSARS+LTH_LAGTSARS
#define  PTH_ENDMT          PTH_SRTMT+LTH_SRTMT
#define  PTH_NOSAMPTR       PTH_ENDMT+LTH_ENDMT
#define  PTH_SAMPINTTR      PTH_NOSAMPTR+LTH_NOSAMPTR
#define  PTH_FLDINSTGC      PTH_SAMPINTTR+LTH_SAMPINTTR
#define  PTH_INSTGC         PTH_FLDINSTGC+LTH_FLDINSTGC
#define  PTH_INSTEG         PTH_INSTGC+LTH_INSTGC
#define  PTH_CORRLTD        PTH_INSTEG+LTH_INSTEG
#define  PTH_SWFRSTRT       PTH_CORRLTD+LTH_CORRLTD
#define  PTH_SWFREND        PTH_SWFRSTRT+LTH_SWFRSTRT
#define  PTH_SWLEN          PTH_SWFREND+LTH_SWFREND
#define  PTH_SWTYPCD        PTH_SWLEN+LTH_SWLEN
#define  PTH_SWTAPTLSTRT    PTH_SWTYPCD+LTH_SWTYPCD
#define  PTH_SWTAPTLEND     PTH_SWTAPTLSTRT+LTH_SWTAPTLSTRT
#define  PTH_TAPTYPCD       PTH_SWTAPTLEND+LTH_SWTAPTLEND
#define  PTH_ALFILTFREQ     PTH_TAPTYPCD+LTH_TAPTYPCD
#define  PTH_ALFILTSLP      PTH_ALFILTFREQ+LTH_ALFILTFREQ
#define  PTH_NTCHFILTFREQ   PTH_ALFILTSLP+LTH_ALFILTSLP
#define  PTH_NTCHFILTSLP    PTH_NTCHFILTFREQ+LTH_NTCHFILTFREQ
#define  PTH_LOWCFREW       PTH_NTCHFILTSLP+LTH_NTCHFILTSLP
#define  PTH_HICFREQ        PTH_LOWCFREW+LTH_LOWCFREW
#define  PTH_LOWCSLP        PTH_HICFREQ+LTH_HICFREQ
#define  PTH_HICSLP         PTH_LOWCSLP+LTH_LOWCSLP
#define  PTH_YRDATREC       PTH_HICSLP+LTH_HICSLP
#define  PTH_DAYOFYEAR      PTH_YRDATREC+LTH_YRDATREC
#define  PTH_HOUROFDAY      PTH_DAYOFYEAR+LTH_DAYOFYEAR
#define  PTH_MINOFHOUR      PTH_HOUROFDAY+LTH_HOUROFDAY
#define  PTH_SECOFMIN       PTH_MINOFHOUR+LTH_MINOFHOUR
#define  PTH_TIMEBASIS      PTH_SECOFMIN+LTH_SECOFMIN
#define  PTH_TRWTFAC        PTH_TIMEBASIS+LTH_TIMEBASIS
#define  PTH_ROLLSWPOS1     PTH_TRWTFAC+LTH_TRWTFAC
#define  PTH_FSTTRORG       PTH_ROLLSWPOS1+LTH_ROLLSWPOS1
#define  PTH_LASTTRORG      PTH_FSTTRORG+LTH_FSTTRORG
#define  PTH_GAPSIZE        PTH_LASTTRORG+LTH_LASTTRORG
#define  PTH_OVERAST        PTH_GAPSIZE+LTH_GAPSIZE
#define  PTH_BYTE181        PTH_OVERAST+LTH_OVERAST
#define  PTH_BYTE185        PTH_BYTE181+LTH_BYTE181
#define  PTH_BYTE189        PTH_BYTE185+LTH_BYTE185
#define  PTH_BYTE193        PTH_BYTE189+LTH_BYTE189
#define  PTH_BYTE197        PTH_BYTE193+LTH_BYTE193
#define  PTH_BYTE201        PTH_BYTE197+LTH_BYTE197
#define  PTH_BYTE205        PTH_BYTE201+LTH_BYTE201
#define  PTH_BYTE209        PTH_BYTE205+LTH_BYTE205
#define  PTH_BYTE213        PTH_BYTE209+LTH_BYTE209
#define  PTH_BYTE217        PTH_BYTE213+LTH_BYTE213
#define  PTH_BYTE221        PTH_BYTE217+LTH_BYTE217
#define  PTH_BYTE225        PTH_BYTE221+LTH_BYTE221
#define  PTH_BYTE229        PTH_BYTE225+LTH_BYTE225
#define  PTH_BYTE233        PTH_BYTE229+LTH_BYTE229
#define  PTH_BYTE237        PTH_BYTE233+LTH_BYTE233
#define  PTH_E_O_R          PTH_BYTE237+LTH_BYTE237
/* Redefinitions of fields in the Byte181-byte237 region, where needed: */
#define  PTH_XCOORDENS      PTH_OVERAST+LTH_OVERAST
#define  PTH_YCOORDENS      PTH_XCOORDENS+LTH_XCOORDENS
#define  PTH_XLNUMBER       PTH_YCOORDENS+LTH_YCOORDENS
#define  PTH_SHOTPNO        PTH_XLNUMBER+LTH_XLNUMBER
#define  PTH_SCALSPNO       PTH_SHOTPNO+LTH_SHOTPNO
#define  PTH_TRVALMU        PTH_SCALSPNO+LTH_SCALSPNO
#define  PTH_TRANSCNST      PTH_TRVALMU+LTH_TRVALMU
#define  PTH_TRANSUNIT      PTH_TRANSCNST+LTH_TRANSCNST
#define  PTH_DEVTRID        PTH_TRANSUNIT+LTH_TRANSUNIT
#define  PTH_TIMSCALAR      PTH_DEVTRID+LTH_DEVTRID
#define  PTH_SRCTORIENT     PTH_TIMSCALAR+LTH_TIMSCALAR
#define  PTH_SRCENGDIR      PTH_SRCTORIENT+LTH_SRCTORIENT
#define  PTH_SRCMEASURE     PTH_SRCENGDIR+LTH_SRCENGDIR
#define  PTH_SRCMEASUN      PTH_SRCMEASURE+LTH_SRCMEASURE

/*  Trace Data Displacments: */
#define  PTD_RECLEN         0
#define  PTD_RECID          PTD_RECLEN+LTD_RECLEN
#define  PTD_FILEID         PTD_RECID+LTD_RECID
#define  PTD_TRHDRID        PTD_FILEID+LTD_FILEID
#define  PTD_TRDATAID       PTD_TRHDRID+LTD_TRHDRID
#define  PTD_LOBLEN         PTD_TRDATAID+LTD_TRDATAID
#define  PTD_E_O_R          PTD_LOBLEN+LTD_LOBLEN
/* The whole of the above is the SEG2STH prefix area */


/* Global type definitions: */
#define int16_t    short int
#define uint16_t   unsigned short int
#define int32_t    int
#define uint32_t   unsigned int

/* Forward declaration of procedures: */
int         tfclose     (FILE  *a);
void        FHwrite     (unsigned char *a, int *b, FILE *c, char *d);
int         DbgShowFlds (int t1, int t2, int t4);
int         DumpRec      (unsigned char *Rec, int len);
int         ShowTHRecord (unsigned char *THRec, int t12);
int         ShowFHRecord (unsigned char *FHRec, int t14);
int16_t		swap_int16	(int16_t a);
uint16_t	swap_uint16 (uint16_t a);
int32_t		swap_int32	(int32_t a);
uint32_t	swap_uint32	(uint32_t a);
double      swap_double (unsigned long long a);
uint32_t    swap_single (uint32_t a);
void ieee2ibm(void *to, const void *from, int len);
void ibm2ieee(void *to, const void *from, int len);

/* 32-bit Endian definition of *this* machine: */
/*
 * Note that this definition can be
 * (a) extended to cover other formats, such as PDP (0xad), and
 * (b) altered to err at compile time if the format is not
 *     recognised (remove UNDEFINED and uncomment the assert)
 */
static uint32_t endianness = 0xdeadbeef; 
enum endian_type { BIG, LITTLE, UNDEFINED };

#define ENDIANNESS ( *(const char *)&endianness == (char) 0xef ? LITTLE \
: *(const char *)&endianness == (char) 0xde ? BIG \
: UNDEFINED )
// : assert(0))
/*
 * This is tested by (for example):
 *   if (ENDIANNESS == BIG)...
 */

/*******************************************************************************
 *  End of cnvsth2seg.h		
 *  Copyright (c) 2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 ******************************************************************************/
