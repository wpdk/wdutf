/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2014, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	CPU Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkCpuTest)
	{
	public:
		TEST_METHOD_INITIALIZE(DdkCpuTestInit)
		{
			DdkThreadInit();
		}

		TEST_METHOD(DdkCpuCount)
		{
			Assert::IsTrue(KeNumberProcessors > 0 && KeNumberProcessors < 16);
		}

	};
}
