/*
 ** Copyright (c) 2013, 2012, 2011 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 **
 ** pgcwhpolygon.h (part of pgcwhpolygon)
 ** whpolymain.h (part of whpolygon)
 ** 
 */
// Ring boxes and diamonds are not fully set <<< Check this has been fixed <<<
#ifndef POLYMAINH
#define POLYMAINH 

#define PROGRAM_VERSION   "0.39"
#define PROGRAM_EDIT_DATE "20130617:16.25"

/*
 **  Exit conditions. Note that these are more fully described in 
 **  comments in wpgcwhpolygon.c
 */
#define NORMAL_RETURN          0
#define ERROR_PARAMETERS       1
#define FAIL_READ_RINGS        2
#define FAIL_BUILD_CONVEX_HULL 3
#define FAIL_GET_MEMORY_1      4
#define FAIL_BUILD_WHERE       5
#define FAIL_EXCESS_CORNERS    6
#define FAIL_OUTPUT_EXISTS     7
#define FAIL_STRICT            8
#define FAIL_STRICT2           9
#define FAIL_STRICT3          10

/*
 ** Some Definitions for Clarity
 */
#define FALSE        0    /* Logical 'False' value */
#define TRUE         1    /* Logical 'True' value  */

#define NAMEMAX      4096 /* Maximum length of a name  */
#define MAXCORNERS     50 /* Maximum number of corners */
#define UPPERMAXCORN  250 /* Absolute maximum no. of corners */
#define MAXTOL 1000000    /* Upper value of tolerance  */

#define OPANDUNSET   0    /* NOT SET YET       */
#define OPANDFIRST   1    /* First Clause      */
#define OPANDAND     2    /* 'AND' Clause      */
#define OPANDOR      3    /* 'OR' Clause       */
#define OPANDANDNOT  4    /* 'AND NOT' Clause  */

#define SEGCONVEX  0    /* Processing Convex segment of original polygon  */
#define SEGCONCAV  1    /* Processing Concave segment of original polygon */

#define CHPERROR  -1    /* Convex Hull Points : Something went wrong  */
#define CHPCNVEX   0    /* Convex Hull Points : Polygon was convex    */
#define CHPCNCAV   1    /* Convex Hull Points : Polygon was concave   */

#define CVXHULLUK  0    /* Unknown Corner Status               */
#define CVXHULLYS  1    /* Corner IS on the Convex Hull        */
#define CVXHULLNO  2    /* Corner CANNOT be on the Convex Hull */

#define RINGROTUK  0    /* Unknown Rotation Direction          */
#define RINGROTCW  1    /* Clockwise Rotation Direction        */
#define RINGROTAC  2    /* AntiClockwise Rotation Direction    */
#define RINGROTER  -1   /* Error in angles calculation         */

#define PI         3.14159265358979    /* math constant pi     */
#define TWOPI      6.28318530717958    /* math consant 2*pi    */
#define RADDEG     57.295779513082325  /* math constant 180/pi */
/* PI and TWOPI are defined here to avoid run-time calculation */
/* RADDEG is used for converting radians to degrees            */

#define	TFILES	0
#define TRINGS  1
#define TWHERE  2
#define TCONVX  3
#define TPOINT  4
#define TANGLE  5
#define TSORT   6
#define TSTDOUT 7
#define TSTDERR 8
#define TORING  9
#define THULL   10
#define TBWHERE 11
#define TPOINTS 12
#define TDIAMND 13
#define TALLDIR 14

#define ISBIG    0
#define ISLITTLE 1


/*
 ** Global Variables
 */

#define XCOL "XCOLNAME"  /* X Coord Column Name */
#define YCOL "YCOLNAME"  /* Y Coord Column Name */
extern char *xcol;
extern char *ycol;
extern int  tprint;
extern int  tpbits[16];
extern int  maxcorners;
extern int  maxmaxcorners;
extern int  corners;
extern int  strict;
extern int  tolerance;
int maxcorners;
int maxmaxcorners;
int corners;

/*
 ** Some Structures we use
 */

/*
 **  A point has an X xoordinate and a Y coordinate - but we
 **  do NOT know, for all users, what computational type these
 **  are expressed as. Hence we have COORDT which is the type
 **  of these fields, and FMTX and FMTY which may be used within
 **  printf (etc.) strings for the format conversion. So we may
 **  have "int" and "%d" or "float" and "%f" and so on.
 **  By defining these out of the main code we can change this
 **  program to use any of the available types just by modifying
 **  these definitions, and not having to change the rest of the code itself.
 */
#define LONGPOINT "long"
#ifdef LONGPOINT
#define COORDT long       /* C type for coordinates */
#define FMTX   "%dl"      /* matching printf format */
#define FMTY   "%dl"      /* matching printf format */
#define XYMAX  LONG_MAX
#define XYMIN  LONG_MIN
#else
#define COORDT int        /* C type for coordinates */
#define FMTX   "%d"       /* matching printf format */
#define FMTY   "%d"       /* matching printf format */
#define XYMAX  INT_MAX
#define XYMIN  INT_MIN
#endif
struct point {
    COORDT x;             /* X Coordinate */
    COORDT y;             /* Y Coordinate */
};

