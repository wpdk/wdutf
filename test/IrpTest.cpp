/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	IRP Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkIrpTest)
	{
		DEVICE_OBJECT device;
		IRP *pIrp;

	public:
		TEST_METHOD_INITIALIZE(DdkIrpTestInit)
		{
			DdkThreadInit();
		}

		TEST_METHOD_CLEANUP(DdkIrpTestCleanup)
		{
			if (pIrp) IoFreeIrp(pIrp);
		}

		TEST_METHOD(DdkIrpAllocate)
		{
			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);
		}

		TEST_METHOD(DdkIrpFree)
		{
			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);
			IoFreeIrp(pIrp);
			pIrp = 0;
		}

		TEST_METHOD(DdkIrpStackZero)
		{
			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IO_STACK_LOCATION *pStack = IoGetNextIrpStackLocation(pIrp);
			Assert::IsTrue(pStack->MajorFunction == 0);
			Assert::IsTrue(pStack->Context == 0);
		}

		TEST_METHOD(DdkIrpStackNext)
		{
			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IO_STACK_LOCATION *pStack = IoGetNextIrpStackLocation(pIrp);
			IoSetNextIrpStackLocation(pIrp);

			Assert::IsFalse(IoGetNextIrpStackLocation(pIrp) == pStack);
			Assert::IsTrue(IoGetCurrentIrpStackLocation(pIrp) == pStack);
		}

		TEST_METHOD(DdkIrpCompleteRequest)
		{
			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IoSetNextIrpStackLocation(pIrp);

			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		}

		TEST_METHOD(DdkIrpMultipleCompletions)
		{
			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IoSetNextIrpStackLocation(pIrp);

			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			TEST_EXPECT_ASSERT(IoCompleteRequest(pIrp, IO_NO_INCREMENT));
		}

		TEST_METHOD(DdkIrpStackCopyNext)
		{
			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IO_STACK_LOCATION *pStack = IoGetNextIrpStackLocation(pIrp);
			pStack->MajorFunction = IRP_MJ_CREATE;

			IoSetNextIrpStackLocation(pIrp);
			IoCopyCurrentIrpStackLocationToNext(pIrp);
			pStack = IoGetNextIrpStackLocation(pIrp);

			Assert::IsTrue(pStack->MajorFunction == IRP_MJ_CREATE);
		}

		static NTSTATUS IrpCompletion(DEVICE_OBJECT *pDevice, IRP *pIrp, void *pContext)
		{
			*(LONG_PTR *)pContext += (ULONG_PTR)pDevice + 1;
			return (pDevice) ? STATUS_SUCCESS : STATUS_MORE_PROCESSING_REQUIRED;
		}

		TEST_METHOD(DdkIrpCompletion)
		{
			ULONG_PTR v = 0;

			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IO_STACK_LOCATION *pStack = IoGetNextIrpStackLocation(pIrp);
			pStack->DeviceObject = &device;

			IoSetCompletionRoutine(pIrp, IrpCompletion, (PVOID) &v, TRUE, TRUE, TRUE);
			IoSetNextIrpStackLocation(pIrp);

			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			Assert::AreEqual((ULONG_PTR)1, v);
		}

		TEST_METHOD(DdkIrpCompleteSuccess)
		{
			ULONG_PTR v = 0;

			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IO_STACK_LOCATION *pStack = IoGetNextIrpStackLocation(pIrp);
			pStack->DeviceObject = &device;

			IoSetCompletionRoutine(pIrp, IrpCompletion, (PVOID)&v, TRUE, FALSE, FALSE);
			IoSetNextIrpStackLocation(pIrp);

			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			Assert::AreEqual((ULONG_PTR)1, v);
		}

		TEST_METHOD(DdkIrpCompleteNotSuccess)
		{
			ULONG_PTR v = 0;

			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IO_STACK_LOCATION *pStack = IoGetNextIrpStackLocation(pIrp);
			pStack->DeviceObject = &device;

			IoSetCompletionRoutine(pIrp, IrpCompletion, (PVOID)&v, TRUE, FALSE, FALSE);
			IoSetNextIrpStackLocation(pIrp);

			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			Assert::AreEqual((ULONG_PTR)0, v);
		}

		TEST_METHOD(DdkIrpCompleteError)
		{
			ULONG_PTR v = 0;

			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IO_STACK_LOCATION *pStack = IoGetNextIrpStackLocation(pIrp);
			pStack->DeviceObject = &device;

			IoSetCompletionRoutine(pIrp, IrpCompletion, (PVOID)&v, FALSE, TRUE, FALSE);
			IoSetNextIrpStackLocation(pIrp);

			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			Assert::AreEqual((ULONG_PTR)1, v);
		}

		TEST_METHOD(DdkIrpCompleteNotError)
		{
			ULONG_PTR v = 0;

			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IO_STACK_LOCATION *pStack = IoGetNextIrpStackLocation(pIrp);
			pStack->DeviceObject = &device;

			IoSetCompletionRoutine(pIrp, IrpCompletion, (PVOID)&v, FALSE, TRUE, FALSE);
			IoSetNextIrpStackLocation(pIrp);

			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			Assert::AreEqual((ULONG_PTR)0, v);
		}

		TEST_METHOD(DdkIrpCompletePartial)
		{
			ULONG_PTR v1 = 0, v2 = 0;

			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IoSetCompletionRoutine(pIrp, IrpCompletion, (PVOID)&v1, TRUE, TRUE, TRUE);
			IoSetNextIrpStackLocation(pIrp);

			IoSetCompletionRoutine(pIrp, IrpCompletion, (PVOID)&v2, TRUE, TRUE, TRUE);
			IoSetNextIrpStackLocation(pIrp);

			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			Assert::AreEqual((ULONG_PTR)0, v1);
			Assert::AreEqual((ULONG_PTR)1, v2);
		}

		TEST_METHOD(DdkIrpCompleteStack)
		{
			ULONG_PTR v1 = 0, v2 = 0;

			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			IO_STACK_LOCATION *pStack = IoGetNextIrpStackLocation(pIrp);
			pStack->DeviceObject = &device;

			IoSetCompletionRoutine(pIrp, IrpCompletion, (PVOID)&v1, TRUE, TRUE, TRUE);
			IoSetNextIrpStackLocation(pIrp);

			pStack = IoGetNextIrpStackLocation(pIrp);
			pStack->DeviceObject = &device;

			IoSetCompletionRoutine(pIrp, IrpCompletion, (PVOID)&v2, TRUE, TRUE, TRUE);
			IoSetNextIrpStackLocation(pIrp);

			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			Assert::AreEqual((ULONG_PTR)1, v1);
			Assert::AreEqual((ULONG_PTR)&device + 1, v2);
		}
	};
}
