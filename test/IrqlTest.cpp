/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	IRQL Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkIrqlTest)
	{
	public:
		TEST_METHOD_INITIALIZE(DdkIrqlInit) 
		{
			DdkThreadInit();
			KeLowerIrql(PASSIVE_LEVEL);
			Assert::IsTrue(KeGetCurrentIrql() == PASSIVE_LEVEL);
		}

		TEST_METHOD_CLEANUP(DdkIrqlTestCleanup)
		{
			KeLowerIrql(PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkIrqlRaise)
		{
			KIRQL irql;
			KeRaiseIrql(DISPATCH_LEVEL, &irql);
			Assert::IsTrue(irql == PASSIVE_LEVEL);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);
		}

		TEST_METHOD(DdkIrqlRaiseToDpc)
		{
			KIRQL irql = KeRaiseIrqlToDpcLevel();
			Assert::IsTrue(irql == PASSIVE_LEVEL);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);
		}	

		TEST_METHOD(DdkIrqlLower)
		{
			KIRQL irql;
			KeRaiseIrql(DISPATCH_LEVEL, &irql);
			KeLowerIrql(PASSIVE_LEVEL);
			Assert::IsTrue(KeGetCurrentIrql() == PASSIVE_LEVEL);
		}

		TEST_METHOD_ASYNC(DdkIrqlAsync)
		{
			KIRQL lo = (TEST_IS_ASYNC) ? APC_LEVEL : PASSIVE_LEVEL;
			KIRQL hi = (TEST_IS_ASYNC) ? HIGH_LEVEL : DISPATCH_LEVEL;
			KIRQL irql;

			KeLowerIrql(PASSIVE_LEVEL);
			KeRaiseIrql(lo, &irql);

			for (int i = 0; i < 100000; i++) {
				irql = KeGetCurrentIrql();
				Assert::IsTrue(irql == ((i & 1) ? hi : lo));
				if (i & 1) KeLowerIrql(lo);
				else KeRaiseIrql(hi, &irql);
			}

			KeLowerIrql(PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkIrqlMulti)
		{
			TEST_ASYNC_START(id, DdkIrqlAsync);
			DdkIrqlAsync();
			TEST_ASYNC_WAIT(id);
		}
	};
}
