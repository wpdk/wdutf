/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Thread Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkThreadTest)
	{
		const int DefaultThreadPriority = 8;

		HANDLE child;
		HANDLE h;

	public:
		TEST_METHOD_INITIALIZE(DdkThreadTestInit)
		{
			DdkThreadInit();
			child = 0;
			h = 0;

			KeSetBasePriorityThread(KeGetCurrentThread(), 0);
			KeSetPriorityThread(KeGetCurrentThread(), DefaultThreadPriority);
		}

		TEST_METHOD_CLEANUP(DdkThreadTestCleanup)
		{
			if (h) ZwClose(h);

			KeSetBasePriorityThread(KeGetCurrentThread(), 0);
			KeSetPriorityThread(KeGetCurrentThread(), DefaultThreadPriority);
		}

		TEST_METHOD(DdkThreadId)
		{
			Assert::IsNotNull(PsGetCurrentThread());
			Assert::IsNotNull(PsGetCurrentThreadId());
		}

		TEST_METHOD(DdkThreadSystem)
		{
			Assert::IsTrue(PsIsSystemThread(PsGetCurrentThread()) == TRUE);
			Assert::IsTrue(PsGetCurrentProcess() == PsInitialSystemProcess);
		}

		TEST_METHOD(DdkThreadProcessId)
		{
			Assert::IsNotNull(PsGetCurrentProcess());
			Assert::IsNotNull(PsGetCurrentProcessId());
			Assert::IsTrue(PsGetProcessId(PsGetCurrentProcess()) == PsGetCurrentProcessId());	
		}

		TEST_METHOD_CALLBACK(DdkThreadProc, PVOID Context)
		{
			PKTHREAD pkThread = KeGetCurrentThread();
			child = PsGetCurrentThreadId();
						
			Assert::IsTrue(KeQueryPriorityThread(pkThread) == DefaultThreadPriority);
			Assert::IsTrue(KeSetBasePriorityThread(pkThread, 0) == 0);
		}
	
		TEST_METHOD(DdkThreadCreate)
		{
			TEST_CALLBACK_INIT(cb);

			NTSTATUS rc = PsCreateSystemThread(&h,
				THREAD_ALL_ACCESS, NULL, NULL, NULL, DdkThreadProc, cb); 

			Assert::IsNotNull(h);
			Assert::IsTrue(h != PsGetCurrentThreadId());

			TEST_CALLBACK_WAIT(cb);

			Assert::IsNotNull(child);
			Assert::IsTrue(child != PsGetCurrentThreadId());
		}

		TEST_METHOD(DdkThreadQueryPriorityDefault)
		{
			PKTHREAD currentThread = KeGetCurrentThread();

			Assert::IsTrue(DefaultThreadPriority == KeQueryPriorityThread(currentThread));
		}

		TEST_METHOD(DdkThreadSetPriorityIncrement)
		{
			PKTHREAD currentThread = KeGetCurrentThread();

			for (int priority = DefaultThreadPriority + 1; priority <= 31; priority++){
				Assert::IsTrue(KeSetPriorityThread(currentThread, priority) == priority - 1);
				Assert::IsTrue(KeQueryPriorityThread(currentThread) == priority);
			}
		}

		TEST_METHOD(DdkThreadSetPriorityDecrement)
		{
			PKTHREAD currentThread = KeGetCurrentThread();

			for (int priority = DefaultThreadPriority - 1; priority >= 0; priority--){
				Assert::IsTrue(KeSetPriorityThread(currentThread, priority) == priority + 1);
				Assert::IsTrue(KeQueryPriorityThread(currentThread) == priority);
			}
		}

		TEST_METHOD(DdkThreadSetPriorityBelowMin)
		{
			PKTHREAD currentThread = KeGetCurrentThread();

			KeSetPriorityThread(currentThread, -1);
			Assert::IsTrue(KeQueryPriorityThread(currentThread) == 0);
		}

		TEST_METHOD(DdkThreadSetPriorityAboveMax)
		{
			PKTHREAD currentThread = KeGetCurrentThread();

			KeSetPriorityThread(currentThread, 32);
			Assert::IsTrue(KeQueryPriorityThread(currentThread) == 31);
		}

		TEST_METHOD(DdkThreadSetBasePriorityIncrement)
		{
			PKTHREAD currentThread = KeGetCurrentThread();
			PEPROCESS currentProcess = PsGetCurrentProcess();

			KPRIORITY oldBaseDif = KeSetBasePriorityThread(currentThread, 0);
			Assert::IsTrue(oldBaseDif == 0);

			for (int i = 1; i < 16; i++) {
				oldBaseDif = KeSetBasePriorityThread(currentThread, i);
				Assert::IsTrue(oldBaseDif == i - 1);
			}
		}

		TEST_METHOD(DdkThreadSetBasePriorityDecrement)
		{
			PKTHREAD currentThread = KeGetCurrentThread();

			KPRIORITY oldBaseDif = KeSetBasePriorityThread(currentThread, 0);
			Assert::IsTrue(KeSetBasePriorityThread(currentThread, 0) == 0);

			for (int i = -1; i > -16; i--) {
				oldBaseDif = KeSetBasePriorityThread(currentThread, i);
				Assert::IsTrue(oldBaseDif == i + 1);
			}
		}
	};
}