/*
 ** Note that the point elements in a box do *not* actually
 ** indicate real points, but are used merely to hold the
 ** maxima and minima of the x and y values. From these values
 ** the coordinates of a real bounding rectangle are known.
 */
struct box {
    struct point min;     /* Min X and Y Coords               */
    struct point max;     /* Max X and Y Coords               */
};

/*
 ** Note that the points in a diamond ARE real points (unlike
 ** a box). A diamond contains the southern-most ("bottom"), northern-most
 ** ("top"), eastern-most ("eastmost") and western-most ("westmost") points of
 ** the polygon we are considering. Note that in degenerate cases there may not
 ** be four *distinct* points in this set - but all four will be expressed in
 ** the diamond.
 */
struct diamond {
	struct point bottom;  /* Min Y Coordinate                 */
	struct point top;     /* Max Y Coordinate                 */
	struct point westmost;/* Min X Coordinate                 */
	struct point eastmost;/* Max X Coordinate                 */
};


/*
 ** In this program we so far (date 20121006) are assuming that "min" and "max"
 ** have their natural meanings. If the area being considered contains either
 ** the zero line of longitude or the zero line of latitude, then we have to be
 ** sure that we are expressing the co-ordinates with their correct signs.
 */


/*
 ** A "ring" is a group of polygons.
 */

struct ring {
    struct ring   *prev;  /* Ptr to Previous in list          */
    struct ring   *next;  /* Ptr to Next in list              */
    struct points *pts;   /* Ptr to Start of Points circ list */
    int    dir;           /* Direction of Rotation            */
    int    opand;         /* Is this OR or AND in the WHERE   */
    int    pos;           /* Position in list: 0 = first      */
    int    cnt;           /* No of Lines in Ring              */
	/*  Associate bounding box and interior diamond with ring */
	struct box     ringbox;
	struct diamond ringdiamond;
};


/*
 ** The "points" structure allows us to capture a polygon. Each element is
 ** related to just one vertex of the polygon, with some extra information
 ** about that vertex point.
 */
struct points {
    struct points *prev;     /* Ptr to Previous in circular list */
    struct points *next;     /* Ptr to Next in circular list     */
    struct point   pt;       /* The actual point                 */
    int            chflag;   /* Is it on the Convex Hull         */
    int            pos;      /* Position in list: 0 = first      */
    float          angle;    /* External Angle at _this_ Point   */
    long double    distance; /* Distance from 'line'             */
};


/*
 ** Forward Declaration of Functions
 */
struct ring   *NewRing();
struct ring   *GetRings(FILE **fp, struct box *Box, struct diamond *Diamond);
int            ConvexHullRings(struct ring  *startRing);
int            BuildWhereRing(FILE **fp, struct ring *startRing);
void           setBoxDiamond(struct ring  *thisRing);
void           ringDirection(struct ring  *thisRing);
void           ShowRings(struct ring *startRing);

struct points *NewPoint(struct points *oldPoint);
struct points *GetPoints(FILE **fp, COORDT firstX, COORDT firstY, 
                         struct box *Box, struct diamond *Diamond,
                         struct box *outerBox, struct diamond *outerDiamond);
struct ring   *ConvexHullInitialPoints(struct ring *thisRing);
int            ConvexHullPoints(struct ring *thisRing);
int            ConvexHullConcave(struct ring *thisRing); 
int            SetPointReverse(struct points *startPoint);
int            SetPointPosition(struct points *startPoint, int resetCHFlag);
int            BuildWherePoints(FILE **fp, struct ring *thisRing);
void           ShowPoints(struct points *startPoint);

void           BuildWhereBox(FILE **fp, struct box *Box);
void           ShowBox(struct box *Box);
void           ShowDiamond(struct diamond *Diamond); 
void           ShowRingBoxes(struct ring *startRing);
void           ShowRingDiamonds(struct ring *startRing);
void           SetPointsAngle(struct points *thisPoint);

int            PointLine(struct point *lineStart, struct point *lineEnd, 
                         struct point *pointTest); 
void           AllDirections (int ringCnt, struct ring *rings);
int           *ringNesting (int ringCnt, struct ring *thisRing); 
int            inDiamond(struct diamond testDiamond, struct point testPoint); 


/* Forward declaration of Utility procedures: */
int         tfclose     (FILE  *a);
int16_t		swap_int16	(int16_t a);
uint16_t	swap_uint16 (uint16_t a);
int32_t		swap_int32	(int32_t a);
uint32_t	swap_uint32	(uint32_t a);
double      swap_double (unsigned long long a);

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

#endif

/*
 ** End of File
 ** Copyright (c) 2013, 2012, 2011 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 */
