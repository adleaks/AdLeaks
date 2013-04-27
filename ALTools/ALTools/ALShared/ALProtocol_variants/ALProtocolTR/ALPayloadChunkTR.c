/* This file is part of AdLeaks.
 * Copyright (C) 2013 Benjamin Güldenring
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

#include "ALPayloadChunkTR.h"

struct ALPayloadChunkTR{
	struct OCObject _obj;
	OCByte* m;
	size_t mLength;
	OCByte* r0;
	size_t r0Length;
};

ALPayloadChunkTRRef ALPayloadChunkTRCreate(	OCByte* m, size_t mLength,
										OCByte* r0, size_t r0Length){
	VAR_UNUSED(m);
	VAR_UNUSED(mLength);
	VAR_UNUSED(r0);
	VAR_UNUSED(r0Length);
	
	return NULL;
}


OCStatus ALPayloadChunkTRSerialize(ALPayloadChunkTRRef me, OCSerializerRef s){

	OCSerializerWriteBytes(s, me->m, me->mLength);
	OCSerializerWriteBytes(s, me->r0, me->r0Length);
	
	return SUCCESS;
}

OCStatus ALPayloadChunkTRGetKey(ALPayloadChunkTRRef me, OCByte** key, size_t* keyLength){
	VAR_UNUSED(me);
	VAR_UNUSED(key);
	VAR_UNUSED(keyLength);
	
	return FAILED;
}
