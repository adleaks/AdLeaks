/* Voting application based on the Paillier cryptosystem
 * Copyright (C) 2008-2009  Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil, Switzerland
 */

/* Please see 
 *	http://security.hsr.ch/msevote/vote-cgi/paillier.c
 * for the original version
 */

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

#include <stdio.h>
#include <stdlib.h>

#include "damgard_jurik.h"
#include "OCBase.h"
#include "crt.h"

void dj_get_rand_file( char* buf, int len, char* file )
{
	FILE* fp;
	char* p;
	
	fp = fopen(file, "r");
	
	p = buf;
	while( len )
	{
		size_t s;
		s = fread(p, 1, len, fp);
		p += s;
		len -= s;
	}
	
	fclose(fp);
}
void dj_get_rand_devrandom(void* buf, int len){
	dj_get_rand_file(buf, len, "/dev/urandom");	
}


void
init_rand( gmp_randstate_t randState, int bytes )
{
	void* buf;
	mpz_t s;
	
	buf = malloc(bytes);
	dj_get_rand_devrandom(buf, bytes);
	
	gmp_randinit_default(randState);
	mpz_init(s);
	mpz_import(s, bytes, 1, 1, 0, 0, buf);
	gmp_randseed(randState, s);
	mpz_clear(s);
	
	free(buf);
}

void damgard_jurik_encrypt(mpz_t c, 
						   mpz_t m, 
						   mpz_t r, 
						   mpz_t g, 
						   mpz_t ns, 
						   mpz_t ns1, 
						   int s,
						   mpz_t h,
						   int b)
{	
	VAR_UNUSED(b);
	mpz_t gm, rns;
	
	mpz_init(gm);
	mpz_init(rns);
	
	if(s == 1)
	{
		mpz_mul(gm, ns, m);
		mpz_add_ui(gm, gm, 1);
		mpz_mod(gm, gm, ns1);
	}
	else
		mpz_powm(gm, g, m, ns1);
	
	if(h == NULL)
	{
		mpz_powm(rns, r, ns, ns1);
	}
	else
	{
		mpz_powm(rns, h, r, ns1);
		//		mpz_mul_si(rns, rns, b);
	}
	mpz_mul(c, gm, rns);
	mpz_mod(c, c, ns1);
	
	mpz_clear(gm);
	mpz_clear(rns);	
}

void damgard_jurik_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t mu, mpz_t nj[], unsigned int s, mpz_t p, mpz_t q )
{
	if(1)
	{
		mpz_t ps1;
		mpz_t qs1;
		mpz_t gcd;
		mpz_init(gcd);
		
		mpz_init_set(ps1, p);
		mpz_init_set(qs1, q);
		
		mpz_pow_ui(ps1, ps1, s+1);
		mpz_pow_ui(qs1, qs1, s+1);
		
		mpz_gcd(gcd, ps1, qs1);
		
		mpz_ptr t[2], pp[2];
		int i;
		for(i = 0; i < 2; i++)
		{
			t[i] = (mpz_ptr)malloc(sizeof(mpz_t));
			pp[i] = (mpz_ptr)malloc(sizeof(mpz_t));
		}	
		for(i = 0; i < 2; i++)
			mpz_init(t[i]);
		
		mpz_init_set(pp[0], ps1);
		mpz_init_set(pp[1], qs1);	
		mpz_powm(t[0], c, d, pp[0]);
		mpz_powm(t[1], c, d, pp[1]);
		
		CRT(m, t, pp, 2);
	}
	else
	{
		mpz_powm(m, c, d, nj[s]);
	}	
	iterative_decrypt(m, m, nj, s);
	mpz_mul(m, m, mu);
	
	mpz_mod(m, m, nj[s-1]);
}

void iterative_decrypt(mpz_t i, mpz_t c, mpz_t nj[], unsigned int s)
{
	unsigned int j, k, kfac;
	mpz_t a, t1, t2, t3;
	
	mpz_init(t1);
	mpz_init(t2);
	mpz_init(t3);
	mpz_init_set(a, c);
	mpz_set_ui(i, 0);
	
	for (j = 1; j <=s; j++)
	{
		mpz_set(t1, a);
		//EXC_ARITHMETIC received here
		mpz_mod(t1, t1, nj[j]);
		mpz_sub_ui(t1, t1, 1);
		mpz_div(t1, t1, nj[0]);
		mpz_set(t2, i);
		kfac = 1;
		
		for (k = 2; k <= j; k++)
		{
			kfac *= k;
			mpz_sub_ui(i, i, 1);
			mpz_mul(t2, t2, i);
			mpz_mod(t2, t2, nj[j-1]);
			mpz_set_ui(t3, kfac);
			if (mpz_invert(t3, t3, nj[j-1]) == 0)
			{
				printf("\nFAILURE: could not invert %u! = %u\n", k, kfac);
			}
			mpz_mul(t3, t3, t2);
			mpz_mod(t3, t3, nj[j-1]);
			mpz_mul(t3, t3, nj[k-2]);
			mpz_mod(t3, t3, nj[j-1]);
			mpz_sub(t1, t1, t3);
			mpz_mod(t1, t1, nj[j-1]);
		}
		mpz_set(i, t1);
	}
	mpz_clear(a);
	mpz_clear(t1);
	mpz_clear(t2);
	mpz_clear(t3);
}
