/* This file is part of the OCBase library.
 * Copyright (C) 2012-2013 Benjamin Güldenring
 * Freie Universität Berlin, Germany
 *
 * OCBase is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 *
 * OCBase is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OCBase.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "OCConfig.h"
#include "OCLog.h"
#include "libs/inih/ini.h"

struct OCConfig{
	struct OCObject _obj;
	
	OCDictionaryRef dict;
};

// private
static int inih_handler(void* user, const char* section, const char* name, const char* value);
static int OCConfigInihHandler(OCConfigRef me, const char* section, const char* name, const char* vale);


void OCConfigDealloc(void* _me){
	OCConfigRef me = (OCConfigRef) _me;
	if(me->dict)
		OCRelease(&me->dict);
}

OCConfigRef OCConfigCreate(void){
	OCOBJECT_ALLOCINIT(OCConfig);
	if(me == NULL)
		return me;
	
	me->dict = OCDictionaryCreate(100);
	
	return me;
}

static int inih_handler(void* user, const char* section, const char* name, const char* value){
	return OCConfigInihHandler((OCConfigRef)user, section, name, value);
}

OCStatus OCConfigLoadFromFile(OCConfigRef me, OCStringRef filename){

	int ret = ini_parse(OCStringGetCString(filename), inih_handler, me);
	if(ret == 0)
		return SUCCESS;
	return FAILED;
}

int OCConfigInihHandler(OCConfigRef me, const char* section, const char* name, const char* value){
	OCAutoreleasePoolCreate();

	OCDictionaryRef sectionDict = (OCDictionaryRef)OCDictionaryGet(me->dict, OC(section));
	// insert section dictionary if new
	if(sectionDict == NULL)
	{
		sectionDict = (OCDictionaryRef)OCAutorelease(OCDictionaryCreate(10));
		OCDictionaryPut(me->dict, OC(section), (OCObjectRef)sectionDict);
	}
	OCObjectRef found = OCDictionaryGet(sectionDict, OC(name));
	if(found == NULL)
		OCDictionaryPut(sectionDict, OC(name), (OCObjectRef)OC(value));
	else
		OCLog("Dublicate entry in [%s]: %s", section, name);
	
	OCAutoreleasePoolDestroy();
	return 1; // return non-zero on success requested by inih
}

OCDictionaryRef  OCConfigGetDictionary(OCConfigRef me){
	return me->dict;
}

