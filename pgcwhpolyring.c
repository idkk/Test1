/*
 ** Copyright (c) 2011-2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 **
 ** pgcwhpolyring.c (part of pgcwhpolygon)
 ** 
 */

/* This file last updated 20130815:1205 */

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

#include "cnvswap.h"
#include "cnvendian.h"
#include "pgcwhpolygon.h"


/*
 ** RING FUNCTIONS:-
 */
/*
 ** Function :NewRing()
 ** Purpose  : Grab space, initialise, return ptr
 ** Arguments: None
 ** Returns  : Ptr to ring
 ** Notes    :
 */
struct ring *
NewRing()
{
    struct ring *ptr;
	/* Comment out box and diamond IDKK 20120402: */
//	struct boxptr *box;
//	struct diamondptr *diamond; 

	
    if ((ptr = malloc(sizeof(struct ring))) == (struct ring *)NULL)
    {
        fprintf(stderr, "ERROR: Unable to malloc space for ring\n");
    }
    else
    {
        /*
		 ** Initialise the Structure
		 */
        ptr->prev     = (struct ring *)NULL;
        ptr->next     = (struct ring *)NULL;
        ptr->pts      = (struct points *)NULL;
        ptr->dir      = RINGROTUK;
        ptr->opand    = OPANDUNSET;
        ptr->pos      = -1;
        ptr->cnt      = -1;
		/* IDKK: now set the box and diamond private to this one ring: */
		/* NOTE: there is no initial value for the internal box and diamond */
		/* End of IDKK insertion 20120330 */
    }
	
    return(ptr);
}


/*
 ** Function : GetRings()
 ** Purpose  : 
 ** Arguments: Input File Pointer
 ** Returns  : Ptr to Rings
 ** Notes    :
 */
struct ring *
GetRings(FILE       **fp,
         struct box  *Box,
		 struct diamond *Diamond)
{
    struct ring *startRing = (struct ring *)NULL;
    struct ring *thisRing;
    struct ring *nextRing;
    COORDT readX, readY;
    int    ringCnt = 0;
	
    /*
	 ** While there's another 'ring' to build ...
	 */
	while (fscanf(*fp, FMTX ", " FMTY "\n", &readX, &readY) == 2)
    {
        /*
		 ** Set the Box values for Max and Min
		 */
        if (ringCnt == 0)
        {
            /*
			 ** Initialise to the first Coords
			 */
            Box->min.x = readX;
            Box->min.y = readY;
            Box->max.x = readX;
            Box->max.y = readY;
        }
		/*
		 ** Set the diamond values for Max and Min
		 */ 
        if (ringCnt == 0)
        {
            /*
			 ** Initialise to the first Coords
			 */
            Diamond->top.x = readX;
            Diamond->top.y = readY;
            Diamond->bottom.x = readX;
            Diamond->bottom.y = readY;
			Diamond->westmost.x = readX;
            Diamond->westmost.y = readY;
            Diamond->eastmost.x = readX;
            Diamond->eastmost.y = readY;
			
        }
		
        /*
		 ** Start of a (new) 'ring' ...
		 */
        if ((nextRing = NewRing()) == (struct ring *)NULL)
        {
            return((struct ring *)NULL);
        }
        nextRing->pos = ringCnt++;
		
        /*
		 ** Now read the rest of this 'ring' Point by Point (Coord by Coord)
		 */
  		if ((nextRing->pts = GetPoints(fp, readX, readY, &nextRing->ringbox, 
                                       &nextRing->ringdiamond,
                                       Box, Diamond)) == (struct points *)NULL)
        {
            fprintf(stderr, "(%d) Failed to read Points\n", ringCnt);
			
            return((struct ring *)NULL);
        }
		
        
        /* TODO
		 ** Check 'cnt' to make sure we got 3 or more points ...
		 */
        nextRing->cnt = nextRing->pts->prev->pos + 1;
		
        /*
		 ** Successfully read a Ring ... Add this new 'ring' to the rings list
		 */
        if (startRing == (struct ring *)NULL)
        {
            /*
			 ** Link to the First Ring
			 */
            startRing = nextRing;
			
            /*
			 ** Build Circular List of one 'ring' for _this_ 'ring'
			 */
            nextRing->next = nextRing;
            nextRing->prev = nextRing;
			
            /*
			 ** And set the Operand for 'this' ring
			 */
            nextRing->opand = OPANDFIRST;
        }
        else
        {
            /*
			 ** Insert _next_ 'ring' into circular linked list just after _this_
			 ** 1st: Link to the 'ring' after ...
			 */
            nextRing->next       = thisRing->next;
            nextRing->next->prev = nextRing;
			
            /*
			 ** 2nd: Link to the 'ring' before ...
			 */
            thisRing->next = nextRing;
            nextRing->prev = thisRing;
			
            /*
			 ** And set the Operand for 'this' ring
			 */
            nextRing->opand = OPANDOR;
        }
		
        /*
		 ** Is this 'new' Ring Clockwise or Anticlockwise?
		 */
        ringDirection(nextRing);
		
        /*
		 ** Step forward onto the newest 'line' ...
		 */
        thisRing = nextRing;
    }
	
    
	
