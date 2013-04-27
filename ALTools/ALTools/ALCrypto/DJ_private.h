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

#ifndef _DJ_PRIVATE_H_
#define _DJ_PRIVATE_H_


struct DJPrivateKey{
	struct OCObject _obj;
	
	mpz_t p;
	mpz_t q;
	
	mpz_t d; // == lambda
			// =phi(p,q)
};

struct DJPublicKey{
	struct OCObject _obj;
	
	int nModulusBits;
	mpz_t n; // modulus
	mpz_t g; // generator
};

struct DJ
{
	struct OCObject _obj;
	
	DJPrivateKeyRef prv;
	DJPublicKeyRef pub;
	int s;
	
	mpz_t h; // generator for random numbers
			// - depends on s!

	struct
	{
		mpz_t n_pow_s;
		mpz_t n_pow_s_plus_1;
		mpz_t* n_pow_si;
		
		mpz_t mu;

	} cache;
};


#endif
