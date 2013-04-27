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

#ifndef _OC_STRING_H_
#define _OC_STRING_H_
#include <stdlib.h>

#include "OCAutoreleasePool.h"
#include "OCList.h"

// NOTE An OCString shall not be mutable
typedef struct OCString* OCStringRef;


//NOTE: CString == '\0' terminated string!
OCStringRef OCStringCreateWithCString(const char* cstring);//, size_t length);

// chars does not have to be '\0' terminated
OCStringRef OCStringCreateWithCharsAndLength(const char* chars, size_t length);
OCStringRef OCStringCreateSubString(OCStringRef s, size_t begin, size_t end);

size_t OCStringGetLength(OCStringRef me);

OCStringRef OCStringCreateCopy(OCStringRef s);
const char* OCStringGetCString(OCStringRef s);

OCStringRef OCStringCreateAppended(OCStringRef s1, OCStringRef s2);

// returns a list of OCStringRefs 
// returns NULL if token is not found in the string
// the resulting strings do not contain the token
// the resulting strings are not empty ("")
OCListRef OCStringSplitByToken(OCStringRef s, char token);

OCBool OCStringIsEqual(OCStringRef me, OCStringRef with);
OCBool OCStringIsEqualCString(OCStringRef me, const char* cstring);


//  Autoreleased conversion macros
// OC : cstring -> OCString
// CO: OCString -> cstring

#define OC(STR)  \
({ \
OCStringRef __pool_str_tmp = OCStringCreateWithCString(STR); \
OCStringRef __pool_str_tmp2 = (OCStringRef)__pool_str_tmp; \
if((__pool_str_tmp)!=NULL)\
OCListAppend( __ocar_pool, (OCObjectRef)__pool_str_tmp ); \
OCObjectRelease((OCObjectRef*)&(__pool_str_tmp)); \
(__pool_str_tmp2); \
})

#define CO(STR) OCStringGetCString(STR)


#endif
