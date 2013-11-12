/*
 *  cnvfloat.h
 *
 *  Created by Ian Kelly on 14/08/2013.
 *  Copyright 2013 Westheimer Energy Consultants Inc/Ltd. All rights reserved.
 *
 */

#ifndef CNVFLOAT
#define CNVFLOAT

void ieee2ibm(void *to, const void *from, int len);
void ibm2ieee(void *to, const void *from, int len);

#endif

/*******************************************************************************
 *  End of cnvfloat.c		
 *  Copyright 2013 Westheimer Energy Consulting Ltd. All rights reserved.
 ******************************************************************************/