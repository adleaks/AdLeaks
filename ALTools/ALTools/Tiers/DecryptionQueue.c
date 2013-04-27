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

#include <pthread.h>
#include <unistd.h>

#include "DecryptionQueue.h"
#include "OCObject.h"
#include "OCList.h"
#include "ALChunk.h"
#include "TreeDecrypter.h"
#include "OCThread.h"
#include "ALPayloadChunk.h"

struct DecryptionQueue{
	struct OCObject _obj;
	
	OCListRef enqueuedChunks;
	pthread_mutex_t enqueuedChunksMutex;
	OCListRef toDecryptChunks;

	OCThreadRef thread;
	
	ALProtocolRef alp;
	
	DecryptionQueueDelegate delegate;
	OCBool doQuitGracefully;
};

void DecryptionQueueDealloc(void* ptr){
	DecryptionQueueRef me = (DecryptionQueueRef) ptr;
	OCObjectRelease(&me->enqueuedChunks);
	OCObjectRelease(&me->alp);
	OCRelease(&me->thread);
}

static void DecryptionQueueMain(DecryptionQueueRef me);

DecryptionQueueRef DecryptionQueueCreate(ALProtocolRef alp, DecryptionQueueDelegate delegate){
	OCOBJECT_ALLOCINIT(DecryptionQueue);
	me->enqueuedChunks = OCListCreate();
	me->toDecryptChunks = OCListCreate();
	me->alp = (ALProtocolRef)OCObjectRetain((OCObjectRef)alp);
	me->delegate = delegate;

	OCThreadDelegate del;
	del.delegateObject = (OCObjectRef)me;
	del.exitCB = NULL;
	del.mainCB = (OCThreadMainCB)DecryptionQueueMain;
	del.stopCB = NULL;
	me->thread = OCThreadCreate(del);
	pthread_mutex_init(&me->enqueuedChunksMutex, NULL);
	me->doQuitGracefully = NO;
	
	return me;
}

void DecryptionQueueRun(DecryptionQueueRef me){
	OCThreadStart(me->thread);
}

OCStatus DecryptionQueueStop(DecryptionQueueRef me){
	me->doQuitGracefully = YES;
	return SUCCESS;
}

OCStatus DecryptionQueueWaitTillStopped(DecryptionQueueRef me){
	return OCThreadJoin(me->thread);
}


OCStatus DecryptionQueueEnqueue(DecryptionQueueRef me, ALChunkRef chunk){
	pthread_mutex_lock(&me->enqueuedChunksMutex);
	OCObjectRetain((OCObjectRef)chunk);
	OCListFifoPush(me->enqueuedChunks, (OCObjectRef)chunk);
	pthread_mutex_unlock(&me->enqueuedChunksMutex);	
	
	return SUCCESS;
}

static void DecryptionQueueMain(DecryptionQueueRef me){
	do
	{		
		if(me->doQuitGracefully)
		{
			OCObjectLock((OCObjectRef)me->enqueuedChunks);
			if(OCListGetCount(me->enqueuedChunks) == 0 &&
			   OCListGetCount(me->toDecryptChunks) == 0)
			{
				OCObjectUnlock((OCObjectRef)me->enqueuedChunks);
				break;
			}
			OCObjectUnlock((OCObjectRef)me->enqueuedChunks);
		}
		
		OCObjectLock((OCObjectRef)me->enqueuedChunks);
		if(OCListGetCount(me->enqueuedChunks) == 0)
		{
			OCObjectUnlock((OCObjectRef)me->enqueuedChunks);
			OCThreadSleep(me->thread, 1);
			continue;
		}
		OCListRef tmp = me->enqueuedChunks;
		me->enqueuedChunks = me->toDecryptChunks;
		me->toDecryptChunks = tmp;		

		OCObjectUnlock((OCObjectRef)me->enqueuedChunks);
		size_t nToDecrypt = OCListGetCount(me->toDecryptChunks);
		
		TreeDecrypterRef td = ALProtocolCreateTreeDecrypter(me->alp, nToDecrypt);
		size_t i = 0;
		OCListIteratorRef iter = OCListCreateIterator(me->toDecryptChunks);
		ALChunkRef chunk = NULL;
		for(i=0; (chunk=(ALChunkRef)OCListIteratorNext(iter)) != NULL; i++)
		{
			TDItemPtr item = ALProtocolGetTDItemForChunk(me->alp, chunk);
			TreeDecrypterSetLeaf(td, i, item);
		}
		OCObjectRelease(&iter);

		TreeDecrypterPopulateTree(td);
		int nDecryptions = TreeDecrypterDoAlgorithm(td);
		int nZero = 0;
		int nNonZeroOk = 0;
		int nNonZeroFailed = 0;
		iter = OCListCreateIterator(me->toDecryptChunks);

		for(i=0; (chunk=(ALChunkRef)OCListIteratorNext(iter)) != NULL; i++)
		{
			if(TreeDecrypterIsLeafZero(td, i) == YES)
			{
				nZero++;
			}
			else
			{
				OCByte* data = NULL;
				TDItemPtr item = TreeDecrypterGetLeaf(td, i);
				ALChunkRef c = chunk;//me->toDecrypt[i];
				ALProtocolTreeDecrypterPostprocess(me->alp, c, item);
				ALPayloadChunkRef plC;

				OCStatus ok = ALProtocolTreeDecrypterDecVrfyRest(me->alp, c, &plC);
				if(ok)
				{
					nNonZeroOk++;
					OCDeallocate(ocDefaultAllocator, data);
					me->delegate.decryptedCB(me->delegate.delegate, plC);
				}
				else
				{
					nNonZeroFailed++;
				}
			}
			OCRelease(&chunk);
		}
		OCRelease(&iter);
		OCRelease(&me->toDecryptChunks);
		me->toDecryptChunks = OCListCreate();
		OCLog("##Log %d - #%d: %d(0), %d(ok), %d(failed)", nToDecrypt, nDecryptions, nZero, nNonZeroOk, nNonZeroFailed);
		fflush(stdout);

		OCObjectRelease(&td);
		
	}while(1);
	
	return;
}

