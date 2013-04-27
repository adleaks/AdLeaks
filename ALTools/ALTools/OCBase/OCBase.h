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

#ifndef _OC_BASE_H_
#define _OC_BASE_H_

#include "OCTypes.h"
#include "OCObject.h"
#include "OCLog.h"
#include "OCString.h"

void OCBaseInit(void);

// helper for warnings regarding unused parameters
#define VAR_UNUSED(x) (void)(x)

OCStringRef OCBaseGetHomeDirectory(void);

int OCExit(int ret) ;

#endif
