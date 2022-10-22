/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2014, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Inline Tests
 */

#define _DDKINLINE_

#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WIN7
#define NT_INLINE_IRQL
#define NT_INLINE_GET_CURRENT_IRQL

#define KeGetCurrentThread DdkUnitTest_KeGetCurrentThread
#define PsGetCurrentThread DdkUnitTest_PsGetCurrentThread
#define KeGetCurrentIrql DdkUnitTest_KeGetCurrentIrql
#define KzLowerIrql DdkUnitTest_KzLowerIrql
#define KzRaiseIrql DdkUnitTest_KzRaiseIrql

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkInlineTest)
	{
	public:
		TEST_METHOD_INITIALIZE(DdkInlineTestInit)
		{
			DdkThreadInit();
		}

		TEST_METHOD_CLEANUP(DdkInlineTestCleanup)
		{
			KeLowerIrql(PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkInlineIrql)
		{
			KIRQL irql = KeGetCurrentIrql();
			Assert::IsTrue(irql == PASSIVE_LEVEL);

			KeRaiseIrql(DISPATCH_LEVEL, &irql);
			Assert::IsTrue(irql == PASSIVE_LEVEL);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);

			KeLowerIrql(PASSIVE_LEVEL);
			Assert::IsTrue(KeGetCurrentIrql() == PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkInlineThread)
		{
			PKTHREAD Thread = KeGetCurrentThread();
			Assert::IsTrue(PsIsSystemThread(Thread) == TRUE);
		}

		TEST_METHOD(DdkInlineTickCount)
		{
			for (int i = 0; true; i++) {
				ULONGLONG x = GetTickCount64(), v;

				KeQueryTickCount(&v);
				v = (v * KeQueryTimeIncrement()) / 10000;

				if (x == v) break;
				Assert::IsTrue(i < 10);
			}
		}
	};
}
