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

#ifndef _OC_AUTORELEASE_POOL_H_
#define _OC_AUTORELEASE_POOL_H_

// some convenience makros for emulating an allocation pool:
// the bottom line: you can do
// { ...
//	OCAutoreleasePoolCreate();
//
//	OC("foo")
//
//	OCAutoreleasePoolDestroy();
// and "foo" gets automatically released after OCAutoreleasePoolDestroy().
// however, we should (should we?) replace this with a true AutoreleasePool in the future
// since this does not apply for string created with OCStringCreateAppended etc

#define OCAutoreleasePoolCreate()  { OCListRef __ocar_pool = OCListCreate();
#define OCAutoreleasePoolDestroy() OCObjectRelease(&__ocar_pool); }
#define OCAutorelease(OBJ) \
({ \
OCObjectRef __pool_obj_tmp = (OCObjectRef)(OBJ); \
OCObjectRef __pool_obj_tmp2 = (OCObjectRef)(__pool_obj_tmp); \
if((__pool_obj_tmp)!=NULL)\
	OCListAppend( __ocar_pool, __pool_obj_tmp ); \
OCObjectRelease(&(__pool_obj_tmp)); \
(__pool_obj_tmp2); \
})

#endif
