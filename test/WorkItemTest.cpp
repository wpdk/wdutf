/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Work Item Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkWorkItemTest)
	{
		PDRIVER_OBJECT pDriver;
		PDEVICE_OBJECT pDevice;
		PIO_WORKITEM pWork;
		PIO_WORKITEM pIoWork;

		static const int nvec = 10;
		PIO_WORKITEM vec[nvec];

		volatile LONG count;
		volatile LONG blocked;
		volatile LONG waiting;

		char DriverName[100];

		static NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
		{
			DdkWorkItemTest *pTest = (DdkWorkItemTest *)_wtoll(&DriverObject->DriverName.Buffer[3]);
			pTest->pDriver = DriverObject;
			return STATUS_SUCCESS;
		}

	public:
		TEST_METHOD_INITIALIZE(DdkWorkItemInit)
		{
			DdkThreadInit();

			sprintf(DriverName, "DRV%I64d:%d", (LONGLONG)this, GetUniqueId());
			memset(vec, 0, sizeof(vec));
			pDriver = 0;
			pDevice = 0;
			pWork = 0;
			pIoWork = 0;
			count = 0;
			blocked = 0;
			waiting = 0;

			NTSTATUS rc = DdkInitDriver(DriverName, DriverEntry);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsNotNull(pDriver);

			rc = IoCreateDevice(pDriver, 200, NULL, FILE_DEVICE_DISK, 0, FALSE, &pDevice);
			Assert::IsTrue(NT_SUCCESS(rc));
			Assert::IsNotNull(pDevice);
		}

		TEST_METHOD_CLEANUP(DdkWorkItemCleanup)
		{
			if (pWork) {
				IoUninitializeWorkItem(pWork);
				free(pWork);
			}

			if (pIoWork) IoFreeWorkItem(pIoWork);

			for (int i = 0; i < nvec; i++)
				if (vec[i]) IoFreeWorkItem(vec[i]);

			if (pDevice) IoDeleteDevice(pDevice);
			if (pDriver) DdkUnloadDriver(DriverName);
		}

		TEST_METHOD_CALLBACK(DdkWorkItemProc, PDEVICE_OBJECT DeviceObject, PVOID Context)
		{
			if (blocked) {
				InterlockedIncrement(&waiting);
				while (blocked) Sleep(10);
			}

			InterlockedIncrement(&count);
		}

		TEST_METHOD_CALLBACK(DdkWorkItemProcEx, PVOID IoObject, PVOID Context, PIO_WORKITEM Item)
		{
			InterlockedIncrement(&count);
		}
	
		TEST_METHOD(DdkWorkItemInitialize)
		{
			pWork = (PIO_WORKITEM)malloc(IoSizeofWorkItem());
			Assert::IsNotNull(pWork);

			IoInitializeWorkItem(pDevice, pWork);

			TEST_CALLBACK_INIT(id);
			IoQueueWorkItem(pWork, DdkWorkItemProc, DelayedWorkQueue, id);
			TEST_CALLBACK_WAIT(id);
			Assert::IsTrue(count == 1);
		}

		TEST_METHOD(DdkWorkItemInitializeEx)
		{
			pWork = (PIO_WORKITEM)malloc(IoSizeofWorkItem());
			Assert::IsNotNull(pWork);

			IoInitializeWorkItem(pDevice, pWork);

			TEST_CALLBACK_INIT(id);
			IoQueueWorkItemEx(pWork, DdkWorkItemProcEx, DelayedWorkQueue, id);
			TEST_CALLBACK_WAIT(id);
			Assert::IsTrue(count == 1);
		}

		TEST_METHOD(DdkWorkItemAlloc)
		{
			pIoWork = IoAllocateWorkItem(pDevice);
			Assert::IsNotNull(pIoWork);

			TEST_CALLBACK_INIT(id);
			IoQueueWorkItem(pIoWork, DdkWorkItemProc, DelayedWorkQueue, id);
			TEST_CALLBACK_WAIT(id);
			Assert::IsTrue(count == 1);
		}

		TEST_METHOD(DdkWorkItemAllocEx)
		{
			pIoWork = IoAllocateWorkItem(pDevice);
			Assert::IsNotNull(pIoWork);

			TEST_CALLBACK_INIT(id);
			IoQueueWorkItemEx(pIoWork, DdkWorkItemProcEx, DelayedWorkQueue, id);
			TEST_CALLBACK_WAIT(id);
			Assert::IsTrue(count == 1);
		}

		TEST_METHOD(DdkWorkItemConcurrency)
		{
			InterlockedExchange(&blocked, 1);

			for (int i = 0; i < nvec; i++)
				Assert::IsNotNull((vec[i] = IoAllocateWorkItem(pDevice)));

			TEST_CALLBACK_INIT_VEC(id, nvec);

			for (int i = 0; i < nvec; i++) {
				IoQueueWorkItem(vec[i], DdkWorkItemProc, DelayedWorkQueue, id[i]);
				TEST_CALLBACK_STARTED(id[i]);
			}

			Sleep(20);
			InterlockedExchange(&blocked, 0);

			TEST_CALLBACK_WAIT_VEC(id);

			Assert::IsTrue(waiting > 1);
			Assert::IsTrue(count == nvec);
		}
	};
}
