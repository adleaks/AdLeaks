/* This file is part of AdLeaks.
 * Copyright (C) 2013 Benjamin Güldenring
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

#include "ALProtocolTR.h"
#include "DJ.h"
#include "OCObject.h"
#include "CryptoRandom.h"
#include <string.h>
#include "ALProtocolTR.h"
#include "ALChunkTR.h"
#include "ALChunkTR_private.h"
#include "CryptoMac.h"
#include "OCLog.h"
#include <gmp.h>
#include <math.h>
#include "OCTime.h"

#define KLENGTH 32

struct ALProtocolTR{
	struct OCObject _obj;
	
	OCByte HKey[KLENGTH];
	
	DJRef djM;
	DJRef djT;
	int sM;
	int sT;
	
	// for perdersons commitment scheme:
	mpz_t gen_g; 
	mpz_t x; // private for tier-3
	mpz_t gen_h; // h = g^x
	
	
	ALProtocolType type;
	ALProtocolSerial serial;
	
	// TODO: ECDH + AES 
};

static void helperCreateHashedNum(mpz_t a, mpz_t b, OCByte* key, mpz_t hnum);



static void ALProtocolTRDealloc(void* _me){
	ALProtocolTRRef me = (ALProtocolTRRef) _me;
	OCObjectRelease((OCObjectRef*) &(me->djM));
	OCObjectRelease((OCObjectRef*) &(me->djT));
	
	mpz_clear(me->gen_g);
	mpz_clear(me->gen_h);
	mpz_clear(me->x);
}

ALProtocolTRRef ALProtocolTRCreateNewInstance(int modulusBits, int sM){
	OCOBJECT_ALLOCINIT(ALProtocolTR);
	
	if(me == NULL)
		return NULL;
	
	CryptoRandomRand(me->HKey, KLENGTH, CryptoRandomSourceDevURandom);
	
	me->type = PRIVATE;
	
	DJPublicKeyRef pub = NULL;
	DJPrivateKeyRef prv = NULL;
	me->sM = sM;
	
	DJKeyGen(modulusBits, &pub, &prv);
	me->djM = DJCreate(sM, pub, prv, NULL);
	me->djT = DJCreate(1, pub, prv, NULL);
	OCObjectRelease(&pub);
	OCObjectRelease(&prv);
	
	mpz_t r;
	mpz_init(r);
	gmp_randstate_t randState;
	init_rand(randState, DJGetModulusBits(me->djM) / 8 + 1);
	
	// create/get random generator g (gen_g) of the quadratic residues of Z*_N
	mpz_init_set(me->gen_g, *DJGetH(me->djM));
	
	// create the random secret x for the commitment
	mpz_init(me->gen_h);
	mpz_init(me->x);
	mpz_t n;
	n[0]= (*DJGetModulus(me->djM))[0];
	int cmp;
	do
	{
		mpz_urandomb(me->x, randState, DJGetModulusBits(me->djM));
		mpz_powm(me->gen_h, me->gen_g, me->x, *DJGetCipherModulus(me->djM));	
		cmp = mpz_cmp_ui(me->gen_h, 1);
	}while( mpz_cmp(me->x, n) >= 0 || cmp == 0);

	mpz_clear(r);
	gmp_randclear(randState);

	return me;
}


ALProtocolTRRef ALProtocolTRCreateInstanceFromFile(OCStringRef path){
	OCOBJECT_ALLOCINIT(ALProtocolTR);
	
	FILE* fp = fopen(OCStringGetCString(path), "r");
	if(fp == NULL)
		return FAILED;
	
	
	OCDeserializerRef ds = OCDeserializerCreateFromFile(fp);
	
	OCByte buf[3];
	OCDeserializerReadBytes(ds, buf, 2);
	if(memcmp(buf, "TR", 2) != 0)
	{
		OCLog("not a ALProtocolTR file");
		exit(-1);
	}
	
	OCDeserializerReadBytes(ds, buf, 3);
	OCBool isPrivate = NO;
	if(memcmp(buf, "prv", 3) == 0)
		isPrivate = YES;
	else
		isPrivate = NO;
	
	OCDeserializerReadBytes(ds, me->HKey, KLENGTH);
	
	int s = 0;
	OCDeserializerReadInt(ds, &s);
	
	DJPublicKeyRef pub = NULL;
	DJPrivateKeyRef prv = NULL;
	mpz_t h;
	mpz_init(h);
		
	pub =  DJPublicKeyDeserialize(ds);
	DJDeserializeMpz(ds, h);
	if(isPrivate)
		prv = DJPrivateKeyDeserialize(ds);
	me->djM = DJCreate(s, pub, prv, h);
	OCObjectRelease(&pub);
	OCObjectRelease(&prv);
		
	pub =  DJPublicKeyDeserialize(ds);
	DJDeserializeMpz(ds, h);
	if(isPrivate)
		prv = DJPrivateKeyDeserialize(ds);
	me->djT = DJCreate(1, pub, prv, h);
	OCObjectRelease(&pub);		
	OCObjectRelease(&prv);
		
	mpz_init(me->gen_g);
	DJDeserializeMpz(ds, me->gen_g);
	if(isPrivate)
	{
		mpz_init(me->x);
		DJDeserializeMpz(ds, me->x);
	}
	mpz_init(me->gen_h);
	DJDeserializeMpz(ds, me->gen_h);

	me->type = (isPrivate) ? PRIVATE : PUBLIC;

	mpz_clear(h);;	
	fclose(fp);
	
	return me;
}

OCStatus ALProtocolTRWriteInstanceToFile(ALProtocolTRRef me, OCStringRef path, OCBool private){
	
	FILE* fp = fopen(OCStringGetCString(path), "w");
	if(fp == NULL)
		return FAILED;
	
	OCSerializerRef s = OCSerializerCreate();
	
	OCSerializerWriteBytes(s, (OCByte*)"TR", 2);
	if(private)
		OCSerializerWriteBytes(s, (OCByte*)"prv", 3);
	else
		OCSerializerWriteBytes(s, (OCByte*)"pub", 3);
		
	OCSerializerWriteBytes(s, me->HKey, KLENGTH);
	
	OCSerializerWriteInt(s, DJGetS(me->djM));
	
	DJPublicKeySerialize(DJGetPublicKey(me->djM), s);
	DJSerializeMpz(s, *DJGetH(me->djM));

	if(private)
		DJPrivateKeySerialize(DJGetPrivateKey(me->djM), s);
	
	DJPublicKeySerialize(DJGetPublicKey(me->djT), s);
	DJSerializeMpz(s, *DJGetH(me->djT));
	if(private)
		DJPrivateKeySerialize(DJGetPrivateKey(me->djT), s);
	
	DJSerializeMpz(s, me->gen_g);
	if(private)
		DJSerializeMpz(s, me->x);
	DJSerializeMpz(s, me->gen_h);
	
	OCSerializerWriteToStream(s, fp);
	OCObjectRelease(&s);
	
	fclose(fp);
	return SUCCESS;	
}

OCStatus ALProtocolTRWritePrivateInstanceToFile(ALProtocolTRRef me, OCStringRef path){
	return ALProtocolTRWriteInstanceToFile(me, path, YES);
}

OCStatus ALProtocolTRWritePublicInstanceToFile(ALProtocolTRRef me, OCStringRef path){
	return ALProtocolTRWriteInstanceToFile(me, path, NO);
}

size_t ALProtocolTRGetMaxTotalBytes(ALProtocolTRRef me){
	size_t bytesPerS = floor((DJGetModulusBits(me->djM) / 8)) - 1;
	size_t maxTotalBytes = DJGetS(me->djM) * bytesPerS;
	
	return maxTotalBytes;
}

size_t ALProtocolTRGetMaxPayloadBytes(ALProtocolTRRef me){
	return ALProtocolTRGetMaxTotalBytes(me);
}

ALProtocolType ALProtocolTRGetType(ALProtocolTRRef me){
	return me->type;
}

void ALProtocolTRPrintInfo(ALProtocolTRRef alp){
	OCByte buf[2*KLENGTH+1];
	buf[2*KLENGTH] = '\0';
	
	OCBytes2Hex(buf, alp->HKey, KLENGTH);
	OCLog("HKey:  %s", buf);
	
	if(alp->type == PUBLIC)
		OCLog("Type: public");
	else if(alp->type == PRIVATE)
		OCLog("Type: private");
	else
		OCLog("Type: unknown?");
	
	OCLog("djM:");
	DJPrintInfo(alp->djM);
	OCLog("djT");
	DJPrintInfo(alp->djT);
	
	fprintf(stdout, "g: ");
	mpz_out_str(stdout, 16, alp->gen_g);
	fprintf(stdout, "\nh: ");
	mpz_out_str(stdout, 16, alp->gen_h);
	fprintf(stdout, "\n");
}

#pragma mark Tier-2 features

OCStatus ALProtocolTRAggregateChunk(ALProtocolTRRef me, 
									ALChunkTRRef a, ALChunkTRRef withB){
	
	mpz_mul(a->m, a->m, withB->m);
	mpz_mod(a->m, a->m, *DJGetCipherModulus(me->djM));
	mpz_mul(a->t, a->t, withB->t);
	mpz_mod(a->t, a->t, *DJGetCipherModulus(me->djT));
	
	return SUCCESS;
}

size_t ALProtocolTRBucketIndexForChunk(ALProtocolTRRef alp, ALChunkTRRef chunk, size_t nBuckets){
	VAR_UNUSED(alp);
	// mod
	size_t bucket = (size_t) mpz_fdiv_ui(chunk->m, nBuckets);
	OCByte buf[4];
	CryptoRandomRand(buf, 4, CryptoRandomSourceDevURandom);
	bucket = 0;
	bucket |= (buf[0] << 24);
	bucket |= (buf[1] << 16);
	bucket |= (buf[2] << 8);
	bucket |= buf[3];
	bucket %= nBuckets;
	//	bucket *= (bucket < 0) ? -1 : 1;
	
	return bucket;
}

ALChunkTRRef ALProtocolTRCreateNeutralChunk(ALProtocolTRRef alp){
	VAR_UNUSED(alp);
	mpz_t m;
	mpz_t t;
	mpz_init_set_ui(m, 1);
	mpz_init_set_ui(t, 1);
	
	ALChunkTRRef c = ALChunkTRCreate(m, t);
	
	// ALChunkTRCreate makes a deep copy of m and t, so cleanup:
	mpz_clear(m);
	mpz_clear(t);
	
	// this is a "create" method - so the caller owns the chunk
	// so: no Release or Retain here
	return c;
}


OCStatus ALProtocolTRMakeChunkNeutral(ALProtocolTRRef alp, ALChunkTRRef chunk){
	VAR_UNUSED(alp);
	mpz_init_set_ui(chunk->m, 1);
	mpz_init_set_ui(chunk->t, 1);
	
	return SUCCESS;
}


DJRef ALProtocolTRGetDJT(ALProtocolTRRef alp){
	return alp->djT;
}
DJRef ALProtocolTRGetDJM(ALProtocolTRRef alp){
	return alp->djM;
}


#pragma mark algorithms from the paper

ALChunkTRRef ALProtocolTREncData(ALProtocolTRRef me, OCByte* data, size_t nData){
	mpz_t c;
	mpz_init(c);
	mpz_t t;
	mpz_init(t);
	
	size_t maxPayloadBytes = ALProtocolTRGetMaxPayloadBytes(me);
	if(nData > maxPayloadBytes)
	{
		OCLog("nData > maxPayloadBytes");
		exit(-1);
	}
	
	OCByte r0[32];
	if(data != NULL)
		CryptoRandomRand(r0, 32, CryptoRandomSourceDevURandom);
	else
		memset(r0, 0, 32);
	mpz_t numr0;
	mpz_init(numr0);
	mpz_import(numr0, 32, 1, 1, 1, 0, r0);
	
	OCByte r1[128];
	CryptoRandomRand(r1, 128, CryptoRandomSourceDevURandom);
	mpz_t numr1;
	mpz_init(numr1);
	mpz_import(numr1, 128, 1, 1, 1, 0, r1);
	
	OCByte r2[128];
	CryptoRandomRand(r2, 128, CryptoRandomSourceDevURandom);
	mpz_t numr2;
	mpz_init(numr2);
	mpz_import(numr2, 128, 1, 1, 1, 0, r1);
	
	mpz_t m;
	mpz_init(m);
	//HINT: this could be a mistake
	mpz_import(m, nData, 1, 1, 1, 0, data);

	DJEncrypt(me->djM, c, m, numr1);
	
	if(data != NULL)
	{
		mpz_t chk;
		mpz_init(chk);
		helperCreateHashedNum(m, numr0, me->HKey, chk);
		
		mpz_t hchk;
		mpz_init(hchk);
		mpz_powm(hchk, me->gen_h, chk, *DJGetCipherModulus(me->djM));
		mpz_mul(c, c, hchk);
				
		mpz_mod(c, c, *DJGetCipherModulus(me->djM));
	}
		
	mpz_t r0r1num;
	mpz_init_set_ui(r0r1num, 0);
	mpz_add(r0r1num, r0r1num, numr0);
	mpz_mul_2exp(r0r1num, r0r1num, 256+1024);
	mpz_add(r0r1num, r0r1num, numr1);
	
	DJEncrypt(me->djT, t, r0r1num, numr2);
	
	mpz_clear(r0r1num);
	ALChunkTRRef chunk = ALChunkTRCreate(c, t);
	mpz_clear(c);
	mpz_clear(t);
	
	return chunk;
}

ALChunkTRRef ALProtocolTREncNeutral(ALProtocolTRRef me){
	return ALProtocolTREncData(me, NULL, 0);
}

OCStatus ALProtocolTRDecVrfy(ALProtocolTRRef alp, ALChunkTRRef chunk) {

	mpz_t k;
	mpz_init(k);
	mpz_mod(k, chunk->m, *DJGetModulus(alp->djM));
	
	ALProtocolDecrypt(alp, chunk);
	
	mpz_t r0;
	mpz_t r1;
	mpz_init(r0);
	mpz_init(r1);
	
	mpz_div_2exp(r0, chunk->t, 1024+256);
	mpz_mod_2exp(r1, chunk->t, 1024+256);
	
	mpz_out_str(stdout, 16, r0);
	fprintf(stdout, "\n");
	mpz_out_str(stdout, 16, r1);
	fprintf(stdout, "\n");
	
	mpz_t chknum;
	mpz_init(chknum);
	mpz_set_ui(chknum, 0);
	if(mpz_cmp_ui(r0, 0) == 0)
		return SUCCESS;

	helperCreateHashedNum(chunk->m, r0, alp->HKey, chknum);
	
	mpz_t ktest;
	mpz_init(ktest);
	mpz_mul(ktest, alp->x, chknum);
	mpz_add(ktest, ktest, r1);
	mpz_powm(ktest, alp->gen_g, ktest, *DJGetModulus(alp->djM));
	
	if(mpz_cmp(k, ktest) == 0)
	{
		return SUCCESS;
	}
	else
		return FAILED;
}


OCStatus ALProtocolTRTreeDecrypterDecVrfyRest(ALProtocolTRRef me, 
											  ALChunkTRRef chunk, 
											  ALPayloadChunkTRRef* pChunk){
	OCStatus status = FAILED;
	mpz_t k;
	mpz_init(k);
	mpz_mod(k, chunk->m, *DJGetModulus(me->djM));
	
	{
		mpz_t m;
		mpz_init(m);
		DJDecrypt(me->djM, m, chunk->m);
		mpz_set(chunk->m, m);
		mpz_clear(m);
	}	
	// t has already been decrypted in TD
	
	mpz_t r0;
	mpz_t r1;
	mpz_init(r0);
	mpz_init(r1);
	
	mpz_div_2exp(r0, chunk->t, 1024+256);
	mpz_mod_2exp(r1, chunk->t, 1024+256);
	
	mpz_t chknum;
	mpz_init(chknum);
	mpz_set_ui(chknum, 0);
	if(mpz_cmp_ui(r0, 0) == 0)
	{
		*pChunk = NULL;
		return SUCCESS;
	}
	helperCreateHashedNum(chunk->m, r0, me->HKey, chknum);
	
	mpz_t ktest;
	mpz_init(ktest);
	mpz_mul(ktest, me->x, chknum);
	mpz_add(ktest, ktest, r1);
//	mpz_powm(ktest, *DJGetH(me->djM), ktest, *DJGetModulus(me->djM));
	mpz_powm(ktest, me->gen_g, ktest, *DJGetModulus(me->djM));
		
	if(mpz_cmp(k, ktest) == 0)
	{
		OCByte bytes_m[10000];	
		OCByte bytes_r0[10000];	
		
		size_t mLength = 0;
		mpz_export(bytes_m, &mLength, 1, 1, 1, 0, chunk->m);
		size_t r0Length = 0;
		mpz_export(bytes_r0, &r0Length, 1, 1, 1, 0, r0);
		ALPayloadChunkTRRef c = ALPayloadChunkTRCreate(bytes_m, mLength, bytes_r0, r0Length);
		*pChunk = c;
		status = SUCCESS;		
	}
	else
	{
		*pChunk = NULL;
		status = FAILED;
	}
	return status;
}

#pragma mark tree decryption functions

OCBool ALProtocolTRTDIsNeutralChunk(ALProtocolTRRef alp, ALChunkTRRef chunk){
	VAR_UNUSED(alp);
	if(mpz_cmp_ui(chunk->t, 0) == 0)
		return YES;
	else
		return NO;
}

void ALProtocolTRTDDecTestNeutral(ALProtocolTRRef alp, ALChunkTRRef chunk){
	VAR_UNUSED(alp);
	mpz_t t;
	mpz_init(t);
	
	DJDecrypt(alp->djT, t, chunk->t);
	mpz_set(chunk->t, t);
	mpz_clear(t);	
}

TDItemPtr ALProtocolTRGetTDItemForChunk(ALProtocolTRRef me, ALChunkTRRef chunk){
	VAR_UNUSED(me);
	return chunk->t;
}

void ALProtocolTRTreeDecrypterPostprocess(ALProtocolTRRef alp, ALChunkTRRef chunk, TDItemPtr item){
	VAR_UNUSED(alp);
	mpz_set(chunk->t, (mpz_ptr)item);
}

#pragma mark tree decryption delegate

static TDItemPtr createEncryptedNeutral(OCObjectRef delegate){
	VAR_UNUSED(delegate);
	mpz_ptr item = OCAllocate(ocDefaultAllocator, sizeof(mpz_ptr));
	mpz_init_set_ui(item, 1);
	return item;
}

static TDItemPtr createDecryptedNeutral(OCObjectRef delegate){
	VAR_UNUSED(delegate);
	mpz_ptr item = OCAllocate(ocDefaultAllocator, sizeof(mpz_ptr));
	mpz_init_set_ui(item, 0);
	return item;	
}

static OCBool isDecryptedNeutral(OCObjectRef delegate, TDItemPtr item){
	VAR_UNUSED(delegate);
	
	if(mpz_sizeinbase((mpz_ptr)item, 2) < 1024+256-1)
		//	if(mpz_cmp_ui((mpz_ptr) item, 0) == 0)
		return YES;
	else
		return NO;
}

static void combine(OCObjectRef delegate, TDItemPtr to, TDItemPtr a, TDItemPtr b){
	ALProtocolTRRef me = (ALProtocolTRRef)delegate;
	DJMult(me->djT, to, a, b);
}

static void uncombine(OCObjectRef delegate, TDItemPtr to, TDItemPtr a, TDItemPtr b){
	ALProtocolTRRef me = (ALProtocolTRRef)delegate;
	mpz_sub((mpz_ptr)to, (mpz_ptr)a, (mpz_ptr)b);
	mpz_mod((mpz_ptr)to, (mpz_ptr)to, *DJGetModulus(me->djM));
}

static void decrypt(OCObjectRef delegate, TDItemPtr to, TDItemPtr from){
	ALProtocolTRRef me = (ALProtocolTRRef)delegate;
	DJDecrypt(me->djT, (mpz_ptr)to, (mpz_ptr)from);
}

static void setItem(OCObjectRef delegate, TDItemPtr to, TDItemPtr from){
	VAR_UNUSED(delegate);
	mpz_set((mpz_ptr)to, (mpz_ptr)from);
}

static void deleteItem(OCObjectRef delegate, TDItemPtr item){
	VAR_UNUSED(delegate);
	mpz_clear((mpz_ptr)item);
	OCDeallocate(ocDefaultAllocator, item);
}

TreeDecrypterRef ALProtocolTRCreateTreeDecrypter(ALProtocolTRRef me, int nLeaves){
	struct TDDelegate delegate;
	delegate.delegateObject = (OCObjectRef)me;
	delegate.createEncryptedNeutral = createEncryptedNeutral;
	delegate.createDecryptedNeutral = createDecryptedNeutral;
	delegate.isDecryptedNeutral = isDecryptedNeutral;
	delegate.combine = combine;
	delegate.uncombine = uncombine;
	delegate.decrypt = decrypt;
	delegate.setItem = setItem;
	delegate.deleteItem = deleteItem;
	
	TreeDecrypterRef td = TreeDecrypterCreate(nLeaves, delegate);
	return td;
}

#pragma mark helpers
static void helperCreateHashedNum(mpz_t a, mpz_t b, OCByte* key, mpz_t hnum){
	size_t nA = ceil(mpz_sizeinbase(a, 2) / 8.);
	size_t nB = ceil(mpz_sizeinbase(b, 2) / 8.);
	size_t n = nA + nB;
	OCByte* tmp = NULL;
	tmp = OCAllocate(ocDefaultAllocator, n);
	size_t nWritten = 0;
	mpz_export(tmp, &nWritten, 1, 1, 1, 0, a);
	if(nWritten != nA) 
		OCLog("error here");
	mpz_export(tmp+nA, &nWritten, 1, 1, 1, 0, b);
	if(nWritten != nB) 
		OCLog("error here");
	
	OCByte hbytes[KLENGTH];
	CryptoMacDoMAC(key, KLENGTH, tmp, n, hbytes);
	mpz_import(hnum, KLENGTH, 1, 1, 1, 0, hbytes);
	OCDeallocate(ocDefaultAllocator, tmp);
	return;
}

OCBool ALProtocolTRIsChunkValid(ALProtocolTRRef me, ALChunkTRRef chunk){
	if(mpz_cmp(chunk->m, *DJGetCipherModulus(me->djM)) >= 0)
		return NO;
	if(mpz_cmp(chunk->t, *DJGetCipherModulus(me->djT)) >= 0)
		return NO;
	   
	return YES;
}

OCStatus ALProtocolTRDecrypt(ALProtocolTRRef me, ALChunkTRRef chunk){

	DJDecrypt(me->djM, chunk->m, chunk->m);
	DJDecrypt(me->djT, chunk->t, chunk->t);
	
	return SUCCESS;
}


