/*
 ** Copyright (c) 2013, 2012, 2011 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 **
 ** pgcwhpolypoint.c (part of pgcwhpolygon)
 ** 
 ** This program reads a *.shp file derived *.csv file and produces a file
 ** containing a suitable WHERE clause
 **
 ** Use 'deshape2' to convert the .shp part of a Shape File into the .csv
 **
 ** e.g. whpolygon -i xxx.csv -o where.sql
 **
 ** NOTE: This copy has been modified by IDKK to deal with (a) sorting rings,
 **       and (b) determining the nesting of rings, and (c) scanning rings in
 **       the order required for generating the correct SQL "WHERE" clause. 
 */

/* This file last updated 20130617:1645 */

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

#include "pgcwhpolygon.h"


/*
 ** POINTS FUNCTIONS:-
 */
/*
 ** Function : NewPoint()
 ** Purpose  : 
 ** Arguments: Ptr to 'points' structure to copy, or NULL for New points
 ** Returns  : Ptr to Points
 ** Notes    :
 */
struct points *
NewPoint(struct points *oldPoint)
{
    struct points *ptr;
    
    /*
     **  Check that this new point does not go over the maximum number of
     **  corners allowed. If it does, then error exit:
     */
//    fprintf(stderr,"DEBUG: NewPoint corners=%d maxcorners=%d\n",corners,maxcorners);
    corners++;
    if (corners > maxcorners)
    {
        fprintf(stderr, 
                "ERROR: There are %d points - more than maximum of %d\n", 
                corners, maxcorners);
        exit (FAIL_EXCESS_CORNERS);
    }
	
    if ((ptr = malloc(sizeof(struct points))) == (struct points *)NULL)
    {
        fprintf(stderr, "ERROR: Unable to malloc space for points\n");
    }
    else
    {
        /*
		 ** Initialise the Structure
		 */
        if (oldPoint == (struct points *)NULL)
        {
            ptr->prev      = (struct points *)NULL;
            ptr->next      = (struct points *)NULL;
            ptr->pt.x      = -1;
            ptr->pt.y      = -1;
            ptr->chflag    = CVXHULLUK;
            ptr->pos       = -1;
            ptr->angle     = 0;
            ptr->distance  = -1;
            ptr->angle     = 0;
        }
        else
        {
            ptr->prev      = oldPoint->prev;
            ptr->next      = oldPoint->next;
            ptr->pt.x      = oldPoint->pt.x;
            ptr->pt.y      = oldPoint->pt.y;
            ptr->chflag    = oldPoint->chflag;
            ptr->pos       = oldPoint->pos;
            ptr->angle     = oldPoint->angle;
            ptr->distance  = oldPoint->distance;
            ptr->angle     = oldPoint->angle;
        }
    }
	
    return(ptr);
}


/*
 ** Function : GetPoints()
 ** Purpose  : Read the corners of all the polygons
 ** Arguments: Input File Pointer, First X and Y coords, Ptr to Box
 ** Returns  : Ptr to Start of points circular list, or NULL
 ** Notes    : The 1st pair of Coords already read ...
 */
struct points *
GetPoints(FILE       **fp,
          /* int          firstX,
          int          firstY, */
          COORDT       firstX,
          COORDT       firstY,
          /* struct box  *Box) */
		  struct box     *Box,
		  struct diamond *Diamond,
          struct box     *outerBox,
          struct diamond *outerDiamond)
{
    struct points *firstPoint = (struct points *)NULL;
    struct points *thisPoint;
    struct points *nextPoint  = (struct points *)NULL;
/*    int    readX = firstX;
    int    readY = firstY; */
    COORDT readX = firstX;
    COORDT readY = firstY;
    COORDT prevX = firstX;
    COORDT prevY = firstY;
    int    cnt = -1;              /* Count within this 'ring' 1st is 0th */
	
