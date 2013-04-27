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

#include "OCTypes.h"


static OCByte hexLookup[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
void OCBytes2Hex(OCByte* hexString, OCByte* bytes, size_t nBytes){
	size_t i = 0;
	for(i = 0; i < nBytes; i++)
	{
		OCByte b = bytes[i];
		hexString[2*i] = hexLookup[(b>>4)&0xf];
		hexString[2*i+1] = hexLookup[b&0xf];
	}
}
