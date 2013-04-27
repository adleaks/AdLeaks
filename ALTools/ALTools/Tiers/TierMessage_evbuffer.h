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

#ifndef _TIER_MESSAGE__EV_BUFFER_H_
#define _TIER_MESSAGE__EV_BUFFER_H_

#include "TierMessage.h"

#include <event.h>

OCStatus TierMessageReadHeaderFromEVBuffer(TierMessageHeaderPtr to, struct evbuffer* buf); 
OCStatus TierMessageWriteHeaderToEVBuffer(TierMessageHeaderPtr from, struct evbuffer* buf); 

OCStatus TierMessageWriteToEVBuffer(TierMessagePtr ptr, struct evbuffer* buf);

#endif
