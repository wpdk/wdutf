/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	MDL Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkMdlTest)
	{
		MDL *pMdl;
		MDL *pMdl2;
		IRP *pIrp;

	public:
		TEST_METHOD_INITIALIZE(DdkMdlInit)
		{
			DdkThreadInit();
			pMdl = pMdl2 = 0;
			pIrp = 0;
		}

		TEST_METHOD_CLEANUP(DdkMdlCleanup)
		{
			if (pMdl) IoFreeMdl(pMdl);
			if (pMdl2) IoFreeMdl(pMdl2);
			if (pIrp) IoFreeIrp(pIrp);
		}
		
		TEST_METHOD(DdkMdlSize)
		{
			SIZE_T base = sizeof(MDL);
			Assert::IsTrue(MmSizeOfMdl(0, 0) == base);
			Assert::IsTrue(MmSizeOfMdl(0, 1) == base + sizeof(PFN_NUMBER));
			Assert::IsTrue(MmSizeOfMdl((PVOID)(PAGE_SIZE-1), 2) == base + sizeof(PFN_NUMBER) * 2);
		}

		TEST_METHOD(DdkMdlAllocate)
		{
			pMdl = IoAllocateMdl((PVOID)1, PAGE_SIZE*3, FALSE, FALSE, NULL);
			Assert::IsNotNull(pMdl);
			Assert::IsTrue(MmGetMdlByteOffset(pMdl) == 1);
			Assert::IsTrue(MmGetMdlByteCount(pMdl) == PAGE_SIZE*3);
			Assert::IsTrue(MmGetMdlVirtualAddress(pMdl) == (PVOID)1);
			Assert::IsTrue(pMdl->MdlFlags == 0);
		}

		TEST_METHOD(DdkMdlFree)
		{
			pMdl = IoAllocateMdl((PVOID)1, PAGE_SIZE*3, FALSE, FALSE, NULL);
			Assert::IsNotNull(pMdl);
			IoFreeMdl(pMdl);
			pMdl = 0;
		}

		TEST_METHOD(DdkMdlProbeAndLock)
		{
			pMdl = IoAllocateMdl((PVOID)1, PAGE_SIZE*3, FALSE, FALSE, NULL);
			Assert::IsNotNull(pMdl);

			MmProbeAndLockPages(pMdl, KernelMode, IoModifyAccess);

			for (int i = 0; i < 4; i++)
				Assert::IsTrue(MmGetMdlPfnArray(pMdl)[i] == i);
		}

		TEST_METHOD(DdkMdlBuildPartial)
		{
			pMdl = IoAllocateMdl((PVOID)1, PAGE_SIZE*3, FALSE, FALSE, NULL);
			pMdl2 = IoAllocateMdl((PVOID)1, PAGE_SIZE*3, FALSE, FALSE, NULL);
			Assert::IsNotNull(pMdl);
			Assert::IsNotNull(pMdl2);

			MmProbeAndLockPages(pMdl, KernelMode, IoModifyAccess);
			IoBuildPartialMdl(pMdl, pMdl2, (PVOID)100, 0);

			Assert::IsTrue(MmGetMdlByteOffset(pMdl2) == 100);
			Assert::IsTrue(MmGetMdlByteCount(pMdl2) == PAGE_SIZE*3-99);
			Assert::IsTrue(MmGetMdlVirtualAddress(pMdl2) == (PVOID)100);

			for (int i = 0; i < 4; i++)
				Assert::IsTrue(MmGetMdlPfnArray(pMdl2)[i] == i);
		}

		TEST_METHOD(DdkMdlPrepareReuse)
		{
			pMdl = IoAllocateMdl((PVOID)1, PAGE_SIZE*3, FALSE, FALSE, NULL);
			pMdl2 = IoAllocateMdl((PVOID)1, PAGE_SIZE*3, FALSE, FALSE, NULL);
			Assert::IsNotNull(pMdl);
			Assert::IsNotNull(pMdl2);

			MmProbeAndLockPages(pMdl, KernelMode, IoModifyAccess);
			IoBuildPartialMdl(pMdl, pMdl2, (PVOID)100, 0);
			MmPrepareMdlForReuse(pMdl2);
		}

		TEST_METHOD(DdkMdlAttachIrp)
		{
			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);

			pMdl = IoAllocateMdl((PVOID)4095, PAGE_SIZE*3, FALSE, FALSE, pIrp);
			Assert::IsNotNull(pMdl);
			Assert::IsTrue(pIrp->MdlAddress == pMdl);
		}

		TEST_METHOD(DdkMdlAttachIrpSecondary)
		{
			pIrp = IoAllocateIrp(3, FALSE);
			Assert::IsNotNull(pIrp);
			pMdl = IoAllocateMdl((PVOID)4095, PAGE_SIZE*3, FALSE, FALSE, pIrp);
			Assert::IsNotNull(pMdl);

			pMdl2 = IoAllocateMdl((PVOID)40960, PAGE_SIZE, TRUE, FALSE, pIrp);
			Assert::IsNotNull(pMdl2);
			Assert::IsTrue(pIrp->MdlAddress == pMdl);
			Assert::IsTrue(pIrp->MdlAddress->Next == pMdl2);
		}
	};
}
