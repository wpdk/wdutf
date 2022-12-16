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
#include <fileapi.h>
#include <handleapi.h>


DWORD DdkGetFileDisposition(ULONG Disposition, ULONG *pInfo);
DWORD DdkGetFileFlags(ULONG CreateOptions);
bool DdkIsFilePath(PUNICODE_STRING path);
BOOL DdkGetFileInfo(HANDLE h, ULONG FileInformationClass,
	PVOID buffer, ULONG Length, size_t InfoSize, DWORD *pBytesCopied);
NTSTATUS DdkGetFileStatus(DWORD rc);


typedef struct _FILEOBJ : public OBJECT {
	FILE_OBJECT File;
} FILEOBJ, *PFILEOBJ;


static PFILEOBJ GetFile(OBJECT *pObj)
{
	if (!pObj || pObj->type != IoFileType)
		ddkfail("Invalid file object specified");

	return static_cast<FILEOBJ *>(pObj);
}


static PFILEOBJ GetFileFromHandle(HANDLE h)
{
	OBJECT *pObj = FromHandle(h);

	if (!pObj || pObj->type != IoFileType)
		return NULL;

	return static_cast<FILEOBJ *>(pObj);
}


static NTSTATUS SetStatus(
	PIO_STATUS_BLOCK IoStatusBlock, NTSTATUS status, ULONG info)
{
	IoStatusBlock->Information = info;
	IoStatusBlock->Status = status;
	return status;
}


static FILEOBJ *DdkAllocFile()
{
	FILEOBJ *pFile = (FILEOBJ *)DdkAllocObject(sizeof(FILEOBJ), IoFileType, true);

	if (!pFile) return NULL;

	pFile->File.Type = IO_TYPE_FILE;
	pFile->File.Size = sizeof(FILE_OBJECT);

	KeInitializeEvent(&pFile->File.Lock, SynchronizationEvent, TRUE);
	KeInitializeEvent(&pFile->File.Event, NotificationEvent, FALSE);
	KeInitializeSpinLock(&pFile->File.IrpListLock);
	InitializeListHead(&pFile->File.IrpList);
	return pFile;
}


static
NTSTATUS DdkIoCreateDevice(FILEOBJ **ppFile, POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock, ULONG Options)
{
	OBJECT *pObj = DdkLookupName(ObjectAttributes->ObjectName);

	if (pObj && pObj->type == IoSymbolicLinkType) {
		if (Options & IO_STOP_ON_SYMLINK)
			return SetStatus(IoStatusBlock, STATUS_STOPPED_ON_SYMLINK, FILE_DOES_NOT_EXIST);

		pObj = DdkResolveSymbolicLink(pObj);
	}

	if (!pObj || pObj->type != IoDeviceType)
		return SetStatus(IoStatusBlock, STATUS_OBJECT_NAME_NOT_FOUND, FILE_DOES_NOT_EXIST);

	FILEOBJ *pFile = DdkAllocFile();

	if (!pFile) {
		DdkDereferenceObject(pObj);
		return SetStatus(IoStatusBlock, STATUS_INSUFFICIENT_RESOURCES, FILE_EXISTS);
	}

	pFile->File.DeviceObject = (PDEVICE_OBJECT)ToPointer(pObj);

	*ppFile = pFile;
	return SetStatus(IoStatusBlock, STATUS_SUCCESS, FILE_OPENED);
}


static
NTSTATUS DdkIoCreateDir(WCHAR *path,
	ULONG Disposition, ULONG *pDisp, bool *pCreated)
{
	if (Disposition != FILE_CREATE && Disposition != FILE_OPEN &&
			Disposition != FILE_OPEN_IF)
		return STATUS_INVALID_PARAMETER;

	if (Disposition == FILE_OPEN)
		return STATUS_SUCCESS;

	DWORD disp;
	*pDisp = DdkGetFileDisposition(FILE_OPEN, &disp);

	if (!CreateDirectoryW(path, NULL)) {
		DWORD rc = GetLastError();

		if (rc == ERROR_ALREADY_EXISTS) {
			*pCreated = false;
			return (Disposition != FILE_CREATE) ? STATUS_SUCCESS :
				STATUS_OBJECT_NAME_COLLISION;
		}

		return DdkGetFileStatus(rc);
	}

	*pCreated = true;
	return STATUS_SUCCESS;
}