    return(startRing);
}

/* !!!! TODO This routine can be altered to use the nesting information */
/*
 ** Function : BuildWhereRing()
 ** Purpose  : 
 ** Arguments: Ouptut File Pointer
 **            Ptr to Start of Ring circle
 ** Returns  : 0 if OK, non zero otherwise
 ** Notes    :
 */
int
BuildWhereRing(FILE        **fp,
               struct ring  *startRing)
{
    struct ring *thisRing = (struct ring *)NULL;
    int ret = 0;                 /* Initialise: OK so far */
	
    /*
	 ** For each 'ring' in the circular linked list
	 */
    while (thisRing != startRing)
    {
        /*
		 ** If this is the first time in, we need to initialise to startRing
		 ** And we can throw in some column titles
		 */
        if (thisRing == (struct ring *)NULL)
        {
            thisRing = startRing;
        }
		
        /*
		 ** For each 'line' ...
		 */
        if (! BuildWherePoints(fp, thisRing))
        {
            fprintf(stderr, "ERROR: (%d) Failed to build WHERE Clause (%d)\n",
					thisRing->pos, ret);
            thisRing = startRing;  /* Force Premature Exit */
			
            ret = -2;
            break;
        }
		
        /*
		 ** Step to the next 'line' in the circular linked list
		 **   - if there is one -
		 */
        if (thisRing->next == (struct ring *)NULL)
        {
            thisRing = startRing;  /* Force Premature Exit */
        }
        else
        {
            thisRing = thisRing->next;
        }
    }
	
    return(ret);
}

void
setBoxDiamond(struct ring *thisRing)
{
    struct points *startPoint = thisRing->pts;
    struct points *thisPoint = (struct points *)NULL;
    while (thisPoint != startPoint)
    {
        /*
		 ** If this is the first time in, we need to initialise to startPoint
		 ** ... and the initial values for ringbox and ringdiamond 
		 */
        if (thisPoint == (struct points *)NULL)
        {
            thisPoint = startPoint;
			thisRing->ringbox.min.x = startPoint->pt.x;
			thisRing->ringbox.min.y = startPoint->pt.y;
			thisRing->ringbox.max.x = startPoint->pt.x;
			thisRing->ringbox.max.y = startPoint->pt.y;
			thisRing->ringdiamond.top.x = startPoint->pt.x;
			thisRing->ringdiamond.top.y = startPoint->pt.y;
			thisRing->ringdiamond.bottom.x = startPoint->pt.x;
			thisRing->ringdiamond.bottom.y = startPoint->pt.y;
			thisRing->ringdiamond.westmost.x = startPoint->pt.x;
			thisRing->ringdiamond.westmost.y = startPoint->pt.y;
			thisRing->ringdiamond.eastmost.x = startPoint->pt.x;
			thisRing->ringdiamond.eastmost.y = startPoint->pt.y;
        }
		else 
		{
			/* This is not the first point in the ring - calculate
			 * the box and diamond limits
			 */
			if (thisPoint->pt.x < thisRing->ringbox.min.x)
			{
				thisRing->ringbox.min.x = thisPoint->pt.x;
				thisRing->ringdiamond.westmost.x = thisPoint->pt.x;
				thisRing->ringdiamond.westmost.y = thisPoint->pt.y;
			}
			if (thisPoint->pt.x > thisRing->ringbox.max.x)
			{
				thisRing->ringbox.max.x = thisPoint->pt.x;
				thisRing->ringdiamond.eastmost.x = thisPoint->pt.x;
				thisRing->ringdiamond.eastmost.y = thisPoint->pt.y;
			}
			if (thisPoint->pt.y > thisRing->ringbox.max.y)
			{
				thisRing->ringbox.max.y = thisPoint->pt.y;
				thisRing->ringdiamond.top.x = thisPoint->pt.x;
				thisRing->ringdiamond.top.y = thisPoint->pt.y;
			}
			if (thisPoint->pt.y < thisRing->ringbox.min.y)
			{
				thisRing->ringbox.min.y = thisPoint->pt.y;
				thisRing->ringdiamond.bottom.x = thisPoint->pt.x;
				thisRing->ringdiamond.bottom.y = thisPoint->pt.y;
			}
		}
        /*
		 ** Step to the next 'points' in the circular linked list
		 **   - if there is one -
		 */
        if (thisPoint->next == (struct points *)NULL)
        {
            thisPoint = startPoint;  /* Force Premature Exit */
        }
        else
        {
            thisPoint = thisPoint->next;
        }
    }
}

