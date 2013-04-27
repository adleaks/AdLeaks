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

#include "TierClientConnection.h"
#include "OCObject.h"
#include "OCBase.h"

#include <event2/dns.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>

#include "TierMessage.h"
#include "TierMessage_evbuffer.h"
#include "OCValue.h"

struct TierClientConnection{
	struct OCObject _obj;
	
	struct event_base* eventLoop;
	struct evdns_base* dnsBase;
	struct event* timer;
	struct bufferevent* be;
	
	OCStringRef host;
	unsigned short port;
	
	struct TierMessage msg;
	OCBool hasHeader;	
	
	OCBool isConnected;
	OCBool isConnecting;
	OCBool clearToSend;
	OCBool doQuitGracefully;
	OCBool hasQuit;
	OCBool logReconnectMessage;
	
	pthread_mutex_t quitMutex;
	pthread_cond_t quitCond;
	
	// send buffers:
	OCListRef newChunks;
	OCListRef toSendChunks;
	OCListRef unackedChunks;	
};

#define TCLOG(MSG) OCLog( ("[%s:%d] "MSG), OCStringGetCString(me->host), me->port)
// private methods
void TierClientConnectionTimerCB(TierClientConnectionRef me);
void TierClientConnectionReadCB(TierClientConnectionRef me);
void TierClientConnectionWriteCB(TierClientConnectionRef me);
void TierClientConnectionEventCB(TierClientConnectionRef me, short events);

void TierClientConnectionDealloc(void* _me){
	TierClientConnectionRef me = (TierClientConnectionRef)_me;
	OCRelease(&me->newChunks);
	OCRelease(&me->toSendChunks);
	OCRelease(&me->unackedChunks);
	pthread_mutex_destroy(&me->quitMutex);
	pthread_cond_destroy(&me->quitCond);
	OCRelease(&me->host);
}

#define BUFFERSIZE 100

TierClientConnectionRef TierClientConnectionCreate(struct event_base* eventLoop, 
										    struct evdns_base* dnsBase,
										    OCStringRef host,
										    unsigned short port){
	OCOBJECT_ALLOCINIT(TierClientConnection);
	
	me->eventLoop = eventLoop;
	me->dnsBase = dnsBase;
	
	me->host = (OCStringRef) OCObjectRetain((OCObjectRef)host);
	me->port = port;
	
	me->timer = NULL;
	me->be = NULL;
	me->isConnected = NO;
	me->isConnecting = NO;
	me->clearToSend = NO;
	me->doQuitGracefully = NO;
	me->hasQuit = NO;
	me->logReconnectMessage = YES;
	
	pthread_mutex_init(&(me->quitMutex), NULL);
	pthread_cond_init(&(me->quitCond), NULL);
	
	me->newChunks = OCListCreate();
	me->toSendChunks = OCListCreate();
	me->unackedChunks = OCListCreate();
		
	return me;
}


static void readCBWrapper(struct bufferevent *be, void *ptr){
	VAR_UNUSED(be);
	TierClientConnectionReadCB((TierClientConnectionRef) ptr);
}

static void writeCBWrapper(struct bufferevent* be, void* ptr){
	VAR_UNUSED(be);
	TierClientConnectionWriteCB((TierClientConnectionRef) ptr);
}

static void eventCBWrapper(struct bufferevent *be, short events, void *ptr){
	VAR_UNUSED(be);
	TierClientConnectionEventCB((TierClientConnectionRef) ptr, events);
}


static void timerCBWrapper(evutil_socket_t fd, short what, void *ptr){
	VAR_UNUSED(fd);
	VAR_UNUSED(what);
	TierClientConnectionTimerCB((TierClientConnectionRef) ptr);	
}


