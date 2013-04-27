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

#include "OCSerializer.h"
#include "OCObject.h"

//TODO: this is evil and should really be fixed soon
#define NBUFFER 20000

struct OCSerializer{
	struct OCObject _obj;
	
	OCByte* buffer;
	OCByte* currentPtr;
	size_t nLeft;
	/*const*/ size_t nBuffer;
	size_t nInBuffer;
};

void OCSerializerDealloc(void* ptr){
	OCSerializerRef obj = (OCSerializerRef) ptr;
	OCDeallocate(obj->_obj.allocator, obj->buffer);
}

OCSerializerRef OCSerializerCreate(){
	OCOBJECT_ALLOCINIT(OCSerializer);	
	
	me->nBuffer = NBUFFER;
	me->nInBuffer = 0;
	me->nLeft = me->nBuffer;
	me->buffer = OCAllocate(ocDefaultAllocator, me->nBuffer);
	me->currentPtr = me->buffer;
	
	return me;
}

OCStatus OCSerializerWriteInt(OCSerializerRef s, int32_t toWrite){
	if(s->nLeft < 4)
		return FAILED;
	
	s->currentPtr[0] = (toWrite >> 24) & 0xff;
	s->currentPtr[1] = (toWrite >> 16) & 0xff;
	s->currentPtr[2] = (toWrite >> 8) & 0xff;
	s->currentPtr[3] = toWrite & 0xff;
	
	s->nLeft -= 4;
	s->currentPtr += 4;
	s->nInBuffer += 4;
	
	return SUCCESS;
}

OCStatus OCSerializerWriteBytes(OCSerializerRef s, OCByte* buf, size_t n){
	if(s->nLeft < n)
		return FAILED;
	
	memcpy(s->currentPtr, buf, n*sizeof(OCByte));
	
	s->nLeft -= n;
	s->currentPtr += n;
	s->nInBuffer += n;
	
	return SUCCESS;
}

size_t OCSerializerWriteToStream(OCSerializerRef s, FILE* fp){
	
	fwrite(s->buffer, sizeof(OCByte), s->nInBuffer, fp);
	
	return s->nInBuffer;
}

size_t OCSerializerGetNumberOfBytes(OCSerializerRef s){
	return s->nInBuffer;
}

size_t OCSerializerCopyBytes(OCSerializerRef s, OCByte* to){
	memcpy(to, s->buffer, s->nInBuffer);
	return s->nInBuffer;
}

void OCSerializerReset(OCSerializerRef s){
	s->currentPtr = s->buffer;
	s->nLeft = s->nBuffer;
	s->nInBuffer = 0;
}

#pragma mark OCDeserializer

struct OCDeserializer{
	struct OCObject _obj;
	
	int type;
	
	union
	{
		struct
		{
			OCByte* buffer;
			OCByte* currentPtr;
			size_t nLeft;
			size_t nBuffer;
		};
		OCBufferedStreamReaderRef sr;
	};
};

void OCDeserializerDealloc(void* ptr){
	OCDeserializerRef obj = (OCDeserializerRef) ptr;
	if(obj->type == 0)
		OCDeallocate(obj->_obj.allocator, obj->buffer);
	else
		OCObjectRelease(&obj->sr);
}

OCDeserializerRef OCDeserializerCreateWithBytes(OCByte* buf, size_t n){
	OCOBJECT_ALLOCINIT(OCDeserializer);
	if(me == NULL)
		return me;
	
	me->buffer = OCAllocate(ocDefaultAllocator, n);
	memcpy(me->buffer, buf, n*sizeof(OCByte));
	
	me->type = 0;
	
	me->currentPtr = me->buffer;
	me->nLeft = n;
	me->nBuffer = n;
	
	return me;
}

OCDeserializerRef OCDeserializerCreateFromFile(FILE* fp){
	OCByte buf[NBUFFER];
	
	fpos_t pos;
	fgetpos(fp, &pos);
	int read = fread(buf, sizeof(OCByte), NBUFFER, fp);;
	if(read >= NBUFFER)
	{
		fsetpos(fp, &pos);
		return NULL;
	}
	
	return OCDeserializerCreateWithBytes(buf, read);
}

OCDeserializerRef OCDeserializerCreateFromStream(OCBufferedStreamReaderRef sr){
	OCOBJECT_ALLOCINIT(OCDeserializer);
	
	me->type = 1;
	me->sr = (OCBufferedStreamReaderRef) OCObjectRetain((OCObjectRef)sr);
	
	return me;
}



OCStatus OCDeserializerReadInt(OCDeserializerRef s, int32_t* to){

	if(s->type == 0)
	{
		if(s->nLeft < 4)
			return FAILED;
	
		int i = 0;
		i |= (s->currentPtr[0] << 24);
		i |= (s->currentPtr[1] << 16);
		i |= (s->currentPtr[2] << 8);
		i |= s->currentPtr[3];
		*to = i;
	
		s->nLeft -= 4;
		s->currentPtr += 4;
	
		return SUCCESS;
	}
	else
	{
		int i = 0;
		OCByte buf[4];
		int count = OCBufferedStreamReaderRead(s->sr, buf, 4);
		if(count < 4)
			return FAILED;
		i |= (buf[0] << 24);
		i |= (buf[1] << 16);
		i |= (buf[2] << 8);
		i |= buf[3];
		*to = i;
		
		return SUCCESS;
	}
}

OCStatus OCDeserializerReadBytes(OCDeserializerRef s, OCByte* buf, size_t n){

	if(s->type == 0)
	{
		if(s->nLeft < n)
			return FAILED;
	
		memcpy(buf, s->currentPtr, n*sizeof(OCByte));
	
		s->nLeft -= n;
		s->currentPtr += n;
	
		return SUCCESS;
	}
	else
	{
		int count = OCBufferedStreamReaderRead(s->sr, buf, n);
		if(count != (int)n)
			return FAILED;
		else
			return SUCCESS;
	}
}


