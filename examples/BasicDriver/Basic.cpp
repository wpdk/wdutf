/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2017, DataCore Software Corporation. All rights reserved.
 */

#include <ddk.h>

#define WPPFILE "Basic.tmh"
#include "Trace.h"

extern "C" {
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;
DRIVER_DISPATCH BasicCreateClose;

LONG BasicTestFunction(LONG v);
}


NTSTATUS
DriverEntry(
    IN OUT PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING      RegistryPath
    )
{
	ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

    WPP_INIT_TRACING(DriverObject, RegistryPath);

	DriverObject->MajorFunction[IRP_MJ_CREATE] = BasicCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = BasicCreateClose;

    DriverObject->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}


VOID
DriverUnload(
	IN PDRIVER_OBJECT DriverObject
	)
{
	WPP_CLEANUP(DriverObject);
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
