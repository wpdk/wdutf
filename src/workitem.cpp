/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Work Item Routines
 */

#include "stdafx.h"


typedef struct _IO_WORKITEM {
	PTP_WORK Work;
	void *IoObject;
	PIO_WORKITEM_ROUTINE WorkerRoutine;
	PIO_WORKITEM_ROUTINE_EX WorkerRoutineEx;
	WORK_QUEUE_TYPE QueueType;
	void *Context;
	volatile LONG queued;
} IO_WORKITEM;


__declspec(thread) bool DdkWorkItemActive = false;


static VOID DdkWorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	PIO_WORKITEM pWork = (IO_WORKITEM *)Context;
	IO_WORKITEM w = *pWork;

	DdkThreadInit();
	DdkWorkItemActive = true;
	KeLowerIrql(PASSIVE_LEVEL);
	InterlockedExchange(&pWork->queued, 0);

	if (w.WorkerRoutineEx)
		(*w.WorkerRoutineEx)(w.IoObject, w.Context, pWork);

	else (*w.WorkerRoutine)((PDEVICE_OBJECT)(w.IoObject), w.Context);

	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	ObDereferenceObject(w.IoObject);
	DdkWorkItemActive = false;
}


DDKAPI
ULONG IoSizeofWorkItem()
{
	return (ULONG)sizeof(IO_WORKITEM);
}


DDKAPI
VOID IoInitializeWorkItem(PVOID IoObject, PIO_WORKITEM IoWorkItem)
{
	DDKASSERT(IoObject);
	memset(IoWorkItem, 0, sizeof(IO_WORKITEM));

	IoWorkItem->Work = CreateThreadpoolWork(DdkWorkCallback, (PVOID)IoWorkItem, NULL);

	if (!IoWorkItem->Work)
		ddkfail("Unable to initialize WorkItem");

	IoWorkItem->IoObject = IoObject;
}


DDKAPI
VOID IoUninitializeWorkItem(PIO_WORKITEM IoWorkItem)
{
	DDKASSERT(IoWorkItem);
	DDKASSERT(!IoWorkItem->queued);

	CloseThreadpoolWork(IoWorkItem->Work);
	memset(IoWorkItem, 0, sizeof(IO_WORKITEM));
}


DDKAPI
PIO_WORKITEM IoAllocateWorkItem(PDEVICE_OBJECT DeviceObject)
{
	DDKASSERT(DeviceObject);
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	IO_WORKITEM *pWork = (IO_WORKITEM *)ExAllocatePool(NonPagedPool, sizeof(IO_WORKITEM));

	if (pWork) IoInitializeWorkItem(DeviceObject, pWork);
	return pWork;
}


DDKAPI
VOID IoQueueWorkItem(PIO_WORKITEM IoWorkItem,
		PIO_WORKITEM_ROUTINE WorkerRoutine, WORK_QUEUE_TYPE QueueType, PVOID Context)
{
	DDKASSERT(IoWorkItem);
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	IoWorkItem->WorkerRoutine = WorkerRoutine;
	IoWorkItem->QueueType = QueueType;
	IoWorkItem->Context = Context;
	IoWorkItem->queued = true;

	ObReferenceObject(IoWorkItem->IoObject);
	SubmitThreadpoolWork(IoWorkItem->Work);
}


DDKAPI
VOID IoQueueWorkItemEx(PIO_WORKITEM IoWorkItem,
    PIO_WORKITEM_ROUTINE_EX WorkerRoutine, WORK_QUEUE_TYPE QueueType, PVOID Context)
{
	DDKASSERT(IoWorkItem);
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	IoWorkItem->WorkerRoutineEx = WorkerRoutine;
	IoWorkItem->QueueType = QueueType;
	IoWorkItem->Context = Context;
	IoWorkItem->queued = true;

	ObReferenceObject(IoWorkItem->IoObject);
	SubmitThreadpoolWork(IoWorkItem->Work);
}


DDKAPI
VOID IoFreeWorkItem(PIO_WORKITEM IoWorkItem)
{
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	IoUninitializeWorkItem(IoWorkItem);
	ExFreePool(IoWorkItem);
}


BOOLEAN DdkIsWorkItem()
{
	return (DdkWorkItemActive != false);
}
