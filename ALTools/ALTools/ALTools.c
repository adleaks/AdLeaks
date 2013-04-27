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

#include "ALToolsConfig.h"

#include "OCBase.h"
#include "ALProtocol.h"
#include "OCTime.h"
#include "TreeDecrypter.h"
#include "CryptoMac.h"
#include <string.h>
#include <math.h>
#include "TierConfig.h"
#include "libs/libb64/cdecode.h"
#include <unistd.h>
#include "CryptoRandom.h"
#include "ALPayloadChunk.h"

#define N_BENCHMARK 2

OCStringRef getDefaultConfigPath(void){
	OCStringRef path = NULL;
	OCAutoreleasePoolCreate();
	path = OCStringCreateAppended(OCBaseGetHomeDirectory(), OC("/adleaks/adleaks.conf"));	
	OCAutoreleasePoolDestroy();
	return path;
}

void printUsage(char* execName){
	OCLog("Usage: %s -f <path> [-c | -p | -b <type> | -d <base64> [-r]]", execName);
	OCLog("  -f the <path> to the protocol file");
	OCLog("  -c create a new protocol at <path>");
	OCLog("  -p print the protocol info from <path>");
	OCLog("  -d decrypt a base64 encoded chunk (needs the private key)");
	OCLog("	   * -r write the raw contents to stdout");
	OCLog("  -b benchmark, the possible values for <type> are:");
	OCLog("      * proto2: benchmark public protocol");
	OCLog("      * proto3: benchmark private protocol (needs the private key)");
	OCLog("      * crypto: benchmark crypto (not supported yet)");
#if defined(CHOOSE_ALP_1C)
	OCLog("NOTE: The 1C protocol is the only one supported in this build.");
#elif defined(CHOOSE_ALP_TR)
	OCLog("NOTE: The TR protocol is the only one supported in this build.");
#endif
}

struct commandline{
	int cFlag;
	int pFlag;
	int bFlag;
	int dFlag;
	int rFlag;
	OCStringRef protocolFile;
	OCStringRef benchmark;
	OCStringRef base64Chunk;
};
OCStatus readCommandline(int argc, char** argv, struct commandline* cmd ){
	int c = 0;
	
	while( (c=getopt(argc, argv, "hcpb:d:f:r")) != -1)
	{
		switch(c)
		{
			case 'c': cmd->cFlag = 1; break;
			case 'p': cmd->pFlag = 1; break;
			case 'b':
				cmd->bFlag = 1; 
				cmd->benchmark = OCStringCreateWithCString(optarg);
			break;
			case 'd':
				cmd->dFlag = 1; 
				cmd->base64Chunk = OCStringCreateWithCString(optarg);
			break;
			case 'f':
				cmd->protocolFile = OCStringCreateWithCString(optarg);
			break;
			case 'r':
				cmd->rFlag = 1;
			break;
			case 'h':
				printUsage(argv[0]);
			break;
			default:
				return FAILED;
			break;
		}
	}
	
	return SUCCESS;
}

void doProto2Benchmark(ALProtocolRef alp);
void doProto3Benchmark(ALProtocolRef alp);
void doCryptoBenchmark(ALProtocolRef alp);
void doDecryption(ALProtocolRef alp, OCStringRef base64Chunk, OCBool doBinary);

int main(int argc, char *argv[])
{
	OCStatus status = FAILED;	
	OCAutoreleasePoolCreate();
	do
	{
		struct commandline cmd;
		bzero(&cmd, sizeof(struct commandline));
		if(argc == 1)
		{
			printUsage(argv[0]);
			break;
		}
		if(readCommandline(argc, argv, &cmd) == FAILED)
		{
			printUsage(argv[0]);
			break;
		}
		if(cmd.protocolFile == NULL)
		{
			OCLog("You must specify a protocol file");
			break;
		}
		int nFlags = 0;
		if(cmd.cFlag) nFlags++;
		if(cmd.pFlag) nFlags++;
		if(cmd.bFlag) nFlags++;
		if(cmd.dFlag) nFlags++;
		if(nFlags == 0)
		{
			printUsage(argv[0]);
			break;
		}
		if(nFlags != 1)
		{
			OCLog("You must specify exactly one of [-c -p -b -d]");
			printUsage(argv[0]);
			break;
		}
		if(cmd.cFlag)
		{
#if defined(CHOOSE_ALP_1C)
			OCLog("Creating protocol with 2048 bits modulus and s=1");
			ALProtocolRef newalp = ALProtocol1CCreateNewInstance(2028, 1);
#elif defined(CHOOSE_ALP_TR)
			OCLog("Creating protocol with 2048 bits modulus and s=2");
			ALProtocolRef newalp = ALProtocolTRCreateNewInstance(2048, 2);
#endif
			status = SUCCESS;
			OCStringRef pubPath = (OCStringRef)OCAutorelease(OCStringCreateAppended(cmd.protocolFile, OC(".pub")));
			OCStringRef prvPath = (OCStringRef)OCAutorelease(OCStringCreateAppended(cmd.protocolFile, OC(".prv")));
			if( (status &= ALProtocolWritePublicInstanceToFile(newalp, pubPath)) != SUCCESS)
				OCLog("Error writing public instance to file %s", CO(pubPath));
			if( (status &= ALProtocolWritePrivateInstanceToFile(newalp, prvPath)) != SUCCESS)
				OCLog("Error writing public instance to file %s", CO(prvPath));
			break;
		}
		
		ALProtocolRef alp = ALProtocolCreateInstanceFromFile(cmd.protocolFile);
		OCAutorelease(alp);
		if(alp == NULL)
		{
			OCLog("Can not load protocol file %s", CO(cmd.protocolFile));
			status = FAILED;
			break;
		}
		if(cmd.pFlag)
			ALProtocolPrintInfo(alp);
		else if(cmd.bFlag)
		{
			if(OCStringIsEqual(cmd.benchmark, OC("proto2")))
				doProto2Benchmark(alp);				
			else if(OCStringIsEqual(cmd.benchmark, OC("proto3")))
				doProto3Benchmark(alp);				
			else if(OCStringIsEqual(cmd.benchmark, OC("crypto")))
				doCryptoBenchmark(alp);
			else
				OCLog("Unknown benchmark type: %s", CO(cmd.benchmark));
		}
		else if(cmd.dFlag)
			doDecryption(alp, cmd.base64Chunk, cmd.rFlag);
		
		status = SUCCESS;
	}while(0);
	OCAutoreleasePoolDestroy();
	
	return OCExit((status == SUCCESS) ? 0 : -1);
}

