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

#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "OCTime.h"

OCTimeInterval OCTimeIntervalSinceReferenceDate(void){
	struct timeval t;
	gettimeofday(&t, NULL);	
	return t.tv_sec + ((double)t.tv_usec)/1000000.f;
}


OCTimeInterval OCTimeDiff(OCTimeInterval t1, OCTimeInterval t2){
	return t2-t1;
}

void OCTimeSleep(OCTimeInterval t){
	unsigned int secs = (unsigned int)t;
	sleep(t);
	unsigned int usecs = (t - secs) * 1000000.f;
	usleep(usecs);
}
