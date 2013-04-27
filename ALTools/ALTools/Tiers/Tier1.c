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

// so far only a fake server


#include "OCBase.h"
#include  "ALProtocol.h"
#include "OCSerializer.h"
#include <gmp.h>
#include <math.h>
#include "CryptoRandom.h"
#include "DJ.h"
#include <fcgi_stdio.h>
#include "OCLog.h"
#include <unistd.h>
#include <string.h>
#include "OCDictionary.h"
#include "OCValue.h"
#include "TierConfig.h"
#include "TierClient.h"

void DieWithError(char* error){
	OCLog(error);
	exit(-1);
}

#define DEBUG 1


OCStatus parseQueryString(OCStringRef query_string, 
					 OCStringRef* val, 
					 OCStringRef* addr){
	if(query_string == NULL ||
	   val == NULL ||
	   addr == NULL)
		return NO;
	
	OCBool success = NO;
	
	OCListRef splittedList = OCStringSplitByToken(query_string, '&');
	do
	{
		if(OCListGetCount(splittedList) != 2)
			break;
		OCListIteratorRef iter = OCListCreateIterator(splittedList);
		
		OCStringRef param = NULL;
		OCStringRef valString = NULL;
		OCStringRef addrString = NULL;
		
		while( (param=(OCStringRef)OCListIteratorNext(iter)) != NULL)
		{
			if(strncmp(OCStringGetCString(param), "val=", 4) == 0)
			{
				if(valString != NULL)		// found more than once
					break;
				valString = param;
			}
			else if(strncmp(OCStringGetCString(param), "addr=", 5) == 0)
			{
				if(addrString != NULL)	// found more than once
					break;
				addrString = param;
			}
		}
		
		if(valString == NULL || addrString == NULL)
			break;
		
		// consider the case
		//		?val=&addr=
		// in this case *val and *addr will be set to NULL - which is ok for us here.
		*val = OCStringCreateSubString(valString, 4, OCStringGetLength(valString)-1);
		*addr = OCStringCreateSubString(addrString, 5, OCStringGetLength(addrString)-1);
		
		success = YES;
	}while(0);
	
	// we don't own param, valString and addrString (splittedList does),
	// so we let it do the cleanup
	OCObjectRelease(&splittedList);
	
	return success;
}


OCStatus extractAlChunk(ALProtocolRef alp, OCStringRef val, ALChunkRef* chunk){
	ALChunkRef c = ALChunkCreateFromBase64(val);
		
	if(ALProtocolIsChunkValid(alp, c))
		*chunk = c;
	else
	{
		OCRelease(&c);		
		return FAILED;
	}

	return SUCCESS;
}


OCStatus processRequest(TierClientRef tierclient, ALProtocolRef alp, OCStringRef* cookie, char* query){
	OCStringRef strquery = OCStringCreateWithCString(query);
	OCStringRef strval = NULL;
	OCStringRef straddr = NULL;
	
	OCStatus success = FAILED;
	do
	{
		success = parseQueryString(strquery, &strval, &straddr);
		if(success != SUCCESS)
		{
			OCLog("parseQuery failed");
			break;
		}
		
		ALChunkRef chunk = NULL;
		success = extractAlChunk(alp, strval, &chunk);
		if(success != SUCCESS)
		{
			OCLog("extractAlChunk failed");
			break;
		}

		// send the chunk to tier-2
		success = TierClientSendChunk(tierclient, chunk, 0);

		OCObjectRelease(&chunk);
		
		*cookie = NULL;
	}while(0);
	
	return success;
}


int runFCGI(TierClientRef tierclient, ALProtocolRef alp){
	while(FCGI_Accept() >= 0)
	{
		if(getenv("QUERY_STRING") == 0)
		{
			OCLog("no query string");
			continue;
		}
		OCStringRef cookie = NULL;
		OCStatus successful =  processRequest(tierclient, alp, &cookie, getenv("QUERY_STRING"));
		
		if(cookie)
			printf("Set-Cookie: Val=%s\r\n", OCStringGetCString(cookie));			
		printf(
			   "Content-type: text/html\r\n"
			   "\r\n"
			   "<html>"
			   "running on host <i>%s</i>\n"
			   , getenv("SERVER_NAME"));
		printf("success: %s", (successful) ? "yes":"no");
		printf("</html>");
		
		OCLog("sent header: %d", successful);
	}
	printf("</html>");	
	return 0;
}

TierConfigRef loadConfig(OCStringRef path){
	TierConfigRef tc = TierConfigCreate();
	OCStatus s = TierConfigLoad(tc, path, NULL);
	if(s == FAILED)
		OCRelease(&tc);
	return tc;
}


int main(int argc, char *argv[])
{
	VAR_UNUSED(argc);
	VAR_UNUSED(argv);
	TierConfigRef config = NULL;
	
	int retStatus = 0;
	OCAutoreleasePoolCreate();
	OCStringRef cfgFile = NULL;
	if(argc == 2)
		cfgFile = OC(argv[1]);
	else
		cfgFile = (OCStringRef)OCAutorelease(OCStringCreateAppended(OCBaseGetHomeDirectory(), OC("/adleaks/adleaks.conf")));	
	config = loadConfig(cfgFile);
	
	if(config == NULL)
	{
		
		OCLog("Failed to load config from file: %s", CO(cfgFile));
		exit(-1);
	}
	
	ALProtocolRef alp = ALProtocolCreateInstanceFromFile(TierConfigGetPubPath(config));
	if(alp == NULL)
	{
		OCLog("Can not load protocol file %s", TierConfigGetPubPath(config));
		exit(-1);
	}
	
	struct TierConfigService serv;
	TierConfigGetService(config, &serv);
	if(serv.type != TCTier1)
	{
		OCLog("wrong type");
		exit(-1);
	}

	TierClientRef tierclient = TierClientCreate();
	size_t i = 0;
	for(i = 0; i < TierConfigGetNumberOfDestinations(config); i++)
	{
		struct TierConfigDestination dest;
		TierConfigGetDestination(config, i, &dest);
		OCLog("Adding destination [%s]: %s:%d (type %d)", 
			  CO(dest.label), CO(dest.host), dest.port, dest.type);
		TierClientAddServer(tierclient, dest.host, dest.port);
	}		
	TierClientStart(tierclient);
	sleep(3);
	
	retStatus = runFCGI(tierclient, alp);
	OCLog("stopping service gracefully");
	TierClientStop(tierclient);
	TierClientWaitTillStopped(tierclient);
	
	OCAutoreleasePoolDestroy();

	return retStatus;
}

