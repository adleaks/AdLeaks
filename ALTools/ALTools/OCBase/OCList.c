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

#include <stdlib.h>
#include <stdio.h>
#include "OCList.h"
#include "OCObject.h"
#include "OCLog.h"
#include "OCBase.h"

typedef struct OCListElement* OCListElementPtr;

struct OCListElement{
	OCObjectRef item;
	
	OCListElementPtr next;
	OCListElementPtr prev;
};

struct OCList{
	struct OCObject _obj;
	
	OCListElementPtr head;
	OCListElementPtr tail;
	
	size_t size;
	unsigned long changeCount; 
};

struct OCListIterator{
	struct OCObject _obj;

	OCListRef list;
	unsigned long lastChangeCount;
	OCListElementPtr next;
};

void OCListDealloc(void* _me){
	OCListRef me = (OCListRef) _me;

	OCListElementPtr el = me->head;
	while(el != NULL)
	{
		OCObjectRelease(&el->item);
		OCListElementPtr toDel = el;
		el = el->next;
		OCDeallocate(ocDefaultAllocator, toDel);
	}	
}

OCListRef OCListCreate(){
	OCObjectInfo info = {sizeof(struct OCList), OCListDealloc};
	OCListRef list = (OCListRef) OCObjectCreateObjectWithAllocator(ocDefaultAllocator, info);
	
	if(list == NULL)
		return NULL;
	
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	list->changeCount = 0;
	
	return list;
}

size_t OCListGetCount(OCListRef me){
	return me->size;
}

OCObjectRef OCListGetItem(OCListRef me, size_t index){
	if(index >= me->size)
		return NULL;
		
	struct OCListElement* el = me->head;
	
	size_t i = 0;
	for(i=0; i < index; i++)
		el = el->next;
	
	return el->item;
}

OCStatus OCListRemoveItem(OCListRef me, size_t index){
	if(index >= me->size)
		return FAILED;
	
	// find item to remove:
	OCListElementPtr el = me->head;
	size_t i = 0;
	for(i=0; i < index; i++)
		el = el->next;	
		
	if(el->prev)
		el->prev->next = el->next;
	if(el->next)
		el->next->prev = el->prev;
	
	if(index == 0)
		me->head = el->next;		
	if(index == me->size-1)
		me->tail = el->prev;
	
	me->size--;
	
	OCObjectRelease(&el->item);
	OCDeallocate(me->_obj.allocator, el);
		
	return SUCCESS;
}

OCStatus OCListRemove(OCListRef me, OCObjectRef obj){
	size_t index = 0;
	OCListElementPtr el = me->head;
	OCBool found = NO;
	
	// find item:
	for(index=0; index < me->size; index++)
	{
		if(el->item == obj)
		{
			found = YES;
			break;
		}
		el = el->next;
	}
	
	if(found)
		return OCListRemoveItem(me, index);
	return FAILED;
}

OCStatus OCListAppend(OCListRef list, OCObjectRef obj){
	OCListElementPtr el = OCAllocate(ocDefaultAllocator, sizeof(struct OCListElement));
	if(el == NULL)
		return FAILED;
	el->item = OCObjectRetain(obj);

	if(list->size == 0)
	{
		// this is the first list element
		el->next = NULL;
		el->prev = NULL;
		list->head = el;
		list->tail = el;		
	}
	else
	{
		el->next = NULL;
		el->prev = list->tail;
		
		list->tail->next = el;
		list->tail = el;
	}
	list->size++;
	list->changeCount++;
	
	return SUCCESS;
}

OCStatus OCListAppendList(OCListRef me, OCListRef src){

	if(me->tail)
		me->tail->next = src->head;
	if(src->head)
		src->head->prev = me->tail;

	if(me->head == NULL)
		me->head = src->head;
	me->tail = src->tail;

	me->changeCount++;
	me->size += src->size;	
	
	src->head = NULL;
	src->tail = NULL;
	src->size = 0;
	src->changeCount++;

	return SUCCESS;
}


#pragma mark FIFO
OCStatus OCListFifoPush(OCListRef me, OCObjectRef obj){
	OCListElementPtr el = OCAllocate(ocDefaultAllocator, sizeof(struct OCListElement));
	if(el == NULL)
		return FAILED;
	el->item = OCObjectRetain(obj);
	if(el->item == NULL)
		return FAILED;
	
	if(me->size == 0)
	{
		// this is the first list element
		el->next = NULL;
		el->prev = NULL;
		me->head = el;
		me->tail = el;		
	}
	else
	{
		el->next = me->head;
		el->prev = NULL;
		
		me->head->prev = el;

		me->head = el;
	}
	me->size++;
	me->changeCount++;	

	return SUCCESS;
}

OCObjectRef OCListFifoPop(OCListRef me){
	if(me->size == 0)
		return NULL;
	if(me->size == 1)
	{
		OCListElementPtr el = me->tail;
		me->head = NULL;
		me->tail = NULL;
		me->size = 0;
		
		OCObjectRef item = el->item;
//		OCObjectRelease(&el->item);
		OCDeallocate(me->_obj.allocator, el);

		return item;
	}
	else
	{
		OCListElementPtr el = me->tail;
		me->tail = el->prev;
		el->prev->next = NULL;
		me->size--;
		
		OCObjectRef item = el->item;
//		OCObjectRelease(&el->item);
		OCDeallocate(me->_obj.allocator, el);

		return item;
	}
}

void OCListIteratorDealloc(void* obj){
	OCListIteratorRef iter = (OCListIteratorRef) obj;
	OCObjectRelease((OCObjectRef*) &(iter->list));
}

OCListIteratorRef OCListCreateIterator(OCListRef list){
	OCObjectInfo info = {sizeof(struct OCListIterator), OCListIteratorDealloc};
	OCListIteratorRef iter = (OCListIteratorRef) OCObjectCreateObjectWithAllocator(ocDefaultAllocator, info);
	
	if(iter == NULL)
		return NULL;
	
	iter->list = (OCListRef) OCObjectRetain((OCObjectRef)list);
	iter->next = list->head;
	iter->lastChangeCount = list->changeCount;
	
	return iter;
}


OCObjectRef OCListIteratorNext(OCListIteratorRef iter){
	OCListRef list = iter->list;
	if(list->changeCount != iter->lastChangeCount)
	{
		OCLog("OCListIterator Error: List was altered since iterator was created");
		return NULL;
	}
	
	if(iter->next != NULL)
	{
		OCObjectRef item = iter->next->item;
		iter->next = iter->next->next;
		return item;
	}
	
	return NULL;
}

OCStatus OCListIteratorReset(OCListIteratorRef iter){
	OCListRef list = iter->list;
	iter->next = list->head;
	iter->lastChangeCount = list->changeCount;
	
	return SUCCESS;
}


