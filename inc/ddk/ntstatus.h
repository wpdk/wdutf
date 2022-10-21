/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Define NTSTATUS Codes.
 */

#ifndef _DDK_NTSTATUS_H_
#define _DDK_NTSTATUS_H_


// Ensure definition occurs

#undef _NTSTATUS_
#undef WIN32_NO_STATUS

// Ignore redefinition warnings

#pragma warning(push)
#pragma warning(disable:4005)

// Use path to bypass include alias

#include <../shared/ntstatus.h>

// Re-enable warnings

#pragma warning(pop)

// Prevent redefinitions

#define WIN32_NO_STATUS


#endif /* _DDK_NTSTATUS_H_ */