    /*
	 ** Loop 'For Ever' reading the Coord X,Y Pairs
	 **   Break from the loop later when we find the 1st Coords again
	 */
    while (1 == 1)
    {
        cnt++;
		
        /*
		 ** Set the Box values for Max and Min
		 */
        if (readX < Box->min.x)
        {
            Box->min.x = readX;
        }
        if (readY < Box->min.y)
        {
            Box->min.y = readY;
        }
        if (readX > Box->max.x)
        {
            Box->max.x = readX;
        }
        if (readY > Box->max.y)
        {
            Box->max.y = readY;
        }
		
		/*
		 ** IDKK: Set the Diamond values for Max and Min
		 */
        if (readX < Diamond->westmost.x)
        {
            Diamond->westmost.x = readX;
			Diamond->westmost.y = readY;
        }
        if (readY < Diamond->bottom.y)
        {
            Diamond->bottom.y = readY;
			Diamond->bottom.x = readX;
        }
        if (readX > Diamond->eastmost.x)
        {
            Diamond->eastmost.x = readX;
			Diamond->eastmost.y = readY;
        }
        if (readY > Diamond->top.y)
        {
            Diamond->top.y = readY;
			Diamond->top.x = readX;
        }
		/* End of IDKK insertion 20120330 */
		
        /*
		 ** If we've come full circle for this 'ring' of 'points' we can stop
		 ** But there must be at least 3 points or there's an error!
		 */
        if (cnt != 0 && 
            firstPoint != nextPoint && readX == firstX && readY == firstY)
        {
//			if (tpbits[TPOINT] != 0)
//			{
//                    fprintf(stderr,"GetPoints(%2d)   Found end of 'ring'\n", 
//                            cnt);
//			}
			
            break;
        }
		
        /*
		 ** Grab a new 'point'
		 */
//		if (tpbits[TPOINT] != 0)
//		{
//                fprintf(stderr,"GetPoints(%2d)   Get New Point\n", cnt);
//		}
        if ((nextPoint = NewPoint((struct points *)NULL)) ==
			(struct points *)NULL)
        {
            return((struct points *)NULL);
        }
		
        /*
		 ** Populate the new 'points'
		 */
//		if (tpbits[TPOINT] != 0)
//		{
//                fprintf(stderr,"GetPoints(%2d)   Populate\n", cnt);
//		}
        if (firstPoint == (struct points *)NULL)
        {
//			if (tpbits[TPOINT] != 0)
//			{
//                    fprintf(stderr,"GetPoints(%2d)     First Point\n", cnt);
//			}
			
            firstPoint = nextPoint;   /* Remember the First Point */
			
            /*
			 ** Build Circular List of one 'points' for _this_ point
			 */
            nextPoint->next = nextPoint;
            nextPoint->prev = nextPoint;
        }
        else
        {
//			if (tpbits[TPOINT] != 0)
//			{
//                    fprintf(stderr,"GetPoints(%2d)     Subsequent Points\n", 
//                            cnt);
//			}
			
            /*
			 ** Insert _next_ 'points' into circular linked list just after
			 ** _this_
			 ** 1st: Link to the 'points' after ...
			 */
            nextPoint->next       = thisPoint->next;
            nextPoint->next->prev = nextPoint;
			
            /*
			 ** 2nd: Link to the 'points' before ...
			 */
            thisPoint->next = nextPoint;
            nextPoint->prev = thisPoint;
        }
//		if (tpbits[TPOINT] != 0)
//		{
//                fprintf(stderr,"GetPoints(%2d)     Set X,Y (" FMTX "," FMTY 
//                        ")\n", cnt, readX, readY);
//		}
        nextPoint->pt.x = readX;
        nextPoint->pt.y = readY;
        nextPoint->pos  = cnt;
        /*
         *  If "cnt" is not zero then this point and the previous point should
         *  be different from each other by at lease "tolerance" - if we are
         *  performing strict checking. If they are not, then terminate with an
         *  error message:
         */
        if (strict != 0)
        {
            if ((abs(prevX - nextPoint->pt.x) < tolerance) ||
                (abs(prevY - nextPoint->pt.y) < tolerance))
            {
                fprintf(stderr, "ERROR: This point (%d, %d) is within tolerance (%d)\n"
                        "       of previous point (%d, %d)\n"
                        "       Run terminates\n", nextPoint->pt.x, nextPoint->pt.y,
                        tolerance, prevX, prevY);
                exit(FAIL_STRICT2);
            }
        }
        prevX = nextPoint->pt.x;
        prevY = nextPoint->pt.y;
        /* **** */
		
        /*
		 ** From the 3rd point onwards, we can calculate the 'Angle' of the
		 ** corners (as we have the first pair of 'lines')
		 */
        if (cnt > 1)
        {
            SetPointsAngle(nextPoint->prev);
        }
		
        /*
		 ** Step forward onto the newest 'point' ...
		 */
        thisPoint = nextPoint;
		
        /*
		 ** Grab the next pair of Coords
		 */
//		if (tpbits[TPOINT] != 0)
//		{
//                fprintf(stderr,"GetPoints(%2d) Read Point\n", cnt);
//		}
		if (fscanf(*fp, FMTX ", " FMTY "\n", &readX, &readY) == 0)
        {
            fprintf(stderr, "ERROR: (%d): Failed to read next pair of Coords\n", 
                    cnt);
			
            return((struct points *)NULL);
        }
//		if (tpbits[TPOINT] != 0)
//		{
//                fprintf(stderr,"GetPoints(%2d) Read X(" FMTX ") Y(" FMTY ")\n", 
//                    cnt, readX, readY);
//		}
		
		/*
		 * We have just got the coordinates of another point on this ring,
		 * so now we can update the ring box and the diamond for this ring.
		 */ 
//        if (tpbits[TPOINT] != 0)
//        {
//            fprintf(stderr,"DEBUG: Got point (" FMTX "," FMTY 
//                "), comparing against ring box=\n",readX,readY);
//            ShowBox(Box);
//            fprintf(stderr,"       and against outer box=\n");
//            ShowBox(outerBox);
//        }
		if (readX < Box->min.x)
		{
			Box->min.x = readX;
			Diamond->westmost.x = readX;
			Diamond->westmost.y = readY;
		}
		if (readX > Box->max.x)
		{
			Box->max.x = readX;
			Diamond->eastmost.x = readX;
			Diamond->eastmost.y = readY;
		}
		if (readY < Box->min.y)
		{
			Box->min.y = readY;
			Diamond->bottom.y = readY;
			Diamond->bottom.x = readX;
		}
		if (readY > Box->max.y)
		{
			Box->max.y = readY;
			Diamond->top.y = readY;
			Diamond->top.x = readX;
		}
        /*
         * And now set the outer Box ring and diamond:
         */
		if (readX < outerBox->min.x)
		{
			outerBox->min.x = readX;
			outerDiamond->westmost.x = readX;
			outerDiamond->westmost.y = readY;
		}
		if (readX > outerBox->max.x)
		{
			outerBox->max.x = readX;
			outerDiamond->eastmost.x = readX;
			outerDiamond->eastmost.y = readY;
		}
		if (readY < outerBox->min.y)
		{
			outerBox->min.y = readY;
			outerDiamond->bottom.y = readY;
			outerDiamond->bottom.x = readX;
		}
		if (readY > outerBox->max.y)
		{
			outerBox->max.y = readY;
			outerDiamond->top.y = readY;
			outerDiamond->top.x = readX;
		}
        
		/* IDKK 20120314 end of insert */
    }
	
