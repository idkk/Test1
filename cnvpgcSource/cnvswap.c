/*
 *  cnvswap.c
 *
 *  Created by Ian Kelly on 14/08/2013.
 *  Copyright 2013 Westheimer Energy Consultants Inc/Ltd. All rights reserved.
 *
 */

#include "cnvswap.h"


/*
 **! Byte swap unsigned short
 */
uint16_t swap_uint16( uint16_t val ) 
{
	return (val << 8) | (val >> 8 );
}

/*
 **! Byte swap short
 */
int16_t swap_int16( int16_t val ) 
{
	return (val << 8) | ((val >> 8) & 0xFF);
}

/*
 **! Byte swap unsigned int
 */
uint32_t swap_uint32( uint32_t val )
{
	val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
	return (val << 16) | (val >> 16);
}

/*
 **! Byte swap int
 */
int32_t swap_int32( int32_t val )
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | ((val >> 16) & 0xFFFF);
}


/*
 ** unswap using char pointers
 */

uint32_t swap_single(uint32_t a)
{
    uint32_t d;
    uint32_t aa;
    unsigned char *src = (unsigned char *)&aa;
    unsigned char *dst = (unsigned char *)&d;
    aa = a;
    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];
    return d;
}


/*
 ** unswap using char pointers
 */
double swap_double(unsigned long long a) 
{
	
    double d;
    unsigned char *src = (unsigned char *)&a;
    unsigned char *dst = (unsigned char *)&d;
	
    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];
	
    return d;
}

/*******************************************************************************
 *  End of cnvswap.c		
 *  Copyright 2013 Westheimer Energy Consulting Ltd. All rights reserved.
 ******************************************************************************/


