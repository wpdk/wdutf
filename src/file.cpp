/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * File Routines.
 */

#include "stdddk.h"
#include <ntifs.h>


typedef struct _FILEOBJ : public OBJECT {
	FILE_OBJECT File;
} FILEOBJ, *PFILEOBJ;


static PFILEOBJ GetFile(OBJECT *pObj)
{
	if (!pObj || pObj->type != IoFileType)
		ddkfail("Invalid file object specified");

	return static_cast<FILEOBJ *>(pObj);
}


static NTSTATUS SetStatus(
	PIO_STATUS_BLOCK IoStatusBlock, NTSTATUS status, ULONG info)
{
	IoStatusBlock->Information = info;
	IoStatusBlock->Status = status;
	return status;
}


DDKAPI
NTSTATUS IoCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess,
    ULONG Disposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength,
    CREATE_FILE_TYPE CreateFileType, PVOID InternalParameters, ULONG Options)
{
	DDKASSERT(ObjectAttributes);
	DDKASSERT(ObjectAttributes->ObjectName);
	DDKASSERT(IoStatusBlock);

	// TODO - handle create of file/directory

	OBJECT *pObj = DdkLookupName(ObjectAttributes->ObjectName);

	if (pObj && pObj->type == IoSymbolicLinkType) {
		if (Options & IO_STOP_ON_SYMLINK)
			return SetStatus(IoStatusBlock, STATUS_STOPPED_ON_SYMLINK, FILE_DOES_NOT_EXIST);

		pObj = DdkResolveSymbolicLink(pObj);
	}

	if (!pObj || pObj->type != IoDeviceType)
		return SetStatus(IoStatusBlock, STATUS_OBJECT_NAME_NOT_FOUND, FILE_DOES_NOT_EXIST);

	FILEOBJ *pFile = (FILEOBJ *)DdkAllocObject(sizeof(FILEOBJ), IoFileType, true);

	if (!pFile) {
		DdkDereferenceObject(pObj);
		return SetStatus(IoStatusBlock, STATUS_INSUFFICIENT_RESOURCES, FILE_EXISTS);
	}

	pFile->File.Type = IO_TYPE_FILE;
	pFile->File.Size = sizeof(FILE_OBJECT);
	pFile->File.DeviceObject = (PDEVICE_OBJECT)ToPointer(pObj);

	KeInitializeEvent(&pFile->File.Lock, SynchronizationEvent, TRUE);
	KeInitializeEvent(&pFile->File.Event, NotificationEvent, FALSE);
	KeInitializeSpinLock(&pFile->File.IrpListLock);
	InitializeListHead(&pFile->File.IrpList);

	*FileHandle = ToHandle(pFile);
	return SetStatus(IoStatusBlock, STATUS_SUCCESS, FILE_OPENED);
}


DDKAPI
PDEVICE_OBJECT IoGetRelatedDeviceObject(PFILE_OBJECT FileObject)
{
	FILEOBJ *pFile = GetFile(FromPointer(FileObject));

	return IoGetAttachedDevice(pFile->File.DeviceObject);
}


PFILE_OBJECT DdkGetFilePointer(HANDLE FileHandle)
{
	FILEOBJ *pObj = GetFile(FromHandle(FileHandle));
	return (PFILE_OBJECT)ToPointer(pObj);
}


void DdkFreeFileObject(OBJECT *pObj)
{
	FILEOBJ *pFile = GetFile(pObj);

	if (pFile->File.DeviceObject)
		DdkDereferenceObject(FromPointer(pFile->File.DeviceObject));
}
