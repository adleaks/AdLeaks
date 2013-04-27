// Copyright: please see https://github.com/blynn/pbc/blob/master/guru/indexcalculus.c
//

#ifndef __CRT_H__
#define __CRT_H__

#include <gmp.h>

// Garner's Algorithm.
// See Algorithm 14.71, Handbook of Cryptography.
void CRT(mpz_t x, mpz_ptr *v, mpz_ptr *m, int t);


#endif
