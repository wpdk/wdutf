/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Object Routines
 */

#include "stdafx.h"
#include <time.h>


void DdkFreeObject(OBJECT *pObj);


static VOID DdkObjectCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	OBJECT *pObj = (POBJECT)Context;

	DdkFreeObject(pObj);
	CloseThreadpoolWork(Work);
}


OBJECT *DdkAllocObject(size_t size, USHORT type, bool opt)
{
	if (type != ThreadType) DdkThreadInit();

	OBJECT *pObj = (OBJECT *)calloc(1, size);

	if (!pObj) {
		if (!opt) ddkfail("Unable to allocate object");
		return 0;
	}

	pObj->refcount = 1;
	pObj->type = type;
	pObj->alloc = 1;
	return pObj;
}


void DdkFreeObject(OBJECT *pObj)
{
	if (pObj->type == IoFileType) DdkFreeFileObject(pObj);
	if (pObj->type == IoDriverType) DdkFreeDriverObject(pObj);
	if (pObj->type == IoDeviceType) DdkFreeDeviceObject(pObj);

	if (pObj->h) {
		DDKASSERT(pObj->h != GetCurrentProcess());
		CloseHandle(pObj->h);
		pObj->h = NULL;
	}

	DdkRemoveObjectName(pObj);
	if (pObj->alloc) free(pObj);
}


void DdkInitializeObject(OBJECT *pObj, size_t size, size_t maxsize)
{
	DdkThreadInit();

	if (size > maxsize)
		ddkfail("Object is too small to initialize");

	memset(pObj, 0, maxsize);
	pObj->refcount = 1;
}


void DdkReferenceObject(OBJECT *pObj)
{
	InterlockedIncrement(&pObj->refcount);
}


DDKAPI
LONG_PTR ObfReferenceObjectWithTag(PVOID Object, ULONG Tag)
{
	OBJECT *pObj = FromPointer(Object);

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	InterlockedIncrement(&pObj->refcount);
	return (LONG_PTR)0;
}


DDKAPI
LONG_PTR ObfReferenceObject(PVOID Object)
{
	return ObfReferenceObjectWithTag(Object, 'tlfD');
}


#undef ObReferenceObject

DDKAPI
LONG_PTR ObReferenceObject(PVOID Object)
{
	return ObfReferenceObjectWithTag(Object, 'tlfD');
}


#undef ObReferenceObjectWithTag

DDKAPI
LONG_PTR ObReferenceObjectWithTag(PVOID Object, ULONG Tag)
{
	return ObfReferenceObjectWithTag(Object, Tag);
}


DDKAPI
NTSTATUS ObReferenceObjectByPointerWithTag(PVOID Object, ACCESS_MASK DesiredAccess,
	POBJECT_TYPE ObjectType, KPROCESSOR_MODE AccessMode, ULONG Tag)
{
	OBJECT *pObj = FromPointer(Object);

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	if (ObjectType && ObjectType->type != pObj->type)
		return STATUS_OBJECT_TYPE_MISMATCH;

	InterlockedIncrement(&pObj->refcount);
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS ObReferenceObjectByPointer(PVOID Object,
	ACCESS_MASK DesiredAccess, POBJECT_TYPE ObjectType, KPROCESSOR_MODE AccessMode)
{
	return ObReferenceObjectByPointerWithTag(Object,
		DesiredAccess, ObjectType, AccessMode, 'tlfD');
}


DDKAPI
NTSTATUS ObReferenceObjectByHandleWithTag(HANDLE Handle,
    ACCESS_MASK DesiredAccess, POBJECT_TYPE ObjectType, KPROCESSOR_MODE AccessMode,
    ULONG Tag, PVOID *Object, POBJECT_HANDLE_INFORMATION HandleInformation)
{
	OBJECT *pObj = FromHandle(Handle);

	if (!pObj) return STATUS_INVALID_HANDLE;

	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	DDKASSERT(HandleInformation == NULL);

	NTSTATUS rc = ObReferenceObjectByPointerWithTag(
		ToPointer(pObj), DesiredAccess, ObjectType, AccessMode, Tag);

	if (rc != STATUS_SUCCESS) return rc;

	*Object = ToPointer(pObj);
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS ObReferenceObjectByHandle(HANDLE Handle,
    ACCESS_MASK DesiredAccess, POBJECT_TYPE ObjectType, KPROCESSOR_MODE AccessMode,
    PVOID *Object, POBJECT_HANDLE_INFORMATION HandleInformation)
{
	return ObReferenceObjectByHandleWithTag(Handle,
		DesiredAccess, ObjectType, AccessMode, 'tlfD', Object, HandleInformation);
}


void DdkDereferenceObject(OBJECT *pObj)
{
	if (!InterlockedDecrement(&pObj->refcount))
		DdkFreeObject(pObj);
}


DDKAPI
LONG_PTR ObfDereferenceObjectWithTag(PVOID Object, ULONG Tag)
{
	OBJECT *pObj = FromPointer(Object);

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	if (InterlockedDecrement(&pObj->refcount))
		return (ULONG_PTR)0;

	DdkFreeObject(pObj);
	return (ULONG_PTR)0;
}


DDKAPI
LONG_PTR ObfDereferenceObject(PVOID Object)
{
	return ObfDereferenceObjectWithTag(Object, 'tlfD');
}


#undef ObDereferenceObject

DDKAPI
LONG_PTR ObDereferenceObject(PVOID Object)
{
	return ObfDereferenceObjectWithTag(Object, 'tlfD');
}


#undef ObDereferenceObjectWithTag

DDKAPI
LONG_PTR ObDereferenceObjectWithTag(PVOID Object, ULONG Tag)
{
	return ObfDereferenceObjectWithTag(Object, Tag);
}


DDKAPI
VOID ObDereferenceObjectDeferDeleteWithTag(PVOID Object, ULONG Tag)
{
	OBJECT *pObj = FromPointer(Object);

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	if (InterlockedDecrement(&pObj->refcount)) return;

	PTP_WORK Work = CreateThreadpoolWork(DdkObjectCallback, (PVOID)pObj, NULL);

	if (!Work)
		ddkfail("Unable to defer object deletion");

	SubmitThreadpoolWork(Work);
}


DDKAPI
VOID ObDereferenceObjectDeferDelete(PVOID Object)
{
	ObDereferenceObjectDeferDeleteWithTag(Object, 'tlfD');
}


DDKAPI
NTSTATUS ZwClose(HANDLE Handle)
{
	OBJECT *pObj = FromHandle(Handle);

	if (!pObj) return STATUS_INVALID_HANDLE;

	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	ObfDereferenceObject(pObj);
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS ObCloseHandle(HANDLE Handle, KPROCESSOR_MODE PreviousMode)
{
	return ZwClose(Handle);
}


BOOLEAN DdkWaitLastReference(POBJECT pObj, int maxsecs, int count)
{
	time_t start = time(NULL);

	while (pObj->refcount > count) {
		if (time(NULL) - start >= maxsecs)
			return FALSE;

		SleepEx(1, FALSE);
	}

	return TRUE;
}
