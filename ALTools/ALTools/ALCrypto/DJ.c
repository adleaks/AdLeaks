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

#include "DJ.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

#include "OCAllocator.h"
#include "OCAllocator.h"
#include "CryptoRandom.h"
#include "OCBase.h"
#include "OCTime.h"
#include "DJ_private.h"
#include "damgard_jurik.h"

// for exports/imports
#define ORDER 1
#define SIZE 1
#define ENDIAN 0
#define NAILS 0

size_t __gmpz_out_str (FILE *, int, mpz_srcptr);


void DJPublicKeyDealloc(void* obj){
	DJPublicKeyRef pub = (DJPublicKeyRef) obj;
	mpz_clear(pub->g);
	mpz_clear(pub->n);
}

void DJPrivateKeyDealloc(void* obj){
	DJPrivateKeyRef prv = (DJPrivateKeyRef) obj;
	mpz_clear(prv->d);
	mpz_clear(prv->p);
	mpz_clear(prv->q);
}

void DJKeyGen(int nModulusBits, DJPublicKeyRef* pub, DJPrivateKeyRef* prv){
	mpz_t p;
	mpz_t q;
	gmp_randstate_t randState;
	
	OCObjectInfo prvInfo = {sizeof(struct DJPrivateKey), DJPrivateKeyDealloc};
	*prv = (DJPrivateKeyRef) OCObjectCreateObjectWithAllocator(ocDefaultAllocator, prvInfo);
	OCObjectInfo pubInfo = {sizeof(struct DJPublicKey), DJPublicKeyDealloc};
	*pub = (DJPublicKeyRef) OCObjectCreateObjectWithAllocator(ocDefaultAllocator, pubInfo);
	
	mpz_init((*pub)->n);
	mpz_init((*pub)->g);
	mpz_init((*prv)->d);
	mpz_init(p);
	mpz_init(q);

	// generate primes p,q, so that p,q mod 4 = 3 and gcd(p,q)=2
	
	init_rand(randState, nModulusBits / 8 + 1);
	
	mpz_t gcd;
	mpz_t p1, q1;
	mpz_init(gcd);
	mpz_init(p1);
	mpz_init(q1);
	mpz_t tmp;
	mpz_init(tmp);
	do
	{
		do
		{
			mpz_urandomb(p, randState, nModulusBits / 2);
			mpz_mod_ui(tmp, p, 4);
		}
		while( !mpz_probab_prime_p(p, 10) || mpz_cmp_ui(tmp, 3) != 0);
		
		do
		{
			mpz_urandomb(q, randState, nModulusBits / 2);
			mpz_mod_ui(tmp, q, 4);
		}
		while( !mpz_probab_prime_p(q, 10) || mpz_cmp_ui(tmp, 3) != 0);
		
		mpz_mul((*pub)->n, p, q);
		mpz_sub_ui(p1, p, 1);
		mpz_sub_ui(q1, q, 1);
		mpz_gcd(gcd, p1, q1);
	} 
	while( ! mpz_tstbit((*pub)->n, nModulusBits - 1) || (mpz_cmp_ui(gcd, 2) != 0));
	mpz_clear(tmp);

	(*pub)->nModulusBits = nModulusBits;
	
	mpz_add_ui((*pub)->g, (*pub)->n, 1);
	
	mpz_init_set((*prv)->p, p);
	mpz_init_set((*prv)->q, q);
	mpz_sub_ui(p, p, 1);
	mpz_sub_ui(q, q, 1);
	mpz_lcm((*prv)->d, p, q);

	
	mpz_clear(p);
	mpz_clear(q);
	gmp_randclear(randState);	
}


void DJSerializeMpz(OCSerializerRef s, mpz_ptr num){
	
	OCByte* buf = (OCByte*)mpz_get_str(NULL, 16, num);
	size_t count = strlen((char*)buf)+1;
	OCSerializerWriteInt(s, count);
	OCSerializerWriteBytes(s, buf, count);	
	free(buf);
}

