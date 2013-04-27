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

#ifndef _TIER_CONFIG_H_
#define _TIER_CONFIG_H_

#include "OCString.h"

typedef struct TierConfig* TierConfigRef;

typedef enum{
	TCTierInvalid = 0,
	TCTier1 = 1,
	TCTier2 = 2,
	TCTier3 = 3,
	TCLoadTest = 4
} TierConfigServiceType;

struct TierConfigService{
	OCStringRef label;
	TierConfigServiceType type;
	unsigned short port;
};
typedef struct TierConfigService* TierConfigServicePtr;

struct TierConfigDestination{
	OCStringRef label;
	OCStringRef host;
	unsigned short port;
	TierConfigServiceType type;
};
typedef struct TierConfigDestination* TierConfigDestinationPtr;

TierConfigRef TierConfigCreate(void);
// serviceLabel may be NULL
OCStatus TierConfigLoad(TierConfigRef me, OCStringRef path, OCStringRef serviceLabel);

OCStatus TierConfigGetService(TierConfigRef me, TierConfigServicePtr to);
OCStringRef TierConfigGetServiceValue(TierConfigRef me, OCStringRef key);

OCStatus TierConfigGetDestination(TierConfigRef me, size_t index, TierConfigDestinationPtr to);
size_t TierConfigGetNumberOfDestinations(TierConfigRef me);

OCStringRef TierConfigGetPubPath(TierConfigRef me);
OCStringRef TierConfigGetPrvPath(TierConfigRef me);


#endif