/*
 ** Function : ringDirection()
 ** Purpose  : 
 ** Arguments: Ptr to Start of Ring circle
 ** Returns  : Void
 ** Notes    : Sets 'dir' to indicate direction of rotation.
 **              Clockwise     indicating _this_ is an 'outer' ring
 **              Anticlockwise indicating _this_ is an 'inner' ring
 **                  eg the 'hole' in a doughnut
 */
void
ringDirection(struct ring  *thisRing)
{
    struct points *startPoint = thisRing->pts;
    struct points *thisPoint = (struct points *)NULL;
    float  angleTot = 0;
    float  err = 0.01;
	float  maxangle, minangle;
	int    pointcnt = 0; 
    COORDT xprev = 0;
    COORDT yprev = 0;
	
	maxangle = TWOPI + err;
	minangle = TWOPI - err;
	
	/*
	 ** While we are going round the points in this ring, we
	 ** can set up the ring-box and ring-diamond at the same time, as well
	 ** as count the points
	 */
	
    /*
	 ** For each 'points' in the circular linked list
	 */
    while (thisPoint != startPoint)
    {
        /*
		 ** If this is the first time in, we need to initialise to startPoint
		 ** ... and the initial values for ringbox and ringdiamond 
		 */
        if (thisPoint == (struct points *)NULL)
        {
            thisPoint = startPoint;
			thisRing->ringbox.min.x = startPoint->pt.x;
			thisRing->ringbox.min.y = startPoint->pt.y;
			thisRing->ringbox.max.x = startPoint->pt.x;
			thisRing->ringbox.max.y = startPoint->pt.y;
			thisRing->ringdiamond.top.x = startPoint->pt.x;
			thisRing->ringdiamond.top.y = startPoint->pt.y;
			thisRing->ringdiamond.bottom.x = startPoint->pt.x;
			thisRing->ringdiamond.bottom.y = startPoint->pt.y;
			thisRing->ringdiamond.westmost.x = startPoint->pt.x;
			thisRing->ringdiamond.westmost.y = startPoint->pt.y;
			thisRing->ringdiamond.eastmost.x = startPoint->pt.x;
			thisRing->ringdiamond.eastmost.y = startPoint->pt.y;
        }
		else 
		{
			/* This is not the first point in the ring - calculate
			 * the box and diamond limits
			 */
			if (thisPoint->pt.x < thisRing->ringbox.min.x)
			{
				thisRing->ringbox.min.x = thisPoint->pt.x;
				thisRing->ringdiamond.westmost.x = thisPoint->pt.x;
				thisRing->ringdiamond.westmost.y = thisPoint->pt.y;
			}
			if (thisPoint->pt.x > thisRing->ringbox.max.x)
			{
				thisRing->ringbox.max.x = thisPoint->pt.x;
				thisRing->ringdiamond.eastmost.x = thisPoint->pt.x;
				thisRing->ringdiamond.eastmost.y = thisPoint->pt.y;
			}
			if (thisPoint->pt.y > thisRing->ringbox.max.y)
			{
				thisRing->ringbox.max.y = thisPoint->pt.y;
				thisRing->ringdiamond.top.x = thisPoint->pt.x;
				thisRing->ringdiamond.top.y = thisPoint->pt.y;
			}
			if (thisPoint->pt.y < thisRing->ringbox.min.y)
			{
				thisRing->ringbox.min.y = thisPoint->pt.y;
				thisRing->ringdiamond.bottom.x = thisPoint->pt.x;
				thisRing->ringdiamond.bottom.y = thisPoint->pt.y;
			}
		}
		
        /*
		 ** Add up the angles ...
		 */
        angleTot = angleTot + thisPoint->angle;
		pointcnt++;
		
        /*
		 ** Step to the next 'points' in the circular linked list
		 **   - if there is one -
		 */
        if (thisPoint->next == (struct points *)NULL)
        {
            thisPoint = startPoint;  /* Force Premature Exit */
        }
        else
        {
            /*
             *  If we are applying strict rules, then check whether the next
             *  point (which we know is not NULL) is within tolerance of this
             *  point, and if it is then issue an error message and stop:
             */
            if (strict != 0)
            {
                if ((abs(xprev - thisPoint->pt.x) < tolerance) ||
                    (abs(yprev - thisPoint->pt.y) < tolerance))
                {
                    fprintf(stderr, 
                            "ERROR: Previous point (" FMTX ", " FMTY 
                            ") in ring is\n"
                            "       within tolerance (%d) of this point (" FMTX 
                            ", " FMTY ")\n"
                            "       Run terminates.\n", xprev, yprev, tolerance,
                            thisPoint->pt.x, thisPoint->pt.y);
                    exit (FAIL_STRICT);
                }
            }
            xprev = thisPoint->pt.x;
            yprev = thisPoint->pt.y;
            thisPoint = thisPoint->next;
        }
    }
	
    /*
	 ** If the total of the angles for this ring is 2PI, then it is Clockwise,
	 ** and if it's -2PI it's Anticlockwise ... but there may be a few decimal
	 ** places out ...
	 */
    if (angleTot < maxangle && angleTot > minangle)
    {
        thisRing->dir = RINGROTCW;
    }
    else if (angleTot < -1 * (maxangle) && 
             angleTot > -1 * (minangle))
    {
        thisRing->dir = RINGROTAC;
    }
    else
    {
        fprintf(stderr, "Warning: Ring(%d points) angles total = %.3f\n", 
                pointcnt, angleTot);
        thisRing->dir = RINGROTER;
        /* START DEBUG */
        /* DEBUG: if the total angle is positive, set to be clockwise - other-
                  wise set to be ant-clockwise: */
        if (angleTot > 0)
        {
            thisRing->dir = RINGROTCW;
            fprintf(stderr, "Warning: fix - Ring [%p] forced clockwise\n",
                    thisRing);
        }
        else 
        {
            thisRing->dir = RINGROTAC;
            fprintf(stderr, "Warning: fix - Ring [%p] forced anti-clockwise\n",
                    thisRing);
        }
        /* END DEBUG */
    }
}



