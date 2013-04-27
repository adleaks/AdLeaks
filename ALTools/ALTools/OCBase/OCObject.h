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

#ifndef _OC_OBJECT_H_
#define _OC_OBJECT_H_

#include "OCAllocator.h"
#include <assert.h>
#include <pthread.h>

#include "OCAutoreleasePool.h"

typedef void (*deallocMethodPtr)(void*);

// The CBase Object system:
struct OCObject{
	int retainCounter;
	OCAllocatorRef allocator;
	deallocMethodPtr dealloc;
	pthread_mutex_t mutex;
};
typedef struct OCObject* OCObjectRef;

typedef struct{
	size_t size;
	deallocMethodPtr dealloc;
} OCObjectInfo;


#define OCOBJECT_ALLOCINIT(CLASS) \
OCObjectInfo __info = {sizeof(struct CLASS), CLASS ## Dealloc};\
CLASS ## Ref me = OCObjectCreateObjectWithAllocator(ocDefaultAllocator, __info)

void* OCObjectCreateObjectWithAllocator(OCAllocatorRef allocator, OCObjectInfo info);

OCObjectRef OCObjectRetain(OCObjectRef obj);

// NOTE: obj or *obj may be NULL
void OCObjectRelease(void* obj);

#define OCRetain(OBJ) \
(typeof(OBJ)) OCObjectRetain((OCObjectRef)(OBJ))

#define OCRelease(OBJ) \
OCObjectRelease((OCObjectRef)(OBJ))

void OCObjectLock(OCObjectRef obj);
void OCObjectUnlock(OCObjectRef obj);

#endif