void DJDeserializeMpz(OCDeserializerRef ds, mpz_ptr num){
	
	int32_t count = 0;
	OCDeserializerReadInt(ds, &count);
	
	OCByte* buf = OCAllocate(ocDefaultAllocator, count+1);
	OCDeserializerReadBytes(ds, buf, count);
	//	mpz_import(prv->d, count, ORDER, SIZE, ENDIAN, NAILS, buf);
	buf[count] = '\0';

	mpz_set_str(num, (char*)buf, 16);

	OCDeallocate(ocDefaultAllocator, buf);
}

OCStatus DJPrivateKeySerialize(DJPrivateKeyRef prv, OCSerializerRef s){
	OCByte buf[10000];
	
	size_t count = 0;
	
	mpz_get_str((char*)buf, 16, prv->d);
	count = strlen((char*)buf)+1;
	buf[count-1] = '\n';
//	mpz_export(buf, &count, ORDER, SIZE, ENDIAN, NAILS, prv->d);
	OCSerializerWriteInt(s, count);
	OCSerializerWriteBytes(s, buf, count);
	
	mpz_get_str((char*)buf, 16, prv->p);
	count = strlen((char*)buf)+1;
	buf[count-1] = '\n';
//	mpz_export(buf, &count, ORDER, SIZE, ENDIAN, NAILS, prv->p);
	OCSerializerWriteInt(s, count);
	OCSerializerWriteBytes(s, buf, count);
	
	mpz_get_str((char*)buf, 16, prv->q);
	count = strlen((char*)buf)+1;
	buf[count-1] = '\n';
//	mpz_export(buf, &count, ORDER, SIZE, ENDIAN, NAILS, prv->q);
	OCSerializerWriteInt(s, count);
	OCSerializerWriteBytes(s, buf, count);

	return SUCCESS;
}

DJPrivateKeyRef DJPrivateKeyDeserialize(OCDeserializerRef ds){
	OCObjectInfo info = {sizeof(struct DJPrivateKey), DJPrivateKeyDealloc};
	DJPrivateKeyRef prv = (DJPrivateKeyRef) OCObjectCreateObjectWithAllocator(ocDefaultAllocator, info);
	
	OCByte buf[10000];
	int32_t count = 0;
	
	OCDeserializerReadInt(ds, &count);
	OCDeserializerReadBytes(ds, buf, count);
//	mpz_import(prv->d, count, ORDER, SIZE, ENDIAN, NAILS, buf);
	buf[count] = '\0';
	mpz_init_set_str(prv->d, (char*)buf, 16);
	
	OCDeserializerReadInt(ds, &count);
	OCDeserializerReadBytes(ds, buf, count);
//	mpz_import(prv->p, count, ORDER, SIZE, ENDIAN, NAILS, buf);
	buf[count] = '\0';
	mpz_init_set_str(prv->p, (char*)buf, 16);

	OCDeserializerReadInt(ds, &count);
	OCDeserializerReadBytes(ds, buf, count);
//	mpz_import(prv->q, count, ORDER, SIZE, ENDIAN, NAILS, buf);
	buf[count] = '\0';
	mpz_init_set_str(prv->q, (char*)buf, 16);
	
	return prv;
}

OCStatus DJPublicKeySerialize(DJPublicKeyRef pub, OCSerializerRef s){
	
	OCByte buf[10000];
	
	size_t count = 0;
	
	OCSerializerWriteInt(s, pub->nModulusBits);
	
//	mpz_export(buf, &count, ORDER, SIZE, ENDIAN, NAILS, pub->g);
	mpz_get_str((char*)buf, 16, pub->g);
	count = strlen((char*)buf)+1;
	buf[count-1] = '\n';
	OCSerializerWriteInt(s, count);
	OCSerializerWriteBytes(s, buf, count);
	
//	mpz_export(buf, &count, ORDER, SIZE, ENDIAN, NAILS, pub->n);
	mpz_get_str((char*)buf, 16, pub->n);
	count = strlen((char*)buf)+1;
	buf[count-1] = '\n';
	OCSerializerWriteInt(s, count);
	OCSerializerWriteBytes(s, buf, count);
	
	return SUCCESS;
}

