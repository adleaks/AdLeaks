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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "OCBase.h"
#include "OCTime.h"

static OCBool timestampEnabled = 0;

static FILE* logfile = 0; 

void OCLogSetLogFile(OCStringRef path){
	if(logfile != 0)
		fclose(logfile);

	logfile = fopen(OCStringGetCString(path), "a");
}

void OCLogSetTimestamp(int enabled){
	timestampEnabled = enabled;
}

void OCLog(char* format, ...){
	if(logfile == 0)
		logfile = stderr;
	va_list args;
	va_start(args,format);
	if(timestampEnabled)
		fprintf(logfile, "[%f]: ", OCTimeIntervalSinceReferenceDate());
	vfprintf(logfile, format, args);
	fprintf(logfile, "\n");
	fflush(logfile);
	va_end(args);
}

void OCLogOCString(OCStringRef string){
	OCLog("%s", OCStringGetCString(string));
}

void _OCLogError(char* file, int line, const char* function, const char* format, ...){
	if(logfile == 0)
		logfile = stderr;
	va_list args;
	va_start(args, format);
	char* found = strrchr(file, '/');
	if(found)
		file = found+1;
	if(timestampEnabled)
		fprintf(logfile, "[%f]: ", OCTimeIntervalSinceReferenceDate());
	fprintf(logfile, "|%s:%d| %s(): ", file, line, function);
	vfprintf(logfile, format, args);
	fprintf(logfile, "\n");
	fflush(logfile);
	va_end(args);
}
