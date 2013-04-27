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

#include "AggregationQueue.h"
#include "OCBase.h"
#include "OCList.h"
#include "OCTime.h"
#include "OCThread.h"
#include "ALProtocol.h"

#include <unistd.h>

struct AggregationQueue{
	struct OCObject _obj;
	
	ALProtocolRef alp;
	
	OCListRef newChunks;
	OCListRef toAggregateChunks;
	
	OCThreadRef thread;

	ALChunkRef* destChunks;
	OCBool* touchedChunks;
	size_t nDestChunks;
	
	OCTimeInterval sendTimeout;
	AggregationQueueDelegate delegate;
	void* cbParam;
	OCBool finished;
	
	OCTimeInterval lastT;
	unsigned int counter;
	unsigned int dropCounter;
	unsigned int dropLimit;
	
	OCTimeInterval lastRun;
};

#pragma mark private declarations
// aggregation thread
static void* AggregationQueueAggregateMain(AggregationQueueRef me);

#pragma mark implementation

void AggregationQueueDealloc(void* ptr){
	AggregationQueueRef me = (AggregationQueueRef)ptr;
	
	OCRelease(&me->newChunks);
	OCRelease(&me->toAggregateChunks);
	OCRelease(&me->alp);
	
	size_t i = 0;
	for(i = 0; i < me->nDestChunks; i++)
		OCObjectRelease(&me->destChunks[i]);
	OCDeallocate(ocDefaultAllocator, me->destChunks);
	OCDeallocate(ocDefaultAllocator, me->touchedChunks);
}

AggregationQueueRef AggregationQueueCreate(ALProtocolRef alp,
										   size_t nDestChunks,
										   size_t dropLimit,
										   OCTimeInterval sendTimeout,
										   AggregationQueueDelegate delegate){
	OCOBJECT_ALLOCINIT(AggregationQueue);
	VAR_UNUSED(alp);
	
	me->alp = (ALProtocolRef)OCObjectRetain((OCObjectRef)alp);
	me->nDestChunks = nDestChunks;
	me->destChunks = OCAllocate(ocDefaultAllocator, nDestChunks * sizeof(ALChunkRef));
	me->touchedChunks = OCAllocate(ocDefaultAllocator, nDestChunks * sizeof(OCBool));
	size_t i = 0;
	for(i = 0; i < nDestChunks; i++)
	{
		me->destChunks[i] = ALProtocolCreateNeutralChunk(alp);
		me->touchedChunks[i] = NO;
	}
	me->sendTimeout = sendTimeout;
	me->finished = NO;
	me->delegate = delegate;
	me->lastT = OCTimeIntervalSinceReferenceDate();
	me->counter = 0;
	
	me->dropLimit = dropLimit;
	
	me->dropCounter = 0;
	me->newChunks = OCListCreate();
	me->toAggregateChunks = OCListCreate();
	
	OCThreadDelegate del;
	del.delegateObject = (OCObjectRef) me;
	del.exitCB = NULL;
	del.mainCB = (OCThreadMainCB)AggregationQueueAggregateMain;
	del.stopCB = NULL;
	
	me->thread = OCThreadCreate(del);
	OCThreadStart(me->thread);
	
	return me;
}

OCStatus AggregationQueueEnqueueChunks(AggregationQueueRef me, OCListRef chunks){
	
	OCObjectLock((OCObjectRef)me->newChunks);
//	if(OCListGetCount(me->newChunks) > me->dropLimit)
//		me->dropCounter++;
//	else
//	{
	size_t newCounter = OCListGetCount(chunks);
	OCListAppendList(me->newChunks, chunks);
//		OCListFifoPush(me->newChunks, (OCObjectRef)chunk);
	me->counter+=newCounter;
//	}
	OCObjectUnlock((OCObjectRef)me->newChunks);
	
	OCTimeInterval now = OCTimeIntervalSinceReferenceDate();
	if(now-me->lastT > 1)
	{
//		OCLog("%d received (%d dropped)  in the last %fs", me->counter, me->dropCounter, now-me->lastT);
		OCLog("AQ: %d received in the last %fs", me->counter, me->dropCounter, now-me->lastT);
		me->lastT = now;
//		me->dropCounter = 0;
		me->counter = 0;
	}
	
	return SUCCESS;
}

