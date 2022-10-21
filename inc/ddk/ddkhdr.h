/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Wrapper for DDK header.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <ntstatus.h>
#include <ntdef.h>

/*
 *	Check character set
 */

#ifdef UNICODE
#error "The project property 'Use Multi-Byte Character Set' is required"
#endif

#ifndef _WINNT_

#ifndef _DDKHDR_H_
#define _DDKHDR_H_


/*
 *	Remove conflicts
 */

#undef DEVICE_TYPE
#undef DEVICE_TYPE_FROM_CTL_CODE
#undef METHOD_FROM_CTL_CODE


// Use consistent PROCESS and THREAD definitions
#define _EPROCESS _KPROCESS
#define _ETHREAD _KTHREAD

#ifdef _DDKLIBBUILD_
#define dllimport dllexport
#endif

// Adjust data definitions
#ifdef _DDKLIBBUILD_
#include "data.h"
#endif

#include <_ddkhdr_>
#undef dllimport

// Ensure that winnt.h is included
#undef NT_INCLUDED

// Defines from devioctl.h are done
#define _DEVIOCTL_


/*
 *	Fixup residual issues
 */

#ifndef _DDKINLINE_
#undef KI_USER_SHARED_DATA
#define KI_USER_SHARED_DATA 0x7FFE0000UI64
#endif


#undef _DDKHDR_H_
#else
#include <_ddkhdr_>
#endif

#else

/*
 *	Windows headers have been included
 */

#include "ntddk_defs.h"
#endif
	

#ifdef __cplusplus
};
#endif
