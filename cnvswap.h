/*
 *  cnvswap.h
 *
 *  Created by Ian Kelly on 14/08/2013.
 *  Copyright 2013 Westheimer Energy Consultants Inc/Ltd. All rights reserved.
 *
 */

#ifndef CNVSWAP
#define CNVSWAP

/*
 *  The following definitions must match the native lengths of the types
 *  in the native machine. This avoids the body of the bulk of the code making
 *  any assumptions as to the number of bits in a long or an int etc.
 *  The four types declared here are for 16-bit ints, and for 32-bit ints,
 *  both signed and unsigned:
 */
#define int16_t    short int
#define uint16_t   unsigned short int
#define int32_t    int
#define uint32_t   unsigned int
/*
 *  And below we have a 32-bit float:
 */
#define float32_t  float


int16_t		swap_int16	(int16_t a);
uint16_t	swap_uint16 (uint16_t a);
int32_t		swap_int32	(int32_t a);
uint32_t	swap_uint32	(uint32_t a);
double      swap_double (unsigned long long a);
uint32_t    swap_single (uint32_t a);

#endif

/*******************************************************************************
 *  End of cnvswap.h		
 *  Copyright 2013 Westheimer Energy Consulting Ltd. All rights reserved.
 ******************************************************************************/
