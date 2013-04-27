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

#include <pthread.h>
#include "OCAllocator.h"

struct OCAllocator{
	
};

OCAllocatorRef ocDefaultAllocator;
//static pthread_mutex_t ocDefaultMutex;


void* OCAllocate(OCAllocatorRef al, size_t size){
	if(al == ocDefaultAllocator)
	{
//		pthread_mutex_lock(&ocDefaultMutex);
		void* ptr = malloc(size);
//		pthread_mutex_unlock(&ocDefaultMutex);
		return ptr;
	}
	else
		return NULL;
}

void OCDeallocate(OCAllocatorRef al, void* ptr){
	if(al == ocDefaultAllocator)
	{
//		pthread_mutex_lock(&ocDefaultMutex);
		free((void*)ptr);
//		pthread_mutex_unlock(&ocDefaultMutex);
	}
	else
		return;
}



void OCAllocatorInitialize(void){
	ocDefaultAllocator = NULL;
//	pthread_mutex_init(&ocDefaultMutex, NULL);
}
