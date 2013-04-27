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

#include "OCDictionary.h"
#include "OCObject.h"
#include "OCLog.h"

typedef struct OCDictionaryEntry* OCDictionaryEntryRef;

struct OCDictionaryEntry{
	struct OCObject _obj;
	OCStringRef key;
	OCObjectRef value;
	size_t changeCount;
};

void OCDictionaryEntryDealloc(void* _me){
	OCDictionaryEntryRef me = (OCDictionaryEntryRef) _me;
	OCObjectRelease(&me->key);
	OCObjectRelease(&me->value);
}

OCDictionaryEntryRef OCDictionaryEntryCreate(OCStringRef key, OCObjectRef value){
	OCOBJECT_ALLOCINIT(OCDictionaryEntry);
	if(me == NULL)
		return me;
	
	me->key = (OCStringRef) OCRetain(key);
	me->value = (OCObjectRef) OCRetain(value);

	return me;
}


struct OCDictionary{
	struct OCObject _obj;
	
	OCListRef* entries;
	size_t nEntries;
	
	size_t changeCount;
};

struct OCDictionaryIterator{
	struct OCObject _obj;
	
	OCDictionaryRef dict;
	OCListIteratorRef listIter;
	size_t lastChangeCount;
	size_t nextIndex;
};


static size_t OCDictionaryHashFunction(OCStringRef obj){
	size_t h = 0;
	const char* str = OCStringGetCString(obj);
	size_t len = OCStringGetLength(obj);
	size_t i = 0;
	
	for (i = 0; i < len; i++)
		h = 31*h + str[i];
	return h;
}

void OCDictionaryDealloc(void* _me){
	OCDictionaryRef me = (OCDictionaryRef) _me;
	size_t i = 0;
	for(i=0; i < me->nEntries; i++)
		OCObjectRelease(&(me->entries[i]));
	OCDeallocate(ocDefaultAllocator, me->entries);
}

OCDictionaryRef OCDictionaryCreate(size_t sizeHint){
	OCOBJECT_ALLOCINIT(OCDictionary);
	if(me == NULL)
		return me;
	
	me->entries = OCAllocate(ocDefaultAllocator, sizeHint*sizeof(OCListRef));
	me->nEntries = sizeHint;
	me->changeCount = 0;
	size_t i = 0;
	for(i=0; i < sizeHint; i++)
	{
		me->entries[i] = OCListCreate();
		if(me->entries[i] == NULL)
		{
			OCLog("OCList: Out of memory?");
			exit(-1);
		}
	}
	
	return me;
}

OCObjectRef OCDictionaryGet(OCDictionaryRef me, OCStringRef key){
	size_t hash = OCDictionaryHashFunction(key);
	size_t index = hash % me->nEntries;
	
	OCDictionaryEntryRef found = 0;
	OCListIteratorRef iter = OCListCreateIterator(me->entries[index]);

	while( (found=(OCDictionaryEntryRef)OCListIteratorNext(iter)) != NULL)
		if(OCStringIsEqual(found->key, key))
			break;
	
	OCObjectRelease(&iter);
	if(found)
		return found->value;
	return NULL;
}

OCStatus OCDictionaryPut(OCDictionaryRef me, OCStringRef key, OCObjectRef value){
	size_t hash = OCDictionaryHashFunction(key);
	size_t index = hash % me->nEntries;

	OCDictionaryEntryRef found = 0;
	OCListIteratorRef iter = OCListCreateIterator(me->entries[index]);
	while( (found=(OCDictionaryEntryRef)OCListIteratorNext(iter)) != NULL)
		if(OCStringIsEqual(found->key, key))
			break;
	OCObjectRelease(&iter);
	if(found != NULL)
		return FAILED;
	
	OCDictionaryEntryRef entry = OCDictionaryEntryCreate(key, value);
	OCListAppend(me->entries[index], (OCObjectRef)entry);
	OCObjectRelease(&entry);
	
	me->changeCount++;
	
	return SUCCESS;
}

void OCDictionaryIteratorDealloc(void* _me){
	OCDictionaryIteratorRef me = (OCDictionaryIteratorRef)_me;
	OCObjectRelease(me->dict);
}

OCDictionaryIteratorRef OCDictionaryCreateIterator(OCDictionaryRef dict){
	OCOBJECT_ALLOCINIT(OCDictionaryIterator);
	if(me == NULL)
		return me;
	
	me->dict = (OCDictionaryRef) OCRetain(dict);
	me->lastChangeCount = dict->changeCount; 
	me->nextIndex = 0;
	me->listIter = NULL;
	
	return me;
}

OCObjectRef OCDictionaryIteratorNext(OCDictionaryIteratorRef me){
	if(me->lastChangeCount != me->dict->changeCount)
		return NULL;
	if(me->nextIndex >= me->dict->nEntries)
		return NULL;
	
	struct OCDictionaryEntry* result = NULL;
	if(me->listIter == NULL)
	{
		for( ; me->nextIndex < me->dict->nEntries; me->nextIndex++)
		{
			if(OCListGetCount(me->dict->entries[me->nextIndex]) > 0)
			{
				me->listIter = OCListCreateIterator(me->dict->entries[me->nextIndex]);
				break;
			}
		}
	}
	// me->listIter == NULL <=> no more entries left <=> me->nextIndex > nEntries
	
	if(me->listIter == NULL)
		return NULL;
	result = (struct OCDictionaryEntry*)OCListIteratorNext(me->listIter);
	if(result == NULL)
	{
		OCObjectRelease(&me->listIter);
		me->nextIndex++;
		return OCDictionaryIteratorNext(me);
	}
	return result->value;
}


