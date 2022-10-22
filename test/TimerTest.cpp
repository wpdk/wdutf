/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Timer Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkTimerTest)
	{
		KTIMER timer;
		KEVENT asyncEvent;
		LARGE_INTEGER dueTime;
		LARGE_INTEGER maxTime;
		LARGE_INTEGER timeStart;
		LONG count;

		static const int ThreadCount = 3;
		static const int PeriodicTimerIterations = 10;

		static const LONGLONG msec = 10000I64;

	public:
		TEST_METHOD_INITIALIZE(DdkTimerTestInit)
		{
			DdkThreadInit();
			KeInitializeEvent(&asyncEvent, SynchronizationEvent, FALSE);
			KeQuerySystemTime(&timeStart);

			dueTime.QuadPart = -30 * msec;
			maxTime.QuadPart = -60 * 1000 * msec;
			count = 0;
		}

		TEST_METHOD_CLEANUP(DdkTimerTestCleanup)
		{
			KeCancelTimer(&timer);
			KeFlushQueuedDpcs();
		}

		/*
		 *	On a loaded system the time intervals can be significantly
		 *	extended so the validation is fairly slack.
		 */
		bool CheckTime(LONGLONG expect) {
			LARGE_INTEGER timeEnd;
			KeQuerySystemTime(&timeEnd);

			LONGLONG delta = (timeEnd.QuadPart - timeStart.QuadPart);
			return (delta + 30 * msec >= expect) && (delta <= expect + 20 * 1000 * msec);
		}

		/*
		 *	Initialise and set a timer and test the initial state
		 */
		TEST_METHOD(DdkTimerInit)
		{
			KeInitializeTimer(&timer);
			Assert::IsTrue(KeReadStateTimer(&timer) == FALSE);

			KeInitializeTimerEx(&timer, NotificationTimer);
			Assert::IsTrue(KeReadStateTimer(&timer) == FALSE);

			KeInitializeTimerEx(&timer, SynchronizationTimer);
			Assert::IsTrue(KeReadStateTimer(&timer) == FALSE);
		}

		/*
		 *	Initialise and cancel a timer
		 */
		TEST_METHOD(DdkTimerCancel)
		{
			KeInitializeTimer(&timer);
			Assert::IsTrue(KeReadStateTimer(&timer) == FALSE);
			Assert::IsTrue(KeCancelTimer(&timer) == FALSE);
		}

		/*
		 *	Initialise and set a one-shot timer and then wait for it to complete
		 */
		TEST_METHOD(DdkTimerNotificationWait)
		{
			KeInitializeTimerEx(&timer, NotificationTimer);

			Assert::IsTrue(KeSetTimerEx(&timer, dueTime, 0, NULL) == FALSE);
			Assert::IsTrue(KeWaitForSingleObject(&timer, Executive, KernelMode, FALSE, &maxTime) == STATUS_SUCCESS);

			Assert::IsTrue(CheckTime(-dueTime.QuadPart));
			Assert::IsTrue(KeReadStateTimer(&timer) == TRUE);
		}

		/*
		 *	Initialise and set a one-shot timer and then wait for it to complete
		 */
		TEST_METHOD(DdkTimerSynchronizationWait)
		{
			KeInitializeTimerEx(&timer, SynchronizationTimer);

			Assert::IsTrue(KeSetTimerEx(&timer, dueTime, 0, NULL) == FALSE);
			Assert::IsTrue(KeWaitForSingleObject(&timer, Executive, KernelMode, FALSE, &maxTime) == STATUS_SUCCESS);

			Assert::IsTrue(CheckTime(-dueTime.QuadPart));
			Assert::IsTrue(KeReadStateTimer(&timer) == FALSE);
		}

		TEST_METHOD_CALLBACK(DdkTimerCallback, PRKDPC Dpc,
			PVOID Context, PVOID Arg1, PVOID Arg2)
		{
			Assert::IsTrue(KeReadStateTimer(&timer) == TRUE);
			count++;
		}

		/*
		 *	Initialise and set a one-shot timer, specify a DPC then wait for the DPC to run
		 */
		TEST_METHOD(DdkTimerDpc)
		{
			TEST_CALLBACK_INIT(id);

			KDPC dpc;
			KeInitializeDpc(&dpc, &DdkTimerCallback, id);
			KeInitializeTimerEx(&timer, NotificationTimer);
			Assert::IsTrue(KeSetTimerEx(&timer, dueTime, 0, &dpc) == FALSE);

			TEST_CALLBACK_WAIT(id);

			Assert::IsTrue(CheckTime(-dueTime.QuadPart));
			Assert::IsTrue(count == 1);
			Assert::IsTrue(KeReadStateTimer(&timer) == TRUE);
		}

		/*
		 *	Initialise and set a periodic Synchronization Timer, wait for it to complete
		 *	several times, then cancel it. Using (TIMER_TYPE = NotificationTimer and Period != 0)
		 *	is not so useful as in this case, according to the DDK: "all waiting threads are
		 *	released and the timer remains in the signaled state until it is explicitly reset"
		 *	as it's not obvious which API to use to reset the timer.
		 */
		TEST_METHOD(DdkTimerPeriodicSynchronization)
		{
			KeInitializeTimerEx(&timer, SynchronizationTimer);

			const int repeatMsec = 20;
			Assert::IsTrue(KeSetTimerEx(&timer, dueTime, repeatMsec, NULL) == FALSE);

			for (int i = 0; i < PeriodicTimerIterations; i++) {
				Assert::IsTrue(KeWaitForSingleObject(&timer, Executive, KernelMode, FALSE, &maxTime) == STATUS_SUCCESS);
				Assert::IsTrue(KeReadStateTimer(&timer) == FALSE);
				Assert::IsTrue(CheckTime(-dueTime.QuadPart + i * repeatMsec * msec));
			}

			Assert::IsTrue(KeCancelTimer(&timer) == TRUE);
		}

		/*
		 *	Initialise and set a periodic timer, specify a DPC, wait for it to complete
		 *	several times, then cancel it.
		 *	Warning: calling Logger::PrintMessage() completely messes up the timings (!)
		 */
		TEST_METHOD(DdkTimerPeriodicDpc)
		{
			TEST_CALLBACK_INIT(id);

			KDPC dpc;
			KeInitializeDpc(&dpc, &DdkTimerCallback, id);
			KeInitializeTimerEx(&timer, NotificationTimer);

			const int repeatMsec = 10;
			Assert::IsTrue(KeSetTimerEx(&timer, dueTime, repeatMsec, &dpc) == FALSE);

			for (int i = 0; i < PeriodicTimerIterations; i++) {
				TEST_CALLBACK_WAIT(id);
				Assert::IsTrue(CheckTime(-dueTime.QuadPart + i * repeatMsec * msec));
			}

			Assert::IsTrue(KeCancelTimer(&timer) == TRUE);
		}

		TEST_METHOD_ASYNC(DdkTimerAsync)
		{
			Assert::IsTrue(KeWaitForSingleObject(&timer, Executive, KernelMode, FALSE, &maxTime) == STATUS_SUCCESS);
			count++;
			KeSetEvent(&asyncEvent, IO_NO_INCREMENT, FALSE);
		}

		/*
		 *	Create several threads all waiting on a synchronization timer.
		 *	Verify that only one thread is released each time the timer expires.
		 */
		TEST_METHOD(DdkTimerSynchronizationMultipleThreads)
		{
			KeInitializeTimerEx(&timer, SynchronizationTimer);

			TEST_ASYNC_START_VEC(id, DdkTimerAsync, ThreadCount);

			for (int i = 0; i < ThreadCount; i++) {
				Assert::IsTrue(KeSetTimerEx(&timer, dueTime, 0, NULL) == FALSE);
				Assert::IsTrue(KeWaitForSingleObject(&asyncEvent, Executive, KernelMode, FALSE, &maxTime) == STATUS_SUCCESS);
				Assert::IsTrue(KeReadStateTimer(&timer) == FALSE);
				Assert::IsTrue(count == i+1);
			}

			TEST_ASYNC_WAIT_VEC(id);
			Assert::IsTrue(KeCancelTimer(&timer) == FALSE);
		}

		/*
		 *	Create several threads waiting on a notification timer.
		 *	All threads should be released when the timer expires.
		 */
		TEST_METHOD(DdkTimerNotificationMultipleThreads)
		{
			KeInitializeTimerEx(&timer, NotificationTimer);

			TEST_ASYNC_START_VEC(id, DdkTimerAsync, ThreadCount);
			Assert::IsTrue(KeSetTimerEx(&timer, dueTime, 0, NULL) == FALSE);
			TEST_ASYNC_WAIT_VEC(id);

			Assert::IsTrue(KeReadStateTimer(&timer) == TRUE);
			Assert::IsTrue(KeCancelTimer(&timer) == FALSE);
		}
	};
}