/*
 ** Function : AllDirections()
 ** Purpose  : 
 ** Arguments: counter of number of rings, pointer to ring circle
 ** Notes    : 
 */
void
AllDirections (int ringCnt, struct ring *rings)
{
    struct ring *thisRing = (struct ring *)NULL;
    int i;
    
    thisRing = rings;
    for (i=0;i<ringCnt;i++)
    {
        ringDirection(thisRing);
        thisRing = thisRing->next;
    }
}    

/*
 ** Function : ringNesting()
 ** Purpose  : 
 ** Arguments: counter of number of rings, pointer to ring circle
 ** Returns  : pointer to array of nesting information
 ** Notes    : 
 */
int *
ringNesting(int ringCnt, struct ring *rings)
{
	int *nesting = (int *)NULL;
//	struct ring *ringPointers = (struct ring *)NULL;
//    struct ring *tempPointer = (struct ring *)NULL;
	int inner;
	int outer;
	struct ring *innerRing;
	struct ring *outerRing;
	struct point thisPoint;
    struct point firstPoint;
    struct point lineStart;
    struct point lineEnd;
    struct points *aPoints;
    struct points *firstLine;
    struct points *aLine;
    int isfirstP = 0;
    int isfirstL = 0;
    int touching = 0;
	int i, j; /* work variables */
	
	/*
	 ** This is where we consider the nesting of the various rings.
	 ** We do this by setting up a matrix - ringisin - which takes two
	 ** subscripts. The values in this matrix are:
	 **   -2    ring subscript_1 is NOT in ring subscript_2
	 **   -1    ring subscript_1 is NOT in ring subscript_2, but touches it
	 **    0    nesting not decided
	 **   +1    ring subscript_1 is in ring subscript_2, and touches it
	 **   +2    ring subscript_1 is in ring subscript_2
	 **
	 ** The nesting of N rings requires us (initially) to make N*(N-1) 
	 ** ring-in-ring tests. This is not the most efficient it could be, 
	 ** but given the number of rings that (in reality) we are likely 
	 ** to encounter simultaneously, it's probably not too bad.
	 **     N     1    2    3    4    5    6    7    8    9 ...
	 **     Tests 0    2    6   12   20   30   42   56   72 ...
	 **
	 ** Each test tries to determine as quickly as possible the gross
	 ** inclusion and exclusions. If we are looking at rings A and B,
	 ** and asking whether ring A is in ring B then we proceed as follows:
	 **   1) this algorithm works *only* if ring B is convex
	 **   2) for each point on ring A:
	 **      i)  is this point outside box B?
	 **          yes - A is outside B
	 **          touching - A touches B - test more points
	 **          no - test more points
	 **      ii) is this point inside diamond B?
	 **          yes or touching - test more points
	 **          no - test this point of A against each line in B
	 ** So we have to test a point of A against every line of B only if it
	 ** is both inside B's box and outside B's diamond.
	 */
	
    /* DEBUG PRINT: */
    if (tprint != 0)
    {
        fprintf(stderr,"\nRing Nesting:\n");
    }
    /* END DEBUG PRINT */
    
	/* Get enough memory for the array: */
	nesting = malloc(ringCnt*(ringCnt/*-1*/)*sizeof(int));
	/* 
	 * If we could not get enough memory, return the NULL pointer
	 */
	if (nesting == (int *)NULL)
	{
		fprintf(stderr, "ERROR: Could not evaluate ring nesting - no memory\n");
		return nesting;
	}
	
	/*
	 *  Initialise array to all zeros - indicating not yet calculated
	 */
	for (i=0; i<ringCnt; i++)
		for (j=0; j<ringCnt; j++)
			*((nesting)+(ringCnt*i+j)) = 0;
    
    /* 
     *  Now we loop round every ring ("inner") and ask is this in ring "outer",
     *  where "outer" also loops round every ring.
     *  Begin by pointing to the first inner ring:
     */
    innerRing = rings; 
	for (inner=1; inner<=ringCnt; inner++)
	{		
        /*
         *  Point to first outer ring:
         */
        outerRing = rings;
		for (outer=1; outer<=ringCnt; outer++)
		{
            /*
             *  This loop asks (and answers!) the question:
             *    is "inner" inside "outer"?
             */
			
			/* Go round each point on ring "inner" 
			* Logic: 
			*   0) default mark inner as inside outer
			*   1) get the first point of inner
			*   2) test point against box
			*      if outside box then inner is outside outer; exit (go to 6)
			*   3) test point against diamond
			*      if inside diamond then still inside - go to 5)
			*   4) test point against each line of outer
			*      if outside any line, then mark as outside and exit (go to 6)
			*   5) get next point of inner
			*      if this is the first point again, then exit (go to 6)
			*      otherwise go back to 2)
			*   6) end of calculation - exit point
			 */
			
			/* 0) Default mark inner as inside outer: */
			*((nesting)+(ringCnt*inner+outer)) = 2;
            /*    ...and also that they are NOT touching: */
            touching = 0;
			
            /*
             *  If the two rings are the same (i.e. inner == outer) then
             *  clearly they nest - and no further tests are needed:
             */
            if (inner == outer)
            {
                goto foundnest;
            }
            
			/* 1) Get first point of inner ring */
			thisPoint = innerRing->pts->pt;
            firstPoint = thisPoint;
            isfirstP = 0;
            aPoints = innerRing->pts;
			
            while (isfirstP == 0)
            {
			/* 2) Test point against box */
			/* If it is outside the box, then it is outside the ring */
			if (   (thisPoint.x > outerRing->ringbox.max.x)
				|| (thisPoint.x < outerRing->ringbox.min.x)
				|| (thisPoint.y > outerRing->ringbox.max.y)
				|| (thisPoint.y < outerRing->ringbox.min.y))
			{
				*((nesting)+(ringCnt*inner+outer)) = -2;
				goto foundnest;
			}
			
			/* 3) Test point against diamond */
			/* If it is inside the diamond, then it is inside the ring */
			if ( inDiamond(outerRing->ringdiamond, thisPoint) >=0 )
			{
				*((nesting)+(ringCnt*inner+outer)) = 2;
				goto foundnest;
			}

			/* 4) test point against each line of ring */
			/* 
			 * So we know that this point is inside the box and outside
			 * the diamond - thus we have to test this point against
			 * every line of the (nominally outer) ring. Tedium.
			 * We go into a loop, testing each line separately.
			 * If we are ever outside the line, then we are also outside
			 * the ring. If we are ever touching the line, then we are
			 * touching the ring.
			 * FOR THE MOMENT we are not checking all of the touching
			 * situations - so we only have "inside" and "outside" (20120416)
			 */
                firstLine = outerRing->pts;
                aLine = firstLine;
                isfirstL = 0;
                while (isfirstL == 0)
                {
                    lineStart = aLine->pt;
                    lineEnd = aLine->next->pt;
                    i = PointLine(&lineStart, &lineEnd, &thisPoint);
                    if (i < 0)
                    {
                        *((nesting)+(ringCnt*inner+outer)) = -2;
                        goto foundnest;
                    }
                    else if (i == 0)
                    {
                        /*
                         *  >>>> INSERT MORE CODE HERE >>>>
                         *  This is the "touching" case - that is, we know that
                         *  the point is on the line (or a projection of that
                         *  line).
                         *  ?? do we have to determine whether the point is on
                         *  a projection of the line?? I think we do - because
                         *  only when the point is ON the line do we have the
                         *  case of "touching".
                         */
                    }
                    /*
                     *  Get the next line in the sequence, and check whether we
                     *  are back to the beginning of the outer ring:
                     */
                    aLine = aLine->next;
                    if (aLine == firstLine)
                    {
                        isfirstL = 1;
                    }
                }
                /* 5) get next point of inner */
                aPoints = aPoints->next;
                thisPoint = aPoints->pt;
                if ((thisPoint.x == firstPoint.x) && 
                    (thisPoint.y == firstPoint.y))
                {
                    isfirstP = 1;
                }
            }
			/* 6) end of calculation for *this* inner ring: */
		foundnest:
            /*
             *  If we have the "touching" case, then divide the indicator
             *  by two, to record that fact:
             */
            if (touching != 0)
            {
                *((nesting)+(ringCnt*inner+outer)) = 
                *((nesting)+(ringCnt*inner+outer)) / 2;
            }
            /*
             *  Now point to the next outer ring, and go back to try it
             */
            outerRing = outerRing->next; //NEW
		}
        /*
         *  Now point to the next inner ring, and go back to try it
         */
        innerRing = innerRing->next; 
	}
    
    /* DEBUG PRINT: */
    if (tprint != 0)
    {
        for (inner=1; (inner<=ringCnt); inner++)
        {
            for (outer=1; (outer<=ringCnt); outer++)
            {
                fprintf(stderr,"Nesting: in=%d out=%d nest=%d\n",inner,outer,
                        *((nesting)+(ringCnt*inner+outer)));
            }
        }
    }
    /* END DEBUG PRINT */
    
	return nesting;
}

