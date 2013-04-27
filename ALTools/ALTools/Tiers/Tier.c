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

#include "OCBase.h"
#include "OCLog.h"
#include "TierMessage.h"
#include "TierServer.h"
#include "TierClient.h"
#include <unistd.h>
#include <signal.h>
#include "ALProtocol.h"
#include "ALChunk.h"
#include "OCTime.h"
#include "OCConfig.h"
#include "OCAutoreleasePool.h"
#include <string.h>
#include <search.h>

#include "OCDictionary.h"
#include "OCValue.h"
#include "TierConfig.h"
#include "Tier2.h"
#include "Tier3.h"
#include "TierLoadTest.h"


// NOTE: do not do anything else in the signal handler except
// what it already does below, see: https://www.securecoding.cert.org/confluence/display/seccode/SIG31-C.+Do+not+access+or+modify+shared+objects+in+signal+handlers
volatile sig_atomic_t tier_gotSignal = 0;
static void sighandler(int thesignal){
	tier_gotSignal = thesignal;
}

void initSigHandlers(void){
	sigset_t thesigset;
	sigemptyset(&thesigset);
	struct sigaction siginfo = {
		.sa_handler = sighandler,
		.sa_mask = thesigset,
		.sa_flags = SA_RESTART,
	};
	sigaction(SIGINT, &siginfo, NULL);
	sigaction(SIGTERM, &siginfo, NULL);	
}


OCStringRef getDefaultConfigPath(void){
	OCStringRef path = NULL;
	OCAutoreleasePoolCreate();
	path = OCStringCreateAppended(OCBaseGetHomeDirectory(), OC("/adleaks/adleaks.conf"));	
	OCAutoreleasePoolDestroy();
	return path;
}

void printUsage(char* execName){
	OCLog("Usage: %s [-f path] [-s name]", execName);
	OCLog("\t -f \t path to config file");
	OCLog("\t -s \t override service label in config file");
}

struct commandline{
	OCStringRef configFile;
	OCStringRef serverLabel;
};
OCStatus readCommandline(int argc, char** argv, struct commandline* cmd ){
	cmd->configFile = NULL;
	cmd->serverLabel = NULL;
	int c = 0;
	
	while( (c=getopt(argc, argv, "f:s:")) != -1)
	{
		switch(c)
		{
			case 'f':
				cmd->configFile = OCStringCreateWithCString(optarg);
			break;
			case 's':
				cmd->serverLabel = OCStringCreateWithCString(optarg);				
			break;
			default:
				return FAILED;
			break;
		}
	}
	
	return SUCCESS;
}

int main(int argc, char *argv[])
{
	Tier2Ref tier2;
	Tier3Ref tier3;
	TierLoadTestRef tierloadtest;
	
	initSigHandlers();

	OCStatus status = FAILED;

	OCAutoreleasePoolCreate();
	do
	{
		struct commandline cmd;
		if(readCommandline(argc, argv, &cmd) == FAILED)
		{
			printUsage(argv[0]);
			break;
		}
		OCStringRef cfgFile = NULL;
		if(cmd.configFile)
			cfgFile = cmd.configFile;
		else
			cfgFile = getDefaultConfigPath();
		OCAutorelease(cfgFile);
		
		TierConfigRef config = TierConfigCreate();
		OCAutorelease(config);
		if(TierConfigLoad(config, cfgFile, cmd.serverLabel) == FAILED)
		{
			OCLog("Failed to load config");
			status = FAILED;
			break;
		}
		
		ALProtocolRef alp = NULL;
		
		struct TierConfigService serv;
		TierConfigGetService(config, &serv);
		
		if(serv.type == TCTier2 || serv.type == TCTier3)
		{
			alp = ALProtocolCreateInstanceFromFile(TierConfigGetPrvPath(config));
			OCAutorelease(alp);			
		}
		else
		{
			alp = ALProtocolCreateInstanceFromFile(TierConfigGetPubPath(config));
			OCAutorelease(alp);
		}
		
		if(alp == NULL)
		{
			OCLog("Can not load protocol file %s", TierConfigGetPubPath(config));
			status = FAILED;
			break;
		}
		
		if(serv.type == TCTier2)
		{
			tier2 = (Tier2Ref)OCAutorelease(Tier2Create(config, alp));
			Tier2Main(tier2);
		}
		else if(serv.type == TCTier3)
		{
			tier3 = (Tier3Ref)OCAutorelease(Tier3Create(config, alp));
			Tier3Main(tier3);
		}
		else if(serv.type == TCLoadTest)
		{
			tierloadtest = (TierLoadTestRef)OCAutorelease(TierLoadTestCreate(config, alp));
			TierLoadTestMain(tierloadtest);
		}
		else
			OCLog("unsupported type");
		
		status = SUCCESS;
		OCRelease(&cmd.configFile);
		OCRelease(&cmd.serverLabel);
	}while(0);
	OCAutoreleasePoolDestroy();
	
	return OCExit((status == SUCCESS) ? 0 : -1);
}