    /*
	 ** ... and we just have to set the angle for the last, and first, corners
	 ** to finish up
	 */
    SetPointsAngle(thisPoint);       /* last point in the ring  */
    SetPointsAngle(thisPoint->next); /* first point in the ring */
//	if (tpbits[TPOINT] != 0)
//	{
//            fprintf(stderr,"       Last Angle = %f Adjusted\n", thisPoint->angle);
//            fprintf(stderr,"       Frst Angle = %f Adjusted\n", 
//                thisPoint->next->angle);
//	}
	
    /*
	 ** Let's see what we just read ...
	 */
    ShowPoints(firstPoint);
	
//	if (tpbits[TPOINT] != 0)
//	{
//            fprintf(stderr,"Found %d points in this 'ring'\n", cnt);
//	}
    return(firstPoint);
}


/*
 ** Function : SetPointReverse()
 ** Purpose  : Reverse the order of Points in the Circ Linked List
 ** Arguments: Ptr to (first?) Points in the 'ring'
 ** Returns  : TRUE if OK, or FALSE if anything dodgy happens
 ** Notes    : 
 */
int
SetPointReverse(struct points *startPoint)
{
    struct points *thisPoint = (struct points *)NULL;
    struct points *tempPoint;
    int ret = TRUE;
	
    /*
	 ** For each 'points' in the circular linked list
	 */
    while (thisPoint != startPoint)
    {
        /*
		 ** If this is the first time in, we need to initialise to startPoint
		 */
        if (thisPoint == (struct points *)NULL)
        {
            thisPoint = startPoint;
        }
		
        /*
		 ** Now we reverse the order of the Circ List by swapping the pointers
		 */
        tempPoint       = thisPoint->next;
        thisPoint->next = thisPoint->prev;
        thisPoint->prev = tempPoint;
		
        /*
		 ** Step to the next 'points' in the circular linked list
		 **   - if there is one -
		 **  NB we _could_ check the 'prev' ptr too ...
		 */
        if (thisPoint->next == (struct points *)NULL)
        {
            fprintf(stderr, "          **** Circle Incomplete ****\n");
            thisPoint = startPoint;  /* Force Premature Exit */
			
            ret = FALSE;
            break;
        }
        else
        {
            /*
			 ** Because we swapped the ptrs, we now step to the 'next' in the
			 ** old order, which is now the 'prev' ... easy!
			 */
            thisPoint = thisPoint->prev;
        }
    }
	
    /*
	 ** Let's see the Points now ...
	 */
    ShowPoints(startPoint);
	
    return(ret);
}


