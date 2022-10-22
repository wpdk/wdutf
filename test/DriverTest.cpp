/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Driver Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkDriverTest)
	{
		PDRIVER_OBJECT pDriver;
		char DriverName[100];
		int unload;

		static void DriverUnload(PDRIVER_OBJECT DriverObject)
		{
			DdkDriverTest *pTest = (DdkDriverTest *)_wtoll(&DriverObject->DriverName.Buffer[3]);
			pTest->pDriver = 0;
			pTest->unload++;
		}

		static NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
		{
			DdkDriverTest *pTest = (DdkDriverTest *)_wtoll(&DriverObject->DriverName.Buffer[3]);
			DriverObject->DriverUnload = DriverUnload;
			pTest->pDriver = DriverObject;
			return STATUS_SUCCESS;
		}

	public:
		TEST_METHOD_INITIALIZE(DdkDriverTestInit)
		{
			DdkThreadInit();
			sprintf(DriverName, "DRV%I64d:%d", (LONGLONG)this, GetUniqueId());

			unload = 0;
			pDriver = 0;
			NTSTATUS rc = DdkInitDriver(DriverName, DriverEntry);

			Assert::IsTrue(rc == STATUS_SUCCESS);
		}

		TEST_METHOD_CLEANUP(DdkDriverTestCleanup)
		{
			if (pDriver) DdkUnloadDriver(DriverName);
		}

	public:
		TEST_METHOD(DdkDriverLoad)
		{
			Assert::IsNotNull(pDriver);
		}

		TEST_METHOD(DdkDriverUnload)
		{
			DdkUnloadDriver(DriverName);
			Assert::IsTrue(unload == 1);
		}
	};
}
