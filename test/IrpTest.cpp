/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
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
	};
}
