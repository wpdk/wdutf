/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2017, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Wait Routines
 */

#include "stdafx.h"


extern void DdkUpdateMutex(OBJECT *pObj);
extern void DdkUpdateSemaphore(OBJECT *pObj);


static HANDLE nullevent;


void DdkWaitInit()
{
	nullevent = CreateEvent(NULL, FALSE, FALSE, NULL);
}


static POBJECT GetDispatch(OBJECT *pObj)
{
	if (!pObj || !isDispatchObject(pObj) || !pObj->h)
		ddkfail("Invalid dispatch object");

	return pObj;
}


static NTSTATUS WaitStatus(OBJECT **pVec, ULONG count, int all, DWORD rc)
{
	if ((LONG)rc >= WAIT_OBJECT_0 && rc < WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS) {
		for (ULONG i = (all) ? 0 : (rc - WAIT_OBJECT_0); i < count; i++) {
			if (pVec[i]->type == SemaphoreType) DdkUpdateSemaphore(pVec[i]);
			if (pVec[i]->type == MutexType) DdkUpdateMutex(pVec[i]);
			if (!all) break;
		}

		return (STATUS_WAIT_0 + (rc - WAIT_OBJECT_0));
	}

	if (rc == WAIT_TIMEOUT) return STATUS_TIMEOUT;
	if (rc == WAIT_IO_COMPLETION) return STATUS_ALERTED;
	return STATUS_UNSUCCESSFUL;
}


DDKAPI
NTSTATUS KeWaitForSingleObject(PVOID Object, KWAIT_REASON WaitReason,
	KPROCESSOR_MODE WaitMode, BOOLEAN Alertable, PLARGE_INTEGER Timeout)
{
	return KeWaitForMultipleObjects(1, &Object, WaitAny, WaitReason, WaitMode, Alertable, Timeout, NULL);
}


DDKAPI
NTSTATUS KeWaitForMultipleObjects(ULONG Count, PVOID Object[], WAIT_TYPE WaitType,
	KWAIT_REASON WaitReason, KPROCESSOR_MODE WaitMode, BOOLEAN Alertable,
	PLARGE_INTEGER Timeout, PKWAIT_BLOCK WaitBlockArray)
{
	POBJECT obj[MAXIMUM_WAIT_OBJECTS];
	HANDLE h[MAXIMUM_WAIT_OBJECTS];
	ULONG i = 0;

	if (Count > (WaitBlockArray ? MAXIMUM_WAIT_OBJECTS : ThreadWaitObjects))
		KeBugCheck(MAXIMUM_WAIT_OBJECTS_EXCEEDED);

	for (; i < Count; i++) {
		obj[i] = GetDispatch(FromPointer(Object[i]));
		h[i] = obj[i]->h;
	}

	// Adding a dummy entry seems to significantly speed up
	// WaitForSingleObject when used with timers and threadpools

	if (i == 1 && nullevent) h[i++] = nullevent;

	DWORD rc = WaitForMultipleObjectsEx(i, h,
		(WaitType == WaitAll), DdkGetWaitTime(Timeout), (Alertable != FALSE));

	return WaitStatus(obj, Count, (WaitType == WaitAll), rc);
}


DDKAPI
NTSTATUS KeDelayExecutionThread(KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable, PLARGE_INTEGER Interval)
{
	DDKASSERT(KeGetCurrentIrql() <= APC_LEVEL);

	DWORD rc = SleepEx(DdkGetDelayTime(Interval), (Alertable != FALSE));

	if (rc == WAIT_IO_COMPLETION) return STATUS_ALERTED;
	return STATUS_SUCCESS;
}
