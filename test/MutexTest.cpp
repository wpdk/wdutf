/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Mutex Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkMutexTest)
	{
		LARGE_INTEGER nowait;
		KMUTEX mutex;
		ULONG value;
		LONG count;
		LONG asynccount;

	public:
		TEST_METHOD_INITIALIZE(DdkMutexTestInit)
		{
			DdkThreadInit();
			KeInitializeMutex(&mutex, 0);
			nowait.QuadPart = 0;
			value = 0;
			count = 0;
			asynccount = 0;
		}

		TEST_METHOD(DdkMutexInit)
		{
			Assert::IsTrue(KeReadStateMutex(&mutex) == 1);
		}

		TEST_METHOD(DdkMutexWait)
		{
			NTSTATUS rc = KeWaitForSingleObject(&mutex, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsTrue(KeReadStateMutex(&mutex) == 0);
		}

		TEST_METHOD(DdkMutexRelease)
		{
			NTSTATUS rc = KeWaitForSingleObject(&mutex, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);

			KeReleaseMutex(&mutex, FALSE);
			Assert::IsTrue(KeReadStateMutex(&mutex) == 1);
		}

		TEST_METHOD(DdkMutexWaitRecursive)
		{
			NTSTATUS rc = KeWaitForSingleObject(&mutex, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);

			rc = KeWaitForSingleObject(&mutex, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsTrue(KeReadStateMutex(&mutex) == 0);
		}

		TEST_METHOD(DdkMutexReleaseRecursive)
		{
			NTSTATUS rc = KeWaitForSingleObject(&mutex, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);

			rc = KeWaitForSingleObject(&mutex, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);

			KeReleaseMutex(&mutex, FALSE);
			Assert::IsTrue(KeReadStateMutex(&mutex) == 0);

			KeReleaseMutex(&mutex, FALSE);
			Assert::IsTrue(KeReadStateMutex(&mutex) == 1);
		}

		TEST_METHOD_ASYNC(DdkMutexAsync)
		{
			LARGE_INTEGER timeout;
			timeout.QuadPart = (-20) * 10000I64;

			for (int i = 0, last = 0; i < 20000; i++) {
				NTSTATUS rc = KeWaitForSingleObject(&mutex, Executive, KernelMode, FALSE, &timeout);
				if (rc == STATUS_TIMEOUT) continue;

				if (TEST_IS_ASYNC) {
					Assert::IsTrue((value & 0xffff) == last);
					value = (value & 0xffff0000) + i;
					asynccount++;
				} else {
					Assert::IsTrue((value >> 16) == last);
					value = (value & 0xffff) + (i << 16);
					count++;
				}

				last = i;
				KeReleaseMutex(&mutex, FALSE);
			}
		}

		TEST_METHOD(DdkMutexMulti)
		{
			TEST_ASYNC_START(id, DdkMutexAsync);
			DdkMutexAsync();
			TEST_ASYNC_WAIT(id);
			Assert::IsTrue(count && asynccount);
		}
	};
}
