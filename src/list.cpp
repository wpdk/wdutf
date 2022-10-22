/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Doubly Linked List Routines
 */

#include "stdddk.h"


DDKAPI
PLIST_ENTRY ExInterlockedRemoveHeadList(PLIST_ENTRY ListHead, PKSPIN_LOCK Lock)
{
	PLIST_ENTRY Entry;
	KIRQL irql;
	KeAcquireSpinLock(Lock, &irql);
	Entry = RemoveHeadList(ListHead);
	KeReleaseSpinLock(Lock, irql);
	return Entry;
}


DDKAPI
PLIST_ENTRY ExInterlockedInsertHeadList(PLIST_ENTRY ListHead,
    PLIST_ENTRY ListEntry, PKSPIN_LOCK Lock)
{
	PLIST_ENTRY Entry;
	KIRQL irql;
	KeAcquireSpinLock(Lock, &irql);
	Entry = !IsListEmpty(ListHead) ? ListHead->Flink : 0;
	InsertHeadList(ListHead, ListEntry);
	KeReleaseSpinLock(Lock, irql);
	return Entry;
}


DDKAPI
PLIST_ENTRY ExInterlockedInsertTailList(PLIST_ENTRY ListHead,
    PLIST_ENTRY ListEntry, PKSPIN_LOCK Lock)
{
	PLIST_ENTRY Entry;
	KIRQL irql;
	KeAcquireSpinLock(Lock, &irql);
	Entry = !IsListEmpty(ListHead) ? ListHead->Blink : 0;
	InsertTailList(ListHead, ListEntry);
	KeReleaseSpinLock(Lock, irql);
	return Entry;
}
