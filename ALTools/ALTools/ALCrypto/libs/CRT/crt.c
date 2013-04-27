// Copyright: please see https://github.com/blynn/pbc/blob/master/guru/indexcalculus.c
//


#include "crt.h"

// void CRT(...) taken from https://github.com/blynn/pbc/blob/master/guru/indexcalculus.c
// Garner's Algorithm.
// See Algorithm 14.71, Handbook of Cryptography.
void CRT(mpz_t x, mpz_ptr *v, mpz_ptr *m, int t) {
	mpz_t u;
	mpz_t C[t];
	int i, j;
	
	mpz_init(u);
	for (i=1; i<t; i++) {
		mpz_init(C[i]);
		mpz_set_ui(C[i], 1);
		for (j=0; j<i; j++) {
			mpz_invert(u, m[j], m[i]);
			mpz_mul(C[i], C[i], u);
			mpz_mod(C[i], C[i], m[i]);
		}
	}
	mpz_set(u, v[0]);
	mpz_set(x, u);
	for (i=1; i<t; i++) {
		mpz_sub(u, v[i], x);
		mpz_mul(u, u, C[i]);
		mpz_mod(u, u, m[i]);
		for (j=0; j<i; j++) {
			mpz_mul(u, u, m[j]);
		}
		mpz_add(x, x, u);
	}
	
	for (i=1; i<t; i++) mpz_clear(C[i]);
	mpz_clear(u);
}
