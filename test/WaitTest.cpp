/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Wait Tests
 */

#include "stdafx.h"

namespace DdkUnitTest
{
	TEST_CLASS(DdkWaitTest)
	{
		const static LONG nTicks = 10;

		TEST_METHOD_INITIALIZE(DdkWaitTestInit)
		{
			DdkThreadInit();
		}

		TEST_METHOD(DdkWaitDelayInterval)
		{
			LARGE_INTEGER begin, end, delay;
			delay.QuadPart = -nTicks;
			delay.QuadPart *= KeQueryTimeIncrement();

			KeQueryTickCount(&begin);
			NTSTATUS rc = KeDelayExecutionThread(KernelMode, FALSE, &delay);
			KeQueryTickCount(&end);

			Assert::IsTrue(NT_SUCCESS(rc));
			Assert::IsTrue(end.QuadPart - begin.QuadPart >= nTicks - 1);
		}

		TEST_METHOD(DdkWaitDelayTime)
		{
			LARGE_INTEGER begin, end, finish;

			KeQuerySystemTime(&finish);
			finish.QuadPart = finish.QuadPart + (nTicks * KeQueryTimeIncrement());

			KeQueryTickCount(&begin);
			NTSTATUS rc = KeDelayExecutionThread(KernelMode, FALSE, &finish);
			KeQueryTickCount(&end);

			Assert::IsTrue(NT_SUCCESS(rc));
			Assert::IsTrue(end.QuadPart - begin.QuadPart >= nTicks - 1);
		}
	};
}