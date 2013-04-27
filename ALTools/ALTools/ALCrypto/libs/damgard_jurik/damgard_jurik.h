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

#include <gmp.h>


void dj_get_rand_file(char* buf, int len, char* file);
void dj_get_rand_devrandom(void* buf, int len);
void init_rand(gmp_randstate_t randState, int bytes);
void damgard_jurik_encrypt(mpz_t c, mpz_t m, mpz_t r, mpz_t g, mpz_t ns, mpz_t ns1, int s, mpz_t h, int b);
void damgard_jurik_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t mu, mpz_t nj[], unsigned int s, mpz_t p1, mpz_t q1 );
void iterative_decrypt(mpz_t i, mpz_t c, mpz_t nj[], unsigned int s);
