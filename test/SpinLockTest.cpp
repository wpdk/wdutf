/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2013-2016, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DdkUnitTest
{
	TEST_CLASS(DdkSpinLockTest)
	{
		KSPIN_LOCK lock;
		EX_SPIN_LOCK xlock;
		ULONG value;
		LONG count;
		LONG asynccount;
		LONG tryfail;
		LONG tryok;

	public:
		TEST_METHOD_INITIALIZE(DdkSpinLockTestInit)
		{
			DdkThreadInit();
			value = 0;
			count = asynccount = tryfail = tryok = 0;
			KeInitializeSpinLock(&lock);
			xlock = 0;
			KeLowerIrql(PASSIVE_LEVEL);
		}

		TEST_METHOD_CLEANUP(DdkSpinLockTestCleanup)
		{
			KeLowerIrql(PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkSpinLockAcquire)
		{
			KIRQL irql;
			KeAcquireSpinLock(&lock, &irql);
			Assert::IsTrue(irql == PASSIVE_LEVEL);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);
		}

		TEST_METHOD(DdkSpinLockTryAcquire)
		{
			KIRQL irql;
			KeRaiseIrql(DISPATCH_LEVEL, &irql);
			Assert::IsTrue(KeTryToAcquireSpinLockAtDpcLevel(&lock) != 0);
		}

		TEST_METHOD(DdkSpinLockTryAcquireBusy)
		{
			KIRQL irql;
			KeAcquireSpinLock(&lock, &irql);
			Assert::IsTrue(KeTryToAcquireSpinLockAtDpcLevel(&lock) == 0);
		}

		TEST_METHOD(DdkSpinLockRelease)
		{
			KIRQL irql;
			KeAcquireSpinLock(&lock, &irql);
			KeReleaseSpinLock(&lock, irql);
			Assert::IsTrue(KeGetCurrentIrql() == PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkSpinLockAcquireAtDpc)
		{
			KIRQL irql;
			KeRaiseIrql(HIGH_LEVEL, &irql);
			KeAcquireSpinLockAtDpcLevel(&lock);
			Assert::IsTrue(KeGetCurrentIrql() == HIGH_LEVEL);

			KeReleaseSpinLockFromDpcLevel(&lock);
			Assert::IsTrue(KeGetCurrentIrql() == HIGH_LEVEL);
			KeLowerIrql(irql);
		}

		TEST_METHOD(DdkSpinLockAcquireForDpc)
		{
			KIRQL irql = KeAcquireSpinLockForDpc(&lock);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);

			KeReleaseSpinLockForDpc(&lock, irql);
			Assert::IsTrue(KeGetCurrentIrql() == PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkSpinLockRaiseToDpc)
		{
			KIRQL irql = KeAcquireSpinLockRaiseToDpc(&lock);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);

			KeReleaseSpinLock(&lock, irql);
			Assert::IsTrue(KeGetCurrentIrql() == PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkSpinLockAcquireQueued)
		{
			KLOCK_QUEUE_HANDLE h;
			KeAcquireInStackQueuedSpinLock(&lock, &h);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);
			KeReleaseInStackQueuedSpinLock(&h);
			Assert::IsTrue(KeGetCurrentIrql() == PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkSpinLockAcquireQueuedAtDpc)
		{
			KLOCK_QUEUE_HANDLE h;
			KIRQL irql;
			KeRaiseIrql(DISPATCH_LEVEL, &irql);
			KeAcquireInStackQueuedSpinLockAtDpcLevel(&lock, &h);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);
			KeReleaseInStackQueuedSpinLockFromDpcLevel(&h);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);
			KeLowerIrql(irql);
		}

		TEST_METHOD_ASYNC(DdkSpinLockAsync)
		{
			for (int v, i = 0, last = 0; i < 8000000; i++) {
				KIRQL irql;

				if (i > 1000000 && count && asynccount && tryfail && tryok)
					break;

				if ((i % 10) != 0) KeAcquireSpinLock(&lock, &irql);
				else {
					KeRaiseIrql(DISPATCH_LEVEL, &irql);
					if (!KeTryToAcquireSpinLockAtDpcLevel(&lock)) {
						InterlockedIncrement(&tryfail);
						KeLowerIrql(irql);
						continue;
					}
					InterlockedIncrement(&tryok);
				}

				if (TEST_IS_ASYNC) {
					v = (value & 0xffff);
					value = (value & 0xffff0000) + (i & 0xffff);
					asynccount++;
				} else {
					v = (value >> 16) & 0xffff;
					value = (value & 0xffff) + (i << 16);
					count++;
				}

				KeReleaseSpinLock(&lock, irql);
				Assert::IsTrue(v == last);
				last = (i & 0xffff);
			}
		}

		TEST_METHOD(DdkSpinLockMulti)
		{
			TEST_ASYNC_START(id, DdkSpinLockAsync);
			DdkSpinLockAsync();
			TEST_ASYNC_WAIT(id);
			Assert::IsTrue(count && asynccount && tryfail && tryok);
		}

		TEST_METHOD(DdkSpinLockSharedAcquire)
		{
			KIRQL irql = ExAcquireSpinLockShared(&xlock);
			Assert::IsTrue(irql == PASSIVE_LEVEL);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);
		}

		TEST_METHOD(DdkSpinLockSharedRelease)
		{
			KIRQL irql = ExAcquireSpinLockShared(&xlock);
			ExReleaseSpinLockShared(&xlock, irql);
			Assert::IsTrue(KeGetCurrentIrql() == PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkSpinLockSharedAcquireAtDpc)
		{
			KIRQL irql;
			KeRaiseIrql(HIGH_LEVEL, &irql);
			ExAcquireSpinLockSharedAtDpcLevel(&xlock);
			Assert::IsTrue(KeGetCurrentIrql() == HIGH_LEVEL);

			ExReleaseSpinLockSharedFromDpcLevel(&xlock);
			Assert::IsTrue(KeGetCurrentIrql() == HIGH_LEVEL);
			KeLowerIrql(irql);
		}

		TEST_METHOD(DdkSpinLockExclusiveAcquire)
		{
			KIRQL irql = ExAcquireSpinLockExclusive(&xlock);
			Assert::IsTrue(irql == PASSIVE_LEVEL);
			Assert::IsTrue(KeGetCurrentIrql() == DISPATCH_LEVEL);
		}

		TEST_METHOD(DdkSpinLockExclusiveRelease)
		{
			KIRQL irql = ExAcquireSpinLockExclusive(&xlock);
			ExReleaseSpinLockExclusive(&xlock, irql);
			Assert::IsTrue(KeGetCurrentIrql() == PASSIVE_LEVEL);
		}

		TEST_METHOD(DdkSpinLockExclusiveAcquireAtDpc)
		{
			KIRQL irql;
			KeRaiseIrql(HIGH_LEVEL, &irql);
			ExAcquireSpinLockExclusiveAtDpcLevel(&xlock);
			Assert::IsTrue(KeGetCurrentIrql() == HIGH_LEVEL);

			ExReleaseSpinLockExclusiveFromDpcLevel(&xlock);
			Assert::IsTrue(KeGetCurrentIrql() == HIGH_LEVEL);
			KeLowerIrql(irql);
		}

		TEST_METHOD_ASYNC(DdkSpinLockSharedAsync)
		{
			for (int v, i = 0, last = 0; i < 1000000; i++) {
				bool ex = ((i % 10) == 0);
				KIRQL irql = (ex) ? ExAcquireSpinLockExclusive(&xlock)
								  : ExAcquireSpinLockShared(&xlock);

				if (TEST_IS_ASYNC) {
					v = (value & 0xffff);
					if (ex) value = (value & 0xffff0000) + (i & 0xffff);
					if (ex) asynccount++;
				} else {
					v = (value >> 16) & 0xffff;
					if (ex) value = (value & 0xffff) + (i << 16);
					if (ex) count++;
				}

				if (ex) ExReleaseSpinLockExclusive(&xlock, irql);
				else ExReleaseSpinLockShared(&xlock, irql);

				Assert::IsTrue(v == last);
				if (ex) last = (i & 0xffff);
			}
		}

		TEST_METHOD(DdkSpinLockSharedMulti)
		{
			TEST_ASYNC_START(id, DdkSpinLockSharedAsync);
			DdkSpinLockSharedAsync();
			TEST_ASYNC_WAIT(id);
			Assert::IsTrue(count && asynccount);
		}
	};
}
