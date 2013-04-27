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

#include "Tier3.h"

#include "OCBase.h"
#include "ALProtocol.h"
#include "DecryptionQueue.h"
#include "TierConfig.h"
#include "TierServer.h"
#include <unistd.h>
#include "OCTime.h"
#include "ALPayloadChunk.h"
#include "Tier3Storage.h"
#include "Tier.h"

struct Tier3{
	struct OCObject _obj;
	
	TierConfigRef config;
	
	DecryptionQueueRef deq;
	ALProtocolRef alp;
	
	TierServerRef tserver;
	
	Tier3StorageRef storage;
	
	size_t nReceived;
	OCTimeInterval last;
};

// private
static void Tier3MessageReceived(Tier3Ref me, TierMessagePtr message);
static void Tier3DecryptedCallback(Tier3Ref me, ALPayloadChunkRef plc);

void Tier3Dealloc(void* _me){
	Tier3Ref me = (Tier3Ref) _me;

	OCRelease(&me->config);
	OCRelease(&me->deq);
	OCRelease(&me->alp);
	OCRelease(&me->tserver);
	OCRelease(&me->storage);
}


Tier3Ref Tier3Create(TierConfigRef config, ALProtocolRef alp){
	OCOBJECT_ALLOCINIT(Tier3);
	if(me == NULL)
		return me;
	
	me->config = OCRetain(config);
	me->nReceived = 0;
	me->last = OCTimeIntervalSinceReferenceDate();
	me->alp = OCRetain(alp);
	DecryptionQueueDelegate del;
	del.delegate = (OCObjectRef)me;
	del.decryptedCB = (DecryptionQueueDecryptedCB)Tier3DecryptedCallback;
	me->deq = DecryptionQueueCreate(alp, del);
	
	//TODO: put this in the config file
	me->storage = Tier3StorageCreate(OCStringCreateWithCString("/Adleaks/storage/"));
	
	return me;
}

int Tier3Main(Tier3Ref me){
	struct TierConfigService serv;
	TierConfigGetService(me->config, &serv);
	OCLog("Starting Service: [%s]:%d (type %d)", CO(serv.label), serv.port, serv.type);
	
	DecryptionQueueRun(me->deq);

	TierServerDelegate del;
	del.delegateObject = (OCObjectRef)me;
	del.messageReceived = (TierServerReceivedCallback)Tier3MessageReceived;
	me->tserver = TierServerCreateWithPortNo(serv.port, del);
	TierServerStart(me->tserver);	
	
	Tier3StorageStart(me->storage);
	
	while(tier_gotSignal == 0)
		sleep(1);
	OCLog("Trying to exit gracefully...");
	
	TierServerStop(me->tserver);
	OCLog("Waiting till service has stopped...");
	TierServerWaitTillStopped(me->tserver);	
	
	OCLog("Waiting till the decryption has finished");
	//AggregationQueueStop(me->aq);
		
	OCLog("Waiting till the storage has finished");
	Tier3StorageStop(me->storage);
	Tier3StorageWaitTillStopped(me->storage);
	
	return 0;
}

static void Tier3MessageReceived(Tier3Ref me, TierMessagePtr message){
	OCTimeInterval now = OCTimeIntervalSinceReferenceDate();
	if(me->last == 0)
		me->last = now;
	me->nReceived++;
	if(now - me->last > 1)
	{
		OCLog("received %d in the last %1.1fs", me->nReceived, now-me->last);
		me->last = now;
		me->nReceived = 0;
	}
	if(message->header.messageType != TMCHUNK)
		return;
	
	OCDeserializerRef ds = OCDeserializerCreateWithBytes(message->data, message->header.dataLength); 
	ALChunkRef chunk = NULL;
	if(ALChunkDeserialize(ds, &chunk) == SUCCESS)
		DecryptionQueueEnqueue(me->deq, chunk);
	if(chunk)
		OCRelease(&chunk);
	OCRelease(&ds);	
}

static void Tier3DecryptedCallback(Tier3Ref me, ALPayloadChunkRef plc){
	Tier3StorageStore(me->storage, plc);
}