/*
 ** Function : inDiamond()
 ** Purpose  : test whether a point is in, on or outside diamond
 ** Arguments: diamond, point
 ** Returns  : -1 if outside, 0 if on, +1 if in diamond
 ** Notes    :
 */
int
inDiamond (struct diamond testDiamond,
		   struct point testPoint)
{
	int retval = 0;
	int retval2 = 0;
	
	retval = PointLine(&testDiamond.eastmost, &testDiamond.top, &testPoint);
	if (retval >= 0)
	{
		retval2 = PointLine(&testDiamond.top, &testDiamond.westmost, &testPoint);
		if (retval2 == 0)
		{
			retval = 0;
		}
		else if (retval2 < 0)
		{
			return retval2;
		}
		
		retval2 = PointLine(&testDiamond.westmost, &testDiamond.bottom, &testPoint);
		if (retval2 == 0)
		{
			retval = 0;
		}
		else if (retval2 < 0)
		{
			return retval2;
		}

		retval2 = PointLine(&testDiamond.bottom, &testDiamond.eastmost, &testPoint);
		if (retval2 == 0)
		{
			retval = 0;
		}
		else if (retval2 < 0)
		{
			return retval2;
		}
		
	}
	return retval;
}

/*
 ** Function : ShowRings()
 ** Purpose  : Display all the Rings
 ** Arguments: Ptr to Ring
 ** Returns  : void
 ** Notes    :
 */