#define N N_BENCHMARK
void doProto2Benchmark(ALProtocolRef alp){
	OCTimeInterval encTimes[N];
	
	OCByte* bytes[N];
	size_t nBytes = ALProtocolGetMaxPayloadBytes(alp);
	ALChunkRef chunks[N];
	
	mpz_t m;	mpz_init(m);

	OCByte* buf = OCAllocate(ocDefaultAllocator, nBytes);
	
	int i = 0;
	for(i=0; i < N; i++)
	{
		bytes[i] = OCAllocate(ocDefaultAllocator, nBytes);
		CryptoRandomRand(bytes[i], nBytes, CryptoRandomSourceDevURandom);
		mpz_import(m, nBytes, 1, 1, 1, 0, buf);

		OCTimeInterval begin = OCTimeIntervalSinceReferenceDate();
		chunks[i] = ALProtocolEncData(alp, bytes[i], nBytes);
		OCTimeInterval end = OCTimeIntervalSinceReferenceDate();
		encTimes[i] = end-begin;
	}
		
	OCLog("Encrypt:");
	for(i=0; i < N; i++) OCLog("%f", encTimes[i]);
}

void doProto3Benchmark(ALProtocolRef alp){
	OCTimeInterval encTimes[N];
	OCTimeInterval aggregateTimes[N];
	OCTimeInterval tdnTimes[N];
	OCTimeInterval decVrfyTimes[N];
	
	OCByte* bytes[N];
	size_t nBytes = ALProtocolGetMaxPayloadBytes(alp);
	ALChunkRef chunks[N];
	ALChunkRef echunks[N];
	
	mpz_t m;	mpz_init(m);
		
	int i = 0;
	for(i=0; i < N; i++)
	{
		bytes[i] = OCAllocate(ocDefaultAllocator, nBytes);
		CryptoRandomRand(bytes[i], nBytes, CryptoRandomSourceDevURandom);
		
		OCTimeInterval begin = OCTimeIntervalSinceReferenceDate();
		chunks[i] = ALProtocolEncData(alp, bytes[i], nBytes);
		echunks[i] = ALProtocolEncNeutral(alp);
		OCTimeInterval end = OCTimeIntervalSinceReferenceDate();
		encTimes[i] = (end-begin)/2;
	}
	
	for(i=0; i < N; i++)
	{
		OCTimeInterval begin = OCTimeIntervalSinceReferenceDate();
		ALProtocolAggregateChunk(alp, chunks[i], echunks[i]);		
		OCTimeInterval end = OCTimeIntervalSinceReferenceDate();
		aggregateTimes[i] = end-begin;
	}	
	
	for(i=0; i < N; i++)
	{
		OCTimeInterval begin = OCTimeIntervalSinceReferenceDate();
		ALProtocolTDDecTestNeutral(alp, chunks[i]);
		OCTimeInterval end = OCTimeIntervalSinceReferenceDate();
		tdnTimes[i] = end-begin;
	}
	
	for(i=0; i < N; i++)
	{
		OCTimeInterval begin = OCTimeIntervalSinceReferenceDate();
		ALPayloadChunkRef plc = NULL;
		OCStatus ok = ALProtocolTreeDecrypterDecVrfyRest(alp, chunks[i], &plc);

		OCTimeInterval end = OCTimeIntervalSinceReferenceDate();
		decVrfyTimes[i] = end-begin;
		if(ok == FAILED)
			OCLogError("DecVrfy failed");
	}	
	OCLog("Encrypt:");
	for(i=0; i < N; i++) OCLog("%f", encTimes[i]);
	OCLog("Decryption for neutral test:");
	for(i=0; i < N; i++) OCLog("%f", tdnTimes[i]);
	OCLog("DecVrfy:");
	for(i=0; i < N; i++) OCLog("%f", decVrfyTimes[i]);
}

void doCryptoBenchmark(ALProtocolRef alp){
	VAR_UNUSED(alp);
	OCLog("Crypto benchmark not implemented yet, please refer to the seperate benchmark");	
}

void doDecryption(ALProtocolRef alp, OCStringRef base64Chunk, OCBool doBinary){
	VAR_UNUSED(doBinary);
	ALChunkRef chunk = ALChunkCreateFromBase64(base64Chunk);
	if(chunk == NULL)
	{
		OCLog("chunk invalid");
		return;
	}
	
	if(ALProtocolIsChunkValid(alp, chunk) == NO)
	{
		OCLog("chunk invalid");
		return;
	}
	
	OCLog("* decrypting...");
	
	if(ALProtocolTRDecVrfy(alp, chunk))
		OCLog("* check OK");
	else
		OCLog("decryption or check failed.");

	ALChunkPrint(chunk);
}
