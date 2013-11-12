/*
 ** Copyright (c) 2011-2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 **
 ** pgcwhpolyhull.c (part of pgcwhpolygon) 
 ** 
 */

/* This file last updated 20130815:1345 */

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
 ** Function : ConvexHullRings()
 ** Purpose  : For Each Ring, call func to build ConvexHull around the ring
 ** Arguments: Ptr to Rings Structure
 ** Returns  : 1 if OK, 0 if it failed
 ** Notes    :
 */
int
ConvexHullRings(struct ring  *startRing)
{
    struct ring *thisRing      = (struct ring *)NULL;
    int          ringCnt = 0;
    int          ret = 1;
	
    /*
	 ** For each 'ring' in the circular linked list
	 */
    while (thisRing != startRing)
    {
        ringCnt++;
        /*
		 ** If this is the first time in, we need to initialise to startRing
		 */
        if (thisRing == (struct ring *)NULL)
        {
            thisRing = startRing;
        }
		
        /*
		 ** Run through the ring and identify the Points that would be on a
		 ** Box bounding the Polygon - ie identify some points that are
		 ** definitely on the Convex Hull
		 */
        if ((thisRing = ConvexHullInitialPoints(thisRing)) ==
			(struct ring *)NULL)
        {
            fprintf(stderr, 
                    "ERROR: Failed to build Convex Hull Initial Points\n");
            thisRing = startRing;   /* Force Premature Exit */
            ret = 0;
			
            break;
        }
		
        /*
		 ** Use the known Convex Hull points to find any other ones ...
		 ** the first polygon associated with the Points.
		 ** If the polygon has concave sections, the concave sections will be
		 ** turned into their own 'ring'
		 ** ie We may add more than 'ring's
		 */
        if ((ConvexHullPoints(thisRing)) == CHPERROR)
        {
            fprintf(stderr, "ERROR: Failed to build Convex Hull\n");
            thisRing = startRing;   /* Force Premature Exit */
            ret = 0;
			
            break;
        }
		
        /*
		 ** If 'thisRing' has a Concave section we need to chop it out of the
		 ** current ring, and create a new ring for it
		 */
        if (ret == CHPCNCAV)
        {
            if ((ConvexHullConcave(thisRing)) == CHPERROR)
            {
                fprintf(stderr, "ERROR: Failed to extract Concave Segment\n");
                thisRing = startRing;   /* Force Premature Exit */
                ret = 0;
				
                break;
            }
        }
		
        /*
		 ** Step to the next 'ring' in the circular linked list
		 **   - if there is one -
		 */
        if (thisRing->next == (struct ring *)NULL)
        {
            fprintf(stderr, "(%d) Ring Circle Broken for CH Generation\n",
					thisRing->pos);
            ret = 0;
			
            break;
        }
        else
        {
            thisRing = thisRing->next;
        }
    }
	
    /*
     ** Now show the rings again - but this time having set the directions
     ** for all of the rings:
     */
    AllDirections(ringCnt, startRing);

	
    return(ret);
}


/*
 ** Function : ConvexHullInitialPoints()
 ** Purpose  : Find the Initial Convex Hull Points
 ** Arguments: Ptr to 'ring'
 ** Returns  : Ptr to Convex Hull ring, NULL otherwise
 ** Notes    : If this group of points contains any concave sections,
 **            Build them their own 'ring' to be used later
 */
