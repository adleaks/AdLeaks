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

#include "Tier3Storage.h"
#include "OCThread.h"
#include "ALPayloadChunk.h"
#include <string.h>
#include <stdio.h>
#include "OCAutoreleasePool.h"

struct Tier3Storage{
	struct OCObject _obj;
	OCStringRef path;
	
	OCThreadRef thread;
	OCListRef newPLChunks;
	OCListRef toWritePLChunks;
	OCBool doQuitGracefully;
};

// private
void Tier3StorageMain(Tier3StorageRef me);

void Tier3StorageDealloc(void* _me){
	Tier3StorageRef me = (Tier3StorageRef)_me;
	OCRelease(&me->path);
	OCRelease(&me->thread);
	OCRelease(&me->newPLChunks);
	OCRelease(&me->toWritePLChunks);
}

Tier3StorageRef Tier3StorageCreate(OCStringRef path){
	OCOBJECT_ALLOCINIT(Tier3Storage);
	if(me == NULL)
		return me;
	
	me->path = OCRetain(path);
	
	OCThreadDelegate del;
	del.delegateObject = (OCObjectRef)me;
	del.exitCB = NULL;
	del.mainCB = (OCThreadMainCB)Tier3StorageMain;
	del.stopCB = NULL;
	me->thread = OCThreadCreate(del);
	
	me->newPLChunks = OCListCreate();
	me->toWritePLChunks = OCListCreate();
	me->doQuitGracefully = NO;
	
	return me;
}

void Tier3StorageMain(Tier3StorageRef me){
	
	do
	{
		if(me->doQuitGracefully)
		{
			OCObjectLock((OCObjectRef)me->newPLChunks);
			if(OCListGetCount(me->newPLChunks) == 0 &&
			   OCListGetCount(me->toWritePLChunks) == 0)
			{
				OCObjectUnlock((OCObjectRef)me->newPLChunks);
				break;
			}
			OCObjectUnlock((OCObjectRef)me->newPLChunks);
		}
		
		OCObjectLock((OCObjectRef)me->newPLChunks);
		if(OCListGetCount(me->newPLChunks) == 0)
		{
			OCObjectUnlock((OCObjectRef)me->newPLChunks);
			OCThreadSleep(me->thread, 1);
			continue;
		}
		OCListRef _tmp = me->newPLChunks;
		me->newPLChunks = me->toWritePLChunks;
		me->toWritePLChunks = _tmp;
		OCObjectUnlock((OCObjectRef)me->newPLChunks);
		
		// do the writing here
		// NOTE usually we would use a lock file here,
		// however: we are the only process _writing_ to any file - 
		// the worker tool should only read or delete a file,
		// since we can write to a deleted file (posix) we are fine
		// in the case of concurrent access to the same file the worker
		// may fail on this one, which is ok for us
		ALPayloadChunkRef plc = NULL;
		OCAutoreleasePoolCreate();
		
		OCSerializerRef s = (OCSerializerRef)OCAutorelease(OCSerializerCreate());
		while( (plc=(ALPayloadChunkRef)OCListFifoPop(me->toWritePLChunks)) != NULL)
		{
			OCAutorelease(plc);
			
			OCByte* key;
			size_t keyLength;			
			ALPayloadChunkGetKey(plc, &key, &keyLength);
			
			OCByte* keyHexBytes = OCAllocate(ocDefaultAllocator, 2*keyLength+1);
			if(keyHexBytes == NULL)
				continue;
			bzero(keyHexBytes, 2*keyLength+1);
			OCBytes2Hex(keyHexBytes, key, keyLength);
			
			OCStringRef keyStr = (OCStringRef)OC((char*)keyHexBytes);
			if(keyStr == NULL)
			{
				OCDeallocate(ocDefaultAllocator, keyHexBytes);
				continue;
			}

			OCStringRef pathStr = (OCStringRef)OCAutorelease(OCStringCreateAppended(me->path, keyStr));
			if(pathStr == NULL)
			{
				OCDeallocate(ocDefaultAllocator, keyHexBytes);
				continue;
			}
			
			ALPayloadChunkSerialize(plc, s);
			FILE* fp = fopen(CO(pathStr), "a");
			if(fp == NULL)
			{
				OCLog("Storage: Could not open file at %s", CO(pathStr));
				OCDeallocate(ocDefaultAllocator, keyHexBytes);
				continue;
			}
			
			OCSerializerWriteToStream(s, fp);
			fclose(fp);
			
			OCDeallocate(ocDefaultAllocator, keyHexBytes);
			OCSerializerReset(s);
		}
		OCRelease(&me->toWritePLChunks);

		OCAutoreleasePoolDestroy();
		me->toWritePLChunks = OCListCreate();
	}
	while(1);
}


OCStatus Tier3StorageStart(Tier3StorageRef me){
	return OCThreadStart(me->thread);
}

OCStatus Tier3StorageStop(Tier3StorageRef me){
	me->doQuitGracefully = YES;
	return OCThreadStop(me->thread);
}

OCStatus Tier3StorageWaitTillStopped(Tier3StorageRef me){
	return OCThreadJoin(me->thread);
}

OCStatus Tier3StorageStore(Tier3StorageRef me, ALPayloadChunkRef plc){
	if(me->doQuitGracefully)
		return FAILED;
	OCObjectLock((OCObjectRef)me->newPLChunks);
	OCListAppend(me->newPLChunks, (OCObjectRef)plc);
	OCObjectUnlock((OCObjectRef)me->newPLChunks);
	return SUCCESS;
}