/*
 ** Function : SetPointPosition()
 ** Purpose  : Reset the 'pos' counters with a circ linked list of Points
 ** Arguments: Ptr to (first?) Points in the 'ring'
 **            Flag to determine whether to reset the chflag
 **            Flag to determine whether to reverse the order in the list
 ** Returns  : TRUE if OK, or FALSE if anything dodgy happens
 ** Notes    : 
 */
int
SetPointPosition(struct points *startPoint,
                 int            resetCHFlag)
{
    struct points *thisPoint = (struct points *)NULL;
    int pos = 0;
    int ret = TRUE;
	
    /*
	 ** For each 'points' in the circular linked list
	 */
    while (thisPoint != startPoint)
    {
        /*
		 ** If this is the first time in, we need to initialise to startPoint
		 */
        if (thisPoint == (struct points *)NULL)
        {
            thisPoint = startPoint;
        }
//		if (tpbits[TPOINT] != 0)
//		{
//                fprintf(stderr,"SetPointPosition:          Old Pos (%2d)", 
//                    thisPoint->pos);
//		}
        thisPoint->pos = pos++;
//		if (tpbits[TPOINT] != 0)
//		{
//                fprintf(stderr," becomes New Pos (%2d)\n", thisPoint->pos);
//		}
		
        /*
		 ** If asked, we reset the chflag too
		 */
        if (resetCHFlag)
        {
            thisPoint->chflag = CVXHULLUK;
        }
		
        /*
		 ** Step to the next 'points' in the circular linked list
		 **   - if there is one -
		 **  NB we _could_ check the 'prev' ptr too ...
		 */
        if (thisPoint->next == (struct points *)NULL)
        {
            fprintf(stderr, "          **** Circle Incomplete ****\n");
            thisPoint = startPoint;  /* Force Premature Exit */
			
            ret = FALSE;
            break;
        }
        else
        {
            thisPoint = thisPoint->next;
        }
    }
	
    /*
	 ** Let's see the Points now ...
	 */
    ShowPoints(startPoint);
	
    return(ret);
}


/*
 ** Function : BuildWherePoints()
 ** Purpose  : 
 ** Arguments: Ouptut File Pointer
 **            Ptr to _this_ 'ring'
 ** Returns  : 0 if OK, non zero otherwise
 ** Notes    :
 */
int
BuildWherePoints(FILE        **fp,
                 struct ring  *thisRing)
{
    struct points *startPoint = thisRing->pts;
    struct points *thisPoint  = (struct points *)NULL;
/*    int xDiff = 0;
    int yDiff = 0; */
    COORDT xDiff = 0;
    COORDT yDiff = 0;
    int ret   = TRUE;
	
    /*
	 ** Are we an 'OR' or an 'AND' or indeed an 'AND NOT' ...
	 */
    fprintf(*fp, "     -- Ring %2d\n", thisRing->pos);
    switch(thisRing->opand)
    {
        case OPANDFIRST:             /* First   WHERE Clause */
            fprintf(*fp, "WHERE  (");
            break;
			
        case OPANDAND:               /* AND     WHERE Clause */
            /*
			 ** If this is the first 'ring' Clause we need an extra brace
			 fprintf(*fp, "  AND %s(", thisRing->pos == 0 ? "(" : " ");
			 */
            fprintf(*fp, "  AND (");
            break;
        case OPANDOR:                /* OR      WHERE Clause */
			/*
			 fprintf(*fp, "     )   -- End previous clause\n");
			 */
            fprintf(*fp, "   OR (");
            break;
        case OPANDANDNOT:            /* AND NOT WHERE Clause */
			/*
			 fprintf(*fp, "     )   -- End previous clause\n");
			 */
            fprintf(*fp, "AND NOT (");
            break;
			
        case OPANDUNSET:             /* Uninitialised      */
        default:
            fprintf(stderr, "Incorrect WHERE Operand (%d)\n", thisRing->opand);
			
            return(FALSE);
    }
	