void TierClientConnectionReconnect(TierClientConnectionRef me){

	if(me->logReconnectMessage)
	{
		TCLOG("trying to reconnect");
		me->logReconnectMessage = NO;
	}
	if(me->be != NULL)
		bufferevent_free(me->be);
	me->be = bufferevent_socket_new(me->eventLoop, -1, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(me->be, readCBWrapper, writeCBWrapper, eventCBWrapper, me);
	
	bufferevent_enable(me->be, EV_READ|EV_WRITE);
	bufferevent_socket_connect_hostname(me->be, me->dnsBase, AF_UNSPEC, CO(me->host), me->port);

	me->isConnecting = YES;
}

OCStatus TierClientConnectionActivate(TierClientConnectionRef me){
	struct timeval one_sec = {1, 0};
	me->timer = event_new(me->eventLoop, -1, EV_PERSIST, timerCBWrapper, me);
	evtimer_add(me->timer, &one_sec);
	
	TierClientConnectionReconnect(me);
	
	return SUCCESS;
}

void TierClientConnectionTimerCB(TierClientConnectionRef me){
	if(me->doQuitGracefully == YES)
	{
		event_del(me->timer);
		bufferevent_free(me->be);
		me->timer = NULL;
		me->be = NULL;
		me->isConnecting = NO;
		me->isConnected = NO;

		me->hasQuit = YES;
		pthread_mutex_lock(&(me->quitMutex));
		pthread_cond_signal(&(me->quitCond));
		pthread_mutex_unlock(&(me->quitMutex));
		TCLOG("TierClientConnection ended");
		return;
	}
	if(me->isConnected == NO && me->isConnecting == NO)
		TierClientConnectionReconnect(me);
	
	if(me->isConnected == NO)
		return;	
	
	if(OCListGetCount(me->newChunks) == 0)
		return;
	
	size_t threshold = 0;
	size_t readyChunks = OCListGetCount(me->newChunks);
	if(readyChunks < threshold)
		return;
	
	OCObjectLock((OCObjectRef)me->newChunks);
	OCListAppendList(me->toSendChunks, me->newChunks);
	OCObjectUnlock((OCObjectRef)me->newChunks);
	if(OCListGetCount(me->toSendChunks) > 0)
	{
		me->clearToSend = YES;
		TierClientConnectionWriteCB(me);
	}
// send heartbeat
//	struct TierMessageHeader hb = TierMessageMakeHeartbeat();
//	TierMessageWriteHeaderToEVBuffer(&hb, me->be->output);
//	bufferevent_write_buffer(me->be, bufferevent_get_output(me->be));
}

void TierClientConnectionReadCB(TierClientConnectionRef me){
	if(me->doQuitGracefully)
		return;
	while(1)
	{
		if(me->hasHeader == NO)
		{
			size_t nAvailable = evbuffer_get_length(me->be->input);
		
			if(nAvailable < TIER_MESSAGE_HEADER_LENGTH)
				break;
		
			TierMessageReadHeaderFromEVBuffer(&(me->msg.header), me->be->input);
			me->hasHeader = YES;
		}
		if(me->msg.header.messageType == TMHEARTBEAT)
		{
			// OCLog("Client: heartbeat received");
			me->hasHeader = NO;
			continue;
		}
		if(me->msg.header.messageType == TMACK)
		{
			ALChunkRef chunk = (ALChunkRef) OCListFifoPop(me->unackedChunks);
			if(chunk != NULL)
				OCObjectRelease(&chunk);
//			OCLog("Client: ack received");
			me->hasHeader = NO;
			continue;			
		}
		break;
	}
}

void TierClientConnectionWriteCB(TierClientConnectionRef me){
	if(me->doQuitGracefully)
		return;
	if(me->clearToSend == YES)
	{
		OCSerializerRef s = OCSerializerCreate();
		
		ALChunkRef chunk = NULL;
		while( (chunk=(ALChunkRef)OCListFifoPop(me->toSendChunks)) != NULL)
		{
			OCSerializerReset(s);
		
			ALChunkSerialize(chunk, s);
			size_t nBytes = OCSerializerGetNumberOfBytes(s);
			if(nBytes < TIER_MESSAGE_MAX_LENGTH)
			{
				struct TierMessage msg;
				msg.header.messageType = TMCHUNK;
				msg.header.protocolVariant = 0;
				msg.header.protocolVersion = 0;
				msg.header.dataLength = OCSerializerGetNumberOfBytes(s);
				OCSerializerCopyBytes(s, msg.data);
				bufferevent_lock(me->be);
				TierMessageWriteToEVBuffer(&msg, me->be->output);
				bufferevent_write_buffer(me->be, bufferevent_get_output(me->be));		
				bufferevent_unlock(me->be);
				
				OCListFifoPush(me->unackedChunks, (OCObjectRef)chunk);
			}
			// OCListFifoPop does not release the chunk, we have to do this:
			OCObjectRelease(&chunk);
		}
		OCObjectRelease(&s);		
		me->clearToSend = NO;
	}
	me->clearToSend = YES;
}

void TierClientConnectionEventCB(TierClientConnectionRef me, short events){	
	if(me->doQuitGracefully)
		return;
	if(me->isConnecting)
	{
		if(events & BEV_EVENT_ERROR)
		{
			me->isConnecting = NO;
			me->isConnected = NO;
		}
		else if(events & BEV_EVENT_CONNECTED)
		{
			TCLOG("Connected");
			me->isConnecting = NO;
			me->isConnected = YES;
			TierClientConnectionWriteCB(me);
		}
	}
	else if(me->isConnected)
	{
		if(events & BEV_EVENT_EOF)
		{
			TCLOG("Connection closed by remote site");
			me->logReconnectMessage = YES;
			me->isConnected = NO;
		}
		else if(events & BEV_EVENT_ERROR)
		{
			TCLOG("Connection closed by remote site");
			me->logReconnectMessage = YES;
			me->isConnected = NO;
		}	
	}
}

OCStatus TierClientConnectionStop(TierClientConnectionRef me){
	me->doQuitGracefully = YES;
	return SUCCESS;
}

OCStatus TierClientConnectionWaitTillStopped(TierClientConnectionRef me){
	if(me->hasQuit)
		return SUCCESS;
	pthread_mutex_lock(&(me->quitMutex));
	pthread_cond_wait(&(me->quitCond), &(me->quitMutex));
	pthread_mutex_unlock(&(me->quitMutex));
	return SUCCESS;
}

OCStatus TierClientConnectionSendChunk(TierClientConnectionRef me, ALChunkRef chunk){
	if(me->doQuitGracefully)
		return FAILED;
	OCObjectLock((OCObjectRef)me->newChunks);
	OCListAppend(me->newChunks, (OCObjectRef)chunk);
	OCObjectUnlock((OCObjectRef)me->newChunks);

	return SUCCESS;
}

OCStatus TierClientConnectionSendChunks(TierClientConnectionRef me, OCListRef chunks){
	if(me->doQuitGracefully)
		return FAILED;
	OCObjectLock((OCObjectRef)me->newChunks);
	OCListAppendList(me->newChunks, chunks);
	OCObjectUnlock((OCObjectRef)me->newChunks);
	
	return SUCCESS;
}

