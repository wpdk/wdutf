/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * WMI Routines.
 */

#include "stdddk.h"

extern "C" {
#include <wmilib.h>
}


NTSTATUS WmiCompleteRequest(PDEVICE_OBJECT DeviceObject,
    PIRP Irp, NTSTATUS Status, ULONG BufferUsed, CCHAR PriorityBoost)
{
	// TODO
	return STATUS_SUCCESS;
}


NTSTATUS WmiSystemControl(PWMILIB_CONTEXT WmiLibInfo,
    PDEVICE_OBJECT DeviceObject, PIRP Irp, PSYSCTL_IRP_DISPOSITION IrpDisposition)
{
	// TODO
	return STATUS_SUCCESS;
}


NTSTATUS WmiFireEvent(PDEVICE_OBJECT DeviceObject,
    LPCGUID Guid, ULONG InstanceIndex, ULONG EventDataSize, PVOID EventData)
{
	// TODO
	return STATUS_SUCCESS;
}
