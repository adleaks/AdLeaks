/*
 *  ALProtocol_interface.h
 *  ALTools
 *
 *  Created by Benjamin Güldenring on 28.08.12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _AL_PROTOCOL_INTERFACE_H_
#define _AL_PROTOCOL_INTERFACE_H_
/*
#include "ALChunk.h"

//typedef struct ALProtocol* ALProtocolRef;

// ALProtocol instance management
ALProtocolRef ALProtocolCreateInstanceFromFile(OCStringRef path);
OCStatus ALProtocolWritePublicInstanceToFile(ALProtocolRef alp, OCStringRef path);
OCStatus ALProtocolWritePrivateInstanceToFile(ALProtocolRef alp, OCStringRef path);

ALProtocolType ALProtocolGetType(ALProtocolRef alp);
ALProtocolSerial ALProtocolGetSerial(ALProtocolRef alp);

// ALProtocol tier-0 features
OCListRef ALProtocolCreateChunksFromPayloadFile(ALProtocolRef alp, OCStringRef path);

// ALProtocol tier-2 features
// aggregation chunk "a" with "withB" to "a"
OCStatus ALProtocolAggregateChunk(ALProtocolRef alp, ALChunkRef a, ALChunkRef withB);
// return the bucket index for this chunk for a total of nBuckets
size_t ALProtocolBucketIndexForChunk(ALProtocolRef alp, ALChunkRef chunk, size_t nBuckets);
// create and return a neutral chunk (regarding aggregation)
ALChunkRef ALProtocolCreateNeutralChunk(ALProtocolRef alp);
// make chunk "chunk" neutral to aggregation
OCStatus ALProtocolMakeChunkNeutral(ALProtocolRef alp, ALChunkRef chunk);


// ALProtocol tier-3 features
OCBool ALProtocolT3DecTestNeutral(ALProtocolRef alp, ALChunkRef chunk);
OCStatus ALProtocolT3DecVrfyRest(ALProtocolRef alp, 
								   ALChunkRef chunk, 
								   OCByte** to, 
								   size_t* nTo);

// various other features
void ALProtocolPrintInfo(ALProtocolRef alp);

ALChunkRef ALProtocolCreateEncryptedInt(ALProtocolRef alp, int i);


// the protocol algorithms
ALChunkRef ALProtocolEncData(ALProtocolRef me, OCByte* data, size_t nData);
ALChunkRef ALProtocolEncNeutral(ALProtocolRef me);
void ALProtocolSeal(ALProtocolRef me, ALChunkRef chunk);
OCStatus ALProtocolDecVrfy(ALProtocolRef alp, 
							 ALChunkRef chunk, 
							 OCByte** to, 
							 size_t* nTo);
OCBool ALProtocolVrfy(ALProtocolRef alp, 
						OCByte* krHex,
						OCByte* mBytes,
						size_t nmBytes,
						OCByte* gBytes,
						OCByte* hBytes);

// other
void ALProtocolEncChunk(ALProtocolRef alp, ALChunkRef chunk);
void ALProtocolDecChunk(ALProtocolRef alp, ALChunkRef chunk);

*/

#endif
