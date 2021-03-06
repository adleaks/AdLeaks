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

#ifndef _OC_ALLOCATOR_H_
#define _OC_ALLOCATOR_H_

#include <stdlib.h>

typedef struct OCAllocator* OCAllocatorRef;

void* OCAllocate(OCAllocatorRef al, size_t size);
void OCDeallocate(OCAllocatorRef al, void* ptr);


#pragma mark CAllocators
extern OCAllocatorRef ocDefaultAllocator;


#pragma mark Initialization routines

void OCAllocatorInitialize(void);

#endif
