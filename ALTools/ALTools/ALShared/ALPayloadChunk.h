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

#ifndef _AL_PAYLOAD_CHUNK_H_
#define _AL_PAYLOAD_CHUNK_H_

#include "ALToolsConfig.h"

#ifndef MERGE
#define MERGE(X, Y) X ## Y
#endif
#ifndef HELPER
#define HELPER(X, Y) MERGE(X, Y)
#endif


#ifdef CHOOSE_ALP_1C
	#define ALPayloadChunk ALPayloadChunk1C
	#define ALPayloadChunkRef HELPER(ALPayloadChunk, Ref)
	#include "ALPayloadChunk1C.h"
#elif defined CHOOSE_ALP_TR
	#define ALPayloadChunk ALPayloadChunkTR
	#define ALPayloadChunkRef HELPER(ALPayloadChunk, Ref)
	#include "ALPayloadChunkTR.h"
#endif

#define ALPayloadChunkSerialize HELPER(ALPayloadChunk, Serialize)
#define ALPayloadChunkGetKey HELPER(ALPayloadChunk, GetKey)


#endif