OCStatus AggregationQueueEnqueue(AggregationQueueRef me, ALChunkRef chunk){
	
	OCObjectLock((OCObjectRef)me->newChunks);
	if(OCListGetCount(me->newChunks) > me->dropLimit)
		me->dropCounter++;
	else
	{
		OCListFifoPush(me->newChunks, (OCObjectRef)chunk);
		me->counter++;
	}
	OCObjectUnlock((OCObjectRef)me->newChunks);

	OCTimeInterval now = OCTimeIntervalSinceReferenceDate();
	if(now-me->lastT > 1)
	{
		OCLog("%d received (%d dropped)  in the last %fs", me->counter, me->dropCounter, now-me->lastT);
		me->lastT = now;
		me->dropCounter = 0;
		me->counter = 0;
	}

	return SUCCESS;
}

#pragma mark aggregation queue

void* AggregationQueueAggregateMain(AggregationQueueRef me){
	me->lastRun = OCTimeIntervalSinceReferenceDate();
	do 
	{
		OCTimeInterval now = OCTimeIntervalSinceReferenceDate();
		if(now - me->lastRun < 1)
		{
			OCTimeInterval toWait = 1-(now - me->lastRun);
			if(toWait > 0.05)
				OCThreadSleep(me->thread, toWait);
		}		
		now = OCTimeIntervalSinceReferenceDate();
		me->lastRun = now;
		OCObjectLock((OCObjectRef)me->newChunks);
		if(OCListGetCount(me->newChunks) == 0)
		{
			OCObjectUnlock((OCObjectRef)me->newChunks);
			continue;
		}
		OCListRef tmp = me->newChunks;
		me->newChunks = me->toAggregateChunks;
		me->toAggregateChunks = tmp;		
		OCObjectUnlock((OCObjectRef)tmp/*me->newChunks*/);

		size_t nToAggregate = OCListGetCount(me->toAggregateChunks);
		OCTimeInterval begin = OCTimeIntervalSinceReferenceDate();
		
		OCListIteratorRef iter = OCListCreateIterator(me->toAggregateChunks);		
		ALChunkRef toAggregate = NULL;
		while( (toAggregate=(ALChunkRef)OCListIteratorNext(iter)) != NULL)
		{
			size_t chunkDest = ALProtocolBucketIndexForChunk(me->alp, toAggregate, me->nDestChunks);

			ALChunkRef aggregateWith = me->destChunks[chunkDest];
			ALProtocolAggregateChunk(me->alp, aggregateWith, toAggregate);
			
			me->touchedChunks[chunkDest] = YES;
		}
		OCRelease(&iter);
		
		OCTimeInterval begincb = OCTimeIntervalSinceReferenceDate();
		OCListRef send = OCListCreate();
		size_t i = 0;
		for(i=0; i < me->nDestChunks; i++)
		{
			if(me->touchedChunks[i] == YES)
			{
				OCListAppend(send, (OCObjectRef)me->destChunks[i]);
				OCRelease(&me->destChunks[i]);
				me->destChunks[i] = ALProtocolCreateNeutralChunk(me->alp);
				me->touchedChunks[i] = NO;
			}
		}
		me->delegate.sendCB(me->delegate.delegateObject, send);
		OCRelease(&send);
		OCRelease(&me->toAggregateChunks);
		me->toAggregateChunks = OCListCreate();		
		
		OCTimeInterval end = OCTimeIntervalSinceReferenceDate();
		OCLog("Aggregation of %d chunks took me %fs (CB: %fs)", nToAggregate, end-begin, end-begincb);
	}while(! me->finished);
	
	return NULL;
}

void AggregationQueueStop(AggregationQueueRef me){
	do
	{
		OCObjectLock((OCObjectRef)me->newChunks);
	
		// wait till the list is empty
		if(OCListGetCount(me->newChunks) > 0)
		{
			OCObjectUnlock((OCObjectRef)me->newChunks);
			OCTimeSleep(1);
			continue;
		}
		else
		{
			OCObjectUnlock((OCObjectRef)me->newChunks);
			// wait till the rest is done... 1+2*sendTimeout should be enough
			OCTimeSleep(1+2*me->sendTimeout);
			me->finished = YES;
			break;
		}	
	}while(1);
}


