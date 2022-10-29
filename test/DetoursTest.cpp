/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2022, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

 /*
  *	Detours Tests
  */

#include "stdafx.h"


namespace DdkUnitTest
{
	LONG TestDetoursVar = 100;

	// Note: Need to ensure test functions are large enough
	// to detour and don't get inlined

	__declspec(noinline) void DoWork() { GetCurrentThreadId(); }

	__declspec(noinline) LONG TestDetourFunction(LONG arg) {
		return DoWork(), arg + 1;
	}

	__declspec(noinline) LONG TestDetourFunction2(LONG arg) {
		return DoWork(), arg + 2;
	}

	__declspec(noinline) LONG TestDetourFunction3(LONG arg) {
		return DoWork(), arg + 3;
	}

	__declspec(noinline) LONG TestDetourFunction4(LONG arg) {
		return DoWork(), arg + 4;
	}

	__declspec(noinline) LONG TestDetourFunction5(LONG arg) {
		return DoWork(), arg + 5;
	}

	class TestDetourClass {
		volatile LONG value;
	public:
		TestDetourClass(LONG v) : value(v) { }
		__declspec(noinline) LONG Increment(LONG arg) {
			return DoWork(), arg + 1;
		}
	};

	// Test defining mocks at global scope

	TEST_DEFINE_MOCK_FUNCTION(function1, LONG, &TestDetourFunction, (LONG arg)) {
		return TEST_REAL_FUNCTION(arg) + 0x100;
	} TEST_END_MOCK;

	TEST_DEFINE_MOCK_METHOD(method1, TestDetourClass, LONG, &TestDetourClass::Increment, (LONG arg)) {
		return TEST_REAL_METHOD(arg) + 0x100;
	} TEST_END_MOCK;

	// Test global mock enabled for all threads

	TEST_DEFINE_MOCK_FUNCTION(function9, LONG, &TestDetourFunction3, (LONG arg)) {
		return TEST_REAL_FUNCTION(arg) + 0x900;
	} TEST_END_MOCK;