struct ring *
ConvexHullInitialPoints(struct ring *thisRing)
{
    struct points *startPoint = thisRing->pts;
    struct points *thisPoint  = (struct points *)NULL;
    struct box     startBox;
	struct diamond startDiamond; 
	
    /*
	 ** Find the northern-, eastern-, southern-, and western-most points,
	 ** as they are part of the Convex Hull and our starting points ...
	 */
	
    while (thisPoint != startPoint)
    {
        /*
		 ** First time through we set ourselves the end point
		 ** and initialise the Box
		 ** ... and the Diamond 
		 */
        if (thisPoint == (struct points *)NULL)
        {
            thisPoint = startPoint;
			
            startBox.min.x = thisPoint->pt.x;
            startBox.min.y = thisPoint->pt.y;
            startBox.max.x = thisPoint->pt.x;
            startBox.max.y = thisPoint->pt.y;
			/* Diamond inserted by IDKK 20120331 */
			startDiamond.eastmost.x = thisPoint->pt.x;
			startDiamond.eastmost.y = thisPoint->pt.y;
			startDiamond.westmost.x = thisPoint->pt.x;
			startDiamond.westmost.y = thisPoint->pt.y;
			startDiamond.top.x = thisPoint->pt.x;
			startDiamond.top.y = thisPoint->pt.y;
			startDiamond.bottom.x = thisPoint->pt.x;
			startDiamond.bottom.y = thisPoint->pt.y;
        }
        else
        {
            /*
			 ** Replace the Box values with the current values if necessary
			 */
            if (thisPoint->pt.x < startBox.min.x)
            {
                startBox.min.x = thisPoint->pt.x;
            }
            if (thisPoint->pt.y < startBox.min.y)
            {
                startBox.min.y = thisPoint->pt.y;
            }
            if (thisPoint->pt.x > startBox.max.x)
            {
                startBox.max.x = thisPoint->pt.x;
            }
            if (thisPoint->pt.y > startBox.max.y)
            {
                startBox.max.y = thisPoint->pt.y;
            }
			
			/*
			 ** Replace the Diamond values with the current values if necessary
			 */
			if (thisPoint->pt.x < startDiamond.westmost.x)
            {
                startDiamond.westmost.x = thisPoint->pt.x;
				startDiamond.westmost.y = thisPoint->pt.y;
            }
            if (thisPoint->pt.y < startDiamond.bottom.y)
            {
                startDiamond.bottom.y = thisPoint->pt.y;
				startDiamond.bottom.x = thisPoint->pt.x;
            }
            if (thisPoint->pt.x > startDiamond.eastmost.x)
            {
                startDiamond.eastmost.x = thisPoint->pt.x;
				startDiamond.eastmost.y = thisPoint->pt.y;
            }
            if (thisPoint->pt.y > startDiamond.bottom.y)
            {
                startDiamond.bottom.y = thisPoint->pt.y;
				startDiamond.bottom.x = thisPoint->pt.x;
            }
        }
		
        /*
		 ** Move to the next Point in the circular list
		 */
        thisPoint = thisPoint->next;
    }
	
    /*
	 ** Now we must run through the list again and mark all Points that contain
	 ** contain one (or more) of the Box values as on our Convex Hull
	 */
    thisPoint = (struct points *)NULL;
    while (thisPoint != startPoint)
    {
        /*
		 ** First time through we set ourselves the end point
		 ** and initialise the Box
		 */
        if (thisPoint == (struct points *)NULL)
        {
            thisPoint = startPoint;
        }
		
        /*
		 ** If _this_ point is the same as one of the Box values ...
		 ** Then it is DEFINITELY on our Convex Hull ...
		 */
        if (thisPoint->pt.x == startBox.min.x ||
            thisPoint->pt.x == startBox.max.x ||
            thisPoint->pt.y == startBox.min.y ||
            thisPoint->pt.y == startBox.max.y)
        {
            thisPoint->chflag = CVXHULLYS;
        }
		
        /*
		 ** Move to the next Point in the circular list
		 */
        thisPoint = thisPoint->next;
    }
	
    /*
	 ** Let's see the Points list again now ...
	 */
//    fprintf(stderr,"DEBUG: (ConvexHullInitialPoints) points=\n");
//    ShowPoints(startPoint);
	
    return(thisRing);
}


/*
 ** Function : ConvexHullPoints()
 ** Purpose  : Find the rest of the Convex Hull Points in this ring
 ** Arguments: Ptr to 'ring'
 ** Returns  : Value indicating Error, OK, or Concave Section to be processed
 ** Notes    : 
 */
