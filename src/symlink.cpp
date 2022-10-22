/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Symbolic Link Routines.
 */

#include "stdddk.h"


typedef struct _SYMLINK : public OBJECT {
	UNICODE_STRING	Destination;
	BOOLEAN			Enabled;
	WCHAR			Buffer[1];
} SYMLINK, *PSYMLINK;


DDKAPI
NTSTATUS IoCreateSymbolicLink(PUNICODE_STRING SymbolicLinkName, PUNICODE_STRING DeviceName)
{
	DDKASSERT(DeviceName && DeviceName->Length);
	DDKASSERT(SymbolicLinkName && SymbolicLinkName->Length);

	SYMLINK *pLink = (SYMLINK *)DdkAllocObject(
		sizeof(SYMLINK) + DeviceName->Length, IoSymbolicLinkType, true);

	if (!pLink) return STATUS_INSUFFICIENT_RESOURCES;

	wcsncpy(pLink->Buffer, DeviceName->Buffer, DeviceName->Length / sizeof(WCHAR));
	RtlInitUnicodeString(&pLink->Destination, pLink->Buffer);
	pLink->Enabled = TRUE;

	if (!DdkInsertName(SymbolicLinkName, pLink))
		return STATUS_OBJECT_NAME_EXISTS;

	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS IoDeleteSymbolicLink(PUNICODE_STRING SymbolicLinkName)
{
	DDKASSERT(SymbolicLinkName && SymbolicLinkName->Length);

	OBJECT *pObj = DdkLookupName(SymbolicLinkName);

	if (!pObj || pObj->type != IoSymbolicLinkType)
		return STATUS_OBJECT_NAME_NOT_FOUND;

	DdkRemoveObjectName(pObj);
	DdkDereferenceObject(pObj);
	return STATUS_SUCCESS;
}


POBJECT DdkResolveSymbolicLink(OBJECT *pObj)
{
	int count = 0;

	while (pObj && pObj->type == IoSymbolicLinkType) {
		if (count++ > 1000) ddkfail("Recursive Symbolic Link");

		SYMLINK *pSym = static_cast<SYMLINK *>(pObj);
		if (!pSym->Enabled) break;

		pObj = DdkLookupName(&pSym->Destination);
		ObDereferenceObject(pSym);
	}

	return pObj;
}


BOOLEAN DdkIsDeviceInterface(PUNICODE_STRING path, BOOLEAN all)
{
	OBJECT *pObj = DdkLookupName(path);
	BOOLEAN rc = FALSE;

	if (pObj && pObj->type == IoSymbolicLinkType) {
		SYMLINK *pSym = static_cast<SYMLINK *>(pObj);

		if (all || pSym->Enabled) {
			OBJECT *pDev = DdkLookupName(&pSym->Destination);
			rc = (pDev && pDev->type == IoDeviceType);
			if (pDev) DdkDereferenceObject(pDev);		
		}
	}

	if (pObj) DdkDereferenceObject(pObj);
	return rc;
}


DDKAPI
NTSTATUS IoSetDeviceInterfaceState(PUNICODE_STRING SymbolicLinkName, BOOLEAN Enable)
{
	NTSTATUS status = STATUS_OBJECT_NAME_NOT_FOUND;
	OBJECT *pObj = DdkLookupName(SymbolicLinkName);

	if (pObj && pObj->type == IoSymbolicLinkType) {
		SYMLINK *pSym = static_cast<SYMLINK *>(pObj);
		status = (!pSym->Enabled && !Enable) ? STATUS_OBJECT_NAME_NOT_FOUND :
				   (pSym->Enabled && Enable) ? STATUS_OBJECT_NAME_EXISTS : STATUS_SUCCESS;
		pSym->Enabled = Enable;
	}

	if (pObj) DdkDereferenceObject(pObj);
	return status;
}
