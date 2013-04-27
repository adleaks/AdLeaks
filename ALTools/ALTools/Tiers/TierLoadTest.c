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

#include "TierLoadTest.h"
#include "OCObject.h"
#include "TierClient.h"
#include "TierConfig.h"
#include "ALProtocol.h"
#include <time.h>
#include <unistd.h>
#include "Tier.h"

struct TierLoadTest{
	struct OCObject _obj;
	
	TierConfigRef config;
	OCBool doQuit;
	
	ALProtocolRef alp;
	
	TierClientRef tclient;
};

void TierLoadTestDealloc(void* _me){
	TierLoadTestRef me = (TierLoadTestRef) _me;
	OCRelease(&me->tclient);
	OCRelease(&me->config);
	OCRelease(&me->alp);
}

TierLoadTestRef TierLoadTestCreate(TierConfigRef config, ALProtocolRef alp){
	OCOBJECT_ALLOCINIT(TierLoadTest);
	if(me == NULL)
		return me;
	
	me->config = OCRetain(config);
	me->doQuit = NO;
	me->alp = OCRetain(alp);
	
	me->tclient = TierClientCreate();

	return me;
}

int TierLoadTestMain(TierLoadTestRef me){
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
		
	
	useconds_t toWait  = 0;
	OCAutoreleasePoolCreate();
	OCStringRef chunksPerSec = TierConfigGetServiceValue(me->config, OC("chunksPerSec"));
	size_t nChunksPerSec = atoi(CO(chunksPerSec));
	toWait =  1000000/nChunksPerSec;
	OCAutoreleasePoolDestroy();
	
	ALChunkRef chunk = ALProtocolEncNeutral(me->alp);
	while(tier_gotSignal == NO)
	{
		TierClientSendChunk(me->tclient, chunk, 0);
		usleep(toWait);
	}
	
	TierClientStop(me->tclient);
	TierClientWaitTillStopped(me->tclient);
	
	return 0;
}

void TierLoadTestHandleSignal(TierLoadTestRef me, int thesignal){
	VAR_UNUSED(thesignal);
	me->doQuit = YES;	
}

