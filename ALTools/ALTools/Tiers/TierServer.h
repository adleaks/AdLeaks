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

#ifndef _TIER_SERVER_H_
#define _TIER_SERVER_H_

#include "TierMessage.h"
#include "OCObject.h"
#include "OCTypes.h"

typedef struct TierServer* TierServerRef;


typedef void (*TierServerReceivedCallback)(OCObjectRef me, TierMessagePtr message);

typedef struct{
	OCObjectRef delegateObject;

	TierServerReceivedCallback messageReceived;
} TierServerDelegate;

TierServerRef TierServerCreateWithPortNo(unsigned short portNo, TierServerDelegate delegate);

OCStatus TierServerStart(TierServerRef me);

OCStatus TierServerStop(TierServerRef me);
OCStatus TierServerWaitTillStopped(TierServerRef me);

#endif
