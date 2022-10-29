/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Driver Routines.
 */

#include "stdddk.h"


typedef struct _DRIVER : public OBJECT {
	DRIVER_OBJECT Driver;
	DRIVER_EXTENSION Extension;
	volatile LONG loadcount;
	PIMAGE Image;
	WCHAR Name[1];
} DRIVER, *PDRIVER;


void DdkLoadLock();
void DdkLoadUnlock();
PIMAGE DdkLoadImage(char *pPath, char *pName);
PDRIVER_INITIALIZE DdkGetImageEntry(PIMAGE pImage);
void DdkGetDriverName(char *pFile, char *np, wchar_t *wp);
void DdkGetDriverPath(char *pFile, char *pPath, char *pName);
NTSTATUS DdkReferenceDriver(OBJECT *pObj, bool loaded);
NTSTATUS DdkDriverEntry(PDRIVER_OBJECT DriverObject);
void DdkDelayLoad(PIMAGE pImage, HRESULT (*pLoad)(const char *));
void DdkDelayUnload(PIMAGE pImage, INT (*pUnload)(const char *));


DRIVER_DISPATCH DdkDefaultDispatch;


static PDRIVER GetDriver(OBJECT *pObj)
{
	if (!pObj || pObj->type != IoDriverType)
		ddkfail("Invalid driver specified");

	return static_cast<DRIVER *>(pObj);
}


DDKAPI
NTSTATUS DdkLoadDriver(char *pFile, HRESULT (*pLoadAll)(const char *))
{
	char name[MAX_PATH+1], path[MAX_PATH+1];
	wchar_t wname[MAX_PATH+1];

	DdkThreadInit();

	// Check if driver is loaded

	DdkGetDriverName(pFile, name, wname);

	DdkLoadLock();
	OBJECT *pObj = DdkLookupName(wname, L"\\Driver");

	if (pObj) {
		NTSTATUS rc = DdkReferenceDriver(pObj, true);

		if (pLoadAll && NT_SUCCESS(rc))
			DdkDelayLoad(GetDriver(pObj)->Image, pLoadAll);

		DdkLoadUnlock();
		return rc;
	}

	// Load the driver

	DdkGetDriverPath(pFile, path, name);
	PIMAGE pImage = DdkLoadImage(path, name);

	NTSTATUS rc = DdkCreateDriver(name, DdkGetImageEntry(pImage), pImage);

	if (pLoadAll && NT_SUCCESS(rc))
		DdkDelayLoad(pImage, pLoadAll);

	DdkLoadUnlock();
	return rc;
}


DDKAPI_NODECL
NTSTATUS DdkInitDriver(char *pName, PDRIVER_INITIALIZE DriverInit)
{
	DdkThreadInit();

	DdkLoadLock();
	OBJECT *pObj = DdkLookupName(pName, L"\\Driver");

	if (pObj) {
		NTSTATUS rc = DdkReferenceDriver(pObj, false);
		DdkLoadUnlock();
		return rc;
	}

	NTSTATUS rc = DdkCreateDriver(pName, DriverInit, NULL);

	DdkLoadUnlock();
	return rc;
}


static NTSTATUS DdkReferenceDriver(OBJECT *pObj, bool loaded)
{
	DRIVER *pDriver = GetDriver(pObj);

	if ((pDriver->Image != NULL) != loaded)
		ddkfail("DdkReferenceDriver: Driver linked statically and dynamically");

	pDriver->loadcount++;
	return STATUS_SUCCESS;
}


