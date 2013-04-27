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

#ifndef _AL_PROTOCOL_H_
#define _AL_PROTOCOL_H_

#include "ALToolsConfig.h"

#include "OCBase.h"
#include "OCList.h"
#include "OCString.h"


typedef enum {
	PUBLIC = 0,
	PRIVATE = 1
} ALProtocolType;

typedef long unsigned int ALProtocolSerial; 

#ifndef MERGE
#define MERGE(X, Y) X ## Y
#endif
#ifndef HELPER
#define HELPER(X, Y) MERGE(X, Y)
#endif

#ifdef CHOOSE_ALP_1C
	#define ALProtocol ALProtocol1C
	#define ALProtocolRef HELPER(ALProtocol, Ref)
	#include "ALProtocol_variants/ALProtocol1C/ALProtocol1C.h"
#elif defined CHOOSE_ALP_TR
	#define ALProtocol ALProtocolTR
	#define ALProtocolRef HELPER(ALProtocol, Ref)
	#include "ALProtocol_variants/ALProtocolTR/ALProtocolTR.h"
#endif

#include "ALChunk.h"

#define ALProtocolCreateInstanceFromFile HELPER(ALProtocol, CreateInstanceFromFile)
#define ALProtocolWritePublicInstanceToFile HELPER(ALProtocol, WritePublicInstanceToFile)
#define ALProtocolWritePrivateInstanceToFile HELPER(ALProtocol, WritePrivateInstanceToFile)

#define ALProtocolGetType HELPER(ALProtocol, GetType)
#define ALProtocolGetSerial HELPER(ALProtocol, GetSerial)

#define ALProtocolGetMaxPayloadBytes HELPER(ALProtocol, GetMaxPayloadBytes)

#define ALProtocolAggregateChunk HELPER(ALProtocol,  AggregateChunk)
#define ALProtocolBucketIndexForChunk HELPER(ALProtocol,  BucketIndexForChunk)
#define ALProtocolCreateNeutralChunk HELPER(ALProtocol,  CreateNeutralChunk)
#define ALProtocolMakeChunkNeutral HELPER(ALProtocol,  MakeChunkNeutral)

#define ALProtocolCreateTreeDecrypter HELPER(ALProtocol, CreateTreeDecrypter)
#define ALProtocolGetTDItemForChunk HELPER(ALProtocol, GetTDItemForChunk)
#define ALProtocolTreeDecrypterPostprocess HELPER(ALProtocol, TreeDecrypterPostprocess)
#define ALProtocolTreeDecrypterDecVrfyRest HELPER(ALProtocol,  TreeDecrypterDecVrfyRest)

#define ALProtocolTDCreateNeutralChunk HELPER(ALProtocol, TDCreateNeutralChunk)
#define ALProtocolTDDecTestNeutral HELPER(ALProtocol,  TDDecTestNeutral)
#define ALProtocolTDIsNeutralChunk HELPER(ALProtocol, TDIsNeutralChunk)
#define ALProtocolTDCombineChunks HELPER(ALProtocol, TDCombineChunks)
#define ALProtocolTDUncombineChunks HELPER(ALProtocol, TDUncombineChunks)

#define ALProtocolPrintInfo HELPER(ALProtocol,  PrintInfo)

#define ALProtocolEncData HELPER(ALProtocol,  EncData)
#define ALProtocolEncNeutral HELPER(ALProtocol,  EncNeutral)
#define ALProtocolDecVrfy HELPER(ALProtocol,  DecVrfy)

#define ALProtocolIsChunkValid HELPER(ALProtocol, IsChunkValid)

#define ALProtocolDecrypt HELPER(ALProtocol, Decrypt)

#endif
