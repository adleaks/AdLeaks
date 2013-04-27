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

#include "TierConfig.h"
#include "OCObject.h"
#include "OCConfig.h"
#include "OCLog.h"
#include "OCValue.h"
#include "OCBase.h"

#include <string.h>

struct TierConfig{
	struct OCObject _obj;
	OCConfigRef conf;

	OCStringRef alpPub;
	OCStringRef alpPrv;
	
	struct TierConfigService service; 
	OCDictionaryRef serviceDict;
	OCListRef destinations;
};

// private:
TierConfigDestinationPtr TierConfigLoadServer(TierConfigRef me, OCStringRef label, OCDictionaryRef dict);
OCStatus TierConfigLoadService(TierConfigRef me, OCDictionaryRef dict, OCStringRef serviceLabel);
TierConfigServiceType TierConfigTypeFromString(TierConfigRef me, OCStringRef str);


void TierConfigDealloc(void* _me){
	TierConfigRef me = (TierConfigRef)_me;
	OCRelease(&me->conf);
	
	if(me->service.label)
		OCRelease(&me->service.label);
	
	if(me->destinations)
	{
		OCListIteratorRef iter = OCListCreateIterator(me->destinations);
		OCValueRef val = NULL;
		while((val=(OCValueRef)OCListIteratorNext(iter)) != NULL)
		{
			TierConfigDestinationPtr ptr = OCValueGetPointerValue(val);
			OCObjectRelease(&ptr->label);
			OCObjectRelease(&ptr->host);
			
			OCDeallocate(ocDefaultAllocator, OCValueGetPointerValue(val));
		}
		OCRelease(&iter);
		OCRelease(&me->destinations);
	}
	if(me->alpPub)
		OCRelease(&me->alpPub);
	if(me->alpPrv)
	OCRelease(&me->alpPrv);
}

TierConfigRef TierConfigCreate(){
	OCOBJECT_ALLOCINIT(TierConfig);
	if(me == NULL)
		return me;
	
	me->alpPub = NULL;
	me->alpPrv = NULL;
	me->conf = OCConfigCreate();
	me->destinations = OCListCreate();
	me->serviceDict = NULL;
	
	return me;
}

OCStatus TierConfigLoad(TierConfigRef me, OCStringRef path, OCStringRef serviceLabel){
	OCStatus s = OCConfigLoadFromFile(me->conf, path);
	
	if(s == SUCCESS)
	{
		OCAutoreleasePoolCreate();
		while(1)
		{
			OCDictionaryRef dict = OCConfigGetDictionary(me->conf);
			if(dict == NULL) 
				break;
			OCDictionaryRef tmpDict;	
			tmpDict = (OCDictionaryRef)OCDictionaryGet(dict, OC("ALProtocol"));
			if(tmpDict == NULL) 
				break;
			me->alpPrv = (OCStringRef)OCRetain(OCDictionaryGet(tmpDict, OC("prv")));
			me->alpPub = (OCStringRef)OCRetain(OCDictionaryGet(tmpDict, OC("pub")));
			
			tmpDict = (OCDictionaryRef)OCDictionaryGet(dict, OC(""));
			if(tmpDict == NULL)
				break;
			OCStringRef defaultService = NULL;
			if(serviceLabel)
				defaultService = OCRetain(serviceLabel);
			else
				defaultService = (OCStringRef)OCDictionaryGet(tmpDict, OC("defaultService"));
			
			s = TierConfigLoadService(me, dict, defaultService);
			break;
		}
		OCAutoreleasePoolDestroy();
	}
	
	return s;
}


OCStatus TierConfigGetService(TierConfigRef me, TierConfigServicePtr to){
	memcpy(to, &me->service, sizeof(struct TierConfigService));
	return SUCCESS;
}

OCStringRef TierConfigGetServiceValue(TierConfigRef me, OCStringRef key){
	if(me->serviceDict == NULL)
	{
		OCLogError("me->serviceDict == NULL");
		return NULL;
	}
	return (OCStringRef)OCDictionaryGet(me->serviceDict, key);
}

size_t TierConfigGetNumberOfDestinations(TierConfigRef me){
	return OCListGetCount(me->destinations);
}

OCStatus TierConfigGetDestination(TierConfigRef me, size_t i, TierConfigDestinationPtr to){
	if(i < OCListGetCount(me->destinations))
	{
		OCValueRef val = (OCValueRef)OCListGetItem(me->destinations, i);
		TierConfigDestinationPtr dest = OCValueGetPointerValue(val);
		memcpy(to, dest, sizeof(struct TierConfigDestination));
		
		return SUCCESS;
	}
	return FAILED;
}

OCStringRef TierConfigGetPubPath(TierConfigRef me){
	return me->alpPub;
}

