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

#include "OCObject.h"
 #include <assert.h>
#include <pthread.h>

void* OCObjectCreateObjectWithAllocator(OCAllocatorRef allocator, OCObjectInfo info){
	OCObjectRef obj = (OCObjectRef) OCAllocate(allocator, info.size);
	obj->allocator = allocator;
	obj->dealloc = info.dealloc;
	obj->retainCounter = 1;
	pthread_mutex_init(&obj->mutex, NULL);
	
	return obj;
}
 
 OCObjectRef OCObjectRetain(OCObjectRef obj){
	 assert(obj != NULL);
//	 pthread_mutex_lock(&obj->mutex);
	 obj->retainCounter++;
//	 pthread_mutex_unlock(&obj->mutex);
	 return obj;
 }
 
 void OCObjectRelease(void* o){
	 OCObjectRef* obj = (OCObjectRef*)o;
	 
	 if(obj == NULL)
		 return;
	 if(*obj == NULL)
		 return;
	 
	 //NOTE: We have two cases:
	 //	* in parallel one call in Retain and one in Release
	 //	   where the Release call sets the retainCounter to 0
	 //		-> this can not happen, when the Thread calling 
	 //		     Retain already "owns" the object, if it does not
	 //		     this is considered to be an programming error
	 //	* two calls in Release:
	 //		-> when retainCounter = 0 only the later one
	 //		     will free the object, so we are fine
//	 pthread_mutex_lock(&(*obj)->mutex);
	 (*obj)->retainCounter--;

	 assert((*obj)->retainCounter >= 0);
 
	 if((*obj)->retainCounter == 0)
	 {
//		 pthread_mutex_unlock(&(*obj)->mutex);
		 (*obj)->dealloc(*obj);
		 OCDeallocate((*obj)->allocator, *obj);
	 }
	 else
	 {
//		 pthread_mutex_unlock(&(*obj)->mutex);
	 }
	 *obj = NULL;
 }


void OCObjectLock(OCObjectRef obj){
	pthread_mutex_lock(&obj->mutex);
}

void OCObjectUnlock(OCObjectRef obj){
	pthread_mutex_unlock(&obj->mutex);	
}

