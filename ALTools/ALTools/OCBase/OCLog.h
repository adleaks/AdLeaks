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

#ifndef _OCLOG_H_
#define _OCLOG_H_

#include "OCString.h"

void OCLogSetLogFile(OCStringRef path);
void OCLogSetTimestamp(int enabled);
void OCLog(char* format, ...);
void OCLogOCString(OCStringRef string);

void _OCLogError(char* file, int line, const char* function, const char* format, ...);

//#define OCLogError(format, ...) _OCLogError(__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define OCLogError(...) _OCLogError(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#endif
