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
	TEST_CLASS(DdkSemaphoreTest)
	{
		LARGE_INTEGER nowait;
		KSEMAPHORE sema;
		ULONG value;
		LONG count;
		LONG asynccount;

	public:
		TEST_METHOD_INITIALIZE(DdkSemaphoreTestInit)
		{
			DdkThreadInit();
			KeInitializeSemaphore(&sema, 1, 1);
			nowait.QuadPart = 0;
			value = 0;
			count = 0;
			asynccount = 0;
		}

		TEST_METHOD(DdkSemaphoreInit)
		{
			KeInitializeSemaphore(&sema, 0, 1);
			Assert::IsTrue(KeReadStateSemaphore(&sema) == 0);
		}

		TEST_METHOD(DdkSemaphoreInit1)
		{
			KeInitializeSemaphore(&sema, 1, 1);
			Assert::IsTrue(KeReadStateSemaphore(&sema) != 0);
		}

		TEST_METHOD(DdkSemaphoreWait)
		{
			KeInitializeSemaphore(&sema, 0, 1);
			NTSTATUS rc = KeWaitForSingleObject(&sema, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_TIMEOUT);
			Assert::IsTrue(KeReadStateSemaphore(&sema) == 0);
		}

		TEST_METHOD(DdkSemaphoreWait1)
		{
			NTSTATUS rc = KeWaitForSingleObject(&sema, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsTrue(KeReadStateSemaphore(&sema) == 0);
		}

		TEST_METHOD(DdkSemaphoreWait2)
		{
			KeInitializeSemaphore(&sema, 2, 2);
			NTSTATUS rc = KeWaitForSingleObject(&sema, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsTrue(KeReadStateSemaphore(&sema) != 0);
		}

		TEST_METHOD(DdkSemaphoreReadState)
		{
			KeInitializeSemaphore(&sema, 0, 1);
			Assert::IsTrue(KeReadStateSemaphore(&sema) == 0);
		}

		TEST_METHOD(DdkSemaphoreReadState1)
		{
			Assert::IsTrue(KeReadStateSemaphore(&sema) != 0);
		}

		TEST_METHOD(DdkSemaphoreRelease)
		{
			KeInitializeSemaphore(&sema, 0, 1);
			Assert::IsTrue(KeReleaseSemaphore(&sema, IO_NO_INCREMENT, 1, FALSE) == 0);
			Assert::IsTrue(KeReadStateSemaphore(&sema) != 0);
		}

		TEST_METHOD(DdkSemaphoreRelease2)
		{
			KeInitializeSemaphore(&sema, 0, 2);
			Assert::IsTrue(KeReleaseSemaphore(&sema, IO_NO_INCREMENT, 2, FALSE) == 0);
			Assert::IsTrue(KeReadStateSemaphore(&sema) != 0);
			
			NTSTATUS rc = KeWaitForSingleObject(&sema, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsTrue(KeReadStateSemaphore(&sema) != 0);

			rc = KeWaitForSingleObject(&sema, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsTrue(KeReadStateSemaphore(&sema) == 0);
		}

		TEST_METHOD_ASYNC(DdkSemaphoreAsync)
		{
			LARGE_INTEGER timeout;
			timeout.QuadPart = (-20) * 10000I64;

			for (int i = 0, last = 0; i < 20000; i++) {
				NTSTATUS rc = KeWaitForSingleObject(&sema, Executive, KernelMode, FALSE, &timeout);
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
				KeReleaseSemaphore(&sema, IO_NO_INCREMENT, 1, FALSE);
			}
		}

		TEST_METHOD(DdkSemaphoreMulti)
		{
			TEST_ASYNC_START(id, DdkSemaphoreAsync);
			DdkSemaphoreAsync();
			TEST_ASYNC_WAIT(id);
			Assert::IsTrue(count && asynccount);
		}
	};
}
