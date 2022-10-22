/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Windows Header Tests
 */

#include <Windows.h>
#include <ddk.h>
#include <ntifs.h>
#include <wmilib.h>
#include "stdafx.h"


namespace DdkUnitTest
{
	extern size_t _SizeofCriticalSection_;

	TEST_CLASS(DdkWindowsHeaderTest)
	{
	public:
		
		TEST_METHOD(DdkWindowsHeader)
		{
			Assert::IsTrue(sizeof(CRITICAL_SECTION) == _SizeofCriticalSection_);
		}
	};
}
