/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Event Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkEventTest)
	{
		LARGE_INTEGER nowait;
		UNICODE_STRING u;
		KEVENT event;
		HANDLE h1;
		HANDLE h2;

		wchar_t EventName[100];

	public:
		TEST_METHOD_INITIALIZE(DdkEventTestInit)
		{
			DdkThreadInit();
			KeInitializeEvent(&event, NotificationEvent, FALSE);
			swprintf(EventName, sizeof(EventName) / sizeof(WCHAR),
				L"\\BaseNamedObjects\\EVT%I64d:%d", (LONGLONG)this, GetUniqueId());
			RtlInitUnicodeString(&u, EventName);
			nowait.QuadPart = 0;
			h1 = h2 = NULL;
		}

		TEST_METHOD_CLEANUP(DdkEventTestCleanup)
		{
			if (h1) ZwClose(h1);
			if (h1) ZwClose(h2);
		}

		TEST_METHOD(DdkEventInit)
		{
			Assert::IsTrue(KeReadStateEvent(&event) == FALSE);
		}

		TEST_METHOD(DdkEventInitSet)
		{
			KeInitializeEvent(&event, NotificationEvent, TRUE);
			Assert::IsTrue(KeReadStateEvent(&event) == TRUE);
		}

		TEST_METHOD(DdkEventInitSetSync)
		{
			KeInitializeEvent(&event, SynchronizationEvent, TRUE);
			Assert::IsTrue(KeReadStateEvent(&event) == TRUE);
		}

		TEST_METHOD(DdkEventSet)
		{
			KeSetEvent(&event, IO_NO_INCREMENT, FALSE);
			Assert::IsTrue(KeReadStateEvent(&event) == TRUE);
		}

		TEST_METHOD(DdkEventSetAgain)
		{
			KeSetEvent(&event, IO_NO_INCREMENT, FALSE);
			KeSetEvent(&event, IO_NO_INCREMENT, FALSE);
			Assert::IsTrue(KeReadStateEvent(&event) == TRUE);
		}

		TEST_METHOD(DdkEventReset)
		{
			Assert::IsTrue(KeResetEvent(&event) == 0);
			Assert::IsTrue(KeReadStateEvent(&event) == FALSE);
		}

		TEST_METHOD(DdkEventResetAgain)
		{
			KeSetEvent(&event, IO_NO_INCREMENT, FALSE);
			Assert::IsTrue(KeResetEvent(&event) != 0);
			Assert::IsTrue(KeResetEvent(&event) == 0);
			Assert::IsTrue(KeReadStateEvent(&event) == FALSE);
		}

		TEST_METHOD(DdkEventClear)
		{
			KeSetEvent(&event, IO_NO_INCREMENT, FALSE);
			KeClearEvent(&event);
			Assert::IsTrue(KeReadStateEvent(&event) == FALSE);
		}

		TEST_METHOD(DdkEventClearAgain)
		{
			KeClearEvent(&event);
			KeClearEvent(&event);
			Assert::IsTrue(KeReadStateEvent(&event) == FALSE);
		}

		TEST_METHOD(DdkEventReadState)
		{
			Assert::IsTrue(KeReadStateEvent(&event) == FALSE);
			Assert::IsTrue(KeReadStateEvent(&event) == FALSE);
		}

		TEST_METHOD(DdkEventReadStateSet)
		{
			KeSetEvent(&event, IO_NO_INCREMENT, FALSE);
			Assert::IsTrue(KeReadStateEvent(&event) == TRUE);
			Assert::IsTrue(KeReadStateEvent(&event) == TRUE);
		}

		TEST_METHOD(DdkEventReadStateSetSync)
		{
			KeInitializeEvent(&event, SynchronizationEvent, TRUE);
			Assert::IsTrue(KeReadStateEvent(&event) == TRUE);
			Assert::IsTrue(KeReadStateEvent(&event) == TRUE);
		}

		TEST_METHOD(DdkEventWait)
		{
			NTSTATUS rc = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_TIMEOUT);
		}

		TEST_METHOD(DdkEventWaitSet)
		{
			KeSetEvent(&event, IO_NO_INCREMENT, FALSE);
			NTSTATUS rc = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsTrue(KeReadStateEvent(&event) == TRUE);
		}

		TEST_METHOD(DdkEventWaitSetSync)
		{
			KeInitializeEvent(&event, SynchronizationEvent, TRUE);
			NTSTATUS rc = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsTrue(KeReadStateEvent(&event) == FALSE);
		}

		TEST_METHOD(DdkEventCreate)
		{
			PKEVENT pEvent = IoCreateSynchronizationEvent(&u, &h1);
			Assert::IsNotNull(pEvent);
			Assert::IsNotNull(h1);
			Assert::IsTrue(KeReadStateEvent(pEvent) != 0);
		}

		TEST_METHOD(DdkEventCreateOpen)
		{
			PKEVENT pEvent = IoCreateNotificationEvent(&u, &h1);
			Assert::IsNotNull(pEvent);

			KeSetEvent(&event, IO_NO_INCREMENT, FALSE);

			PKEVENT pEvent2 = IoCreateNotificationEvent(&u, &h2);
			Assert::IsNotNull(pEvent2);
			Assert::IsTrue(KeReadStateEvent(pEvent2) != 0);
		}

		TEST_METHOD(DdkEventCloseReset)
		{
			PKEVENT pEvent = IoCreateSynchronizationEvent(&u, &h1);
			Assert::IsNotNull(pEvent);

			KeSetEvent(&event, IO_NO_INCREMENT, FALSE);
			ZwClose(h1);
			h1 = 0;

			PKEVENT pEvent2 = IoCreateSynchronizationEvent(&u, &h2);
			Assert::IsNotNull(pEvent2);
			Assert::IsTrue(KeReadStateEvent(pEvent2) != 0);
		}

		TEST_METHOD(DdkEventCreateWait)
		{
			PKEVENT pEvent = IoCreateSynchronizationEvent(&u, &h1);
			Assert::IsNotNull(pEvent);

			KeClearEvent(pEvent);

			NTSTATUS rc = KeWaitForSingleObject(pEvent, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_TIMEOUT);
		}

		TEST_METHOD(DdkEventCreateWaitSet)
		{
			PKEVENT pEvent = IoCreateSynchronizationEvent(&u, &h1);
			Assert::IsNotNull(pEvent);

			NTSTATUS rc = KeWaitForSingleObject(pEvent, Executive, KernelMode, FALSE, &nowait);
			Assert::IsTrue(rc == STATUS_SUCCESS);
			Assert::IsTrue(KeReadStateEvent(pEvent) == FALSE);
		}
	};
}
