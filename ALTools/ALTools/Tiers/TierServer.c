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

#include "TierServer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <event.h>
#include <pthread.h>

#include "OCBase.h"
#include "TierServerConnection.h"
#include "OCThread.h"

struct TierServer{
	struct OCObject __obj;
	struct event_base* eventLoop;
	struct event connectEvent;
	struct event* timer;
	
	unsigned short portNo;
	
	struct sockaddr_in6 localAddress;
	int listenfd;	
	TierServerDelegate delegate;
	
	OCListRef connectedClients;
	OCBool doQuitGrafecully;
	
	OCThreadRef thread;
};

// private methods
static void TierServerMain(TierServerRef me);

static void TierServerTimerCB(TierServerRef me);
static void TierServerConnectionHasShutdownCB(OCObjectRef me, TierServerConnectionRef sender);

#define TSLOG(MSG, ...) OCLog( "[Service:] "MSG, ##__VA_ARGS__)

void TierServerDealloc(void* _me){
	VAR_UNUSED(_me);
}

void TierServerOnConnect(TierServerRef me, int listenfd, short evtype){
	struct sockaddr_in6 remote_addr;
	socklen_t addrlen = sizeof(remote_addr);
	int sockfd;
	
	if(me->doQuitGrafecully == YES)
		return;
	
	TSLOG("on connect called", NULL);

	if( (evtype&EV_READ) == 0)
	{
		OCLog("unknown event type");
		return;
	}
	sockfd = accept(listenfd, (struct sockaddr *)&remote_addr, &addrlen);
	if(sockfd < 0)
		TSLOG("error accepting", NULL);
	else
	{
		TierServerConnectionDelegate del;
		del.delegateObject = (OCObjectRef) me;
		del.hasShutdown = TierServerConnectionHasShutdownCB;
		
		TierServerConnectionRef client = TierServerConnectionCreate(me->eventLoop, sockfd, me->delegate, del);
		OCListAppend(me->connectedClients, (OCObjectRef)client);
	}
}

static void TierServerConnectionHasShutdownCB(OCObjectRef _me, TierServerConnectionRef sender){
	TierServerRef me = (TierServerRef) _me;
	
	OCListRemove(me->connectedClients, (OCObjectRef)sender);
	OCObjectRelease(&sender);
}


static void onConnectWrapper(int listenfd, short evtype, void* me){
	TierServerOnConnect((TierServerRef)me, listenfd, evtype);
}

static void timerCBWrapper(evutil_socket_t fd, short what, void *ptr){
	VAR_UNUSED(fd);
	VAR_UNUSED(what);
	TierServerTimerCB((TierServerRef) ptr);	
}

TierServerRef TierServerCreateWithPortNo(unsigned short portNo, TierServerDelegate delegate){
	OCOBJECT_ALLOCINIT(TierServer);
	
	me->eventLoop = event_base_new();
	bzero(&(me->localAddress), sizeof(struct sockaddr_in6));
	
	me->localAddress.sin6_family = AF_INET6;
	me->localAddress.sin6_port = htons(portNo);
	me->localAddress.sin6_addr = in6addr_any;
	
	me->listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	me->delegate = delegate;
	me->connectedClients = OCListCreate();
	me->doQuitGrafecully = NO;
	
	int tmp_reuse = 1;
	setsockopt(me->listenfd, SOL_SOCKET, SO_REUSEADDR, &tmp_reuse, sizeof(int));
	bind(me->listenfd, (struct sockaddr *)&(me->localAddress), sizeof(struct sockaddr_in6));
	
	listen(me->listenfd, 8);
	
	int flags;
	flags = fcntl(me->listenfd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(me->listenfd, F_SETFL, flags);
	
	event_set(&(me->connectEvent), me->listenfd, EV_READ | EV_PERSIST, onConnectWrapper, me);
	event_base_set(me->eventLoop, &me->connectEvent);
	event_add(&(me->connectEvent), NULL);

	struct timeval one_sec = {1, 0};
	me->timer = event_new(me->eventLoop, -1, EV_PERSIST, timerCBWrapper, me);
	evtimer_add(me->timer, &one_sec);
	
	OCThreadDelegate threaddelegate;
	threaddelegate.delegateObject = (OCObjectRef)me;
	threaddelegate.mainCB = (OCThreadMainCB)TierServerMain;
	threaddelegate.exitCB = NULL;
	threaddelegate.stopCB = NULL;
	me->thread = OCThreadCreate(threaddelegate);
	
	return me;	
}

OCStatus TierServerStart(TierServerRef me){
	return OCThreadStart(me->thread);
}

static void TierServerMain(TierServerRef me){
	event_base_dispatch(me->eventLoop);
	
	event_del(&me->connectEvent);
	event_base_free(me->eventLoop);	
}

static void TierServerTimerCB(TierServerRef me){
	if(me->doQuitGrafecully && OCListGetCount(me->connectedClients) == 0)
		event_base_loopexit(me->eventLoop, NULL);
}

OCStatus TierServerStop(TierServerRef me){
	OCListIteratorRef iter = OCListCreateIterator(me->connectedClients);
	
	me->doQuitGrafecully = YES;

	TierServerConnectionRef c = NULL;
	while( (c=(TierServerConnectionRef)OCListIteratorNext(iter)) != NULL)
	{
		TierServerConnectionStop(c);
	}
	OCObjectRelease(&iter);

	return SUCCESS;
}

OCStatus TierServerWaitTillStopped(TierServerRef me){
	if(OCThreadJoin(me->thread) == SUCCESS)
		return SUCCESS;
	else
		TSLOG("Error joing Thread, code: %d", errno);
	return FAILED;
}

