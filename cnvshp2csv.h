/*
 *  cnvshp2csv.h
 *
 *  Copyright 2013 idkk Consultancy Ltd. All rights reserved.
 *
 */

/* Last updated 20131001:18.15 */

#ifndef CNVSHP2CSV
#define CNVSHP2CSV

/*
 ** Forward Declarations
 */
int ReadNullshape  (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);
int ReadPoint      (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance, int fsize, int rsize,
                    int maxcorners);
int ReadPolyline   (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);
int ReadPolygon    (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);
int ReadMultipoint (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);
int ReadPointZ     (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance, int fsize, int rsize,
                    int maxcorners);
int ReadPolylineZ  (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);
int ReadPolygonZ   (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);
int ReadMultipointZ(FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);
int ReadPointM     (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance, int fsize, int rsize,
                    int maxcorners);
int ReadPolylineM  (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);
int ReadPolygonM   (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);
int ReadMultipointM(FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);
int ReadMultipatch (FILE *ipf, FILE *opf, int rounding, long double denom, 
                    int iscaling, long double scaling, int strict, int tolerance);


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

#define POINT_RECORD_LENGTH  20
#define POINTM_RECORD_LENGTH 28
#define POINTZ_RECORD_LENGTH 36

#define MAXCORNERS      100
#define MAXMAXCORNERS 32000

#define ISBIG    0
#define ISLITTLE 1

#define NAMELEN  4096
#define MAXDENOM 1000000
#define MINDENOM 0.000001
#define MAXTOL   1000000

#define END_OK            0
#define FAIL_COMMAND      1
#define FAIL_INPUT        2
#define FAIL_OUTPUT       3
#define FAIL_VERSION      4
#define FAIL_SHAPE        5
#define FAIL_PARTS        6
#define FAIL_MALLOC       7
#define FAIL_ARRAY        8
#define FAIL_POINTS       9
#define FAIL_CORNERS     10
#define FAIL_NOINPUT     11
#define FAIL_NOOUTPUT    12
#define FAIL_STRICT      13
#define FAIL_STRICT2     14
#define FAIL_NULLSHAPE   15
#define FAIL_POINT       16
#define FAIL_POLYLINE    17
#define FAIL_POLYGON     18
#define FAIL_MULTIPOINT  19
#define FAIL_POINTZ      20
#define FAIL_POLYLINEZ   21
#define FAIL_POLYGONZ    22
#define FAIL_MULTIPOINTZ 23
#define FAIL_POINTM      24
#define FAIL_POLYLINEM   25
#define FAIL_POLYGONM    26
#define FAIL_MULTIPOINTM 27
#define FAIL_MULTIPATCH  28


#endif

