/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Object Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	static POBJECT_TYPE *TypeVec[] = {
		PsProcessType, CmKeyObjectType, ExEventObjectType, ExSemaphoreObjectType,
		IoFileObjectType, PsThreadType, SeTokenObjectType, TmEnlistmentObjectType,
		TmResourceManagerObjectType, TmTransactionManagerObjectType,
		TmTransactionObjectType, 0
	};

	TEST_CLASS(DdkObjectTest)
	{
	public:
		TEST_METHOD_INITIALIZE(DdkObjectInit)
		{
			DdkThreadInit();
			Assert::IsNotNull(PsGetCurrentProcess());
		}
			
		TEST_METHOD(DdkObjectCheckTypeByPointer)
		{
			for (int i = 1; TypeVec[i]; i++) {
				NTSTATUS rc = ObReferenceObjectByPointer(PsGetCurrentProcess(),
					THREAD_ALL_ACCESS, *TypeVec[i], KernelMode);

				Assert::IsTrue(rc == STATUS_OBJECT_TYPE_MISMATCH);
			}
		}

		TEST_METHOD(DdkObjectCheckTypeByHandle)
		{
			PVOID pObj;

			for (int i = 1; TypeVec[i]; i++) {
				NTSTATUS rc = ObReferenceObjectByHandle(PsGetCurrentProcessId(),
					THREAD_ALL_ACCESS, *TypeVec[i], KernelMode, &pObj, NULL);

				Assert::IsTrue(rc == STATUS_OBJECT_TYPE_MISMATCH);
			}
		}

		TEST_METHOD(DdkObjectReferenceByPointer)
		{
			NTSTATUS rc = ObReferenceObjectByPointer(PsGetCurrentProcess(),
					THREAD_ALL_ACCESS, *PsProcessType, KernelMode);

			Assert::IsTrue(rc == STATUS_SUCCESS);
		}

		TEST_METHOD(DdkObjectReferenceByHandle)
		{
			PVOID pObj;

			NTSTATUS rc = ObReferenceObjectByHandle(PsGetCurrentProcessId(),
				THREAD_ALL_ACCESS, *PsProcessType, KernelMode, &pObj, NULL);

			Assert::IsTrue(rc == STATUS_SUCCESS);
		}

		TEST_METHOD(DdkObjectDereference)
		{
			PVOID pObj;

			NTSTATUS rc = ObReferenceObjectByHandle(PsGetCurrentProcessId(),
				THREAD_ALL_ACCESS, *PsProcessType, KernelMode, &pObj, NULL);

			Assert::IsTrue(rc == STATUS_SUCCESS);

			ObDereferenceObject(pObj);
		}

		TEST_METHOD(DdkObjectFinalClose)
		{
			UNICODE_STRING u;
			HANDLE Event;

			RtlInitUnicodeString(&u, L"Object1");
			PKEVENT pEvent = IoCreateSynchronizationEvent(&u, &Event);
			Assert::IsNotNull(pEvent);

			KeResetEvent(pEvent);
			Assert::IsTrue(KeReadStateEvent(pEvent) == FALSE);

			NTSTATUS rc = ObReferenceObjectByPointer(pEvent,
					THREAD_ALL_ACCESS, *ExEventObjectType, KernelMode);

			Assert::IsTrue(rc == STATUS_SUCCESS);

			ZwClose(Event);
			Assert::IsTrue(KeReadStateEvent(pEvent) == FALSE);

			ObDereferenceObject(pEvent);
			pEvent = IoCreateSynchronizationEvent(&u, &Event);
			Assert::IsNotNull(pEvent);

			Assert::IsTrue(KeReadStateEvent(pEvent) == TRUE);
			ZwClose(pEvent);
		}
	};
}
