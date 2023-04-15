/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Memory Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkMemoryTest)
	{
	public:
		TEST_METHOD_INITIALIZE(DdkMemoryTestInit)
		{
			DdkThreadInit();
		}
		
		TEST_METHOD(DdkMemoryPhysAddr)
		{
			LARGE_INTEGER addr = MmGetPhysicalAddress((PVOID)0x123456789I64);
			Assert::IsTrue(addr.QuadPart == 0x123456789I64);
		}

		TEST_METHOD(DdkMemoryGetSystemRoutine)
		{
			UNICODE_STRING u = RTL_CONSTANT_STRING(L"RtlGetVersion");
			PVOID addr = MmGetSystemRoutineAddress(&u);
			Assert::IsNotNull(addr);
			Assert::AreEqual((PVOID)&RtlGetVersion, addr);
		}

		TEST_METHOD(DdkMemoryGetSystemRoutineFail)
		{
			UNICODE_STRING u = RTL_CONSTANT_STRING(L"RtlGetVersion2");
			PVOID addr = MmGetSystemRoutineAddress(&u);
			Assert::IsNull(addr);
		}

		TEST_METHOD(DdkMemoryGetSystemRoutineDdk)
		{
			UNICODE_STRING u = RTL_CONSTANT_STRING(L"DdkThreadInit");
			PVOID addr = MmGetSystemRoutineAddress(&u);
			Assert::IsNull(addr);
		}
	};
}
