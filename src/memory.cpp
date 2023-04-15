/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2016, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Memory Manager Routines.
 */

#include "stdddk.h"


#undef MmMapLockedPagesSpecifyCache

extern PVOID DdkGetProcAddress(char *name);


DDKAPI
VOID MmProbeAndLockPages(PMDLX Mdl,
	KPROCESSOR_MODE AccessMode, LOCK_OPERATION Operation)
{
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	ULONG pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
		MmGetMdlVirtualAddress(Mdl), MmGetMdlByteCount(Mdl));

	for (ULONG i = 0; i < pages; i++)
		MmGetMdlPfnArray(Mdl)[i] = (PFN_NUMBER)
			((ULONG_PTR)MmGetMdlVirtualAddress(Mdl) >> PAGE_SHIFT) + i;

	Mdl->MdlFlags |= MDL_PAGES_LOCKED;
}


DDKAPI
PVOID MmMapLockedPagesSpecifyCache(PMDL Mdl,
    KPROCESSOR_MODE AccessMode, MEMORY_CACHING_TYPE CacheType,
	PVOID RequestedAddress, ULONG BugCheckOnFailure, ULONG Priority)
{
	// TODO - map virtual memory into MDL

	return (PVOID)((PCHAR)(Mdl->StartVa) + Mdl->ByteOffset);
}


DDKAPI
PVOID MmMapLockedPages(PMDL Mdl, KPROCESSOR_MODE AccessMode)
{
	// TODO - map virtual memory into MDL

	return (PVOID) ((PCHAR) (Mdl->StartVa) + Mdl->ByteOffset);
}


DDKAPI
NTSTATUS MmProtectMdlSystemAddress(PMDL MemoryDescriptorList, ULONG NewProtect)
{
	return STATUS_SUCCESS;
}


DDKAPI
VOID MmUnmapLockedPages(PVOID BaseAddress, PMDL Mdl)
{
}


DDKAPI
VOID MmUnlockPages(PMDLX Mdl)
{
	Mdl->MdlFlags &= ~MDL_PAGES_LOCKED;
}


DDKAPI
PHYSICAL_ADDRESS MmGetPhysicalAddress(PVOID BaseAddress)
{
	PHYSICAL_ADDRESS x;
	x.QuadPart = (ULONG_PTR)BaseAddress;
	return x;
}


DDKAPI
PVOID MmGetSystemRoutineAddress(PUNICODE_STRING SystemRoutineName)
{
	char name[256];

	int n = _snprintf(name, sizeof(name), "%.*S",
		(int)(SystemRoutineName->Length / sizeof(WCHAR)),
		SystemRoutineName->Buffer);

	if (n < 1 || n > sizeof(name) - 1)
		return NULL;

	if (strnicmp(name, "ddk", 3) == 0)
		return NULL;

	return DdkGetProcAddress(name);
}