OCStringRef TierConfigGetPrvPath(TierConfigRef me){
	return me->alpPrv;
}

TierConfigDestinationPtr TierConfigLoadServer(TierConfigRef me, OCStringRef label, OCDictionaryRef dict){
	TierConfigDestinationPtr destination = NULL;
	
	OCAutoreleasePoolCreate();

	do
	{
		OCStringRef strserver = (OCStringRef)OCAutorelease(OCStringCreateAppended(OC("destination "), label));	
		OCDictionaryRef serverdict = (OCDictionaryRef) OCDictionaryGet(dict, strserver);
		if(serverdict == NULL)
			break;
		OCStringRef host = (OCStringRef) OCDictionaryGet(serverdict, OC("host"));
		OCStringRef port = (OCStringRef) OCDictionaryGet(serverdict, OC("port"));
		OCStringRef type = (OCStringRef) OCDictionaryGet(serverdict, OC("type"));
		if( host == NULL || port == NULL || type == NULL )
		{
			OCLog("Incomplete server description: %s", CO(label));
			break;
		}
//		OCLog("Adding [destination %s] \tat\t %s:%s (%s)", CO(label), CO(host), CO(port), CO(type));
		destination = OCAllocate(ocDefaultAllocator, sizeof(struct TierConfigDestination));
		if(destination == NULL) 
			break;
		destination->label = OCRetain(label);
		destination->host = OCRetain(host);
		destination->type = TierConfigTypeFromString(me, type);
		destination->port = atoi(CO(port));
		
	}while(0);

	OCAutoreleasePoolDestroy();	
	
	return destination;
}

TierConfigServiceType TierConfigTypeFromString(TierConfigRef me, OCStringRef str){
	VAR_UNUSED(me);
	TierConfigServiceType type = TCTierInvalid;
	
	OCAutoreleasePoolCreate();
	if(OCStringIsEqual(str, OC("tier1")))
		type = TCTier1;
	else if(OCStringIsEqual(str, OC("tier2")))
		type = TCTier2;
	else if(OCStringIsEqual(str, OC("tier3")))
		type = TCTier3;
	else if(OCStringIsEqual(str, OC("loadtest")))
		type = TCLoadTest;
	OCAutoreleasePoolDestroy();	
	
	return type;
}

OCStatus TierConfigLoadService(TierConfigRef me, OCDictionaryRef dict, OCStringRef serviceLabel){
	OCStatus success = FAILED;
	OCAutoreleasePoolCreate();	
	do
	{
		me->service.label = OCRetain(serviceLabel);
		OCStringRef service = (OCStringRef)OCAutorelease(OCStringCreateAppended(OC("service "), serviceLabel));
		OCDictionaryRef serviceDict = (OCDictionaryRef)OCDictionaryGet(dict, service);
		if(serviceDict == NULL)
			break;
		me->serviceDict = serviceDict;
		
		OCStringRef type = (OCStringRef) OCDictionaryGet(serviceDict, OC("type"));
		if(type == NULL)
		{
			OCLogError("Could not find type of [%s]", service);
			break;
		}
		me->service.type = TierConfigTypeFromString(me, type);
		
		OCStringRef port = (OCStringRef) OCDictionaryGet(serviceDict, OC("port"));
		if(port == NULL)
		{
			if(me->service.type == TCLoadTest || me->service.type == TCTier1)
				;
			else
			{
				OCLogError("Could not find port of [%s]", CO(service));
				break;
			}
		}
		else
			me->service.port = atoi(CO(port));
		
		
		OCStringRef connectTo = (OCStringRef)OCDictionaryGet(serviceDict, OC("connect"));
		if(connectTo == NULL)
		{
			success = SUCCESS;
			break;
		}
		OCListRef listOfConnections = (OCListRef)OCAutorelease(OCStringSplitByToken(connectTo, ','));
		if(listOfConnections == NULL)
		{
			TierConfigDestinationPtr server = TierConfigLoadServer(me, connectTo, dict);
			if(server == NULL)
				break;
			OCListAppend(me->destinations, OCAutorelease(OCValueCreateWithPointer(server)));
			success = SUCCESS;
			break;
		}
		OCListIteratorRef iter = (OCListIteratorRef)OCAutorelease(OCListCreateIterator(listOfConnections));
		OCStringRef label = NULL;
		while( (label=(OCStringRef)OCListIteratorNext(iter)) != NULL)
		{
			TierConfigDestinationPtr server = TierConfigLoadServer(me, label, dict);
			if(server == NULL)
				break;
			OCListAppend(me->destinations, OCAutorelease(OCValueCreateWithPointer(server)));
		}
		success = SUCCESS;
	}while(0);
	OCAutoreleasePoolDestroy();	
	return success;
}
