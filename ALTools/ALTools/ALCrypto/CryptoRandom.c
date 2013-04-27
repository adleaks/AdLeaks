/* This file is part of AdLeaks.
 * Copyright (C) 2012-2013 Benjamin Güldenring
 * Freie Universität Berlin, Germany
 *
 * AdLeaks is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 *
 * AdLeaks is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AdLeaks.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CryptoRandom.h"

#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

// the code below is inspired by the "Secure Programming Cookbook for C and C++"
// -> http://shop.oreilly.com/product/9780596003944.do


static int fp_devURandom = -1;
static int fp_devRandom = -1;

static void init(void){
	fp_devRandom = open("/dev/random", O_RDONLY);
	fp_devURandom = open("/dev/urandom", O_RDONLY);
}

unsigned char* CryptoRandomRand(OCByte* buf, size_t nbytes, CryptoRandomSource source){
	
#ifdef RAND_ALWAYS_DEV_RANDOM
	source = CryptoRandomSourceDevRandom
#elif defined RAND_ALWAYS_DEV_URANDOM
	source = CryptoRandomSourceDevURandom
#endif
	
	if(source == CryptoRandomSourceDevRandom)
	{
		if(fp_devRandom == -1)
			init();
		
		OCByte* to = buf;
		size_t nToRead = nbytes;
		while(nbytes > 0)
		{
			ssize_t r = read(fp_devRandom, to, nToRead);
			if(r == -1)
			{
				if(errno == EINTR) 
					continue;
				fprintf(stderr, "Error while reading from /dev/random");
				exit(-1);
			}
			to += r;
			nToRead -= r;
		}
	}
	else if(source == CryptoRandomSourceDevURandom)
	{
		if(fp_devURandom == -1)
			init();
		
		OCByte* to = buf;
		size_t nToRead = nbytes;
		while(nToRead > 0)
		{
			ssize_t r = read(fp_devURandom, to, nToRead);
			if(r == -1)
			{
				if(errno == EINTR) 
					continue;
				fprintf(stderr, "Error while reading from /dev/urandom");
				exit(-1);
			}
			to += r;
			nToRead -= r;
		}
	}
	
	return buf;
}