int
ConvexHullPoints(struct ring *thisRing)
{
//    struct ring   *nextRing   = (struct ring *)NULL;
    struct points *startPoint = thisRing->pts;
    struct points *beginPoint = (struct points *)NULL;
    struct points *thisPoint  = (struct points *)NULL;
    struct points *nextPoint  = (struct points *)NULL;
    struct points *testPoint  = (struct points *)NULL;
    struct points *distPoint  = (struct points *)NULL;
    int            ret   = CHPCNVEX;
    long double h, s, c, xdiff, ydiff;
	long double distance;
    long double tstX1, tstY1;
	
    thisPoint = startPoint;
    while (thisPoint != beginPoint)
    {
        /*
		 ** Quickly loop through until we find the next Convex Hull Corner to
		 ** start the line from:-
		 */
        if (thisPoint->chflag != CVXHULLYS)
        {
            thisPoint = thisPoint->next;
			
            continue;
        }
		
        /*
		 ** First time in, setup the 'begin' Ptr to the first Convex Hull point
		 */
        if (beginPoint == (struct points *)NULL)
        {
            beginPoint = thisPoint;
        }
		
        /*
		 ** We start with two known Convex Hull points and check all those in
		 ** between to see if any of them are also Convex Hull points.
		 */
        while (1 == 1)
        {
            /*
			 ** Look ahead to find the next Convex Hull Corner to
			 ** end the line at ...
			 ** NB: If we found any last time around,
			 **     we will find a nearer one this time, rinse, wash, repeat!
			 */
            nextPoint = thisPoint->next;
            while (nextPoint->chflag != CVXHULLYS)
            {
                nextPoint = nextPoint->next;
            }
			
            /*
			 ** We now have two ends of a 'line' and can check any intermediate
			 ** points to see if there are any other Convex Hull Corners
			 ** Based on Ian's original:-
			 **    xdiff = linept[i+1].x.fx - linept[i].x.fx;
			 **    ydiff = linept[i+1].y.fy - linept[i].y.fy;
			 **    h = sqrtl (xdiff*xdiff + ydiff*ydiff);
			 **    s = ydiff / h;
			 **    c = xdiff / h;
			 */
            xdiff = nextPoint->pt.x - thisPoint->pt.x;
            ydiff = nextPoint->pt.y - thisPoint->pt.y;
            h     = sqrtl((long double)((xdiff * xdiff) + (ydiff * ydiff)));
            s     = ydiff / h;
            c     = xdiff / h;
			
            /*
			 ** For each point between the Start and End points ...
			 */
            testPoint = thisPoint->next;
            while (testPoint != nextPoint)
            {
                /*
				 ** Only test points if we don't already know it can
				 ** NEVER be on the Hull
				 */
                if (testPoint->chflag != CVXHULLNO)
                {
                    tstX1    = testPoint->pt.x - thisPoint->pt.x;
                    tstY1    = testPoint->pt.y - thisPoint->pt.y;
                    distance = (tstY1 * c) - (tstX1 * s);
					
                    if (distance < 0)       /* OUT */
                    {
                        testPoint->chflag = CVXHULLNO;
                        ret = CHPCNCAV;
                    }
                    else                    /* IN or ON the line */
                    {
						
                        /*
						 ** Remember the greatest 'distance' so far ...
						 printf("Max so far (%Lg)\n", distance); // DEBUG
						 */
                        if (distPoint == (struct points *)NULL)
                        {
                            distPoint = testPoint;
                            distPoint->distance = distance;
                        }
                        else
                        {
                            if (distPoint->distance < distance)
                            {
                                distPoint->distance = -1;
								
                                distPoint = testPoint;
                                distPoint->distance = distance;
                            }
                            else
                            {
                            }
                        }
                    }
                }
                else
                {
                }
				
                /*
				 ** Try the next intermediate point
				 */
                testPoint = testPoint->next;
            }
			
            /*
			 ** If we have a max Distance recorded, then it must also be a 
			 ** Convex Hull point ... so set it
			 */
            if (distPoint != (struct points *)NULL)
            {
                distPoint->chflag   = CVXHULLYS;
                distPoint->distance = 0;
				
                distPoint = (struct points *)NULL;
            }
            else
            {
                /*
				 ** If we didn't find any (more) Convex Hull Corners then we
				 ** MUST have finished looking at this section of the polygon
				 ** So break out and move the Start point forward to the next
				 ** Convex Hull Corner and start again ...
				 */
                break;
            }
        }
		
        /*
		 ** ... and step forward to move the Start point to the next Convex Hull
		 ** point
		 */
        thisPoint = thisPoint->next;
    }
	
    /*
	 ** Let's see the Points list again now ...
	 */
//    fprintf(stderr,"DEBUG: (ConvexHullPoints) points=\n");
//    ShowPoints(startPoint);
	
    return(ret);
}


/*
 ** Function : ConvexHullConcave()
 ** Purpose  : Extract Concave segment(s) from Ring and create new ring(s) for
 **            them
 ** Arguments: Ptr to 'ring'
 ** Returns  : Value indicating Error, Convex, or Concave Section to be processed
 ** Notes    : 
 */
