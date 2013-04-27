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

#ifndef _AL_PROTOCOL_TR_
#define _AL_PROTOCOL_TR_

// *TR implements the protocol described in the tech report


#include "ALProtocol.h"
#include "OCBase.h"
#include "OCString.h"
#include "ALChunkTR.h"
#include "DJ.h"
#include "TreeDecrypter.h"
#include "ALPayloadChunkTR.h"

typedef struct ALProtocolTR* ALProtocolTRRef;

ALProtocolTRRef ALProtocolTRCreateNewInstance(int modulusBits, int s);
ALProtocolTRRef ALProtocolTRCreateInstanceFromFile(OCStringRef path);

// part of the interface:
// ALProtocolTR instance management
ALProtocolTRRef ALProtocolTRCreateInstanceFromFile(OCStringRef path);
OCStatus ALProtocolTRWritePublicInstanceToFile(ALProtocolTRRef alp, OCStringRef path);
OCStatus ALProtocolTRWritePrivateInstanceToFile(ALProtocolTRRef alp, OCStringRef path);

ALProtocolType ALProtocolTRGetType(ALProtocolTRRef alp);

size_t ALProtocolTRGetMaxPayloadBytes(ALProtocolTRRef me);

// ALProtocolTR tier-2 features
// aggregation chunk "a" with "withB" to "a"
OCStatus ALProtocolTRAggregateChunk(ALProtocolTRRef alp, ALChunkTRRef a, ALChunkTRRef withB);
// return the bucket index for this chunk for a total of nBuckets
size_t ALProtocolTRBucketIndexForChunk(ALProtocolTRRef alp, ALChunkTRRef chunk, size_t nBuckets);

// create and return a neutral chunk (regarding aggregation)
ALChunkTRRef ALProtocolTRCreateNeutralChunk(ALProtocolTRRef alp);
// make chunk "chunk" neutral to aggregation
OCStatus ALProtocolTRMakeChunkNeutral(ALProtocolTRRef alp, ALChunkTRRef chunk);


// ALProtocolTR tier-3 features
TreeDecrypterRef ALProtocolTRCreateTreeDecrypter(ALProtocolTRRef alp, int nLeaves);
TDItemPtr ALProtocolTRGetTDItemForChunk(ALProtocolTRRef alp, ALChunkTRRef chunk);
void ALProtocolTRTreeDecrypterPostprocess(ALProtocolTRRef alp, ALChunkTRRef chunk, TDItemPtr item);

OCBool ALProtocolTRT3DecTestNeutral(ALProtocolTRRef alp, ALChunkTRRef chunk);
OCStatus ALProtocolTRTreeDecrypterDecVrfyRest(ALProtocolTRRef alp, 
											  ALChunkTRRef chunk, 
											  ALPayloadChunkTRRef* pChunk);

// various other features
void ALProtocolTRPrintInfo(ALProtocolTRRef alp);

OCStatus ALProtocolTRDecrypt(ALProtocolTRRef alp, ALChunkTRRef chunk);

// the protocol algorithms
ALChunkTRRef ALProtocolTREncData(ALProtocolTRRef me, OCByte* data, size_t nData);
ALChunkTRRef ALProtocolTREncNeutral(ALProtocolTRRef me);

OCStatus ALProtocolTRDecVrfy(ALProtocolTRRef alp, 
							 ALChunkTRRef chunk);
//OCBool ALProtocolTRVrfy(ALProtocolTRRef alp, 
//						OCByte* mBytes,
//						OCByte* krHex,
//						OCByte* gBytes,
//						OCByte* hBytes);

void ALProtocolTRTDDecTestNeutral(ALProtocolTRRef alp, ALChunkTRRef chunk);

OCBool ALProtocolTRIsChunkValid(ALProtocolTRRef alp, ALChunkTRRef chunk);

// other, not part of the interface
DJRef ALProtocolTRGetDJM(ALProtocolTRRef alp);
DJRef ALProtocolTRGetDJT(ALProtocolTRRef alp);


#endif

