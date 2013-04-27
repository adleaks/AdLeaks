/* This file is part of the OCBase library.
 * Copyright (C) 2012-2013 Benjamin Güldenring
 * Freie Universität Berlin, Germany
 *
 * OCBase is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 *
 * OCBase is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OCBase.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "OCThread.h"
#include <sys/time.h>

struct OCThread{
	struct OCObject _obj;
	
	OCThreadDelegate delegate;
	
	pthread_t thread;
	
	pthread_cond_t sleepCond;
	pthread_mutex_t sleepMutex;	
	
	
};

// private methdos
static void* threadMainWrapper(void* param);
static void* OCThreadMain(OCThreadRef me);

void OCThreadDealloc(void* _me){
	OCThreadRef me = (OCThreadRef)_me;
	OCObjectRelease(&me->delegate.delegateObject);
	pthread_mutex_destroy(&me->sleepMutex);
	pthread_cond_destroy(&me->sleepCond);
}

OCThreadRef OCThreadCreate(OCThreadDelegate delegate){
	OCOBJECT_ALLOCINIT(OCThread);
	if(me == NULL)
		return me;
	
	me->delegate = delegate;
	OCObjectRetain(me->delegate.delegateObject);
	pthread_cond_init(&me->sleepCond, NULL);
	pthread_mutex_init(&me->sleepMutex, NULL);
	
	return me;
}


OCStatus OCThreadStart(OCThreadRef me){
	// we need ourselves as long as the thread is running:
	OCObjectRetain((OCObjectRef)me);
	
	if(pthread_create(&me->thread, NULL, threadMainWrapper, me) != 0)
	{		
		OCObjectRelease(&me);
		return FAILED;
	}

	return SUCCESS;
}

OCStatus OCThreadStop(OCThreadRef me){
	if(me->delegate.stopCB)
	{
		me->delegate.stopCB(me->delegate.delegateObject);
		return SUCCESS;
	}
	return FAILED;
}

OCStatus OCThreadSleep(OCThreadRef me, OCTimeInterval dt){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct timespec toWait;
	toWait.tv_sec = tv.tv_sec + (long)dt;
	long nsecs = tv.tv_usec*1000 + (dt-(long)dt)*1000000000;
	if(nsecs > 1000000000)
		toWait.tv_sec += nsecs/1000000000;
	toWait.tv_nsec = nsecs % 1000000000;
	
	pthread_mutex_lock(&me->sleepMutex);
	pthread_cond_timedwait(&me->sleepCond, &me->sleepMutex, &toWait);
	pthread_mutex_unlock(&me->sleepMutex);		

	return SUCCESS;
}

OCStatus OCThreadJoin(OCThreadRef me){
	if(pthread_join(me->thread, NULL) != 0)
		return FAILED;
	return SUCCESS;
}

static void* OCThreadMain(OCThreadRef me){
	
	if(me->delegate.mainCB)
		me->delegate.mainCB(me->delegate.delegateObject);
	
	if(me->delegate.exitCB)
		me->delegate.exitCB(me->delegate.delegateObject);
	
	// the thread is going to exit, so we don't need ourselves any longer
	OCObjectRelease(&me);
	pthread_exit(0);		
}

static void* threadMainWrapper(void* param){
	return OCThreadMain((OCThreadRef) param);
}

