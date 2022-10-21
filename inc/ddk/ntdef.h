/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2013-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Basic Type Definitions.
 */

#ifndef _DDK_NTDEF_H_
#define _DDK_NTDEF_H_


#ifndef _WINNT_

// Use path to bypass include alias
#include <../shared/ntdef.h>

// Ensure that winnt.h is included
#undef NT_INCLUDED

#else

/*
 *	Windows headers have been included
 */

#define _DDK_DRIVER_

// Add some missing definitions
#include <intsafe.h>
#include <winternl.h>
#include <ddk/ntdef_defs.h>

#endif


#endif /* _DDK_NTDEF_H_ */
