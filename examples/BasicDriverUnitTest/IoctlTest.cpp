/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 */

 /*
  *	BasicDriver IOCTL Tests
  */

#include "stdafx.h"


namespace BasicDriverUnitTest
{
	TEST_CLASS(BasicDriverIoctlTest)
	{
		PDEVICE_OBJECT pDevice;
		PFILE_OBJECT pFile;
		KEVENT event;

		TEST_METHOD_INITIALIZE(BasicDriverIoctlTestInit)
		{
			UNICODE_STRING u;

			DdkThreadInit();
			pFile = NULL;

			RtlUnicodeStringInit(&u, L"\\Device\\Basic");
			KeInitializeEvent(&event, NotificationEvent, FALSE);

			NTSTATUS rc = IoGetDeviceObjectPointer(&u, 0, &pFile, &pDevice);
			Assert::IsTrue(NT_SUCCESS(rc));
		}

		TEST_METHOD_CLEANUP(BasicDriverIoctlTestCleanup)
		{
			if (pFile) ObDereferenceObject(pFile);
		}

		// Issue IOCTL

		TEST_METHOD(IssueIoControlRequest)
		{
			IO_STATUS_BLOCK iostatus;
			LONG v = 10;

			PIRP pIrp = IoBuildDeviceIoControlRequest(IOCTL_BASIC_TEST_FUNCTION,
				pDevice, &v, sizeof(v), &v, sizeof(v), FALSE, &event, &iostatus);

			Assert::IsNotNull(pIrp);

			if (IoCallDriver(pDevice, pIrp) == STATUS_PENDING) {
				NTSTATUS rc = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
				Assert::IsTrue(NT_SUCCESS(rc));
			}

			Assert::AreEqual(STATUS_SUCCESS, iostatus.Status);
			Assert::AreEqual(sizeof(LONG), iostatus.Information);
			Assert::AreEqual(14L, v);
		}

		// Issue invalid IOCTL

		TEST_METHOD(InvalidRequest)
		{
			IO_STATUS_BLOCK iostatus;
			LONG v = 10;

			PIRP pIrp = IoBuildDeviceIoControlRequest(IOCTL_BASIC_INVALID,
				pDevice, &v, sizeof(v), &v, sizeof(v), FALSE, &event, &iostatus);

			Assert::IsNotNull(pIrp);

			if (IoCallDriver(pDevice, pIrp) == STATUS_PENDING) {
				NTSTATUS rc = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
				Assert::IsTrue(NT_SUCCESS(rc));
			}

			Assert::AreEqual(STATUS_INVALID_DEVICE_REQUEST, iostatus.Status);
			Assert::AreEqual((ULONG_PTR)0, iostatus.Information);
			Assert::AreEqual(10L, v);
		}

		// Issue IOCTL with invalid parameter

		TEST_METHOD(InvalidParameter)
		{
			IO_STATUS_BLOCK iostatus;
			LONG v = 10;

			PIRP pIrp = IoBuildDeviceIoControlRequest(IOCTL_BASIC_TEST_FUNCTION,
				pDevice, &v, sizeof(v) - 1, &v, sizeof(v), FALSE, &event, &iostatus);

			Assert::IsNotNull(pIrp);

			if (IoCallDriver(pDevice, pIrp) == STATUS_PENDING) {
				NTSTATUS rc = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
				Assert::IsTrue(NT_SUCCESS(rc));
			}

			Assert::AreEqual(STATUS_INVALID_PARAMETER, iostatus.Status);
			Assert::AreEqual((ULONG_PTR)0, iostatus.Information);
			Assert::AreEqual(10L, v);
		}
	};
}