DJPublicKeyRef DJPublicKeyDeserialize(OCDeserializerRef ds){
	OCObjectInfo info = {sizeof(struct DJPublicKey), DJPublicKeyDealloc};
	DJPublicKeyRef pub = (DJPublicKeyRef) OCObjectCreateObjectWithAllocator(ocDefaultAllocator, info);
	
	OCByte buf[10000];
	int32_t count = 0;
	
	OCDeserializerReadInt(ds, &(pub->nModulusBits));
	
	OCDeserializerReadInt(ds, &count);
	OCDeserializerReadBytes(ds, buf, count);
//	mpz_import(pub->g, count, ORDER, SIZE, ENDIAN, NAILS, buf);
	buf[count] = '\0';
	mpz_init_set_str(pub->g, (char*)buf, 16);

	OCDeserializerReadInt(ds, &count);
	OCDeserializerReadBytes(ds, buf, count);
//	mpz_import(pub->n, count, ORDER, SIZE, ENDIAN, NAILS, buf);
	buf[count] = '\0';
	mpz_init_set_str(pub->n, (char*)buf, 16);
	
	return pub;
}



void DJDealloc(void* _me){
	DJRef me = (DJRef) _me;
	OCObjectRelease(&me->pub);
	if(me->prv)
		mpz_clear(me->cache.mu);
	OCObjectRelease(&me->prv);
	
	mpz_clear(me->h);
	
	int i = 0; 
	mpz_clear(me->cache.n_pow_s);
	mpz_clear(me->cache.n_pow_s_plus_1);
	for(i=0; i < me->s+1; i++)
		mpz_clear(me->cache.n_pow_si[i]);
	OCDeallocate(ocDefaultAllocator, me->cache.n_pow_si);
}


DJRef DJCreate(int s, DJPublicKeyRef pub, DJPrivateKeyRef prv, mpz_ptr h){
	OCOBJECT_ALLOCINIT(DJ);
	DJRef dj = me;
	
	if(dj == NULL)
		return NULL;

	dj->s = s;
	dj->pub = (DJPublicKeyRef)OCObjectRetain((OCObjectRef)pub);
	if(prv)
		dj->prv = (DJPrivateKeyRef) OCObjectRetain((OCObjectRef)prv);
	else
		dj->prv = NULL;
	// create caches
	if(pub)
	{
		int i = 0;
		mpz_init(dj->cache.n_pow_s);
		mpz_init(dj->cache.n_pow_s_plus_1);
		
		dj->cache.n_pow_si = OCAllocate(ocDefaultAllocator, (s+1) * sizeof(mpz_t));
		for(i = 0; i < dj->s+1; i++)
			mpz_init(dj->cache.n_pow_si[i]);
		
		mpz_pow_ui(dj->cache.n_pow_s, dj->pub->n, dj->s);
		mpz_pow_ui(dj->cache.n_pow_s_plus_1, dj->pub->n, dj->s+1);
		
		mpz_set(dj->cache.n_pow_si[0], dj->pub->n);
		int j;
		for (j = 1; j <= dj->s; j++)
		{
			mpz_set(dj->cache.n_pow_si[j], dj->cache.n_pow_si[j-1]);
			mpz_mul(dj->cache.n_pow_si[j], dj->cache.n_pow_si[j], pub->n);
		}	
		mpz_init(dj->h);

		if(h == NULL)
		{
			mpz_t r;
			mpz_init(r);
			gmp_randstate_t randState;
				
			init_rand(randState, dj->pub->nModulusBits / 8 + 1);
			do
			{
				mpz_urandomb(r, randState, dj->pub->nModulusBits);
			}
			while( mpz_cmp(r, dj->pub->n) >= 0 );
			// -x^2
			mpz_pow_ui(r, r, 2);
			mpz_mul_si(r, r, -1);
			// "h" will be h_s =h^(N^s) mod N^(s+1)
			mpz_init(dj->h);
			mpz_powm(dj->h, r, dj->cache.n_pow_s, dj->cache.n_pow_s_plus_1);
//			mpz_out_str(stdout, 16, dj->h);
			mpz_clear(r);
			gmp_randclear(randState);
		}
		else
			mpz_set(dj->h, h);
	}
	if(prv)
	{
		mpz_init(dj->cache.mu);
		mpz_powm(dj->cache.mu, dj->pub->g, dj->prv->d, dj->cache.n_pow_s_plus_1);
		iterative_decrypt(dj->cache.mu, dj->cache.mu, dj->cache.n_pow_si, s);
		mpz_invert(dj->cache.mu, dj->cache.mu, dj->cache.n_pow_s);
	}
	
	return dj;
}


