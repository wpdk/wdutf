/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * MDL Routines.
 */

#include "stdddk.h"


DDKAPI
PMDL IoAllocateMdl(PVOID VirtualAddress, ULONG Length,
	BOOLEAN SecondaryBuffer, BOOLEAN ChargeQuota, PIRP Irp)
{
	MDL *pMdl = (MDL *)ExAllocatePool(NonPagedPool, MmSizeOfMdl(VirtualAddress, Length));
	MDL **ppMdl;

	DDKASSERT(ChargeQuota == FALSE);
	DDKASSERT(Irp || !SecondaryBuffer);

	if (!pMdl) return 0;

	memset(pMdl, 0, MmSizeOfMdl(VirtualAddress, Length));
	MmInitializeMdl(pMdl, VirtualAddress, Length);

	if (!Irp) return pMdl;

	for (ppMdl = &Irp->MdlAddress; SecondaryBuffer && *ppMdl; )
		ppMdl = &(*ppMdl)->Next;

	*ppMdl = pMdl;
	return pMdl;
}


DDKAPI
VOID IoBuildPartialMdl(PMDL SourceMdl,
	PMDL TargetMdl, PVOID VirtualAddress, ULONG Length)
{
	ULONG_PTR SourceAddress = (ULONG_PTR)MmGetMdlVirtualAddress(SourceMdl);
	ULONG_PTR Address = (ULONG_PTR)VirtualAddress;

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	DDKASSERT(SourceMdl->MdlFlags & (MDL_PAGES_LOCKED \
		| MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL));

	DDKASSERT(Address >= SourceAddress);
	DDKASSERT(Address < SourceAddress + MmGetMdlByteCount(SourceMdl));

	if (!Length)
		Length = MmGetMdlByteCount(SourceMdl) - (ULONG)(Address - SourceAddress);

	DDKASSERT(Length != 0);
	DDKASSERT(Address + Length <= SourceAddress + MmGetMdlByteCount(SourceMdl));
	DDKASSERT(TargetMdl->Size >= MmSizeOfMdl(VirtualAddress, Length));

	MmInitializeMdl(TargetMdl, VirtualAddress, Length);

	ULONG pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(VirtualAddress, Length);

	for (ULONG i = 0; i < pages; i++)
		MmGetMdlPfnArray(TargetMdl)[i] = (PFN_NUMBER)
			((ULONG_PTR)MmGetMdlVirtualAddress(TargetMdl) >> PAGE_SHIFT) + i;

	TargetMdl->MdlFlags |= MDL_PARTIAL;
}


DDKAPI
VOID IoFreeMdl(PMDL Mdl)
{
	ExFreePool(Mdl);
}


DDKAPI
SIZE_T MmSizeOfMdl(PVOID Base, SIZE_T Length)
{
	return sizeof(MDL) + ADDRESS_AND_SIZE_TO_SPAN_PAGES(Base, Length) * sizeof(PFN_NUMBER);
}
