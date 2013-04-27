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

#ifndef _AL_CHUNK_H_
#define _AL_CHUNK_H_

#include "ALToolsConfig.h"

#ifndef MERGE
#define MERGE(X, Y) X ## Y
#endif
#ifndef HELPER
#define HELPER(X, Y) MERGE(X, Y)
#endif
// choose the chunk implementation
#ifdef CHOOSE_ALP_1C
	#define ALChunk ALChunk1C
	#define ALChunkRef HELPER(ALChunk, Ref)
	#include "ALChunk1C.h"
#elif defined CHOOSE_ALP_TR
	#define ALChunk ALChunkTR
	#define ALChunkRef HELPER(ALChunk, Ref)
	#include "ALChunkTR.h"
#endif


#define ALChunkSerialize HELPER(ALChunk, Serialize)
#define ALChunkDeserialize HELPER(ALChunk, Deserialize)
#define ALChunkCreateFromBase64 HELPER(ALChunk, CreateFromBase64)
#define ALChunkPrint HELPER(ALChunk, Print)
#define ALChunkSet HELPER(ALChunk, Set)

#endif
