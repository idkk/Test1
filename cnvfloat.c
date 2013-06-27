/*
 *  cnvfloat.c
 *  cnvseg2sth
 *  cnvsth2seg
 *
 *  Created by Ian Kelly on 08/01/2013.
 *  Copyright 2013 Westheimer Energy Consultants Inc/Ltd. All rights reserved.
 *
 *  This file last updated 2013.02.12 12:15
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

/* Force no test on overflow: */
#ifdef TEST_FLOAT_OVERFLOW
#undef TEST_FLOAT_OVERFLOW
#endif

/* Conversion between floating-point formats: */

#if (1==1)
void ibm2ieee(void *to, const void *from, int len)
{
    /*
     *  The incoming parameters to this routine are:
     *  to    a pointer to the output word(s), which will receive the ieee
     *  from  a pointer to the input word(s), from which we get the ibm
     *  len   a count of the number of elements to convert in this call
     *
     *  Note that there is no declared type of the to/from parameters, as they
     *  will be used a pure memory address pointers. We cannot be sure whether
     *  this machine is big-endian or little-endian (in all probability, it is
     *  little-endian - but this code does *not* assume that): hence we ensure
     *  that all values are constructed from the composing bits.
     */
    
    /*  Note that the 'register' declarations below are not, strictly,
     *  necessary - though these are the three fields that are most used in
     *  this conversion routine:
     */
    register uint32_t fr;      /* fraction */
    uint32_t dfr; /* DEBUG */
    register /*int*/ char exp; /* exponent */
    register /*int*/ char sgn; /* sign     */
    
#ifdef TEST_FLOAT_OVERFLOW
    int lexp;        /* second (local) copy of exponent */
    /*  We need lexp only when we are testing the *value* of the exponent for
     *  overflow. This requires a value greater than 255 - which is more than
     *  fits into a single byte (char). If we are not testing overflow (which, I
     *  suggest, we normally should *not*), then this declaration is not needed.
     */
#endif
    
    void *lto;       /* local copy of 'to' pointer      */
    void *lfrom;     /* local copy of 'from' pointer    */
    int  llen;       /* local copy of 'len' value       */
    int  ii;         /* DEBUG counter                   */
    
    lto = (char *)to;
    lfrom = (char *)from;
    llen = len;
    
    for (; llen-- > 0; lto = (char *)lto + 4, lfrom = (char *)lfrom + 4) {
        /* split into sign, exponent, and fraction */
        //        fr = ntohl(*(long *)lfrom); /* pick up value */
        /*
         *  The input is IBM floating point numbers read in, and hence in
         *  big-endian format. There is some doubt in my (IDKK's) mind as to
         *  whether we need to perform the ntohl (byte swap) conversion on this
         *  input (when we are running on a little-endian machine) for this
         *  conversion process. Certainly the *value* of the sign is just the
         *  leftmost bit, and the *value* of the exponent is just the next
         *  seven bits. We may be able to partially get round answering this 
         *  'ntohl or not' question by using type char (rather than int) for sgn 
         *  and exp... but it cannot be avoided if we are using the *value* of
         *  the mantissa.
         */
        if (ENDIANNESS == BIG)
        {
            fr = *(uint32_t *)lfrom;
        } else {
            fr = swap_int32(*(uint32_t *)lfrom);
        }
        dfr = fr; /* DEBUG */
        sgn = fr >> 31; /* save sign          */
        fr <<= 1;       /* shift sign out     */
        exp = fr >> 25; /* save exponent      */
        fr <<= 7;       /* shift exponent out */
        
        if (fr == 0) { /* short-circuit for zero */
            exp = 0;
            goto done;
        }
        
        /* Adjust exponent from base 16 offset 64 radix point before first digit
         * to base 2 offset 127 radix point after first digit:
         * (exp - 64) * 4 + 127 - 1 == exp * 4 - 256 + 126 == (exp << 2) - 130 
         */
        exp = (exp << 2) - 130;
#ifdef TEST_FLOAT_OVERFLOW
        lexp = (exp << 2) - 130;
#endif
        
        /* (re)normalize */
        ii = 0; /* DEBUG */
        while ((fr & 0x80000000) == 0) { /* 3 times max for normalized input */
            --exp;
            fr <<= 1;
            ii++; /* DEBUG */
            if (ii>3) 
            {
                fprintf(stderr,"ERROR - Floating convert shift %d bits, "
                        "dfr=%8x fr=%8x exp=%8x sgn=%8x\n",ii,dfr,fr,exp,sgn);
            }
        }
        
        if (exp <= 0) { /* underflow */
            if (exp < -24) /* complete underflow - return properly signed zero */
                fr = 0;
            else /* partial underflow - return denormalized number */
                fr >>= -exp;
            exp = 0;
        } else
#ifdef TEST_FLOAT_OVERFLOW
            if (lexp >= 255) 
            { /* overflow - return infinity */
                fr = 0;
                exp = 255;
            } else 
#endif
            { /* just a plain old number - remove the assumed high bit */
                fr <<= 1;
            }
        
    done:
        /* put the pieces back together and return it by saving it */
        *(uint32_t *)lto = (fr >> 9) | (exp << 23) | (sgn << 31);
    }
}
#endif