    /*
	 ** We _could_ keep some sort of count of '(' and ')' maybe?
	 */
	
    /*
	 ** For each Polygon 'line' in the circular linked list
	 */
//	if (tpbits[TWHERE] != 0)
//	{
//            fprintf(stderr,"  Build WHERE (line)\n"); 
//	}
    while (thisPoint != startPoint)
    {
        /*
		 ** If this is the first time in, we need to initialise to startPoint
		 */
        if (thisPoint == (struct points *)NULL)
        {
            thisPoint = startPoint;
        }
		
        /*
		 ** To determine whether (X,Y) is on the "correct" side of line A->B
		 ** (xA,yA) to (xB,yB) you need
		 **  0 <= sAB * (((xB - xA) * (Y - yA)) - ((yB - yA) * (X - xA)))
		 */
        xDiff = thisPoint->next->pt.x - thisPoint->pt.x;
        yDiff = thisPoint->next->pt.y - thisPoint->pt.y;
        /*
         *  If we are applying strict rules, then check whether the absolute
         *  values of xDiff and yDiff are below "tolerance": if they are, then
         *  we need to issue an error message and stop the run:
         */
        if (strict != 0)
        {
            if ((abs(xDiff) < tolerance) || (abs(yDiff) < tolerance))
            {
                fprintf(stderr, "ERROR: This point (%d, %d) is within tolerance (%d)\n"
                        "       of next point (%d, %d)\n"
                        "       Run terminates", thisPoint->pt.x, thisPoint->pt.y,
                        tolerance, thisPoint->next->pt.x, thisPoint->next->pt.y);
                exit(FAIL_STRICT3);
            }
        }
		fprintf(*fp, 
                "%s0 >= (" FMTX " * (%s - " FMTX ")) - (" FMTY " * (%s - " 
                FMTY ")) %s\n",
				thisPoint->pos != 0 ? "        " : "",
				xDiff,
				ycol,
				thisPoint->pt.y,
				yDiff,
				xcol,
				thisPoint->pt.x,
				thisPoint->next == startPoint ? ")  " : "AND");
		
        /*
		 ** Step to the next 'line' in the circular linked list
		 **   - if there is one -
		 */
        if (thisPoint->next == (struct points *)NULL)
        {
            fprintf(stderr, "          **** Circle Incomplete ****\n");
			
            ret = FALSE;
            break;
        }
        else
        {
            thisPoint = thisPoint->next;
        }
    }
	
    return(ret);
}


/*
 ** Function : SetPointsAngle()
 ** Purpose  : Calculate the exterior angle at _this_ point
 ** Arguments: Ptr to 'corner' point
 ** Returns  : Angle
 ** Notes    : We have to find out what the angle is between lines AB and BC,
 **            when what we know are the coordinates of A, B and C … (xa, ya,),
 **            (xb, yb) and (xc, yc). And we have to do this without using the
 **            tan function, as tangents are unfriendly numbers as we approach
 **            90° or π/2 and multiples thereof.
 **            This would be a lot easier if (xa, ya,) were (0, 0) [that is,
 **            point A is the origin] and line AB lay on the X axis, so that
 **            point B had coordinates (0, y´b)
 **            This give point C new coordinates (x´c, y´c).
 **            So some of this is like the formulae used in the elastic band
 **            algorithm – movement of origin [to A at (xa, ya,)] and rotation
 **            of axes [to make AB lie on the X axis]. 
 */
