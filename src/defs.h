/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2013-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Common definitions.
 */

#ifdef _NTDDK_
#include <winnt.h>
#include <winerror.h>
#define DDKUSEDECL	_Use_decl_annotations_
#else
#include <Windows.h>
#define DDKUSEDECL
#endif
#include <ddk.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" _Analysis_noreturn_ void DdkFail(wchar_t *s);
#define ddkfail(s) DdkFail(L"DDK: " L##s)

#include "object.h"

#undef DDKAPI
#undef DDKAPI_NODECL
#define DDKAPI_NODECL extern "C" __declspec(dllexport)
#define DDKAPI DDKAPI_NODECL DDKUSEDECL

#define DDKASSERT(v) ((!(v)) ? DdkFail( L"DDK: Assert " L#v),0 : 0)

typedef struct _IMAGE IMAGE, *PIMAGE;

BOOLEAN DdkIsDpc();
BOOLEAN DdkIsWorkItem();
DWORD DdkGetWaitTime(LARGE_INTEGER *pTimeout);
DWORD DdkGetDelayTime(LARGE_INTEGER *pTimeout);
FILETIME DdkGetDueTime(LARGE_INTEGER Timeout);
VOID DdkAddDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT DeviceObject);
VOID DdkRemoveDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT DeviceObject);
void DdkCreatePath(PUNICODE_STRING Out, PUNICODE_STRING Name, PWCH Dir = 0, PWCH Prefix = 0);
BOOLEAN DdkInsertName(PUNICODE_STRING Name, POBJECT pObj, PWCH Dir = 0, PWCH *pPath = 0);
POBJECT DdkCreateName(PUNICODE_STRING Name, POBJECT pObj, PWCH Dir = 0, PWCH *pPath = 0);
VOID DdkRemoveName(PUNICODE_STRING Name);
VOID DdkRemoveObjectName(POBJECT Object);
POBJECT DdkLookupName(PUNICODE_STRING Name, PWCH Dir = 0);
POBJECT DdkLookupName(PWCH Name, PWCH Dir = 0);
POBJECT DdkLookupName(char *Name, PWCH Dir = 0);
PFILE_OBJECT DdkGetFilePointer(HANDLE FileHandle);
void DdkFreeFileObject(OBJECT *pObj);
void DdkFreeDriverObject(OBJECT *pObj);
void DdkFreeDeviceObject(OBJECT *pObj);
void DdkFreeKeyObject(OBJECT *pObj);
POBJECT DdkResolveSymbolicLink(OBJECT *pObj);
BOOLEAN DdkIsDeviceInterface(PUNICODE_STRING path, BOOLEAN all);
NTSTATUS DdkCreateDriver(char *pName, PDRIVER_INITIALIZE DriverInit, PIMAGE Image);
VOID DdkUnloadImage(PIMAGE pImage);
NTSTATUS DdkPnpCreateDevice(PDRIVER_OBJECT DriverObject);
void DdkPnpUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS DdkSynchronousIrp(PDEVICE_OBJECT DeviceObject, UCHAR Major, UCHAR Minor, PIRP Irp = NULL);
ULONG DdkGetTickCountMultiplier();
ULONG64 DdkGetTickCount();
ULONG64 DdkReadCR8();
VOID DdkWriteCR8(ULONG64 v);
WCHAR *DdkAllocUnicodeBuffer(size_t len);
char *DdkAllocAnsiBuffer(size_t len);
BOOLEAN DdkWaitLastReference(POBJECT pObj, int maxsecs, int count);
void DdkThreadLock();
void DdkThreadUnlock();
LONG DdkInvokeForAllThreads(LONG (*func)(HANDLE));
WCHAR *DdkUnicodeToString(UNICODE_STRING *u, WCHAR remove = 0);
void DdkGetLocalPath(WCHAR *buffer, int len, UNICODE_STRING *path, bool create);
void DdkGetLocalPath(char *buffer, size_t len, char *path, char *file, char *suffix = "");


#define EXCEPTION_UNITTEST_ASSERTION   (DWORD)0xe3530001


extern size_t _SizeofEvent_, _SizeofMutex_, _SizeofTimer_, _SizeofSemaphore_, _SizeofDpc_;