int
ConvexHullConcave(struct ring *thisRing)
{
    struct ring   *newRing    = (struct ring *)NULL;
    struct points *startPoint = thisRing->pts;
    struct points *thisPoint  = (struct points *)NULL;
    struct points *bgnPoint;
    struct points *endPoint;
    struct points *newbgnPoint;
    struct points *newendPoint;
    int            convexhull = FALSE;
    int            ret        = CHPCNVEX; /* Assume it's convex */
	
	
    /*
	 ** Find the first/next Concave Point after a Convex Point
	 */
    while (thisPoint != startPoint)
    {
        /*
		 ** First time through, set my end condition
		 */
        if (thisPoint == (struct points *)NULL)
        {
            thisPoint = startPoint;
        }
		
        /*
		 ** If it's a Convex Hull Point, remember we're in a Convex Section
		 */
        if (thisPoint->chflag == CVXHULLYS)
        {
            convexhull = TRUE;
			
            /*
			 ** We have to be careful that we didn't start in a Concave segment
			 ** 'cos once we'd cut out the Concave segment we'd never return
			 ** to the startPoint and therefore loop forever!
			 ** If we did, just move the startPoint here
			 */
            if (startPoint->chflag != CVXHULLYS)
            {
                startPoint = thisPoint;
            }
        }
        else
        {
            ret = CHPCNCAV; /* Remember we found a Concave Section */
			
            /*
			 ** This is a 'Concave' Point
			 ** If we were in a Convex Section then we just found the start of
			 ** our first Concave Section ...
			 */
            if (convexhull == TRUE)
            {
                /*
				 ** Previous 'Point' is therefore the Beginning ...
				 ** Scoot forward to the End of this Concave Section
				 */
                bgnPoint = thisPoint->prev;
                while (thisPoint->chflag != CVXHULLYS)
                {
                    thisPoint = thisPoint->next;
                }
                endPoint = thisPoint;
				
                /* THE NEW RING:-
				 ** Get a new 'Ring' and slot it into the Circ List of Rings
				 ** just after the current (AKA 'this') 'ring'
				 */
                if ((newRing = NewRing()) == (struct ring *)NULL)
                {
                    return(CHPERROR);
                }
                newRing->prev       = thisRing;
                newRing->next       = thisRing->next;
                newRing->pos        = thisRing->pos + 1;
                newRing->next->prev = newRing;
                thisRing->next      = newRing;
				
                /*
				 ** Set the SQL Operand for the new Ring
				 */
                switch(thisRing->opand)
                {
                    case OPANDFIRST:  newRing->opand = OPANDANDNOT; break;
                    case OPANDAND:    newRing->opand = OPANDANDNOT; break;
                    case OPANDOR:     newRing->opand = OPANDANDNOT; break;
                    case OPANDANDNOT: newRing->opand = OPANDOR;     break;
						
                    default:
							fprintf(stderr,
                                "ConvexHullConcave: ERROR, Unknown opand (%d)\n",
                                thisRing->opand);
						
					return(CHPERROR);
                }
				
                /*
				 ** Copy the Begin and End points
				 */
                if ((newbgnPoint = NewPoint(bgnPoint)) == (struct points *)NULL)
                {
                    return(CHPERROR);
                }
                if ((newendPoint = NewPoint(endPoint)) == (struct points *)NULL)
                {
                    return(CHPERROR);
                }
				
                /*
				 ** Unlink the Concave Section from the Convex Hull
				 */
                bgnPoint->next = endPoint;
                endPoint->prev = bgnPoint;
				
                /*
				 ** Complete the link for the new Polygon
				 */
                newbgnPoint->prev       = newendPoint;
                newbgnPoint->next->prev = newbgnPoint;
                newendPoint->next       = newbgnPoint;
                newendPoint->prev->next = newendPoint;
				
                /*
				 ** If we just built an 'AND NOT' ring then it will be Anti-
				 ** Clockwise, and we need it to be Clockwise ...
				 */
                if (newRing->opand == OPANDANDNOT ||
                    newRing->opand == OPANDOR)
                {
                    if (! SetPointReverse(newbgnPoint))
                    {
                        return(CHPERROR);
                    }
                }
				
                /*
				 ** We need to reset the 'pos' counts within the new list
				 ** And whilst we're at it, reset the chflag
				 */
                if (! SetPointPosition(newbgnPoint, TRUE))
                {
                    /* !!!!! >>>> Should we issue an error message here??? */
                    return(CHPERROR);
                }
				
                /*
				 ** Add the new Circ list of Points to the new ring
				 ** and grab the count from the last in the Circ list
				 */
                newRing->pts        = newbgnPoint;
                newRing->cnt        = newRing->pts->prev->pos + 1;
                
                /*
                 **  Since we have added a new "ring" structure, we need to
                 **  compute the new diamond and box for it. This is done by
                 **  going round all the points in the circular chain, finding
                 **  the westmost, eastmost, top and bottom, and putting these
                 **  in to the "box" and "diamond" substructures for this new
                 **  ring.
                 */
				
                /*
				 ** Step over the 'Concave' section we just found ...
				 ** NB 'convexhull' is therefore still 'TRUE'
				 */
                thisPoint = endPoint;
            }
        }
		
        /*
		 ** Step to next item
		 */
        thisPoint = thisPoint->next;
    }
	
    /*
	 ** If the original list started on a Concave segment start from a
	 ** new Convex Hull point
	 ** We need to reset the 'pos' counts within the original list of points
	 */
    thisRing->pts = startPoint;
    if (! SetPointPosition(startPoint, FALSE))
    {
        return(CHPERROR);
    }
    /* ????? Debug compilation says that the following statement results in a
     * ????? dereference of a null pointer (loaded from field 'pts') ?????
     */
    thisRing->cnt = thisRing->pts->prev->pos + 1;
	
    /*
	 ** Let the caller know if we found any concave segments ...
	 */
    return(ret);
}


/*
 ** End of File
 ** Copyright (c) 2011-2013 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 */
