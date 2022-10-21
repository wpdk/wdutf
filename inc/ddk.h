/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2017, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Interface to platform DDK.
 */

#ifdef _DEBUG
#define DBG 1
#endif

#if !defined(_DCS_KERNEL_) && !defined(_DDKSTUB_)
#pragma include_alias(<ntddk.h>,<ddklib.h>)
#endif

#if defined(_DCSNODECL_)
#define __declspec(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#ifdef _DDK_DRIVER_
/*
** PFD: include the safe string definitions but - for now - silence the deprecated warning for:
**    ustring.h (*) : warning C4995: '_snwprintf': name was marked as #pragma deprecated
**    dcsdefs.h(550) : warning C4996: 'RtlConvertLongToLargeInteger': was declared deprecated
*/
#include <Ntstrsafe.h>
#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

#endif
#ifdef __cplusplus
}
#endif


// Redefine ASSERT and ASSERTMSG as NT_ASSERT and NT_ASSERTMSG
// ASSERT does not cause the system to crash in Win 2008 when the assertion is false.
// NT_ASSERT does.

#if defined(_IA64_) || defined(_AMD64_)

#ifdef ASSERT
#undef ASSERT
#define ASSERT(exp)	NT_ASSERT(exp)
#endif

#ifdef ASSERTMSG
#undef ASSERTMSG
#define ASSERTMSG(msg, exp)  NT_ASSERTMSG(msg,exp)
#endif

#endif