void DJEncrypt(DJRef ctx, DJCipherTextRef ct, DJPlaintextRef pt, DJRandomRef r){
	if(r == NULL)
	{
		mpz_t rr;
		gmp_randstate_t randState;
	
		mpz_init(rr);
		init_rand(randState, ctx->pub->nModulusBits / 8 + 1);
		DJPublicKeyRef pub = ctx->pub;
		do
			mpz_urandomb(rr, randState, (pub->nModulusBits/8)/2 );
		while( mpz_cmp(rr, ctx->pub->n) >= 0 );
		gmp_randclear(randState);
		OCByte bb;
		CryptoRandomRand(&bb, 1, CryptoRandomSourceDevURandom);
		int b = bb%2;
		b = (b==0)?-1:+1;
				
		damgard_jurik_encrypt(	  ct,
							  pt,
							  rr,
							  ctx->pub->g,
							  ctx->cache.n_pow_s,
							  ctx->cache.n_pow_s_plus_1,
							  ctx->s, 
							  ctx->h,
							  b);
		mpz_clear(rr);
	}
	else
	{				
		damgard_jurik_encrypt(	  ct,
							  pt,
							  r,
							  ctx->pub->g,
							  ctx->cache.n_pow_s,
							  ctx->cache.n_pow_s_plus_1,
							  ctx->s, 
							  ctx->h,
							  1);
	}
	
	return;
}

void DJDecrypt(DJRef ctx, DJPlaintextRef pt, DJCipherTextRef ct){
	damgard_jurik_decrypt(	  pt,
						  ct,
						  ctx->prv->d,
						  ctx->cache.mu,
						  ctx->cache.n_pow_si,
						  ctx->s,
						  ctx->prv->p,
						  ctx->prv->q);
	return;	
}


void DJMult(DJRef ctx, DJCipherTextRef res, 
			DJCipherTextRef ct1, DJCipherTextRef ct2){
	mpz_mul(res, ct1, ct2);	
	mpz_mod(res, res, ctx->cache.n_pow_s_plus_1);
}

int DJGetS(DJRef dj){
	return dj->s;
}

int DJGetModulusBits(DJRef dj){
	return dj->pub->nModulusBits;
}

mpz_t* DJGetPlaintextModulus(DJRef dj){
	return &dj->cache.n_pow_s;
}

mpz_t* DJGetH(DJRef dj){
	return &dj->h;
}

mpz_t* DJGetModulus(DJRef dj){
	return &(dj->pub->n);	
}

mpz_t* DJGetCipherModulus(DJRef dj){
	return &(dj->cache.n_pow_s_plus_1);
}

DJPublicKeyRef DJGetPublicKey(DJRef dj){
	return dj->pub;
}

DJPrivateKeyRef DJGetPrivateKey(DJRef dj){
	return dj->prv;
}

void DJPrintInfo(DJRef dj){
	OCLog("Modulus = %d bits", dj->pub->nModulusBits);
	OCLog("S=%d", dj->s);

	char buf[10000]; // <- i know this is bad... shame on me
	mpz_get_str(buf, 16, dj->pub->n);
	OCLog("public:");
	OCLog("\t n = %s", buf);
	mpz_get_str(buf, 16, dj->pub->g);
	OCLog("\t g = %s", buf);
	mpz_get_str(buf, 16, dj->h);
	OCLog("\t h= %s", buf);
	
	if(dj->prv)
	{
		OCLog("private:");
		mpz_get_str(buf, 16, dj->prv->d);
		OCLog("\t d = %s", buf);
		mpz_get_str(buf, 16, dj->prv->p);
		OCLog("\t p = %s", buf);
		mpz_get_str(buf, 16, dj->prv->q);
		OCLog("\t q = %s", buf);
	}
}

