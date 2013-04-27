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


#ifndef _DJ_H_
#define _DJ_H_

#include <gmp.h>
#include "OCBase.h"
#include "OCSerializer.h"

typedef mpz_t DJPlaintextRef;
typedef mpz_t DJCipherTextRef;
typedef mpz_t DJRandomRef;

typedef struct DJPrivateKey* DJPrivateKeyRef;
typedef struct DJPublicKey* DJPublicKeyRef;
typedef struct DJ* DJRef;

void DJKeyGen(int nModulusBits, DJPublicKeyRef* pub, DJPrivateKeyRef* prv);

OCStatus DJPrivateKeySerialize(DJPrivateKeyRef prv, OCSerializerRef s);
DJPrivateKeyRef DJPrivateKeyDeserialize(OCDeserializerRef ds);
OCStatus DJPublicKeySerialize(DJPublicKeyRef pub, OCSerializerRef s);
DJPublicKeyRef DJPublicKeyDeserialize(OCDeserializerRef ds);

// pub may not be NULL
// prv may be NULL
DJRef DJCreate(int s, DJPublicKeyRef pub, DJPrivateKeyRef prv,
			   mpz_ptr hGen);

void DJEncrypt(DJRef ctx, DJCipherTextRef ct, DJPlaintextRef pt, DJRandomRef r);

void DJDecrypt(DJRef ctx, DJPlaintextRef pt, DJCipherTextRef ct);

void DJMult(DJRef ctx, DJCipherTextRef res, 
			DJCipherTextRef ct1, DJCipherTextRef ct2);

#pragma mark getters
int DJGetS(DJRef dj);
mpz_t* DJGetH(DJRef dj);

int DJGetModulusBits(DJRef dj);
mpz_t* DJGetModulus(DJRef dj);
mpz_t* DJGetPlaintextModulus(DJRef dj);
mpz_t* DJGetCipherModulus(DJRef dj);

DJPublicKeyRef DJGetPublicKey(DJRef dj);
DJPrivateKeyRef DJGetPrivateKey(DJRef dj);


// various other
void DJPrintInfo(DJRef dj);

// helper
void DJSerializeMpz(OCSerializerRef s, mpz_ptr num);
void DJDeserializeMpz(OCDeserializerRef ds, mpz_ptr num);

void dj_get_rand_devrandom(void* buf, int len);
void init_rand( gmp_randstate_t randState, int bytes);

#endif
