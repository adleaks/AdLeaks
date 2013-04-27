/* This file is part of the OCBase library.
 * Copyright (C) 2012-2013 Benjamin Güldenring
 * Freie Universität Berlin, Germany
 *
 * OCBase is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 *
 * OCBase is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OCBase.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _OC_SERIALIZER_H_
#define _OC_SERIALIZER_H_

#include <stdio.h>

#include "OCBase.h"
#include "OCBufferedStreamReader.h"

#pragma mark OCSerializer

typedef struct OCSerializer* OCSerializerRef;

OCSerializerRef OCSerializerCreate(void);

OCStatus OCSerializerWriteInt(OCSerializerRef s, int32_t toWrite);
OCStatus OCSerializerWriteBytes(OCSerializerRef s, OCByte* buf, size_t n);

size_t OCSerializerWriteToStream(OCSerializerRef s, FILE* fp);

size_t OCSerializerGetNumberOfBytes(OCSerializerRef s);
size_t OCSerializerCopyBytes(OCSerializerRef s, OCByte* to);

void OCSerializerReset(OCSerializerRef s);

#pragma mark OCDeserializer

typedef struct OCDeserializer* OCDeserializerRef;

OCDeserializerRef OCDeserializerCreateWithBytes(OCByte* buf, size_t n);
OCDeserializerRef OCDeserializerCreateFromFile(FILE* fp);
OCDeserializerRef OCDeserializerCreateFromStream(OCBufferedStreamReaderRef sr);

OCStatus OCDeserializerReadInt(OCDeserializerRef s, int32_t* to);
OCStatus OCDeserializerReadBytes(OCDeserializerRef s, OCByte* buf, size_t n);

#endif
