/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2014, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	DPC Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkDpcTest)
	{
		KDPC dpc;
		KDPC dpcthreaded;
		LONG count;
		LONG threaded;

	public:
		TEST_METHOD_INITIALIZE(DdkDpcTestInit)
		{
			DdkThreadInit();
			count = 0;
			threaded = 0;
			KeInitializeDpc(&dpc, DdkDpcProc, 0);
			KeInitializeThreadedDpc(&dpcthreaded, DdkDpcProcThreaded, 0);
		}

		TEST_METHOD_CLEANUP(DdkDpcTestCleanup)
		{
			KeFlushQueuedDpcs();
		}

		TEST_METHOD_CALLBACK(DdkDpcProc, PRKDPC Dpc,
			PVOID DeferredContext, PVOID Context, PVOID Arg2)
		{
			count++;
		}

		TEST_METHOD_CALLBACK(DdkDpcProcThreaded, PRKDPC Dpc,
			PVOID DeferredContext, PVOID Context, PVOID Arg2)
		{
			threaded++;
		}

		TEST_METHOD(DdkDpcQueue)
		{
			TEST_CALLBACK_INIT(id);
			Assert::IsTrue(KeInsertQueueDpc(&dpc, id, 0) != 0);

			TEST_CALLBACK_WAIT(id);
			Assert::IsTrue(count == 1);
		}

		TEST_METHOD(DdkDpcQueueThreaded)
		{
			TEST_CALLBACK_INIT(id);
			Assert::IsTrue(KeInsertQueueDpc(&dpcthreaded, id, 0) != 0);

			TEST_CALLBACK_WAIT(id);
			Assert::IsTrue(threaded == 1);
		}

		TEST_METHOD(DdkDpcQueueMany)
		{
			LONG rm = 0, add = 0;

			for (int i = 0; i < 1000; i++) {
				TEST_CALLBACK_INIT(id);

				Assert::IsTrue(KeInsertQueueDpc(&dpc, id, 0) != 0);

				if ((i % 10) == 0 && KeRemoveQueueDpc(&dpc)) {
					TEST_CALLBACK_CANCELLED(id);
					rm++;
				}

				else add++;

				TEST_CALLBACK_WAIT(id);
				Assert::IsTrue(count == add);
			}

			Assert::IsTrue(rm && add);
		}

		TEST_METHOD(DdkDpcFlushQueued)
		{
			TEST_CALLBACK_INIT(id);
			Assert::IsTrue(KeInsertQueueDpc(&dpc, id, 0) != 0);

			KeFlushQueuedDpcs();
			Assert::IsTrue(count == 1);

			TEST_CALLBACK_WAIT(id);
		}
	};
}
