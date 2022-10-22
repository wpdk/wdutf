/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * IRP Routines.
 */

#include "stdddk.h"


IO_COMPLETION_ROUTINE DdkIrpCompletion;
IO_COMPLETION_ROUTINE DdkIoctlCompletion;


NTSTATUS DdkSynchronousIrp(PDEVICE_OBJECT DeviceObject, UCHAR Major, UCHAR Minor, PIRP Irp)
{
	IO_STACK_LOCATION *pStack;
	KEVENT event;

	if (!Irp && (Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE)) == NULL)
 		return STATUS_INSUFFICIENT_RESOURCES;

	// Setup request

	pStack = IoGetNextIrpStackLocation(Irp);
	pStack->MajorFunction = Major;
	pStack->MinorFunction = Minor;

	KeInitializeEvent(&event, NotificationEvent, FALSE);
	IoSetCompletionRoutine(Irp, DdkIrpCompletion, (PVOID)&event, TRUE, TRUE, TRUE);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	Irp->Tail.Overlay.Thread = PsGetCurrentThread();

	// Issue request

	NTSTATUS status = IoCallDriver(DeviceObject, Irp);

	if (status == STATUS_PENDING) {
		KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, 0);
		status = Irp->IoStatus.Status;
	}

	IoFreeIrp(Irp);
	return status;
}


NTSTATUS DdkIrpCompletion(DEVICE_OBJECT *, IRP *, void *pContext)
{
	KeSetEvent((KEVENT *)pContext, IO_NO_INCREMENT, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS DdkIoctlCompletion(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
    if (Irp->Flags & IRP_BUFFERED_IO) {
        RtlCopyMemory(Irp->UserBuffer, Irp->AssociatedIrp.SystemBuffer, Irp->IoStatus.Information);
    }

	// TODO - copy data buffers

	if (Irp->UserIosb)
		*(Irp->UserIosb) = Irp->IoStatus;

	if (Irp->UserEvent)
		KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, FALSE);

	if (Irp->MdlAddress)
		IoFreeMdl(Irp->MdlAddress);

	IoFreeIrp(Irp);
	return STATUS_MORE_PROCESSING_REQUIRED;
}


DDKAPI
PIRP IoBuildDeviceIoControlRequest(ULONG IoControlCode, PDEVICE_OBJECT DeviceObject,
    PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength,
    BOOLEAN InternalDeviceIoControl, PKEVENT Event, PIO_STATUS_BLOCK IoStatusBlock)
{
	PIRP pIrp = IoAllocateIrp(DeviceObject->StackSize, FALSE);
	if (!pIrp) return 0;

	IO_STACK_LOCATION *pStack = IoGetNextIrpStackLocation(pIrp);

	pStack->MajorFunction = (InternalDeviceIoControl)
		? IRP_MJ_INTERNAL_DEVICE_CONTROL : IRP_MJ_DEVICE_CONTROL;

	pStack->MinorFunction = 0;
	pStack->Parameters.DeviceIoControl.IoControlCode = IoControlCode;
	pStack->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
	pStack->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;
	pStack->DeviceObject = DeviceObject;

	switch (METHOD_FROM_CTL_CODE(IoControlCode)) {
	case METHOD_BUFFERED:
		pIrp->AssociatedIrp.SystemBuffer = ExAllocatePool(NonPagedPool,
			(InputBufferLength > OutputBufferLength) ? InputBufferLength : OutputBufferLength);

		if (!pIrp->AssociatedIrp.SystemBuffer) {
			IoFreeIrp(pIrp);
			return 0;
		}

		memcpy(pIrp->AssociatedIrp.SystemBuffer, InputBuffer, InputBufferLength);
        pIrp->UserBuffer = OutputBuffer;
		pIrp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);
		break;

	case METHOD_IN_DIRECT:
	case METHOD_OUT_DIRECT:
		pIrp->AssociatedIrp.SystemBuffer = InputBuffer;
		pIrp->Flags |= IRP_BUFFERED_IO;

		if (!IoAllocateMdl(OutputBuffer, OutputBufferLength, FALSE, FALSE, pIrp)) {
			IoFreeIrp(pIrp);
			return 0;
		}
		break;

	case METHOD_NEITHER:
		pStack->Parameters.DeviceIoControl.Type3InputBuffer = InputBuffer;
		pIrp->UserBuffer = OutputBuffer;
		break;
	}

	pIrp->UserEvent = Event;
	pIrp->UserIosb = IoStatusBlock;
	pIrp->RequestorMode = KernelMode;

	IoSetCompletionRoutine(pIrp, DdkIoctlCompletion, 0, TRUE, TRUE, TRUE);
	return pIrp;
}


DDKAPI
PIRP IoAllocateIrp(CCHAR StackSize, BOOLEAN ChargeQuota)
{
	IRP *pIrp = (IRP *)ExAllocatePool(NonPagedPool, IoSizeOfIrp(StackSize));

	if (pIrp) IoInitializeIrp(pIrp, IoSizeOfIrp(StackSize), StackSize);
	return pIrp;
}


DDKAPI
VOID IoInitializeIrp(PIRP Irp, USHORT PacketSize, CCHAR StackSize)
{
	memset(Irp, 0, PacketSize);

	Irp->Size = PacketSize;
	Irp->StackCount = StackSize;
	Irp->CurrentLocation = StackSize + 1;

	Irp->Tail.Overlay.CurrentStackLocation =
		(PIO_STACK_LOCATION)(((char *)Irp) + PacketSize);
}


DDKAPI
VOID IoFreeIrp(PIRP Irp)
{
	ExFreePool(Irp);
}


DDKAPI
NTSTATUS IofCallDriver(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	DDKASSERT(Irp->CurrentLocation > 0);

	IoSetNextIrpStackLocation(Irp);
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(Irp);

	pStack->DeviceObject = DeviceObject;
	return DeviceObject->DriverObject->MajorFunction[pStack->MajorFunction](DeviceObject, Irp);
}


DDKAPI
VOID IofCompleteRequest(PIRP Irp, CCHAR PriorityBoost)
{
	while (Irp->CurrentLocation <= Irp->StackCount) {
		PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(Irp);
		PIO_COMPLETION_ROUTINE Completion = pStack->CompletionRoutine;
		PVOID Context = pStack->Context;
		UCHAR Control = pStack->Control;

		Irp->PendingReturned = ((Control & SL_PENDING_RETURNED) != 0);

		IoSkipCurrentIrpStackLocation(Irp);
		memset(pStack, 0, sizeof(*pStack));

		if ((NT_SUCCESS(Irp->IoStatus.Status) && (Control & SL_INVOKE_ON_SUCCESS))
		|| (!NT_SUCCESS(Irp->IoStatus.Status) && (Control & SL_INVOKE_ON_ERROR))
		|| (Irp->Cancel && (Control & SL_INVOKE_ON_CANCEL))) {
			PDEVICE_OBJECT DeviceObject = (Irp->CurrentLocation <= Irp->StackCount)
				? IoGetCurrentIrpStackLocation(Irp)->DeviceObject : NULL;

			if (Completion(DeviceObject, Irp, Context) == STATUS_MORE_PROCESSING_REQUIRED)
				return;

			continue;
		}
		
		if (Irp->PendingReturned && Irp->CurrentLocation <= Irp->StackCount)
			IoMarkIrpPending(Irp);
	}

	ddkfail("IoCompleteRequest completing IRP");
}

