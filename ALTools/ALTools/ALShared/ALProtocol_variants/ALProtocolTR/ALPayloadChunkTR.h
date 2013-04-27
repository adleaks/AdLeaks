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

#ifndef _AL_PAYLOAD_CHUNK_TR_H_
#define _AL_PAYLOAD_CHUNK_TR_H_

#include "OCTypes.h"
#include "OCSerializer.h"

typedef struct ALPayloadChunkTR* ALPayloadChunkTRRef;


// this is a "module private" constructor
ALPayloadChunkTRRef ALPayloadChunkTRCreate(	OCByte* m, size_t mLength,
										   OCByte* r0, size_t r0Length);


OCStatus ALPayloadChunkTRSerialize(ALPayloadChunkTRRef me, OCSerializerRef s);
OCStatus ALPayloadChunkTRGetKey(ALPayloadChunkTRRef me, OCByte** key, size_t* keyLength);

#endif
