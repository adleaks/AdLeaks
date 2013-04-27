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

#ifndef _TIER_3_STORAGE_H_
#define _TIER_3_STORAGE_H_

#include "ALPayloadChunk.h"

typedef struct Tier3Storage* Tier3StorageRef;

Tier3StorageRef Tier3StorageCreate(OCStringRef path);

OCStatus Tier3StorageStart(Tier3StorageRef me);
OCStatus Tier3StorageStop(Tier3StorageRef me);
OCStatus Tier3StorageWaitTillStopped(Tier3StorageRef me);

OCStatus Tier3StorageStore(Tier3StorageRef me, ALPayloadChunkRef plc);

#endif
