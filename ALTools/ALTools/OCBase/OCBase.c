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

#include "OCBase.h"

void OCBaseInit(void){
	OCAllocatorInitialize();
}

OCStringRef _home;

OCStringRef OCBaseGetHomeDirectory(){
	if(_home == NULL)
	{
		char* chome = getenv("HOME");
		_home = OCStringCreateWithCString(chome);
	}
	return _home;
}


int OCExit(int ret){
	if(_home)
		OCObjectRelease(&_home);
	exit(ret);
}
