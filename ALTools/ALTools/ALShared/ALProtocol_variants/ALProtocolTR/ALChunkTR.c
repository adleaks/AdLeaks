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

#include "ALChunkTR.h"

#include <string.h>
#include "ALChunkTR_private.h"

#include "../../libs/libb64/cdecode.h"

void ALChunkTRDealloc(void* ptr){
	ALChunkTRRef me = (ALChunkTRRef) ptr;
	mpz_clear(me->m);
	mpz_clear(me->t);
}

ALChunkTRRef ALChunkTRCreateEmpty(void){
	OCOBJECT_ALLOCINIT(ALChunkTR);
	mpz_init(me->m);
	mpz_init(me->t);

	return me;
}


ALChunkTRRef ALChunkTRCreate(mpz_t m, mpz_t t){
	OCOBJECT_ALLOCINIT(ALChunkTR);
	
	mpz_init_set(me->m, m);
	mpz_init_set(me->t, t);
	
	return me;
}

OCStatus ALChunkTRSerialize(ALChunkTRRef me, OCSerializerRef s){
	size_t mCount = 0;
	size_t tCount = 0;
	
	OCByte* m = mpz_export(NULL, &mCount, 1, 1, 1, 0, me->m);
	OCByte* t = mpz_export(NULL, &tCount, 1, 1, 1, 0, me->t);

	OCSerializerWriteInt(s, mCount);
	OCSerializerWriteBytes(s, m, mCount);
	OCSerializerWriteInt(s, tCount);
	OCSerializerWriteBytes(s, t, tCount);

	void (*freefunc) (void *, size_t);
	
	mp_get_memory_functions (NULL, NULL, &freefunc);
	freefunc(m, mCount);
	freefunc(t, tCount);
	
	return SUCCESS;
}

OCStatus ALChunkTRDeserialize(OCDeserializerRef ds, ALChunkTRRef* chunk){
	OCStatus status = SUCCESS;
	int sizeA;
	int sizeB;
	OCByte* cA = NULL;
	OCByte* cB = NULL;
	
	do
	{
		status &= OCDeserializerReadInt(ds, &sizeA);
		cA = OCAllocate(ocDefaultAllocator, sizeA+1);
		if(cA == NULL)
		{
			status = FAILED;
			break;
		}
		status &= OCDeserializerReadBytes(ds, cA, sizeA);
		cA[sizeA] = '\0';
	
		if(status == FAILED)
			break;
			
		status &= OCDeserializerReadInt(ds, &sizeB);
		cB = OCAllocate(ocDefaultAllocator, sizeB+1);
		if(cB == NULL)
		{
			status = FAILED;
			break;
		}
		status &= OCDeserializerReadBytes(ds, cB, sizeB);
		cB[sizeB] = '\0';
	
	}while(0);
	if(cA)
		OCDeallocate(ocDefaultAllocator, cA);
	if(cB)
		OCDeallocate(ocDefaultAllocator, cB);
	if(status == FAILED)
		return FAILED;
	
	OCOBJECT_ALLOCINIT(ALChunkTR);
	mpz_init(me->m);
	mpz_init(me->t);
	mpz_import(me->m, sizeA, 1, 1, 1, 0, cA);
	mpz_import(me->t, sizeB, 1, 1, 1, 0, cB);

	OCDeallocate(ocDefaultAllocator, cA);
	OCDeallocate(ocDefaultAllocator, cB);
	*chunk = me;
	
	return SUCCESS;
}

void ALChunkTRPrint(ALChunkTRRef me){
	printf("(");
	mpz_out_str(stdout, 16, me->m);
	printf(", ");
	mpz_out_str(stdout, 16, me->t);
	printf(")\n");
}

static OCStatus decodeNum(OCStringRef str, mpz_t* to){

	base64_decodestate ds;
	base64_init_decodestate(&ds);
	
	size_t valSize = OCStringGetLength(str);
	if( valSize % 4 != 0 ) // malformed
		return FAILED;
	
	size_t expectedDecoded = 3*(valSize/4);
	char* theVal = OCAllocate(ocDefaultAllocator, expectedDecoded);
	
	size_t nDecoded = base64_decode_block(OCStringGetCString(str), valSize, theVal, &ds);
	if(nDecoded <= 0)
	{
		OCDeallocate(ocDefaultAllocator, theVal);
		return FAILED;
	}
	else
	{
		mpz_import(*to, nDecoded, 1, 1, 1, 0, theVal);
		return SUCCESS;
	}
}

ALChunkTRRef ALChunkTRCreateFromBase64(OCStringRef base64Encoded){
	OCListRef parts = OCStringSplitByToken(base64Encoded, ';');
	ALChunkTRRef chunk = ALChunkTRCreateEmpty();
	OCStatus status = FAILED;
	
	do
	{
		if(parts == NULL)
			break;
		if(OCListGetCount(parts) != 2)
			break;
	
		OCStringRef strM = (OCStringRef) OCListGetItem(parts, 0);
		OCStringRef strT = (OCStringRef) OCListGetItem(parts, 1);
	
		if(decodeNum(strM, &chunk->m) != SUCCESS)
			break;
		if(decodeNum(strT, &chunk->t) != SUCCESS)
			break;
		
		status = SUCCESS;
	}while(0);

	OCObjectRelease(&parts);
	
	if(status == FAILED)
	{
		OCObjectRelease(&chunk);
		return NULL;
	}
	return chunk;
}


