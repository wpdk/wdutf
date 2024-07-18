/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 */

/*
 *	BasicDriver Unit Test Initialization
 */

#include "stdafx.h"


namespace BasicDriverUnitTest
{
	TEST_MODULE_INITIALIZE(BasicDriverUnitTestInit)
	{
		TEST_MODULE_START(BasicDriverUnitTestCleanup);
		DdkLoadDriver("DcsBasic");
	}

	TEST_MODULE_CLEANUP(BasicDriverUnitTestCleanup)
	{
		TEST_MODULE_END();
		DdkUnloadDriver("DcsBasic");
	}

	TEST_CLASS(BasicDriverUnitTest)
	{
		PFILE_OBJECT pFile;

		TEST_METHOD_INITIALIZE(BasicDriverUnitTestInit)
		{
			DdkThreadInit();
			pFile = NULL;
		}

		TEST_METHOD_CLEANUP(BasicDriverUnitTestCleanup)
		{
			if (pFile) ObDereferenceObject(pFile);
		}

		// Load driver and check device

		TEST_METHOD(BasicDriverLoad)
		{
			PDEVICE_OBJECT pDevice;
			UNICODE_STRING u;

			RtlUnicodeStringInit(&u, L"\\Device\\Basic");

			NTSTATUS rc = IoGetDeviceObjectPointer(&u, 0, &pFile, &pDevice);

			Assert::IsTrue(NT_SUCCESS(rc));
			Assert::IsNotNull(pDevice);
			Assert::IsTrue(pFile->DeviceObject == pDevice);
		}
	};
}
