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

#include "CryptoMac.h"
#include "sha2.h"
#include "hmac_sha2.h"


void CryptoMacDoMAC(OCByte* macKey, size_t nMacKey, OCByte* toMac, int length, OCByte* dest){
	hmac_sha256((unsigned char*)macKey, 
				nMacKey, 
				(unsigned char*)toMac, 
				length, 
				(unsigned char*)dest, 
				SHA256_DIGEST_SIZE);
	sha256_ctx ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, macKey, nMacKey);
	sha256_update(&ctx, toMac, length);
	sha256_final(&ctx, dest);
}

int CryptoMacVerifyMAC(OCByte* macKey, size_t nMacKey, OCByte* mac, OCByte* toTest, int length){
	OCByte tmp[SHA256_DIGEST_SIZE];
	
	CryptoMacDoMAC(macKey, nMacKey, toTest, length, tmp);
	size_t i = 0;
	for(i = 0; i < 32; i++)
		if(tmp[i] != mac[i])
			return 0;
	return 1;	
}
