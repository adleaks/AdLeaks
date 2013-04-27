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

#ifndef _DECRYPTION_QUEUE_H_
#define _DECRYPTION_QUEUE_H_

#include "ALChunk.h"
#include "ALProtocol.h"

typedef struct DecryptionQueue* DecryptionQueueRef;

typedef void (*DecryptionQueueDecryptedCB)(OCObjectRef me, void* chunk);

typedef struct{
	OCObjectRef delegate;
	DecryptionQueueDecryptedCB decryptedCB;
}DecryptionQueueDelegate;

DecryptionQueueRef DecryptionQueueCreate(ALProtocolRef alp, DecryptionQueueDelegate delegate);
void DecryptionQueueRun(DecryptionQueueRef me);
OCStatus DecryptionQueueStop(DecryptionQueueRef me);
OCStatus DecryptionQueueWaitTillStopped(DecryptionQueueRef me);

OCStatus DecryptionQueueEnqueue(DecryptionQueueRef me, ALChunkRef chunk);


#endif
