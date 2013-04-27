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

#include "TierMessage.h"

struct TierMessageHeader TierMessageMakeAck(void){
	struct TierMessageHeader h;
	h.messageType = TMACK;
	h.protocolVariant = 0;
	h.protocolVersion = 0;
	h.dataLength = 0;

	return h;
}

struct TierMessageHeader TierMessageMakeHeartbeat(void){
	struct TierMessageHeader h;
	h.messageType = TMHEARTBEAT;
	h.protocolVariant = 0;
	h.protocolVersion = 0;
	h.dataLength = 0;
	
	return h;	
}

struct TierMessageHeader TierMessageMakeShutdown(void){	
	struct TierMessageHeader h;
	h.messageType = TMSHUTDOWN;
	h.protocolVariant = 0;
	h.protocolVersion = 0;
	h.dataLength = 0;
	
	return h;
}