static
NTSTATUS DdkIoCreateFile(FILEOBJ **ppFile, ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock,
	ULONG FileAttributes, ULONG ShareAccess, ULONG Disposition,
	ULONG CreateOptions)
{
	WCHAR path[MAX_PATH + 1];

	DdkGetLocalPath(path, sizeof(path) / sizeof(WCHAR),
		ObjectAttributes->ObjectName,
		(Disposition != FILE_OPEN && Disposition != FILE_OVERWRITE));

	ULONG info;
	bool created = true;
	DWORD disp = DdkGetFileDisposition(Disposition, &info);

	if (CreateOptions & FILE_DIRECTORY_FILE) {
		NTSTATUS status = DdkIoCreateDir(path, Disposition, &disp, &created);

		if (!NT_SUCCESS(status))
			return SetStatus(IoStatusBlock, status, 0);
	}

	SetLastError(ERROR_SUCCESS);

	HANDLE h = CreateFileW(path, DesiredAccess, ShareAccess,
		NULL, disp, (DWORD)FileAttributes | DdkGetFileFlags(CreateOptions), NULL);

	DWORD rc = GetLastError();

	if (h == INVALID_HANDLE_VALUE)
		return SetStatus(IoStatusBlock, DdkGetFileStatus(rc),
			(rc == ERROR_FILE_EXISTS) ? FILE_EXISTS : FILE_DOES_NOT_EXIST);

	if (Disposition == FILE_SUPERSEDE || Disposition == FILE_OPEN_IF ||
			Disposition == FILE_OVERWRITE_IF)
		if (rc != ERROR_ALREADY_EXISTS && created) info = FILE_CREATED;

	FILEOBJ *pFile = DdkAllocFile();

	if (!pFile) {
		CloseHandle(h);
		return SetStatus(IoStatusBlock, STATUS_INSUFFICIENT_RESOURCES, FILE_EXISTS);
	}

	pFile->h = h;

	*ppFile = pFile;
	return SetStatus(IoStatusBlock, STATUS_SUCCESS, info);
}


DDKAPI
NTSTATUS IoCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess,
    ULONG Disposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength,
    CREATE_FILE_TYPE CreateFileType, PVOID InternalParameters, ULONG Options)
{
	return IoCreateFileEx(FileHandle, DesiredAccess, ObjectAttributes,
		IoStatusBlock, AllocationSize, FileAttributes, ShareAccess,
		Disposition, CreateOptions, EaBuffer, EaLength, CreateFileType,
		InternalParameters, Options, NULL);
}


DDKAPI
NTSTATUS IoCreateFileEx(PHANDLE FileHandle, ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess,
    ULONG Disposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength,
    CREATE_FILE_TYPE CreateFileType, PVOID InternalParameters, ULONG Options,
	PIO_DRIVER_CREATE_CONTEXT DriverContext)
{
	UNREFERENCED_PARAMETER(AllocationSize);
	UNREFERENCED_PARAMETER(CreateFileType);
	UNREFERENCED_PARAMETER(InternalParameters);
	UNREFERENCED_PARAMETER(Options);
	UNREFERENCED_PARAMETER(DriverContext);

	if (!ObjectAttributes || !ObjectAttributes->ObjectName || !IoStatusBlock)
		return STATUS_INVALID_PARAMETER;

	DDKASSERT(!ObjectAttributes->RootDirectory);

	FILEOBJ *pFile;
	NTSTATUS status = !DdkIsFilePath(ObjectAttributes->ObjectName) ?
		DdkIoCreateDevice(&pFile, ObjectAttributes, IoStatusBlock, Options) :
		DdkIoCreateFile(&pFile, DesiredAccess, ObjectAttributes, IoStatusBlock,
			FileAttributes, ShareAccess, Disposition, CreateOptions);

	if (!NT_SUCCESS(status))
		return status;

	*FileHandle = ToHandle(pFile);
	return status;
}


