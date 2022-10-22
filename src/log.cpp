/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Event Log Routines
 */

#include "stdddk.h"


DDKAPI
PVOID IoAllocateErrorLogEntry(PVOID IoObject, UCHAR EntrySize)
{
	// TODO
	return 0;
}


DDKAPI
VOID IoWriteErrorLogEntry(PVOID ElEntry)
{
	// TODO
}
