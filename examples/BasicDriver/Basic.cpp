/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2017, DataCore Software Corporation. All rights reserved.
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 *
 */

#include <ddk.h>
#include "BasicIoctl.h"

#define WPPFILE "Basic.tmh"
#include "Trace.h"


extern "C" {
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;
DRIVER_DISPATCH BasicCreateClose;
DRIVER_DISPATCH BasicDeviceControl;
LONG BasicTestFunction(LONG v);
}

DRIVER_OBJECT *pDriverObject;
DEVICE_OBJECT* pDeviceObject;


NTSTATUS
DriverEntry(
    IN OUT PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING      RegistryPath
    )
{
	ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

    pDriverObject = DriverObject;

    WPP_INIT_TRACING(DriverObject, RegistryPath);

	DriverObject->MajorFunction[IRP_MJ_CREATE] = BasicCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = BasicCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = BasicDeviceControl;
    DriverObject->DriverUnload = DriverUnload;

    UNICODE_STRING u;

    RtlInitUnicodeString(&u, L"\\Device\\Basic");

    NTSTATUS status = IoCreateDevice(pDriverObject, 0, &u,
        FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);

    if (!NT_SUCCESS(status))
        return status;

	return STATUS_SUCCESS;
}


VOID
DriverUnload(
	IN PDRIVER_OBJECT DriverObject
	)
{
    if (pDeviceObject)
        IoDeleteDevice(pDeviceObject);

	WPP_CLEANUP(DriverObject);

    pDeviceObject = NULL;
    pDriverObject = NULL;
}


NTSTATUS
BasicCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
	UNREFERENCED_PARAMETER(DeviceObject); 

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}


__declspec(noinline) LONG
PrivateTestFunction(LONG v)
{
    return v + 1;
}


LONG
BasicTestFunction(LONG v)
{
    return PrivateTestFunction(v) + 1;
}


LONG
BasicTestFunction2(LONG v)
{
    return PrivateTestFunction(v) + 2;
}


NTSTATUS
BasicDeviceControl (
    IN DEVICE_OBJECT *pDevice,
    IN IRP *pIrp
    )
{
    UNREFERENCED_PARAMETER(pDevice);

    IO_STACK_LOCATION *pStack = IoGetCurrentIrpStackLocation(pIrp);
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG retbytes = 0;

    switch (pStack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_BASIC_TEST_FUNCTION:
        if (pStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(LONG) ||
                pStack->Parameters.DeviceIoControl.OutputBufferLength != sizeof(LONG)) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        LONG v = PrivateTestFunction(*(LONG *)pIrp->AssociatedIrp.SystemBuffer) + 3;
        RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, &v, sizeof(LONG));

        retbytes = sizeof(LONG);
        status = STATUS_SUCCESS;
        break;
    }

    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = retbytes;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return status;
}
