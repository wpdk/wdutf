/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Singly Linked List Routines
 */

#include "stdafx.h"


/*
 *	Singly Linked List Routines
 */

DDKAPI
USHORT ExQueryDepthSList(PSLIST_HEADER SListHead)
{
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	return QueryDepthSList(SListHead);
}


DDKAPI
PSLIST_ENTRY ExpInterlockedPopEntrySList(PSLIST_HEADER ListHead)
{
	return InterlockedPopEntrySList(ListHead);
}


DDKAPI
PSLIST_ENTRY ExpInterlockedPushEntrySList(
	PSLIST_HEADER ListHead, PSLIST_ENTRY ListEntry)
{
	return InterlockedPushEntrySList(ListHead, ListEntry);
}


DDKAPI
PSLIST_ENTRY ExpInterlockedFlushSList(PSLIST_HEADER ListHead)
{
	return InterlockedFlushSList(ListHead);
}