void
ShowRings(struct ring *startRing)
{
    struct ring *thisRing = (struct ring *)NULL;
    char opand[5];
    char direction[5];
	
    /*
	 ** For each 'ring' in the circular linked list
	 */
    while (thisRing != startRing)
    {
        /*
		 ** If this is the first time in, we need to initialise to startRing
		 ** And we can throw in some column titles
		 */
        if (thisRing == (struct ring *)NULL)
        {
            thisRing = startRing;
        }
		
        switch(thisRing->dir)
        {
            case RINGROTER:  sprintf(direction, "ER"); break;
            case RINGROTUK:  sprintf(direction, "UK"); break;
            case RINGROTCW:  sprintf(direction, "CW"); break;
            case RINGROTAC:  sprintf(direction, "AC"); break;
        }
		
        switch(thisRing->opand)
        {
            case OPANDUNSET:  sprintf(opand, "XX"); break;
            case OPANDFIRST:  sprintf(opand, "st"); break;
            case OPANDAND:    sprintf(opand, "& "); break;
            case OPANDOR:     sprintf(opand, "| "); break;
            case OPANDANDNOT: sprintf(opand, "&!"); break;
        }
		
        /*
		 ** Print out the info
		 */
		
        
        /*
		 ** Step to the next 'line' in the circular linked list
		 **   - if there is one -
		 */
        if (thisRing->next == (struct ring *)NULL)
        {
            thisRing = startRing;  /* Force Premature Exit */
        }
        else
        {
            thisRing = thisRing->next;
        }
    }
    
}