DDKAPI
NTSTATUS ZwOpenFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock,
    ULONG ShareAccess, ULONG OpenOptions)
{
	return IoCreateFileEx(FileHandle, DesiredAccess, ObjectAttributes,
		IoStatusBlock, NULL, 0, ShareAccess, FILE_OPEN, OpenOptions,
		NULL, NULL, CreateFileTypeNone, NULL, 0, NULL);
}


DDKAPI
NTSTATUS ZwCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess,
    ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength)
{
	return IoCreateFileEx(FileHandle, DesiredAccess, ObjectAttributes,
		IoStatusBlock, AllocationSize, FileAttributes, ShareAccess,
		CreateDisposition, CreateOptions, EaBuffer, EaLength,
		CreateFileTypeNone, NULL, 0, NULL);
}


static
NTSTATUS DdkReadWriteFile(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length,
	PLARGE_INTEGER ByteOffset, bool write)
{
	if (!IoStatusBlock || !Buffer)
		return STATUS_INVALID_PARAMETER;

	if (Event) return STATUS_NOT_SUPPORTED;

	FILEOBJ *pFile = GetFileFromHandle(FileHandle);

	if (!pFile) return STATUS_INVALID_HANDLE;

	LARGE_INTEGER offset;
	OVERLAPPED overlap;

	if (!ByteOffset || (ByteOffset->HighPart == -1 &&
			ByteOffset->LowPart == FILE_USE_FILE_POINTER_POSITION))
		offset = pFile->File.CurrentByteOffset;

	else offset = *ByteOffset;

	memset(&overlap, 0, sizeof(overlap));
	overlap.Offset = offset.LowPart;
	overlap.OffsetHigh = offset.HighPart;

	NTSTATUS status = STATUS_SUCCESS;
	DWORD bytes = 0;

	BOOL rc = (write) ?
		WriteFile(pFile->h, Buffer, Length, &bytes, &overlap) :
		ReadFile(pFile->h, Buffer, Length, &bytes, &overlap);

	if (!rc) status = DdkGetFileStatus(GetLastError());

	offset.QuadPart += bytes;
	pFile->File.CurrentByteOffset = offset;

	SetStatus(IoStatusBlock, status, bytes);

	if (ApcRoutine)
		ApcRoutine(ApcContext, IoStatusBlock, 0);

	return status;
}


DDKAPI
NTSTATUS ZwReadFile(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length,
	PLARGE_INTEGER ByteOffset, PULONG Key)
{
	UNREFERENCED_PARAMETER(Key);

	return DdkReadWriteFile(FileHandle, Event, ApcRoutine, ApcContext,
		IoStatusBlock, Buffer, Length, ByteOffset, false);
}


DDKAPI
NTSTATUS ZwWriteFile(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine,
    PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length,
    PLARGE_INTEGER ByteOffset, PULONG Key)
{
	UNREFERENCED_PARAMETER(Key);

	return DdkReadWriteFile(FileHandle, Event, ApcRoutine, ApcContext,
		IoStatusBlock, Buffer, Length, ByteOffset, true);
}


DDKAPI
NTSTATUS ZwFlushBuffersFile(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock)
{
	FILEOBJ *pFile = GetFileFromHandle(FileHandle);
	NTSTATUS status = STATUS_SUCCESS;

	if (!pFile) return STATUS_INVALID_HANDLE;

	if (!FlushFileBuffers(pFile->h))
		status = DdkGetFileStatus(GetLastError());

	return SetStatus(IoStatusBlock, status, 0);
}


