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

#ifndef _OC_LIST_H_
#define _OC_LIST_H_

//#include "OCBase.h"
#include "OCObject.h"
#include "OCTypes.h"

typedef struct OCList* OCListRef;
typedef struct OCListIterator* OCListIteratorRef;


OCListRef OCListCreate(void);

size_t OCListGetCount(OCListRef me);

OCStatus OCListAppend(OCListRef list, OCObjectRef obj);

// append the contents from <src> to <dest> and clear src
OCStatus OCListAppendList(OCListRef me, OCListRef src);

OCStatus OCListFifoPush(OCListRef list, OCObjectRef obj);

OCObjectRef OCListGetItem(OCListRef list, size_t index);
OCStatus OCListRemoveItem(OCListRef list, size_t index);
// removes the first object with pointer <obj> found
OCStatus OCListRemove(OCListRef list, OCObjectRef obj);

// Attention: *Pop does not release the object!
OCObjectRef OCListFifoPop(OCListRef list);

#pragma mark Iterator
// ATTENTION: iterators get invalidated when the list is altered
OCListIteratorRef OCListCreateIterator(OCListRef list);
OCObjectRef OCListIteratorNext(OCListIteratorRef iter);
OCStatus OCListIteratorReset(OCListIteratorRef list);

#endif
