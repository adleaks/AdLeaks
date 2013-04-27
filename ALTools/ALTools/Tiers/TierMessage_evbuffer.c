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

#include "TierMessage_evbuffer.h"
#include <arpa/inet.h>
OCStatus TierMessageReadHeaderFromEVBuffer(TierMessageHeaderPtr to, struct evbuffer* buf){
	OCByte messageType;
	OCByte protocolVariant;
	OCByte protocolVersion;
	u_int32_t dataLength;
	
	evbuffer_remove(buf, &(messageType), sizeof(OCByte));
	evbuffer_remove(buf, &(protocolVariant), sizeof(OCByte));
	evbuffer_remove(buf, &(protocolVersion), sizeof(OCByte));
	evbuffer_remove(buf, &(dataLength), sizeof(u_int32_t));
	
	to->messageType = messageType;
	to->protocolVariant = protocolVariant;
	to->protocolVersion = protocolVersion;
	to->dataLength = ntohl(dataLength);
	
	return SUCCESS;
}

OCStatus TierMessageWriteHeaderToEVBuffer(TierMessageHeaderPtr from, struct evbuffer* buf){
	evbuffer_add(buf, &(from->messageType), sizeof(OCByte));
	evbuffer_add(buf, &(from->protocolVariant), sizeof(OCByte));
	evbuffer_add(buf, &(from->protocolVersion), sizeof(OCByte));
	u_int32_t dataLength = htonl(from->dataLength);
	evbuffer_add(buf, &(dataLength), sizeof(u_int32_t));

	return SUCCESS;
}

OCStatus TierMessageWriteToEVBuffer(TierMessagePtr ptr, struct evbuffer* buf){
	TierMessageWriteHeaderToEVBuffer(&(ptr->header), buf);
	evbuffer_add(buf, &(ptr->data), ptr->header.dataLength*sizeof(OCByte));

	return SUCCESS;
}

