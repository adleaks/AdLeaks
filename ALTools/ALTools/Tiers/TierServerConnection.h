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

#ifndef _TIER_SERVER_CONNECTION_H_
#define _TIER_SERVER_CONNECTION_H_

#include <event.h>
#include "TierServer.h"


typedef struct TierServerConnection* TierServerConnectionRef;

typedef void (*TierServerConnectionHasShutdown)(OCObjectRef  me, TierServerConnectionRef sender);
														 
typedef struct{
	OCObjectRef delegateObject;
	
	TierServerConnectionHasShutdown hasShutdown;
}TierServerConnectionDelegate;

TierServerConnectionRef TierServerConnectionCreate(struct event_base* base, int socketfd,
								    TierServerDelegate delegate, TierServerConnectionDelegate connectionDelegate);

OCStatus TierServerConnectionStop(TierServerConnectionRef me);

#endif
