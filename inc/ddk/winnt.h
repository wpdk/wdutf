/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Basic Type Definitions.
 */

#ifndef _DDK_WINNT_H_
#define _DDK_WINNT_H_


#ifndef _NTDEF_

// Use path to bypass include alias
#include <../um/winnt.h>

#else

/*
 *	DDK headers have been included
 */

#include <minwindef.h>
#include <intsafe.h>
#include <ntifs.h>
#include <ddk/winnt_defs.h>
#include <errhandlingapi.h>

#endif


#endif /* _DDK_WINNT_H_ */