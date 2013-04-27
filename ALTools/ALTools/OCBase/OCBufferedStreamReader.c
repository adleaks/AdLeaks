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

#include <string.h>
#include <unistd.h>	// for read()
#include <stdio.h>

#include "OCBufferedStreamReader.h"
#include "OCObject.h"

struct OCBufferedStreamReader{
	struct OCObject _obj;
	int socket;
	
	/*const*/ size_t nBuffer; // const

	OCByte* buffer;
	OCByte* buffer2;
	OCByte* bufferReadPtr;
	
	size_t nLeftToRead;
};

void OCBufferedStreamReaderDealloc(void* ptr){
	OCBufferedStreamReaderRef me = (OCBufferedStreamReaderRef) ptr;
	OCDeallocate(me->_obj.allocator, me->buffer);
	OCDeallocate(me->_obj.allocator, me->buffer2);
}

OCBufferedStreamReaderRef OCBufferedStreamReaderCreateFromSocket(int socket, size_t bufferSize){
	OCOBJECT_ALLOCINIT(OCBufferedStreamReader);

	me->nBuffer= bufferSize;
	me->socket = socket;
	me->buffer = OCAllocate(ocDefaultAllocator, me->nBuffer);
	me->buffer2 = OCAllocate(ocDefaultAllocator, me->nBuffer);
	me->bufferReadPtr = me->buffer;
	
	me->nLeftToRead = 0;

	return me;
}

// return number of bytes written, EOF (-1) when stream ended 
int OCBufferedStreamReaderRead(OCBufferedStreamReaderRef me, OCByte* buf, size_t n){
	if(me->nLeftToRead > n)
	{
		memcpy(buf, me->bufferReadPtr, n);
		me->bufferReadPtr += n;
		me->nLeftToRead -= n;
		return n;
	}
	else
	{
		if(n > me->nBuffer)
			return -1;
		
		// copy to begin of second buffer
		memcpy(me->buffer2, me->bufferReadPtr, me->nLeftToRead);
		// swap buffers
		OCByte* oldbuffer = me->buffer;
		OCByte* oldbuffer2 = me->buffer2;
		me->buffer = oldbuffer2;
		me->buffer2 = oldbuffer;
		
		// set read ptr to begin of new buffer
		me->bufferReadPtr = me->buffer;			
		// fill buffer with rest
		int nToWrite = me->nBuffer - me->nLeftToRead;
		OCByte* writePtr = me->bufferReadPtr + me->nLeftToRead;
		int nWritten = read(me->socket, writePtr, nToWrite);
		if(nWritten <= 0)
			return nWritten;
		else
		{
			me->nLeftToRead+=nWritten;
			return OCBufferedStreamReaderRead(me, buf, n);
		}
	}
}

