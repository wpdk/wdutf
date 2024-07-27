/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Symbolic Link Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkSymLinkTest)
	{
		PDRIVER_OBJECT pDriver;
		PDEVICE_OBJECT pDevice;
		PFILE_OBJECT pFile;
		UNICODE_STRING udev;
		UNICODE_STRING ulink;
		
		char DriverName[100];
		wchar_t DeviceName[100];
		wchar_t LinkName[100];

		static NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
		{
			DdkSymLinkTest *pTest = (DdkSymLinkTest *)_wtoll(&DriverObject->DriverName.Buffer[3]);
			pTest->pDriver = DriverObject;
			return STATUS_SUCCESS;
		}

	public:
		TEST_METHOD_INITIALIZE(DdkSymLinkTestInit)
		{
			DdkThreadInit();

			sprintf(DriverName, "DRV%I64d:%d", (LONGLONG)this, GetUniqueId());
			swprintf(DeviceName, sizeof(DeviceName) / sizeof(WCHAR),
				L"\\Device\\D%I64d:%d", (LONGLONG)this, GetUniqueId());
			swprintf(LinkName, sizeof(LinkName) / sizeof(WCHAR),
				L"\\Device\\L%I64d:%d", (LONGLONG)this, GetUniqueId());

			pDriver = 0;
			pDevice = 0;
			pFile = 0;

			RtlInitUnicodeString(&udev, DeviceName);
			RtlInitUnicodeString(&ulink, LinkName);

			NTSTATUS rc = DdkInitDriver(DriverName, DriverEntry);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsNotNull(pDriver);

			rc = IoCreateDevice(pDriver, 200, &udev, FILE_DEVICE_DISK, 0, FALSE, &pDevice);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsNotNull(pDevice);
		}

		TEST_METHOD_CLEANUP(DdkSymLinkTestCleanup)
		{
			IoDeleteSymbolicLink(&ulink);
			if (pFile) ObDereferenceObject(pFile);
			if (pDevice) IoDeleteDevice(pDevice);
			if (pDriver) DdkUnloadDriver(DriverName);
		}

		TEST_METHOD(DdkSymLinkCreate)
		{
			NTSTATUS rc = IoCreateSymbolicLink(&ulink, &udev);
			Assert::IsTrue(rc == STATUS_SUCCESS);
		}

		TEST_METHOD(DdkSymLinkCreateExists)
		{
			NTSTATUS rc = IoCreateSymbolicLink(&ulink, &udev);
			Assert::IsTrue(rc == STATUS_SUCCESS);

			rc = IoCreateSymbolicLink(&ulink, &udev);
			Assert::IsTrue(rc == STATUS_OBJECT_NAME_EXISTS);
		}

		TEST_METHOD(DdkSymLinkOpen)
		{
			NTSTATUS rc = IoCreateSymbolicLink(&ulink, &udev);
			Assert::IsTrue(rc == STATUS_SUCCESS);

			PDEVICE_OBJECT pDevice2;
			rc = IoGetDeviceObjectPointer(&ulink, 0, &pFile, &pDevice2);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsTrue(pDevice2 == pDevice);
			Assert::IsTrue(pFile->DeviceObject == pDevice);
		}

		TEST_METHOD(DdkSymLinkDelete)
		{
			NTSTATUS rc = IoCreateSymbolicLink(&ulink, &udev);
			Assert::IsTrue(rc == STATUS_SUCCESS);

			IoDeleteSymbolicLink(&ulink);

			PDEVICE_OBJECT pDevice2;
			rc = IoGetDeviceObjectPointer(&ulink, 0, &pFile, &pDevice2);
			Assert::IsTrue(rc == STATUS_OBJECT_NAME_NOT_FOUND);
		}
	};
}
