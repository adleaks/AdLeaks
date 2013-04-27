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

#include "TierServerConnection.h"

#include "OCObject.h"
#include "OCTypes.h"
#include "OCBase.h"

#include <event.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "TierMessage_evbuffer.h"
#include <errno.h>


struct TierServerConnection{
	struct OCObject __obj;

	int socketfd;
	
	OCBool doShutdown;
	OCBool hasShutdown;
	
	struct event_base* base;
	struct bufferevent* buf_event;
	struct evbuffer* out;
	
	TierServerDelegate delegate;
	TierServerConnectionDelegate connectionDelegate;
	
	struct TierMessage receivedMessage; 
	OCBool hasHeader;
	
	OCBool doQuitGracefully;
};

void TierServerConnectionOnRead(TierServerConnectionRef me);
void TierServerConnectionOnError(TierServerConnectionRef me, short error);
void TierServerConnectionOnWriteCB(TierServerConnectionRef me);

#define TSCLOG(MSG, ...) OCLog( "[Connection:] "MSG, ##__VA_ARGS__)


void TierServerConnectionDealloc(void* _me){
	TierServerConnectionRef me = (TierServerConnectionRef) _me; 
	OCRelease(&me->connectionDelegate.delegateObject);
	close(me->socketfd);	
	bufferevent_free(me->buf_event);
	evbuffer_free(me->out);
}

static void onErrorWrapper(struct bufferevent *buf_event, short error, void *arg){
	VAR_UNUSED(buf_event);
	TierServerConnectionOnError(arg, error);
}

static void onReadWrapper(struct bufferevent *buf_event, void *arg){
	VAR_UNUSED(buf_event);
	TierServerConnectionOnRead(arg);
}

TierServerConnectionRef TierServerConnectionCreate(struct event_base* base, 
										     int socketfd, 
										     TierServerDelegate delegate,
										     TierServerConnectionDelegate connectionDelegate){
	OCOBJECT_ALLOCINIT(TierServerConnection);
	
	int flags = fcntl(socketfd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(socketfd, F_SETFL, flags);
	
	me->buf_event = bufferevent_new(socketfd, onReadWrapper, NULL, onErrorWrapper, me);
	bufferevent_base_set(base, me->buf_event);
	bufferevent_settimeout(me->buf_event, 0, 0);
	bufferevent_enable(me->buf_event, EV_READ);
	me->base = base;
	// outgoing buffer:
	me->out = evbuffer_new();
	
	me->delegate = delegate;
	me->hasHeader = NO;
	me->socketfd = socketfd;
	me->doShutdown = NO;
	me->connectionDelegate = connectionDelegate;
	OCRetain(me->connectionDelegate.delegateObject);
	
	return me;
}

void TierServerConnectionDoShutdown(TierServerConnectionRef me){
	if(me->hasShutdown == YES)
		return;
	if(shutdown(me->socketfd, SHUT_RDWR) != 0)
		TSCLOG("error shutting down: %d", errno);
	me->hasShutdown = YES;
	me->connectionDelegate.hasShutdown(me->connectionDelegate.delegateObject, me);
	TSCLOG("TierServerConnection has shut down", NULL);
}

void TierServerConnectionOnRead(TierServerConnectionRef me){
	if(me->doQuitGracefully == YES)
	{
		me->doShutdown = YES;
		TierServerConnectionDoShutdown(me);
	}
	while(me->doShutdown == NO)
	{
		if(me->hasHeader == NO)
		{
			size_t nAvailable = evbuffer_get_length(me->buf_event->input);
		
			if(nAvailable < TIER_MESSAGE_HEADER_LENGTH)
				break;

			TierMessageReadHeaderFromEVBuffer(&(me->receivedMessage.header), me->buf_event->input);
			me->hasHeader = YES;
		}
		if(me->receivedMessage.header.messageType == TMHEARTBEAT)
		{
//			OCLog("Server: heartbeat received");
			me->hasHeader = NO;
			continue;
		}
		if(me->receivedMessage.header.messageType == TMACK)
		{
			me->hasHeader = NO;
			continue;			
		}
		if(me->receivedMessage.header.messageType != TMCHUNK)
		{
			TSCLOG("Unknown message received, terminating connection", NULL);
			me->doShutdown = YES;
			me->hasHeader = NO;
			break;
		}
		if(me->receivedMessage.header.dataLength > TIER_MESSAGE_MAX_LENGTH)
		{
			TSCLOG("Too long message received, terminating connection", NULL);
			me->doShutdown = YES;
			me->hasHeader = NO;
			break;
		}
	
		size_t nAvailable = evbuffer_get_length(me->buf_event->input);
		if(nAvailable < me->receivedMessage.header.dataLength)
			break;
	
		evbuffer_remove(me->buf_event->input, &(me->receivedMessage.data), me->receivedMessage.header.dataLength);

		me->hasHeader = NO;
		me->delegate.messageReceived(me->delegate.delegateObject, &(me->receivedMessage));

		struct TierMessageHeader ack = TierMessageMakeAck();
		TierMessageWriteHeaderToEVBuffer(&ack, me->out);
	}
	if(me->doShutdown)
		TierServerConnectionDoShutdown(me);
	else
		bufferevent_write_buffer(me->buf_event, me->out);
}

void TierServerConnectionOnWriteCB(TierServerConnectionRef me){
	if(me->doQuitGracefully == YES)
	{
		me->doShutdown = YES;
		TierServerConnectionDoShutdown(me);
	}
}

void TierServerConnectionOnError(TierServerConnectionRef me, short error){	
	if(me->doQuitGracefully == YES)
	{
		me->doShutdown = YES;
		TierServerConnectionDoShutdown(me);
	}
	if(error & EVBUFFER_EOF) 
	{
		TSCLOG("Remote host disconnected", NULL);
		me->doShutdown = YES;
		me->hasShutdown = YES;
		me->connectionDelegate.hasShutdown(me->connectionDelegate.delegateObject, me);
	}
	else if(error & EVBUFFER_TIMEOUT) 
	{
		TSCLOG("Remote host timed out", NULL);
		me->doShutdown = YES;
		me->hasShutdown = YES;
	}
	else 
	{
		TSCLOG("A socket error (0x%hx) occurred", error);
		me->doShutdown = YES;
		me->hasShutdown = YES;
	}
	
	if(me->hasShutdown == NO)
		TierServerConnectionDoShutdown(me);
}

void shutdownWrapper(evutil_socket_t sock, short error, void *arg){
	VAR_UNUSED(sock);
	VAR_UNUSED(error);
	TierServerConnectionDoShutdown((TierServerConnectionRef)arg);
}

OCStatus TierServerConnectionStop(TierServerConnectionRef me){
	me->doQuitGracefully = YES;
	TSCLOG("requested stop", NULL);
	struct timeval one_sec;
	one_sec.tv_sec = 0;
	one_sec.tv_usec = 0;
	struct event * ev1;
	ev1 = event_new(me->base, -1, EV_PERSIST, shutdownWrapper, me);
	event_add(ev1, &one_sec);
	
	return SUCCESS;
}



