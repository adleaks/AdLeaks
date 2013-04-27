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

#include "OCString.h"

#include "OCObject.h"
#include "OCList.h"
#include "OCLog.h"

struct OCString{
	struct OCObject _obj; 
	
	char* theString;
	size_t length;	// not counting the terminating '\0'
};

void OCStringDealloc(void* ptr){
	OCStringRef s = (OCStringRef) ptr;
	OCDeallocate(ocDefaultAllocator, (s->theString));
}

OCStringRef OCStringCreateWithCString(const char* cstring){
	size_t length = strlen(cstring);
	
	OCStringRef me = OCStringCreateWithCharsAndLength(cstring, length);
	
	return me;
}

OCStringRef OCStringCreateCopy(OCStringRef s){
	return OCStringCreateWithCString(s->theString);
}

size_t OCStringGetLength(OCStringRef me){
	return me->length;
}


OCStringRef OCStringCreateWithCharsAndLength(const char* chars, size_t length){
	OCOBJECT_ALLOCINIT(OCString);

	me->length = length;
	me->theString = OCAllocate(ocDefaultAllocator, me->length+1);
	me->theString[me->length] = '\0';
	memcpy(me->theString, chars, length);
	
	return me;
}


OCStringRef OCStringCreateSubString(OCStringRef me, size_t begin, size_t end){
	if(begin > end)
		return NULL;
	
	if(end > me->length)
		return NULL;
	
	// from now on: begin <= end < me->length
	
	OCStringRef newStr = NULL;
	char* beginPtr = me->theString+begin;
	size_t length = end-begin+1;
	newStr = OCStringCreateWithCharsAndLength(beginPtr, length);
	
	return newStr;
}



const char* OCStringGetCString(OCStringRef cs){	
	return cs->theString;
}

OCStringRef OCStringCreateAppended(OCStringRef s1, OCStringRef s2){
	OCOBJECT_ALLOCINIT(OCString);
	if(me == NULL)
		return NULL;
	
	me->theString = OCAllocate(ocDefaultAllocator, s1->length+s2->length+1);

	me->length = s1->length+s2->length;
	memcpy(me->theString, s1->theString, s1->length);
	memcpy(me->theString+s1->length, s2->theString, s2->length);
	me->theString[me->length] = '\0';

	return me;	
}

OCListRef OCStringSplitByToken(OCStringRef s, char token){

	if(s->length < 1)
		return NULL;

	OCListRef list = OCListCreate();

	if(token == '\0')
	{
		OCStringRef found = OCStringCreateCopy(s);
		OCListAppend(list, (OCObjectRef)found);
		OCObjectRelease(&found);
		
		return list;
	}
	
	char* begin = s->theString;
	char* end = NULL;
	while(begin != NULL)
	{
		end = strchr(begin, token);
		if(end == NULL)
			break;
		size_t foundLength = (end-begin);
		if(foundLength != 0)
		{
			OCStringRef found = OCStringCreateWithCharsAndLength(begin, foundLength);
			OCListAppend(list, (OCObjectRef)found);
			OCObjectRelease(&found);
		}
		
		// the token is != '\0' according to the check
		// above. so 
		//	*  end is either NULL (which is handled 
		//	    above and we wouln't have got here)
		//	* end points at a char preceding the
		//	   the terminating \0, so end+1 is
		//	   a valid pointer		
		begin = end+1;	
	}
	
	if(OCListGetCount(list) == 0)
	{
		OCObjectRelease(&list);
		list = NULL;
	}
	else
	{
		end = s->theString + s->length;
		size_t foundLength = (end-begin);
		if(foundLength != 0)
		{
			OCStringRef found = OCStringCreateWithCharsAndLength(begin, foundLength);
			OCListAppend(list, (OCObjectRef)found);
			OCObjectRelease(&found);
		}
	}
	
	return list;
}


OCBool OCStringIsEqual(OCStringRef me, OCStringRef with){
	if(me->length != with->length)
		return NO;
	
	int eq = memcmp(me->theString, with->theString, me->length);
	
	if(eq == 0)
		return YES;
	return NO;
}

OCBool OCStringIsEqualCString(OCStringRef me, const char* cstring){
	size_t len = strlen(cstring);
	if(me->length != len)
		return NO;
	
	int eq = memcmp(me->theString, cstring, me->length);
	
	if(eq == 0)
		return YES;
	return NO;
}

