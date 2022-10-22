/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	DDK Unit Test Initialization
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_MODULE_INITIALIZE(DdkUnitTestInit)
	{
		TEST_MODULE_START(DdkUnitTestCleanup);
	}

	TEST_MODULE_CLEANUP(DdkUnitTestCleanup)
	{
		TEST_MODULE_END();
	}

	LONG GetUniqueId() {
		static volatile LONG value = 0;
		return InterlockedIncrement(&value);
	}
}
