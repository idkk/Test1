/*
 *  endian.h
 *
 *  Created by Ian Kelly on 14/08/2013.
 *  Copyright 2013 Westheimer Energy Consultants Inc/Ltd. All rights reserved.
 *
 */

/* Version 1.1 Modified 6 September 2013:11.35 */

/* 32-bit Endian definition of *this* machine: */
/*
 * Note that this definition can be
 * (a) extended to cover other formats, such as PDP (0xad), and
 * (b) altered to err at compile time if the format is not
 *     recognised (remove UNDEFINED and uncomment the assert)
 */
#ifndef CNVENDIAN
#define CNVENDIAN

static uint32_t endianness = 0xdeadbeef; 
//#enum TENDIANNESS { BIG, LITTLE, UNDEFINED };
#define BIG       0
#define LITTLE    1
#define UNDEFINED 2

#define ENDIANNESS ( *(const char *)&endianness == (char) 0xef ? LITTLE \
: *(const char *)&endianness == (char) 0xde ? BIG \
: UNDEFINED )
// : assert(0))
/*
 * This is tested by (for example):
 *   if (ENDIANNESS == BIG)...
 */

#endif

/*******************************************************************************
 *  End of cnvendian.h		
 *  Copyright 2013 Westheimer Energy Consulting Ltd. All rights reserved.
 ******************************************************************************/
