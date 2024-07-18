/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 */

 /*
  *	BasicDriver Function Call Tests
  */

#include "stdafx.h"

extern "C" LONG BasicTestFunction(LONG v);

extern LONG BasicTestFunction2(LONG v);


namespace BasicDriverUnitTest
{
	TEST_CLASS(BasicDriverFunctionTest)
	{
		TEST_METHOD_INITIALIZE(BasicDriverFunctionTestInit)
		{
			DdkThreadInit();
		}

		// Call exported C function
		TEST_METHOD(ExportedCFunction)
		{
			LONG v = BasicTestFunction(10);

			Assert::AreEqual(12L, v);
		}

		// Call exported C++ function
		TEST_METHOD(ExportedCPPFunction)
		{
			LONG v = BasicTestFunction2(10);

			Assert::AreEqual(13L, v);
		}

		// Call private function

		TEST_METHOD(PrivateFunction)
		{
			TEST_FIND_FUNCTION(testfunc, LONG, "PrivateTestFunction", (LONG v));

			LONG v = testfunc(10);

			Assert::AreEqual(11L, v);
		}

		// Mock private function (see DetoursTest.cpp for more examples)

		TEST_METHOD(MockPrivateFunction)
		{
			TEST_DEFINE_MOCK_FUNCTION(testfunc, LONG, "PrivateTestFunction", (LONG v)) {
				return TEST_REAL_FUNCTION(v) + 100;
			} TEST_END_MOCK;

			TEST_USING_MOCK(testfunc);
			Assert::AreEqual(112L, BasicTestFunction(10));
		}
	};
}
