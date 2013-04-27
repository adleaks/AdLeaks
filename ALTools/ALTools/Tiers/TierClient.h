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

#ifndef _TIER_CLIENT_H_
#define _TIER_CLIENT_H_

#include "ALChunk.h"
#include "OCTypes.h"
#include "OCString.h"
#include "TierMessage.h"

typedef struct TierClient* TierClientRef;


TierClientRef TierClientCreate(void);

// for now you shall not call this after Start()
OCStatus TierClientAddServer(TierClientRef me, OCStringRef host, unsigned short andPort);

OCStatus TierClientStart(TierClientRef me);
OCStatus TierClientSendChunk(TierClientRef me, ALChunkRef chunk, size_t serverIndex);
OCStatus TierClientSendChunks(TierClientRef me, OCListRef chunks, size_t serverIndex);

size_t TierClientGetNumberOfServer(TierClientRef me);

OCStatus TierClientStop(TierClientRef me);
OCStatus TierClientWaitTillStopped(TierClientRef me);

#endif