void
SetPointsAngle(struct points *thisPoint)
{
    struct points *prevPoint = thisPoint->prev;
    struct points *nextPoint = thisPoint->next;
	
    /*
	 ** Move the origin ...
	 */
//    float X2a = 0;
//    float Y2a = 0;
    float X2b = (float)(thisPoint->pt.x - prevPoint->pt.x);
    float Y2b = (float)(thisPoint->pt.y - prevPoint->pt.y);
    float X2c = (float)(nextPoint->pt.x - prevPoint->pt.x);
    float Y2c = (float)(nextPoint->pt.y - prevPoint->pt.y);
	
    /*
	 ** Rotate the axes ...
	 */
//    float X3a = 0;
//    float Y3a = 0;
    float X3b = sqrtf((X2b * X2b) + (Y2b * Y2b));
//    float Y3b = 0;
    float cos = X2b / X3b;            /* CAH: Cos = Adjacent / Hypotenuse */
    float sin = Y2b / X3b;            /* SOH: Sin = Opposite / Hypotenuse */
    float X3c = (cos * X2c) + (sin * Y2c);
    float Y3c = (cos * Y2c) - (sin * X2c);
	
    /*
	 ** Now we can ignore point A and find the angle of the line BC with the
	 ** 'X' axis. 
	 ** We find the value of its cosine, reduced to less than 90 deg
	 ** So we are now looking at a right angle triangle with corners b, c, and
	 ** new right angle point back on the X axis 'd' ...
	 ** Get the 'hypotenuse' length (line BC) first ...
	 */
    float adj = fabsf(X3c - X3b);
    float hyp = sqrtf((adj * adj) + (Y3c * Y3c));
	
    float Cabc;
    float Ang, EAng;
	
    /*
	 ** ... and SOHCAHTOA tells us the Cosine ("CAH") is Adjacent/Hypotenuse ...
	 */
    Cabc = adj / hyp;
	
    /*
	 ** Now it would be tempting to use function acos() at this point, as in 
	 ** acos(Cabc) to find the angle – but, alas, we have to do some 
	 ** correcting first. Function acos works only for positive angles in the 
	 ** range [0, π] (from zero to +180°)... and we don’t yet know which 
	 ** quadrant we are in. So we have to look at whether this is a right turn 
	 ** (X3c is negative) or a left turn (X3c is positive), and also whether the 
	 ** turn is more (X3c is less than X3b) or less (X3c is greater than X3b) 
	 ** than 90° (π/2 radians). So the four quadrants are:
	 **   C is below the axis and to the right of B:
	 **     the angle is less than 90° (π/2) and is positive.
	 **   C is below the axis and to the left of B:
	 **     the angle is greater than 90° (π/2) and is positive.
	 **   C is above the axis and to the right of B:
	 **     the angle is less than 90° (π/2) and is negative.
	 **   C is above the axis and to the left of B:
	 **     the angle is greater than 90° (π/2) and is negative.
	 ** So what we do is compute the angle Ang from acos(Cabc) and then (for the
	 ** four quadrants):    below right: T = Ang
	 **                     above right: T = - Ang
	 **                     below left:  T = π -Ang
	 **                     above left:  T = -π + Ang
	 ** So...
	 */
    Ang = acos(Cabc);
    
    /*
	 ** If X3c is less than X3b then we need to subtract the angle from
	 ** 180, or the radians equivalent, to get the actual angle
	 */
    if (X3c < X3b)
    {
        Ang = PI - Ang;
    }
	
    /*
	 ** If Y3c is < 0 (below the X axis) it is a 'Right Turn', and we want the
	 ** angle formed with the rest of the X axis past 'B' ...
	 */
    if (Y3c < 0)      /* Below The Line : Right Turn */
    {
        EAng = Ang;
    }
    else if (Y3c > 0) /* Above The Line : Left Turn (a 'Concave' Point) */
    {
        EAng = -Ang;
    }
    else              /* On The Line : ERROR */
    {
//		if (tpbits[TANGLE] != 0)
//		{
//                fprintf(stderr,"Warning: Straight Line Detected\n");
//		}
        EAng = 0;
    }
	
    /*
	 ** ... and let's save that angle to the Point
	 */
    thisPoint->angle = EAng;
	
    /*
	 ** Display the calculation information :-
	 */
//	if (tpbits[TANGLE] != 0)
//	{
//		fprintf(stderr,"\n");
//		fprintf(stderr,"   Calculate Angle for Corner %d\n", thisPoint->pos);
//		fprintf(stderr,"     Original Points\n");
//		fprintf(stderr,"       X,Ya, " FMTX ", " FMTY "\n", prevPoint->pt.x, 
//                prevPoint->pt.y);
//		fprintf(stderr,"       X,Yb, " FMTX ", " FMTY "\n", thisPoint->pt.x, 
//                thisPoint->pt.y);
//		fprintf(stderr,"       X,Yc, " FMTX ", " FMTY "\n", nextPoint->pt.x, 
//                nextPoint->pt.y);
//		fprintf(stderr,"     Move to Origin Points\n");
//		fprintf(stderr,"       X,Y2a, %.0f, %.0f\n", X2a, Y2a);
//		fprintf(stderr,"       X,Y2b, %.0f, %.0f\n", X2b, Y2b);
//		fprintf(stderr,"       X,Y2c, %.0f, %.0f\n", X2c, Y2c);
//		fprintf(stderr,"     Rotate to X-Axis Points  cos(%.3f) sin(%.3f)\n", 
//                cos, sin);
//		fprintf(stderr,"       X,Y3a, %.3f, %.3f\n", X3a, Y3a);
//		fprintf(stderr,"       X,Y3b, %.3f, %.3f\n", X3b, Y3b);
//		fprintf(stderr,"       X,Y3c, %.3f, %.3f\n", X3c, Y3c);
//		fprintf(stderr,"     Calculate the Angle ...\n");
//		fprintf(stderr,"       hyp   = %.3f\n", hyp);
//		fprintf(stderr,"       adj   = %.3f\n", adj);
//		fprintf(stderr,"       Cabc  = %.3f\n", Cabc);
//		fprintf(stderr,"       Angle = %.3f\n", Ang);
//		fprintf(stderr,"       Angle = %.3f Adjusted\n", thisPoint->angle);
//		fprintf(stderr,"\n");
//	}
	
    /* 
	 ** At the start of the computation we have:
	 **
	 SEAng = 0;
	 **
	 ** and then for every polygon vertex we compute:
	 **
	 SEAng = SEAng + Eang;
	 **
	 */
    /*
	 ** At the last vertex we look at SEAng/2 and see whether
	 ** it is close to +3.14159... or close to -3.14159...
	 ** If neither, then we have a problem!
	 ** If positive, then polygon was clockwise,
	 ** If negative, then polygon was anticlockwise.
	 */
}

