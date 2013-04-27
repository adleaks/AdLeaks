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

#include "OCBase.h"
#include "OCLog.h"
#include "OCTypes.h"
#include "TierMessage.h"
#include "TierServer.h"
#include "TierClient.h"
#include <unistd.h>
#include <signal.h>
#include "ALProtocol.h"
#include "ALChunk.h"
#include "OCTime.h"
#include "OCConfig.h"

#include "OCDictionary.h"
#include "OCValue.h"
#include "TierConfig.h"
#include "Tier2.h"
#include "Tier.h"

#include "AggregationQueue.h"
#include "CryptoMac.h"

struct Tier2{
	struct OCObject _obj;
	
	TierConfigRef config;
	
	OCListRef aqs; // AggregationQueues
	
	ALProtocolRef alp;
	
	TierServerRef tserver;
	TierClientRef tclient;

	size_t nReceived;
	OCTimeInterval last;
};

// private
static void Tier2MessageReceived(Tier2Ref obj, TierMessagePtr message);
static void Tier2AggregationCallback(Tier2Ref obj, OCListRef chunks);

void Tier2Dealloc(void* _me){
	Tier2Ref me = (Tier2Ref) _me;

	OCRelease(&me->config);
	OCRelease(&me->aqs);
	OCRelease(&me->alp);
	OCRelease(&me->tserver);
	OCRelease(&me->tclient);
}

Tier2Ref Tier2Create(TierConfigRef config, ALProtocolRef alp){
	OCOBJECT_ALLOCINIT(Tier2);
	if(me == NULL)
		return me;
	
	me->config = OCRetain(config);
	me->nReceived = 0;
	me->last = OCTimeIntervalSinceReferenceDate();
	me->alp = OCRetain(alp);
	me->aqs = OCListCreate();
	
	OCStatus success = FAILED;
	OCAutoreleasePoolCreate();
	do
	{
		OCStringRef queues = TierConfigGetServiceValue(me->config, OC("queues"));
		size_t nQueues = 0;
		OCStringRef chunkLimitPerQueue = TierConfigGetServiceValue(me->config, OC("chunkLimitPerQueue"));
		size_t nChunkLimitPerQueue = 0;
		OCStringRef bucketsPerQueue = TierConfigGetServiceValue(me->config, OC("bucketsPerQueue"));
		size_t nBucketsPerQueue = 0;
		
		if(queues == NULL)
		{
			OCLog("Tier2 requires a <queues> parameter in the config");
			break;
		}
		if(chunkLimitPerQueue == NULL)
		{
			OCLog("Tier2 requires a <chunkLimit> parameter in the config");
			break;
		}
		if(bucketsPerQueue == NULL)
		{
			OCLog("Tier2 requires a <chunkLimit> parameter in the config");
			break;
		}
		nQueues = atoi(CO(queues));
		nChunkLimitPerQueue = atoi(CO(chunkLimitPerQueue));
		nBucketsPerQueue = atoi(CO(bucketsPerQueue));
		
		size_t i = 0;
		for(i=0; i < nQueues; i++)
		{
			
			OCTimeInterval timeout = 1;
			AggregationQueueDelegate sqdel;
			sqdel.delegateObject = (OCObjectRef)me;
			sqdel.sendCB = (AggregationQueueSendCB)Tier2AggregationCallback;
			AggregationQueueRef aq = AggregationQueueCreate(alp, nBucketsPerQueue, 0, timeout, sqdel);
			OCListAppend(me->aqs, (OCObjectRef)aq);			
			OCRelease(&aq);
		}
		
		me->tclient = TierClientCreate();
		success = SUCCESS;
	}while(0);
	OCAutoreleasePoolDestroy();
	if(!success)
	{
		OCLogError("Could not start Tier-2");
		exit(-1);
	}
	return me;
}


int Tier2Main(Tier2Ref me){

	size_t i = 0;
	for(i = 0; i < TierConfigGetNumberOfDestinations(me->config); i++)
	{
		struct TierConfigDestination dest;
		TierConfigGetDestination(me->config, i, &dest);
		OCLog("Adding destination [%s]: %s:%d (type %d)", 
				CO(dest.label), CO(dest.host), dest.port, dest.type);
		TierClientAddServer(me->tclient, dest.host, dest.port);
	}		
	TierClientStart(me->tclient);
	
	struct TierConfigService serv;
	TierConfigGetService(me->config, &serv);
	OCLog("Starting Service: [%s]:%d (type %d)", CO(serv.label), serv.port, serv.type);
		
	TierServerDelegate del;
	del.delegateObject = (OCObjectRef)me;
	del.messageReceived = (TierServerReceivedCallback)Tier2MessageReceived;
	TierServerRef ts = TierServerCreateWithPortNo(serv.port, del);
	TierServerStart(ts);	
	
	while(tier_gotSignal == 0)
		sleep(1);

	OCLog("Trying to exit gracefully...");
	TierServerStop(ts);
	OCLog("Waiting till service has stopped...");
	TierServerWaitTillStopped(ts);	
	
	
	OCListIteratorRef iter = OCListCreateIterator(me->aqs);
	AggregationQueueRef aq = NULL;
	while( (aq=(AggregationQueueRef)OCListIteratorNext(iter)) != NULL)
		AggregationQueueStop(aq);
	
	
	TierClientStop(me->tclient);
	OCLog("Waiting till clients have stopped...");
	TierClientWaitTillStopped(me->tclient);
	
	return 0;
}

static OCByte h[32];
static OCByte tkey[32] = {0xAC, 0xC6, 0x40, 0xB4, 0x8F, 0xB3, 0xB7, 0x4D, 0xC5, 0x94, 0xCE, 0xA2, 0xDD, 0x94, 0xCB, 0xC6, 0xEF, 0x7D, 0xCB, 0x76, 0xE2, 0xF1, 0x9B, 0x2D, 0x86, 0x52, 0xB1, 0x21, 0x2A, 0xCD, 0x81, 0x56};
// TODO: put these in the config file

static void Tier2MessageReceived(Tier2Ref me, TierMessagePtr message){
	OCTimeInterval now = OCTimeIntervalSinceReferenceDate();
	if(me->last == 0)
		me->last = now;
	me->nReceived++;
	if(now - me->last > 1)
	{
		me->last = now;
		me->nReceived = 0;
	}
	if(message->header.messageType != TMCHUNK)
		return;
	
	OCDeserializerRef ds = OCDeserializerCreateWithBytes(message->data, message->header.dataLength); 
	tkey[0]++;
	CryptoMacDoMAC(tkey, 32, message->data, message->header.dataLength, h);
	ALChunkRef chunk = NULL;
	size_t i = me->nReceived % OCListGetCount(me->aqs);
	if(ALChunkDeserialize(ds, &chunk) == SUCCESS)
	{
		AggregationQueueRef aq = (AggregationQueueRef)OCListGetItem(me->aqs, i);
		AggregationQueueEnqueue(aq, chunk);
	}
	if(chunk)
		OCRelease(&chunk);
	OCRelease(&ds);
}

static void Tier2AggregationCallback(Tier2Ref me, OCListRef chunks){
	//size_t nServers = TierClientGetNumberOfServer(me->tclient);
	//ALProtocolBucketIndexForChunk(me->alp, chunk, nServers);
	//TODO: distribute chunks uniformly
	size_t i = 0; 
	TierClientSendChunks(me->tclient, chunks, i);
}

