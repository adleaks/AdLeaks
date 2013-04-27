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

#ifndef _TIER_MESSAGE_H_
#define _TIER_MESSAGE_H_

#include <stdlib.h>
#include "OCTypes.h"
#include <stdint.h>

#define TIER_MESSAGE_MAX_LENGTH 5000
#define TIER_MESSAGE_HEADER_LENGTH \
	(3*sizeof(OCByte) +sizeof(u_int32_t))

typedef enum {
	TMCHUNK = 1,
	TMHEARTBEAT = 2,
	TMACK = 3,
	TMSHUTDOWN
}TierMessageTypes;

struct TierMessageHeader{
	OCByte messageType;
	OCByte protocolVariant;
	OCByte protocolVersion;
	u_int32_t dataLength;
};

struct TierMessage{
	struct TierMessageHeader header;
	OCByte data[TIER_MESSAGE_MAX_LENGTH];
};

typedef struct TierMessage* TierMessagePtr;
typedef struct TierMessageHeader* TierMessageHeaderPtr;

struct TierMessageHeader TierMessageMakeAck(void);
struct TierMessageHeader TierMessageMakeHeartbeat(void);
struct TierMessageHeader TierMessageMakeShutdown(void);

#endif