	TEST_CLASS(DdkDetoursTest)
	{
		// Test defining mocks at class scope

		TEST_DEFINE_MOCK_FUNCTION(function2, LONG, &TestDetourFunction, (LONG arg)) {
			return TEST_REAL_FUNCTION(arg) + 0x200;
		} TEST_END_MOCK;

		TEST_DEFINE_MOCK_METHOD(method2, TestDetourClass, LONG, &TestDetourClass::Increment, (LONG arg)) {
			return TEST_REAL_METHOD(arg) + 0x200;
		} TEST_END_MOCK;

	public:
		TEST_METHOD_INITIALIZE(DdkDetoursTestInit)
		{
			DdkThreadInit();

			// Enable mock at class creation for later testing

			TEST_ENABLE_MOCK(function9);
			Assert::IsTrue(TestDetourFunction3(1) == 0x904);
		}

		TEST_METHOD_CLEANUP(DdkDetoursTestCleanup)
		{
			Assert::IsTrue(TestDetourFunction3(1) == 0x904);
			TEST_DISABLE_MOCK(function9);
			Assert::IsTrue(TestDetourFunction3(1) == 0x4);
		}

		TEST_METHOD(DdkDetoursFindFunction)
		{
			// Function in current DLL

			TEST_FIND_FUNCTION(uniq1, LONG, GetUniqueId, ());
			TEST_FIND_FUNCTION(uniq2, LONG, "DdkUnitTest::GetUniqueId", ());

			Assert::IsTrue(uniq1.GetPointer() == uniq2.GetPointer());
			Assert::IsTrue(uniq1.GetPointer() == (void *)&GetUniqueId);

			// Check function can be called

			LONG u1 = GetUniqueId();
			LONG u2 = uniq2();

			Assert::IsTrue(u2 != u1);

			// Taking address of function

			TEST_FIND_FUNCTION(uniq3, void, &GetUniqueId, ());

			Assert::IsTrue(uniq1.GetPointer() == uniq3.GetPointer());

			// Using pointer object

			TEST_FIND_FUNCTION(uniq4, void, uniq1, ());

			Assert::IsTrue(uniq1.GetPointer() == uniq4.GetPointer());

			// Using PVOID

			PVOID ptr = (PVOID)&GetUniqueId;

			TEST_FIND_FUNCTION(uniq6, LONG, ptr, ());
			TEST_FIND_FUNCTION(uniq7, LONG, &ptr, ());

			Assert::IsTrue(uniq1.GetPointer() == uniq6.GetPointer());
			Assert::IsTrue(uniq1.GetPointer() == uniq7.GetPointer());

			// Check function in DDK DLL

			TEST_FIND_FUNCTION(init1, void, DdkThreadInit, ());
			TEST_FIND_FUNCTION(init2, void, "DdkThreadInit", ());

			Assert::IsTrue(init1.GetPointer() == init2.GetPointer());

			// Check function in system DLL

			TEST_FIND_FUNCTION(get1, DWORD, GetLastError, ());
			TEST_FIND_FUNCTION(get2, DWORD, "GetLastError", ());

			Assert::IsTrue(get1.GetPointer() == get2.GetPointer());

			// Check function in specific DLL

			TEST_FIND_FUNCTION(init3, void, "ddk!DdkThreadInit", ());
			TEST_FIND_FUNCTION(init4, void, "dxk!DdkThreadInit", ());

			Assert::IsTrue(init1.GetPointer() == init3.GetPointer());
			Assert::IsFalse(init4.IsValid());

			// Check static function in specific DLL

			char *DdkReadImage(char *pPath, DWORD *pSize);

			TEST_FIND_FUNCTION(read1, char *, "ddk!DdkReadImage", (char *pPath, DWORD *pSize));

			Assert::IsTrue(read1.IsValid());

			// Check function in specific DLL with suffix

			TEST_FIND_FUNCTION(init5, void, "ddk.dll!DdkThreadInit", ());
			TEST_FIND_FUNCTION(init6, void, "dxk.dll!DdkThreadInit", ());

			Assert::IsTrue(init1.GetPointer() == init5.GetPointer());
			Assert::IsFalse(init6.IsValid());
		}

		TEST_METHOD(DdkDetoursFindMethod)
		{
			TestDetourClass test(10);

			// Method in current DLL

			TEST_FIND_METHOD(incr1, TestDetourClass, LONG, &TestDetourClass::Increment, (LONG arg));
			TEST_FIND_METHOD(incr2, TestDetourClass, LONG,
				"DdkUnitTest::TestDetourClass::Increment", (LONG arg));

			Assert::IsTrue(incr1.GetPointer() == incr2.GetPointer());
			Assert::IsTrue(incr1 == &TestDetourClass::Increment);

			// Check method can be called

			LONG u1 = test.Increment(1);
			LONG u2 = (&test->*incr1)(2);

			Assert::IsTrue(u2 == u1 + 1);

			// Using pointer object

			TEST_FIND_METHOD(incr3, TestDetourClass, LONG, incr1, (LONG arg));
			TEST_FIND_METHOD(incr4, TestDetourClass, LONG, incr2, (LONG arg));

			Assert::IsTrue(incr1.GetPointer() == incr3.GetPointer());
			Assert::IsTrue(incr1.GetPointer() == incr4.GetPointer());
		}

		TEST_METHOD(DdkDetoursFindVariable)
		{
			// Variable in current DLL

			TEST_FIND_VARIABLE(var1, LONG, &TestDetoursVar);
			TEST_FIND_VARIABLE(var2, LONG, "DdkUnitTest::TestDetoursVar");

			Assert::IsTrue(var1.GetPointer() == var2.GetPointer());
			Assert::IsTrue(var1.GetPointer() == &TestDetoursVar);

			// Variable in DDK DLL

			TEST_FIND_VARIABLE(var3, CCHAR *, "ddk!KeNumberProcessors");

			Assert::IsTrue(var3.GetPointer() != NULL);

			// Static variable in DDK DLL

			TEST_FIND_VARIABLE(var4, char *, "ddk!cleanupname");

			Assert::IsTrue(var4.GetPointer() != NULL);
		}

		TEST_METHOD(DdkDetoursAttachFunction)
		{
			Assert::IsTrue(TestDetourFunction(1) == 2);

			// Attach with Using
			{
				TEST_USING_MOCK(function1);
				Assert::IsTrue(TestDetourFunction(1) == 0x102);
			}

			Assert::IsTrue(TestDetourFunction(1) == 2);

			// Attach a different mock
			{
				TEST_USING_MOCK(function2);
				Assert::IsTrue(TestDetourFunction(1) == 0x202);
			}

			Assert::IsTrue(TestDetourFunction(1) == 2);

			TEST_DEFINE_MOCK_FUNCTION(function3, LONG, &TestDetourFunction, (LONG arg)) {
				return TEST_REAL_FUNCTION(arg) + 0x300;
			} TEST_END_MOCK;

			// Test attaching mock with local scope
			{
				TEST_USING_MOCK(function3);
				Assert::IsTrue(TestDetourFunction(1) == 0x302);
			}

			Assert::IsTrue(TestDetourFunction(1) == 2);
		}

		TEST_METHOD(DdkDetoursAttachMethod)
		{
			TestDetourClass test(10);

			Assert::IsTrue(test.Increment(1) == 2);

			// Attach with Using
			{
				TEST_USING_MOCK(method1);
				Assert::IsTrue(test.Increment(1) == 0x102);
			}

			Assert::IsTrue(test.Increment(1) == 2);

			// Attach a different mock
			{
				TEST_USING_MOCK(method2);
				Assert::IsTrue(test.Increment(1) == 0x202);
			}

			Assert::IsTrue(test.Increment(1) == 2);

			// Test attaching mock with local scope

			TEST_DEFINE_MOCK_METHOD(method3, TestDetourClass, LONG, &TestDetourClass::Increment, (LONG arg)) {
				return TEST_REAL_METHOD(arg) + 0x300;
			} TEST_END_MOCK;

			{
				TEST_USING_MOCK(method3);
				Assert::IsTrue(test.Increment(1) == 0x302);
			}

			Assert::IsTrue(test.Increment(1) == 2);
		}

		TEST_METHOD(DdkDetoursGlobalEnable)
		{
			// Check mock enabled in constructor

			Assert::IsTrue(TestDetourFunction3(1) == 0x904);
		}

		TEST_METHOD(DdkDetoursEnableFunction)
		{
			Assert::IsTrue(TestDetourFunction(1) == 2);

			// Check Enable and Disable function

			TEST_ENABLE_MOCK(function1);
			Assert::IsTrue(TestDetourFunction(1) == 0x102);
			TEST_DISABLE_MOCK(function1);

			Assert::IsTrue(TestDetourFunction(1) == 2);
		}

		TEST_METHOD(DdkDetoursEnableMethod)
		{
			TestDetourClass test(10);

			Assert::IsTrue(test.Increment(1) == 2);

			// Check Enable and Disable method

			TEST_ENABLE_MOCK(method1);
			Assert::IsTrue(test.Increment(1) == 0x102);
			TEST_DISABLE_MOCK(method1);

			Assert::IsTrue(test.Increment(1) == 2);
		}

		TEST_METHOD(DdkDetoursDisable)
		{
			Assert::IsTrue(TestDetourFunction(1) == 2);

			// Disable after Enable

			TEST_ENABLE_MOCK(function1);
			Assert::IsTrue(TestDetourFunction(1) == 0x102);
			TEST_DISABLE_MOCK(function1);
			Assert::IsTrue(TestDetourFunction(1) == 2);

			// Disable due to out of scope
			{
				TEST_USING_MOCK(function1);
				Assert::IsTrue(TestDetourFunction(1) == 0x102);
			}

			Assert::IsTrue(TestDetourFunction(1) == 2);

			// Disable after Using
			{
				TEST_USING_MOCK(function1);
				Assert::IsTrue(TestDetourFunction(1) == 0x102);

				TEST_DISABLE_MOCK(function1);
				Assert::IsTrue(TestDetourFunction(1) == 2);
			}

			Assert::IsTrue(TestDetourFunction(1) == 2);

			// Detour function out of scope
			{
				TEST_DEFINE_MOCK_FUNCTION(function8, LONG, &TestDetourFunction, (LONG arg)) {
					return TEST_REAL_FUNCTION(arg) + 0x800;
				} TEST_END_MOCK;

				TEST_ENABLE_MOCK(function8);
				Assert::IsTrue(TestDetourFunction(1) == 0x802);
			}

			Assert::IsTrue(TestDetourFunction(1) == 2);
		}

		TEST_METHOD(DdkDetoursAttachRepeatedly)
		{
			// Multiple Attach and Detach sequences

			for (int i = 0; i < 100; i++) {
				Assert::IsTrue(TestDetourFunction(1) == 2);
				TEST_USING_MOCK(function1);
				Assert::IsTrue(TestDetourFunction(1) == 0x102);
			}
		}

		TEST_METHOD_ASYNC(DdkDetoursAsyncThread)
		{
			Assert::IsTrue(TestDetourFunction(1) == 2);

			// Use different mock on another thread
			{
				TEST_USING_MOCK(function2);
				Assert::IsTrue(TestDetourFunction(1) == 0x202);
			}

			Assert::IsTrue(TestDetourFunction(1) == 2);
		}

		TEST_METHOD(DdkDetoursAttachThread)
		{
			Assert::IsTrue(TestDetourFunction(1) == 2);

			TEST_ENABLE_MOCK(function1);
			Assert::IsTrue(TestDetourFunction(1) == 0x102);

			// Start thread to use different mock

			TEST_ASYNC_START(id, DdkDetoursAsyncThread);
			Assert::IsTrue(TestDetourFunction(1) == 0x102);
			TEST_ASYNC_WAIT(id);
			Assert::IsTrue(TestDetourFunction(1) == 0x102);

			TEST_DISABLE_MOCK(function1);
			Assert::IsTrue(TestDetourFunction(1) == 2);
		}

		TEST_METHOD_ASYNC(DdkDetoursAsyncMultiple)
		{
			static volatile LONG count = 0;

			TEST_DEFINE_MOCK_FUNCTION(function3, LONG, &TestDetourFunction, (LONG arg)) {
				return TEST_REAL_FUNCTION(arg) + 0x300;
			} TEST_END_MOCK;

			Assert::IsTrue(TestDetourFunction(1) == 2);

			// Alternate between mocks on different threads

			if (InterlockedIncrement(&count) & 1) {
				TEST_USING_MOCK(function2);
				Assert::IsTrue(TestDetourFunction(1) == 0x202);
			} else {
				TEST_USING_MOCK(function3);
				Assert::IsTrue(TestDetourFunction(1) == 0x302);
			}

			Assert::IsTrue(TestDetourFunction(1) == 2);
		}

		TEST_METHOD(DdkDetoursAttachMultiple)
		{
			Assert::IsTrue(TestDetourFunction(1) == 2);

			TEST_ENABLE_MOCK(function1);
			Assert::IsTrue(TestDetourFunction(1) == 0x102);

			// Start multiple threads using different mocks

			TEST_ASYNC_START_VEC(id, DdkDetoursAsyncMultiple, 100);
			Assert::IsTrue(TestDetourFunction(1) == 0x102);
			TEST_ASYNC_WAIT_VEC(id);
			Assert::IsTrue(TestDetourFunction(1) == 0x102);

			TEST_DISABLE_MOCK(function1);
			Assert::IsTrue(TestDetourFunction(1) == 2);
		}


		TEST_METHOD_ASYNC(DdkDetoursAsyncAllThreads)
		{
			TEST_DEFINE_MOCK_FUNCTION(function4, LONG, &TestDetourFunction2, (LONG arg)) {
				return TEST_REAL_FUNCTION(arg) + 0x400;
			} TEST_END_MOCK;

			Assert::IsTrue(TestDetourFunction2(1) == 0x503);

			// Attach mock for thread with global mock active
			{
				TEST_USING_MOCK(function4);
				Assert::IsTrue(TestDetourFunction2(1) == 0x403);
			}

			Assert::IsTrue(TestDetourFunction2(1) == 0x503);
		}

		TEST_METHOD(DdkDetoursAttachAllThreads)
		{
			TEST_DEFINE_MOCK_FUNCTION(function5, LONG, &TestDetourFunction2, (LONG arg)) {
				return TEST_REAL_FUNCTION(arg) + 0x500;
			} TEST_END_MOCK;

			Assert::IsTrue(TestDetourFunction2(1) == 3);

			// Attach global mock and start thread to use different mock
			{
				TEST_USING_MOCK(function5, TEST_ALL_THREADS);
				Assert::IsTrue(TestDetourFunction2(1) == 0x503);

				TEST_ASYNC_START(id, DdkDetoursAsyncAllThreads);
				Assert::IsTrue(TestDetourFunction2(1) == 0x503);
				TEST_ASYNC_WAIT(id);
				Assert::IsTrue(TestDetourFunction2(1) == 0x503);
			}

			Assert::IsTrue(TestDetourFunction2(1) == 3);
		}

		TEST_METHOD(DdkDetoursDetachAddressRange)
		{
			TEST_DEFINE_MOCK_FUNCTION(function, LONG, &TestDetourFunction4, (LONG arg)) {
				return TEST_REAL_FUNCTION(arg) + 0x100;
			} TEST_END_MOCK;

			TEST_FIND_FUNCTION(DetachAddressRange, NTSTATUS, "ddk!DdkDetachAddressRange",
				(PVOID pAddr, size_t len, HMODULE module));

			Assert::IsTrue(TestDetourFunction4(1) == 5);

			TEST_ENABLE_MOCK(function);
			Assert::IsTrue(TestDetourFunction4(1) == 0x105);

			// Detach function without mock

			NTSTATUS status = DetachAddressRange(&TestDetourFunction5, 1, NULL);
			Assert::IsTrue(NT_SUCCESS(status));
			Assert::IsTrue(TestDetourFunction4(1) == 0x105);

			// Detach function with mock

			status = DetachAddressRange(&TestDetourFunction4, 1, NULL);
			Assert::IsTrue(NT_SUCCESS(status));
			Assert::IsTrue(TestDetourFunction4(1) == 5);
		}

		static __declspec(noinline) LONG TestDetourFunction9(LONG arg0, LONG arg1, LONG arg2,
				LONG arg3, LONG arg4, LONG arg5, LONG arg6, LONG arg7, LONG arg8, LONG arg9) {
			return DoWork(), arg0 + arg1 + arg2 + arg3 + arg4 + arg5 + arg6 + arg7 + arg8 + arg9;
		}

		TEST_METHOD(DdkDetoursMultipleArgs)
		{
			TEST_DEFINE_MOCK_FUNCTION(function, LONG, &TestDetourFunction9,
					(LONG arg0, LONG arg1, LONG arg2, LONG arg3, LONG arg4,
					LONG arg5, LONG arg6, LONG arg7, LONG arg8, LONG arg9)) {
				return TEST_REAL_FUNCTION(arg0,arg1,arg2,arg3,arg4,
					arg5,arg6,arg7,arg8,arg9) + 0x500;
			} TEST_END_MOCK;

			Assert::AreEqual(55L, TestDetourFunction9(1,2,3,4,5,6,7,8,9,10));

			// Check arguments are preserved by mock
			{
				TEST_USING_MOCK(function);
				Assert::AreEqual(0x500 + 55L, TestDetourFunction9(1,2,3,4,5,6,7,8,9,10));
			}
		}
	};
}