/* ieee2ibm - Converts a number from IEEE 754 single precision floating
 point format to IBM 370 single precision format. For normalized
 numbers, the IBM format has greater range but less precision than the
 IEEE format. IEEE Infinity is mapped to the largest representable
 IBM 370 number. When precision is lost, rounding is toward zero
 (because it's fast and easy -- if someone really wants round to nearest
 it shouldn't be TOO difficult). */

#if (1==1)
void ieee2ibm(void *to, const void *from, int len)
{
    register uint32_t fr; /* fraction */ /* unsigned 32-bit integer */
    register int exp; /* exponent */ /* signed, 32 or 16 bit integer */
    register int sgn; /* sign */ /* single bit - 32 or 16 bit integer */
    //    char lexp; /* local, second copy of exponent */
    void *lto;
    void *lfrom;
    int  llen;
    
    /*
     *  The third parameter, len, is the number of 4-byte (32-bit) entries
     *  that are to be converted. This is when we have an array of ieee numbers
     *  that are to be converted into an array of ibm format numbers.
     *  The variables lto and lfrom are local (to this routine) copies of the
     *  incoming parameters which point to the data to be converted and the
     *  results.
     */
    
    lto = (char *)to;
    lfrom = (char *)from;
    llen = len;
    
    for (; llen-- > 0; lto = (char *)lto + 4, lfrom = (char *)lfrom + 4) 
    {
        /* split into sign, exponent, and fraction */
        fr = *(/*unsigned*/ uint32_t *)lfrom; /* pick up value */
        /* Note that the above statement should be a simple memory copy,
         * irrespective of whether this machine is big-endian or little-endian,
         * as the source and target fields are stated to be of the same type.
         * That is, we know that the input is in big-endian format, but for the
         * moment we are not looking at the *value* of the sub-fields (sign,
         * exponent, fraction) within it, only at their *bits*.
         */
        sgn = fr >> 31; /* save sign */
        fr <<= 1; /* shift sign out */
        exp = fr >> 24; /* save exponent */
        fr <<= 8; /* shift exponent out */
        
        if (exp == 255) { /* infinity (or NAN) - map to largest */
            fr = 0xffffff00;
            exp = 0x7f;
            goto done;
        }
        else if (exp > 0) /* add assumed digit */
        {
            /* Note that the following right shift does *not* lose a bit of
             * precision to the right, as field fr was earlier shifted left by
             * one bit, when droping the sign bit
             */
            fr = (fr >> 1) | 0x80000000;
        }
        else if (fr == 0) /* short-circuit for zero */
            goto done;
        
        /* adjust exponent from base 2 offset 127 radix point after first digit
         to base 16 offset 64 radix point before first digit */
        exp += 130;
        fr >>= -exp & 3;
        exp = (exp + 3) >> 2;
        
        /* (re)normalize */
        while (fr < 0x10000000) { /* never executed for normalized input */
            --exp;
            fr <<= 4;
        }
        
    done:
        /* put the pieces back together and return it */
        /*
         *  The following statement depends upon their being NO byte-swapping
         *  performed by the generated code. We are constructing a single 32-bit
         *  (output) word, composed of 24 bits of fr (fraction = mantissa), 
         *  7 bits of exponent (exp) and a single sign bit. If there is swap
         *  code generated by the compiler, then this statement has to change to
         *  something much less elegant-looking, and (possibly) more expensive
         *  in execution time.
         */
        fr = (fr >> 8) | (exp << 24) | (sgn << 31);
        /*
         *  We always want the output to be in big-endian order, so no matter
         *  what the endian-ness of this machine, we do NOT have to byte swap:
         */
        //        *(unsigned *)lto = htonl(fr);
        *(uint32_t *)lto = fr;
    }
}
#endif

#if (0==1)
static void ibm2ieee(int to[], int from[], int n, int endian)
/***********************************************************************
 ibm_to_float - convert between 32 bit IBM and IEEE floating numbers
 ************************************************************************
 Input::
 from		input vector
 to		output vector, can be same as input vector
 endian		byte order =0 little endian (DEC, PC's)
 =1 other systems
 *************************************************************************
 Notes:
 Up to 3 bits lost on IEEE -> IBM
 
 Assumes sizeof(int) == 4
 
 IBM -> IEEE may overflow or underflow, taken care of by
 substituting large number or zero
 
 Only integer shifting and masking are used.
 *************************************************************************
 Credits: CWP: Brian Sumner,  c.1985
 *************************************************************************/
{
    register int fconv, fmant, i, t;
    
    for (i = 0;i < n; ++i) {
        
        fconv = from[i];
        
        /* if little endian, i.e. endian=0 do this */
        if (endian == 0) fconv = (fconv << 24) | ((fconv >> 24) & 0xff) |
            ((fconv & 0xff00) << 8) | ((fconv & 0xff0000) >> 8);
        
        if (fconv) {
            fmant = 0x00ffffff & fconv;
            /* The next two lines were added by Toralf Foerster */
            /* to trap non-IBM format data i.e. conv=0 data  */
//            if (fmant == 0)
//                warn("mantissa is zero data may not be in IBM FLOAT Format !");
            t = (int) ((0x7f000000 & fconv) >> 22) - 130;
            while (!(fmant & 0x00800000)) { --t; fmant <<= 1; }
            if (t > 254) fconv = (0x80000000 & fconv) | 0x7f7fffff;
            else if (t <= 0) fconv = 0;
            else fconv =   (0x80000000 & fconv) | (t << 23)
                | (0x007fffff & fmant);
        }
        to[i] = fconv;
    }
    return;
}
#endif

