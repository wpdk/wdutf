/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2014, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	SList Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkSListTest)
	{
		SLIST_HEADER list;
		KSPIN_LOCK lock;
		LONG count;
		LONG asynccount;

		typedef struct _TEST : public SLIST_ENTRY {
			ULONG v;
			volatile LONG on;
		} TEST, *PTEST;

		static const int nvec = 100;
		struct _TEST vec[nvec];

	public:

		TEST_METHOD_INITIALIZE(DdkSListTestInit)
		{
			DdkThreadInit();
			KeInitializeSpinLock(&lock);
			ExInitializeSListHead(&list);
			Assert::IsTrue(ExQueryDepthSList(&list) == 0);

			for (int i = 0; i < nvec; i++) {
				vec[i].v = i;
				vec[i].on = 0;
			}

			for (int i = 0; i < nvec; i++) {
				Assert::IsTrue(InterlockedExchange(&vec[i].on, 1) == 0);
				ExInterlockedPushEntrySList(&list, &vec[i], &lock);
				Assert::IsTrue(ExQueryDepthSList(&list) == i+1);
			}

			count = asynccount = 0;
		}

		TEST_METHOD(DdkSListAdd)
		{
			for (int i = 0; i < nvec; i++)
				Assert::IsTrue(vec[i].on == 1);
		}

		TEST_METHOD(DdkSListRemove)
		{
			for (int i = nvec-1; i >= 0; i--) {
				TEST *p = (TEST *)ExInterlockedPopEntrySList(&list, &lock);
				Assert::IsTrue(p->v == i);
				Assert::IsTrue(ExQueryDepthSList(&list) == i);
				Assert::IsTrue(InterlockedExchange(&vec[i].on, 0) == 1);
			}

			for (int i = 0; i < nvec; i++)
				Assert::IsTrue(vec[i].on == 0);
		}

		TEST_METHOD(DdkSListFlush)
		{
			TEST *p = (TEST *)ExInterlockedFlushSList(&list);
			Assert::IsTrue(p == &vec[nvec - 1]);
			Assert::IsTrue(ExQueryDepthSList(&list) == 0);
		}

		TEST_METHOD_ASYNC(DdkSListAsync)
		{
			for (int i = 0, last = 0; i < 10000; i++) {
				TEST *p = (TEST *)ExInterlockedPopEntrySList(&list, &lock);
				Assert::IsTrue(InterlockedExchange(&p->on, 0) == 1);
				if (TEST_IS_ASYNC) asynccount++;
				else count++;
				Assert::IsTrue(InterlockedExchange(&p->on, 1) == 0);
				ExInterlockedPushEntrySList(&list, p, &lock);
			}
		}

		TEST_METHOD(DdkMutexMulti)
		{
			TEST_ASYNC_START(id, DdkSListAsync);
			DdkSListAsync();
			TEST_ASYNC_WAIT(id);
			Assert::IsTrue(count && asynccount);
		}
	};
}