DDKAPI
NTSTATUS ZwQueryInformationFile(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass)
{
	FILEOBJ *pFile = GetFileFromHandle(FileHandle);

	if (!pFile) return STATUS_INVALID_HANDLE;

	if (!IoStatusBlock || !FileInformation)
		return STATUS_INVALID_PARAMETER;

	NTSTATUS status = STATUS_SUCCESS;
	DWORD bytes = 0;

	switch (FileInformationClass) {
	case FileBasicInformation:
	{
		if (!DdkGetFileInfo(pFile->h, FileInformationClass, FileInformation,
				Length, sizeof(FILE_BASIC_INFORMATION), &bytes))
			status = DdkGetFileStatus(GetLastError());
		break;
	}

	case FileStandardInformation:
	{
		if (!DdkGetFileInfo(pFile->h, FileInformationClass, FileInformation,
				Length, sizeof(FILE_STANDARD_INFORMATION), &bytes))
			status = DdkGetFileStatus(GetLastError());
		break;
	}

	case FilePositionInformation:
	{
		FILE_POSITION_INFORMATION *pInfo = (FILE_POSITION_INFORMATION *)FileInformation;

		if (Length < sizeof(*pInfo))
			return SetStatus(IoStatusBlock, STATUS_BUFFER_TOO_SMALL, 0);

		memcpy(&pInfo->CurrentByteOffset, &pFile->File.CurrentByteOffset,
			sizeof(LARGE_INTEGER));

		bytes = sizeof(*pInfo);
		break;
	}

	default:
		status = STATUS_NOT_SUPPORTED;
		break;
	}

	return SetStatus(IoStatusBlock, status, bytes);
}


DDKAPI
NTSTATUS ZwDeleteFile(POBJECT_ATTRIBUTES ObjectAttributes)
{
	WCHAR path[MAX_PATH + 1];

	if (!ObjectAttributes)
		return STATUS_INVALID_PARAMETER;

	DdkGetLocalPath(path, sizeof(path) / sizeof(WCHAR),
		ObjectAttributes->ObjectName, false);

	if (!DeleteFileW(path))
		return DdkGetFileStatus(GetLastError());

	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS ZwQueryDirectoryFile(HANDLE FileHandle, HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass,
    BOOLEAN ReturnSingleEntry, PUNICODE_STRING FileName, BOOLEAN RestartScan)
{
	UNREFERENCED_PARAMETER(FileHandle);
	UNREFERENCED_PARAMETER(Event);
	UNREFERENCED_PARAMETER(ApcRoutine);
	UNREFERENCED_PARAMETER(ApcContext);
	UNREFERENCED_PARAMETER(IoStatusBlock);
	UNREFERENCED_PARAMETER(FileInformation);
	UNREFERENCED_PARAMETER(Length);
	UNREFERENCED_PARAMETER(FileInformationClass);
	UNREFERENCED_PARAMETER(ReturnSingleEntry);
	UNREFERENCED_PARAMETER(FileName);
	UNREFERENCED_PARAMETER(RestartScan);
	return STATUS_OBJECT_NAME_NOT_FOUND;
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


static
NTSTATUS DdkGetFileStatus(DWORD rc)
{
	switch (rc) {
	case ERROR_SUCCESS:
		return STATUS_SUCCESS;

	case ERROR_ACCESS_DENIED:
		return STATUS_ACCESS_DENIED;

	case ERROR_NOT_ENOUGH_MEMORY:
		return STATUS_INSUFFICIENT_RESOURCES;

	case ERROR_INVALID_HANDLE:
		return STATUS_INVALID_HANDLE;

	case ERROR_FILE_NOT_FOUND:
		return STATUS_OBJECT_NAME_NOT_FOUND;

	case ERROR_FILE_EXISTS:
		return STATUS_OBJECT_NAME_COLLISION;

	case ERROR_SHARING_VIOLATION:
		return STATUS_SHARING_VIOLATION;

	case ERROR_MORE_DATA:
		return STATUS_BUFFER_TOO_SMALL;

	case ERROR_INVALID_PARAMETER:
		return STATUS_INVALID_PARAMETER;

	case ERROR_NOACCESS:
		return STATUS_ACCESS_VIOLATION;

	case ERROR_HANDLE_EOF:
		return STATUS_END_OF_FILE;

	case ERROR_PATH_NOT_FOUND:
		return STATUS_OBJECT_PATH_NOT_FOUND;
	}

	return STATUS_UNSUCCESSFUL;
}