/* IDKK 20120314: additional functions for nesting rings */
/*
 ** Function : PointLine()
 ** Purpose  : Determine the side of a line for a given point
 ** Arguments: Ptrs to start and end of line, and point being tested
 ** Returns  : int value: -1 for left, +1 for right, 0 for colinear
 ** Notes    : 
 */

int            
PointLine(struct point *lineStart, struct point *lineEnd, struct point *pointTest)
{
	int   retval = 0;
	/*
	 ** IDKK The test for linearity/side is to:
	 ** 1) for the test point, move the axes to the line start, then
	 ** 2) rotate the axes so that the line end is on the X axis, then
	 ** 3) from the new coordinates of the test point determine the return
	 */
	
	/*
	 ** Move the origin ...
	 */
    float X2b = (float)(lineEnd->x - lineStart->x); /* new line end */
    float Y2b = (float)(lineEnd->y - lineStart->y);
    float X2c = (float)(pointTest->x - lineStart->x); /* new test point */
    float Y2c = (float)(pointTest->y - lineStart->y);
	
    /*
	 ** Rotate the axes ...
	 */
    float X3b = sqrtf((X2b * X2b) + (Y2b * Y2b)); /* new line end */
    float cos = X2b / X3b;            /* CAH: Cos = Adjacent / Hypotenuse */
    float sin = Y2b / X3b;            /* SOH: Sin = Opposite / Hypotenuse */
    float Y3c = (cos * Y2c) - (sin * X2c);
	
    /*
	 ** If Y3c is < 0 (below the X axis) it is a 'Right Turn', and we want the
	 ** angle formed with the rest of the X axis past 'B' ...
	 */
    if (Y3c < 0)      /* Below The Line : Right Turn */
    {
        retval = 1;
    }
    else if (Y3c > 0) /* Above The Line : Left Turn (a 'Concave' Point) */
    {
        retval = -1;
    }
    else           
    {
        retval = 0;
    }
	
	return retval;
}
/* IDKK nesting rings insertion end*/

/*
 ** Function : ShowPoints()
 ** Purpose  : Display all Points in _this_ ring (so far)
 ** Arguments: Ptr to (first?) Points in the 'ring'
 ** Returns  : void
 ** Notes    : If the 'ring' is incomplete, display as much as we can
 */
