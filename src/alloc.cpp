/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Allocation Routines
 */

#include "stdddk.h"


#undef ExFreePool
#undef ExInitializeNPagedLookasideList


DDKAPI
PVOID ExAllocatePoolWithTag(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag)
{
	return _aligned_malloc(NumberOfBytes, (NumberOfBytes < PAGE_SIZE) ? 16 : PAGE_SIZE);
}


DDKAPI
PVOID ExAllocatePool(POOL_TYPE PoolType, SIZE_T NumberOfBytes)
{
	return ExAllocatePoolWithTag(PoolType, NumberOfBytes, 0);
}


DDKAPI
PVOID ExAllocatePoolWithTagPriority(POOL_TYPE PoolType, SIZE_T NumberOfBytes,
    ULONG Tag, EX_POOL_PRIORITY Priority)
{
	return ExAllocatePoolWithTag(PoolType, NumberOfBytes, 0);
}


DDKAPI
VOID ExFreePoolWithTag(PVOID P, ULONG Tag)
{
	_aligned_free(P);
}


DDKAPI
VOID ExFreePool(PVOID P)
{
	ExFreePoolWithTag(P, 0);
}


DDKAPI
VOID ExInitializePagedLookasideList(PPAGED_LOOKASIDE_LIST Lookaside,
	PALLOCATE_FUNCTION Allocate, PFREE_FUNCTION Free, ULONG Flags, SIZE_T Size, ULONG Tag, USHORT Depth)
{
	memset(Lookaside, 0, sizeof(PAGED_LOOKASIDE_LIST));
	ExInitializeSListHead(&Lookaside->L.ListHead);
	Lookaside->L.Allocate = Allocate ? Allocate : ExAllocatePoolWithTag;
	Lookaside->L.Free = Free ? Free : ExFreePool;
	Lookaside->L.Size = (ULONG)Size;
	Lookaside->L.Tag = Tag;
	Lookaside->L.Depth = Depth;
	Lookaside->L.Type = PagedPool;
}


DDKAPI
VOID ExDeletePagedLookasideList(PPAGED_LOOKASIDE_LIST Lookaside)
{
	PVOID Entry;
	while ((Entry = (PVOID)InterlockedPopEntrySList(&Lookaside->L.ListHead)) != 0)
		(Lookaside->L.Free)(Entry);
}


DDKAPI
VOID ExInitializeNPagedLookasideList(PNPAGED_LOOKASIDE_LIST Lookaside,
	PALLOCATE_FUNCTION Allocate, PFREE_FUNCTION Free, ULONG Flags, SIZE_T Size, ULONG Tag, USHORT Depth)
{
	memset(Lookaside, 0, sizeof(NPAGED_LOOKASIDE_LIST));
	ExInitializeSListHead(&Lookaside->L.ListHead);
	Lookaside->L.Allocate = Allocate ? Allocate : ExAllocatePoolWithTag;
	Lookaside->L.Free = Free ? Free : ExFreePool;
	Lookaside->L.Size = (ULONG)Size;
	Lookaside->L.Tag = Tag;
	Lookaside->L.Depth = Depth;
	Lookaside->L.Type = NonPagedPool;
}


DDKAPI
VOID ExDeleteNPagedLookasideList(PNPAGED_LOOKASIDE_LIST Lookaside)
{
	PVOID Entry;
	while ((Entry = (PVOID)InterlockedPopEntrySList(&Lookaside->L.ListHead)) != 0)
		(Lookaside->L.Free)(Entry);
}


static
PVOID DdkAllocateEx(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag, PLOOKASIDE_LIST_EX Lookaside)
{
	UNREFERENCED_PARAMETER(Lookaside);
	return ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);
}


static
VOID DdkFreeEx(PVOID P, PLOOKASIDE_LIST_EX Lookaside)
{
	UNREFERENCED_PARAMETER(Lookaside);
	ExFreePoolWithTag(P, 0);
}


DDKAPI
NTSTATUS ExInitializeLookasideListEx(PLOOKASIDE_LIST_EX Lookaside,
    PALLOCATE_FUNCTION_EX Allocate, PFREE_FUNCTION_EX Free, POOL_TYPE PoolType,
	ULONG Flags, SIZE_T Size, ULONG Tag, USHORT Depth)
{
	memset(Lookaside, 0, sizeof(PLOOKASIDE_LIST_EX));
	ExInitializeSListHead(&Lookaside->L.ListHead);
	Lookaside->L.AllocateEx = Allocate ? Allocate : DdkAllocateEx;
	Lookaside->L.FreeEx = Free ? Free : DdkFreeEx;
	Lookaside->L.Type = PoolType;
	Lookaside->L.Size = (ULONG)Size;
	Lookaside->L.Tag = Tag;
	Lookaside->L.Depth = Depth;
	return STATUS_SUCCESS;
}


DDKAPI
VOID ExDeleteLookasideListEx(PLOOKASIDE_LIST_EX Lookaside)
{
	PVOID Entry;
	while ((Entry = (PVOID)InterlockedPopEntrySList(&Lookaside->L.ListHead)) != 0)
		(Lookaside->L.FreeEx)(Entry, Lookaside);
}

