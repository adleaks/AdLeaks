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

#include <event2/dns.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>

#include <string.h>

#include "TierClient.h"
#include "OCList.h"
#include "OCTypes.h"
#include "TierClientConnection.h"
#include "OCThread.h"

struct TierClient{
	struct OCObject _obj;
	
	struct event_base* eventLoop;
	struct evdns_base* dnsBase;
	struct event* timer;
	
	OCListRef connectedServers;
	OCThreadRef thread;
	OCBool doQuitGracefully;
};

// private methods
static void TierClientMain(TierClientRef me);
static void TierClientExit(TierClientRef me);
static void TierClientTimerCB(TierClientRef me);



void TierClientDealloc(void* _me){
	TierClientRef me = (TierClientRef) _me;
	VAR_UNUSED(me);
}

TierClientRef TierClientCreate(){
	OCOBJECT_ALLOCINIT(TierClient);
	if(me == NULL)
		return me;
	
	me->eventLoop = event_base_new();
	me->dnsBase = evdns_base_new(me->eventLoop, 1);
	
	me->connectedServers = OCListCreate();
	me->doQuitGracefully = NO;
	
	OCThreadDelegate delegate;
	delegate.delegateObject = (OCObjectRef)me;
	delegate.mainCB = (OCThreadMainCB)TierClientMain;
	delegate.exitCB = (OCThreadExitCB)TierClientExit;
	delegate.stopCB = NULL;
	me->thread = OCThreadCreate(delegate);
	
	return me;
}

OCStatus TierClientStart(TierClientRef me){
	return OCThreadStart(me->thread);
}

OCStatus TierClientAddServer(TierClientRef me, OCStringRef host, unsigned short andPort){
	TierClientConnectionRef tc = TierClientConnectionCreate(me->eventLoop, 
												me->dnsBase, host, andPort);
	TierClientConnectionActivate(tc);
	
	OCListAppend(me->connectedServers, (OCObjectRef)tc);
	OCObjectRelease(&tc);

	return SUCCESS;
}

static void timerCBWrapper(evutil_socket_t fd, short what, void *ptr){
	VAR_UNUSED(fd);
	VAR_UNUSED(what);
	TierClientTimerCB((TierClientRef) ptr);	
}

static void TierClientMain(TierClientRef me){
	struct timeval one_sec = {1, 0};
	me->timer = event_new(me->eventLoop, -1, EV_PERSIST, timerCBWrapper, me);
	evtimer_add(me->timer, &one_sec);	
	
	event_base_dispatch(me->eventLoop);	
}

static void TierClientExit(TierClientRef me){
	evdns_base_free(me->dnsBase, 1);
	event_base_free(me->eventLoop);
}

OCStatus TierClientStop(TierClientRef me){
	OCListIteratorRef iter = OCListCreateIterator(me->connectedServers);
	TierClientConnectionRef cc = NULL;
	
	me->doQuitGracefully = YES;
	
	while( (cc=(TierClientConnectionRef)OCListIteratorNext(iter)) != NULL)
		TierClientConnectionStop(cc);

//	OCListIteratorReset(iter);
//	while( (cc=(TierClientConnectionRef)OCListIteratorNext(iter)) != NULL)
//	{
//		TierClientConnectionWaitTillStopped(cc);
//		OCObjectRelease(&cc);
//	}
	OCObjectRelease(&iter);
		
	return SUCCESS;
}

OCStatus TierClientWaitTillStopped(TierClientRef me){
	return OCThreadJoin(me->thread);
}


void TierClientTimerCB(TierClientRef me){
	if(me->doQuitGracefully)
		event_base_loopexit(me->eventLoop, NULL);		
}

size_t TierClientGetNumberOfServer(TierClientRef me){
	return OCListGetCount(me->connectedServers);
}


OCStatus TierClientSendChunk(TierClientRef me, ALChunkRef chunk, size_t serverIndex){
	// is this thread-safe?
	// -> no and yes
	//	* no: _when_ a client is removed from me->connectedServers the code below is evil and wrong
	//	* yes: the servers in me->connectedServers are persistent, so they will never be removed 
	//		  (even during shutdown)
	if(me->doQuitGracefully == YES)
	{
		OCLog("Attempt to send a chunk while quitting the client connection gracefully, dropping");
		return FAILED;
	}
	TierClientConnectionRef cc = (TierClientConnectionRef)OCListGetItem(me->connectedServers, serverIndex);
	if(cc == NULL)
		return FAILED;
	
	return TierClientConnectionSendChunk(cc, chunk);
}

OCStatus TierClientSendChunks(TierClientRef me, OCListRef chunks, size_t serverIndex){
	if(me->doQuitGracefully == YES)
	{
		OCLog("Attempt to send a chunk while quitting the client connection gracefully, dropping");
		return FAILED;
	}
	TierClientConnectionRef cc = (TierClientConnectionRef)OCListGetItem(me->connectedServers, serverIndex);
	if(cc == NULL)
		return FAILED;
	
	return TierClientConnectionSendChunks(cc, chunks);	
}