DDKAPI_NODECL
VOID DdkUnloadDriver(char *pName, INT (*pUnload)(const char *))
{
	DdkThreadInit();

	DdkLoadLock();
	OBJECT *pObj = DdkLookupName(pName, L"\\Driver");

	if (!pObj) {
		DdkLoadUnlock();
		return;
	}

	DRIVER *pDriver = GetDriver(pObj);

	if (pUnload && pDriver->Image)
		DdkDelayUnload(pDriver->Image, pUnload);

	if (pDriver->loadcount && !--(pDriver->loadcount)) {
		DdkPnpUnload(&pDriver->Driver);

		if (pDriver->Driver.DriverUnload)
			(*pDriver->Driver.DriverUnload)(&pDriver->Driver);

		DdkRemoveObjectName(pObj);

		if (!DdkWaitLastReference(pObj, 120, 2))
			ddkfail("Timeout unloading driver");
	}

	DdkDereferenceObject(pObj);
	DdkDereferenceObject(pObj);
	DdkLoadUnlock();
}


NTSTATUS DdkCreateDriver(char *pName, PDRIVER_INITIALIZE DriverInit, PIMAGE Image)
{
	DRIVER *pDriver = (DRIVER *)DdkAllocObject(sizeof(DRIVER)
			+ strlen(pName) * sizeof(WCHAR), IoDriverType);

	DDKASSERT(pName && DriverInit);

	pDriver->Driver.Type = IO_TYPE_DRIVER;
	pDriver->Driver.Size = sizeof(DRIVER_OBJECT);
	pDriver->Driver.DriverExtension = &pDriver->Extension;
	pDriver->Driver.DriverInit = DriverInit;
	pDriver->Extension.DriverObject = &pDriver->Driver;
	pDriver->Image = Image;

	for (ULONG i = 0; i <= strlen(pName); i++)
		pDriver->Name[i] = (unsigned char)(pName[i]);

	RtlInitUnicodeString(&pDriver->Driver.DriverName, pDriver->Name);
	
	if (!DdkInsertName(&pDriver->Driver.DriverName, pDriver, L"\\Driver")) {
		DdkDereferenceObject(pDriver);
		return STATUS_OBJECT_NAME_EXISTS;
	}

	for (ULONG i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
		pDriver->Driver.MajorFunction[i] = DdkDefaultDispatch;

	NTSTATUS rc = DdkDriverEntry(&pDriver->Driver);

	if (!NT_SUCCESS(rc)) {
		DdkDereferenceObject(pDriver);
		return rc;
	}

	pDriver->loadcount++;

	if (pDriver->Extension.AddDevice)
		DdkPnpCreateDevice(&pDriver->Driver);

	return STATUS_SUCCESS;
}


NTSTATUS DdkDriverEntry(PDRIVER_OBJECT DriverObject)
{
	wchar_t wname[MAX_PATH+1];
	UNICODE_STRING u;

	swprintf(wname, sizeof(wname),
		L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\%s",
		DriverObject->DriverName.Buffer);

	RtlInitUnicodeString(&u, wname);

	NTSTATUS rc = (*DriverObject->DriverInit)(DriverObject, &u);

	if (rc != STATUS_SUCCESS)
		ddkfail("DriverEntry failed");

	return STATUS_SUCCESS;
}


VOID DdkAddDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT DeviceObject)
{
	DRIVER *pDriver = GetDriver(FromPointer(DriverObject));
	PDEVICE_OBJECT *ppDevice = &pDriver->Driver.DeviceObject;

	while (*ppDevice) ppDevice = &(*ppDevice)->NextDevice;
	*ppDevice = DeviceObject;

	DdkReferenceObject(pDriver);
}


VOID DdkRemoveDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT DeviceObject)
{
	DRIVER *pDriver = GetDriver(FromPointer(DriverObject));
	PDEVICE_OBJECT *ppDevice = &pDriver->Driver.DeviceObject;

	for (; *ppDevice; ppDevice = &(*ppDevice)->NextDevice)
		if (*ppDevice == DeviceObject) {
			*ppDevice = DeviceObject->NextDevice;
			DdkDereferenceObject(pDriver);
			return;
		}
}


void DdkFreeDriverObject(OBJECT *pObj)
{
	DRIVER *pDriver = GetDriver(pObj);

	if (pDriver->Image)
		DdkUnloadImage(pDriver->Image);
}


NTSTATUS DdkDefaultDispatch(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	pIrp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_INVALID_DEVICE_REQUEST;
}
