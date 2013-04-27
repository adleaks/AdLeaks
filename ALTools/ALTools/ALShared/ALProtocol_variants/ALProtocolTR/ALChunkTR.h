/* This file is part of AdLeaks.
 * Copyright (C) 2013 Benjamin Güldenring
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

#ifndef _AL_CHUNK_TR_H_
#define _AL_CHUNK_TR_H_

#include <gmp.h>
#include "OCSerializer.h"

// part of the interface:
typedef struct ALChunkTR* ALChunkTRRef;

// these are not part of the interface:
ALChunkTRRef ALChunkTRCreate(mpz_t m, mpz_t t);


// part of the interface:
OCStatus ALChunkTRSerialize(ALChunkTRRef me, OCSerializerRef s);
OCStatus ALChunkTRDeserialize(OCDeserializerRef ds, ALChunkTRRef* chunk);
ALChunkTRRef ALChunkTRCreateFromBase64(OCStringRef base64Encoded);
void ALChunkTRPrint(ALChunkTRRef chunk);

#endif
