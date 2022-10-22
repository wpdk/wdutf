/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	DDK Header Tests
 */

#include "stdafx.h"
#include <ntifs.h>
#include <wmilib.h>


namespace DdkUnitTest
{
	size_t _SizeofCriticalSection_ = sizeof(CRITICAL_SECTION);

	TEST_CLASS(DdkHeaderTest)
	{
	public:
		
		TEST_METHOD(DdkHeader)
		{
			Assert::IsTrue(true);
		}
	};
}
