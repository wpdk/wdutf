/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 */

/*
 *	BasicDriver Unit Test Initialization
 */

#include "stdafx.h"

extern "C" LONG BasicTestFunction(LONG v);

extern LONG BasicTestFunction2(LONG v);


namespace BasicDriverUnitTest
{		
	TEST_MODULE_INITIALIZE(BasicDriverUnitTestInit)
	{
		TEST_MODULE_START(BasicDriverUnitTestCleanup);
		DdkLoadDriver("DcsBasic");
	}

	TEST_MODULE_CLEANUP(BasicDriverUnitTestCleanup)
	{
		TEST_MODULE_END();
		DdkUnloadDriver("DcsBasic");
	}

	TEST_CLASS(BasicDriverUnitTest)
	{
		TEST_METHOD(BasicDriverLoad)
		{
		}

		TEST_METHOD(BasicDriverTest)
		{
			LONG v = BasicTestFunction(10);

			Assert::IsTrue(v == 11);
		}

		TEST_METHOD(BasicDriverTest2)
		{
			LONG v = BasicTestFunction2(10);

			Assert::IsTrue(v == 12);
		}
	};
}
