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

#ifndef _AGGREGATION_QUEUE_H_
#define _AGGREGATION_QUEUE_H_

#include "ALProtocol.h"
#include "ALChunk.h"
#include "OCTime.h"

// this object is thread safe
typedef struct AggregationQueue* AggregationQueueRef;

typedef void (*AggregationQueueSendCB)(void* cbParam, OCListRef chunksToSend);
typedef struct{
	OCObjectRef delegateObject;
	AggregationQueueSendCB sendCB;
}AggregationQueueDelegate;

AggregationQueueRef AggregationQueueCreate(ALProtocolRef alp,
										   size_t nDestChunks,
										   size_t dropLimit,
										   OCTimeInterval sendTimeout,
										   AggregationQueueDelegate delegate);

OCStatus AggregationQueueEnqueueChunks(AggregationQueueRef me, OCListRef chunks);
OCStatus AggregationQueueEnqueue(AggregationQueueRef me, ALChunkRef chunk);

void AggregationQueueStop(AggregationQueueRef me);

#endif