void
ShowRingBoxes(struct ring *startRing)
{
    struct ring *thisRing = (struct ring *)NULL;
    /*
	 ** For each 'ring' in the circular linked list
	 */
    while (thisRing != startRing)
    {
        if (thisRing == (struct ring *)NULL)
        {
            thisRing = startRing;
        }
        
        /*
		 ** Step to the next 'ring' in the circular linked list
		 **   - if there is one -
		 */
        if (thisRing->next == (struct ring *)NULL)
        {
            thisRing = startRing;  /* Force Premature Exit */
        }
        else
        {
            thisRing = thisRing->next;
        }
    }    
} 
    
void
ShowRingDiamonds(struct ring *startRing)
{
    struct ring *thisRing = (struct ring *)NULL;
    /*
	 ** For each 'ring' in the circular linked list
	 */
    while (thisRing != startRing)
    {
        if (thisRing == (struct ring *)NULL)
        {
            thisRing = startRing;
        }
        ShowDiamond(&thisRing->ringdiamond);
        /*
		 ** Step to the next 'line' in the circular linked list
		 **   - if there is one -
		 */
        if (thisRing->next == (struct ring *)NULL)
        {
            thisRing = startRing;  /* Force Premature Exit */
        }
        else
        {
            thisRing = thisRing->next;
        }
    }        
} 


/*
 ** End of File
 ** Copyright (c) 2011-2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 */