void
ShowPoints(struct points *startPoint)
{
    struct points *thisPoint = (struct points *)NULL;
    float angleTot = 0;
	
    /*
	 ** For each 'points' in the circular linked list
	 */
//	if (tpbits[TPOINT] != 0)
//	{
//            fprintf(stderr,"Display Points:-\n");
//	}
    while (thisPoint != startPoint)
    {
        /*
		 ** If this is the first time in, we need to initialise to startPoint
		 ** And we can throw in some column titles
		 */
        if (thisPoint == (struct points *)NULL)
        {
            thisPoint = startPoint;
//			if (tpbits[TPOINT] != 0)
//			{
//				fprintf(stderr,"    Pos  CH  (  X   ,  Y   )  Angle  (deg)\n");
//			}
        }
		
        /*
		 ** Print out the info
		 */
//		if (tpbits[TPOINT] != 0)
//		{
//			fprintf(stderr,"    (%2d) (%d) (" FMTX "," FMTY ") %s%.3f %s%.3f\n",
//                   thisPoint->pos,
//                   thisPoint->chflag,
//                   thisPoint->pt.x, thisPoint->pt.y,
//                   thisPoint->angle >= 0 ? " " : "", thisPoint->angle,
//                   thisPoint->angle >= 0 ? " " : "", thisPoint->angle * RADDEG); 
            /* NOTE: RADDEG = 180/pi */
//		}
		
        angleTot = angleTot + thisPoint->angle;
		
        /*
		 ** Step to the next 'points' in the circular linked list
		 **   - if there is one -
		 */
        if (thisPoint->next == (struct points *)NULL)
        {
//			if (tpbits[TPOINT] != 0)
//			{
//                    fprintf(stderr,"          **** Circle Incomplete ****\n");
//			}
            thisPoint = startPoint;  /* Force Premature Exit */
        }
        else
        {
            thisPoint = thisPoint->next;
        }
    }
	
//	if (tpbits[TPOINT] != 0)
//	{
//            fprintf(stderr,"                     Total/2: %.3f\n", angleTot / 2);
//	}
}


/*
 ** Box Functions
 */
/*
 ** Function : BuildWhereBox()
 ** Purpose  : Write out WHERE Clause for the Box
 ** Arguments: Ptr to output FILE, Ptr to Box
 ** Returns  : void
 ** Notes    : 
 */
void
BuildWhereBox(FILE       **fp,
              struct box  *Box)
{
//	if (tpbits[TWHERE] != 0)
//	{
//            fprintf(stderr, "Build WHERE (box)\n"); 
//        ShowBox(Box);
//	}
    fprintf(*fp, "FROM  ??trace_RCD\n");
    fprintf(*fp, "WHERE -- Only points within the Box\n");
	fprintf(*fp, "      (%s >= " FMTX " AND %s <= " FMTY " AND\n",
			xcol, Box->min.x, xcol, Box->max.x);
    fprintf(*fp, "       %s >= " FMTX " AND %s <= " FMTY ")\n",
			ycol, Box->min.y, ycol, Box->max.y);
}


/*
 ** Function : ShowBox()
 ** Purpose  : Display the Box info
 ** Arguments: Ptr to Box
 ** Returns  : void
 ** Notes    : 
 */
void
ShowBox(struct box *Box)
{
//	if (tpbits[TPOINT] != 0)
//	{
//            fprintf(stderr,"Display Box:-\n");
//            fprintf(stderr,"    Min X:(" FMTX ") Y:(" FMTY ")\n", 
//                Box->min.x, Box->min.y);
//            fprintf(stderr,"    Max X:(" FMTX ") Y:(" FMTY ")\n", 
//                Box->max.x, Box->max.y);
//	}
}

/*
 ** Function : ShowDiamond()
 ** Purpose  : Display the Diamond info
 ** Arguments: Ptr to Diamond
 ** Returns  : void
 ** Notes    : 
 */
void
ShowDiamond(struct diamond *Diamond)
{
//	if (tpbits[TPOINT] != 0)
//	{
//            fprintf(stderr,"Display Diamond:-\n");
//            fprintf(stderr,"    Top  X:(" FMTX ") Y:(" FMTY ")\n", 
//                Diamond->top.x, Diamond->top.y);
//            fprintf(stderr,"    Bot. X:(" FMTX ") Y:(" FMTY ")\n", 
//                Diamond->bottom.x, Diamond->bottom.y);
//            fprintf(stderr,"    West X:(" FMTX ") Y:(" FMTY ")\n", 
//                Diamond->westmost.x, Diamond->westmost.y);
//            fprintf(stderr,"    East X:(" FMTX ") Y:(" FMTY ")\n", 
//                Diamond->eastmost.x, Diamond->eastmost.y);
//	}
}



/*
 ** End of File
 ** Copyright (c) 2013, 2012, 2011 Westheimer Energy Consultants Ltd ALL RIGHTS RESERVED
 */
