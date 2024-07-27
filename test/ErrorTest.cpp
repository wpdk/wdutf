/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

 /*
  *	Error Tests
  */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkErrorTest)
	{
		KBUGCHECK_CALLBACK_RECORD record;
		KBUGCHECK_CALLBACK_RECORD record2;
		ULONG v;

		struct reason {
			KBUGCHECK_REASON_CALLBACK_RECORD record;
			ULONG v;
		} reason;

		struct reason reason2;

	public:
		TEST_METHOD_INITIALIZE(DdkErrorTestInit)
		{
			DdkThreadInit();

			KeInitializeCallbackRecord(&record);
			v = 0;

			KeInitializeCallbackRecord(&reason.record);
			reason.v = 0;

			KeInitializeCallbackRecord(&reason2.record);
			reason2.v = 0;
		}

		TEST_METHOD_CLEANUP(DdkErrorTestCleanup)
		{
			KeDeregisterBugCheckCallback(&record);
			KeDeregisterBugCheckCallback(&record2);
			KeDeregisterBugCheckReasonCallback(&reason.record);
			KeDeregisterBugCheckReasonCallback(&reason2.record);
		}

		/*
		 *	Check KeBugCheck
		 */
		TEST_METHOD(DdkErrorBugCheck)
		{
			TEST_EXPECT_ASSERT(KeBugCheck(NO_MORE_IRP_STACK_LOCATIONS));
		}

		/*
		 *	Check KeBugCheckEx
		 */
		TEST_METHOD(DdkErrorBugCheckEx)
		{
			TEST_EXPECT_ASSERT(
				KeBugCheckEx(NO_MORE_IRP_STACK_LOCATIONS, 1, 2, 3, 4));
		}

		/*
		 *	Check BugCheck Register
		 */
		static VOID BugCheckCallback(PVOID Buffer, ULONG Length)
		{
			Assert::AreEqual(sizeof(ULONG), (size_t)Length);
			*(ULONG *)Buffer |= 0x4aUL;
		}

		TEST_METHOD(DdkErrorRegister)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckCallback(&record,
				BugCheckCallback, &v, sizeof(v), (PUCHAR)"test"));

			TEST_EXPECT_ASSERT(KeBugCheck(NO_MORE_IRP_STACK_LOCATIONS));
			Assert::AreEqual(0x4aUL, v);

			Assert::AreEqual((BOOLEAN)TRUE, KeDeregisterBugCheckCallback(&record));
		}

		/*
		 *	Check BugCheck Re-register
		 */
		TEST_METHOD(DdkErrorReregister)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckCallback(&record,
				BugCheckCallback, &v, sizeof(v), (PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)FALSE, KeRegisterBugCheckCallback(&record,
				BugCheckCallback, &v, sizeof(v), (PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)TRUE, KeDeregisterBugCheckCallback(&record));
		}

		/*
		 *	Check BugCheck Register Multiple
		 */
		static VOID BugCheckCallback2(PVOID Buffer, ULONG Length)
		{
			Assert::AreEqual(sizeof(ULONG), (size_t)Length);
			*(ULONG *)Buffer |= 0x81UL;
		}

		TEST_METHOD(DdkErrorRegisterMultiple)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckCallback(&record,
				BugCheckCallback, &v, sizeof(v), (PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckCallback(&record2,
				BugCheckCallback2, &v, sizeof(v), (PUCHAR)"test2"));

			TEST_EXPECT_ASSERT(KeBugCheck(NO_MORE_IRP_STACK_LOCATIONS));
			Assert::AreEqual(0x4aUL | 0x81UL, v);

			Assert::AreEqual((BOOLEAN)TRUE, KeDeregisterBugCheckCallback(&record));
			Assert::AreEqual((BOOLEAN)TRUE, KeDeregisterBugCheckCallback(&record2));
		}

		/*
		 *	Check BugCheck Deregister
		 */
		TEST_METHOD(DdkErrorDeregister)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckCallback(&record,
				BugCheckCallback, &v, sizeof(v), (PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)TRUE, KeDeregisterBugCheckCallback(&record));
			Assert::AreEqual((BOOLEAN)FALSE, KeDeregisterBugCheckCallback(&record));
		}

		/*
		 *	Check BugCheck Deregister Multiple
		 */
		TEST_METHOD(DdkErrorDeregisterMultiple)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckCallback(&record,
				BugCheckCallback, &v, sizeof(v), (PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckCallback(&record2,
				BugCheckCallback2, &v, sizeof(v), (PUCHAR)"test2"));

			Assert::AreEqual((BOOLEAN)TRUE, KeDeregisterBugCheckCallback(&record));

			TEST_EXPECT_ASSERT(KeBugCheck(NO_MORE_IRP_STACK_LOCATIONS));
			Assert::AreEqual(0x81UL, v);

			Assert::AreEqual((BOOLEAN)FALSE, KeDeregisterBugCheckCallback(&record));
			Assert::AreEqual((BOOLEAN)TRUE, KeDeregisterBugCheckCallback(&record2));
		}

		/*
		 *	Check BugCheck Reason Register
		 */
		static void BugCheckReasonCallback(KBUGCHECK_CALLBACK_REASON Reason,
			PKBUGCHECK_REASON_CALLBACK_RECORD Record, PVOID ReasonSpecificData,
			ULONG ReasonSpecificDataLength)
		{
			struct reason *rp = CONTAINING_RECORD(Record, struct reason, record);
			rp->v = 0x4aUL + Reason;
		}

		TEST_METHOD(DdkErrorReasonRegister)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckReasonCallback(
				&reason.record, BugCheckReasonCallback, KbCallbackAddPages,
				(PUCHAR)"test"));

			TEST_EXPECT_ASSERT(KeBugCheck(NO_MORE_IRP_STACK_LOCATIONS));
			Assert::AreEqual(0x4aUL + KbCallbackAddPages, reason.v);

			Assert::AreEqual((BOOLEAN)TRUE,
				KeDeregisterBugCheckReasonCallback(&reason.record));
		}

		/*
		 *	Check BugCheck Reason Reregister
		 */
		TEST_METHOD(DdkErrorReasonReregister)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckReasonCallback(
				&reason.record, BugCheckReasonCallback, KbCallbackAddPages,
				(PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)FALSE, KeRegisterBugCheckReasonCallback(
				&reason.record, BugCheckReasonCallback, KbCallbackAddPages,
				(PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)TRUE,
				KeDeregisterBugCheckReasonCallback(&reason.record));
		}

		/*
		 *	Check BugCheck Reason Deregister
		 */
		TEST_METHOD(DdkErrorReasonDeregister)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckReasonCallback(
				&reason.record, BugCheckReasonCallback, KbCallbackAddPages,
				(PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)TRUE,
				KeDeregisterBugCheckReasonCallback(&reason.record));

			Assert::AreEqual((BOOLEAN)FALSE,
				KeDeregisterBugCheckReasonCallback(&reason.record));
		}

		/*
		 *	Check BugCheck AddPages
		 */
		static void BugCheckAddPages(KBUGCHECK_CALLBACK_REASON Reason,
			PKBUGCHECK_REASON_CALLBACK_RECORD Record, PVOID ReasonSpecificData,
			ULONG ReasonSpecificDataLength)
		{
			struct reason *rp = CONTAINING_RECORD(Record, struct reason, record);
			PKBUGCHECK_ADD_PAGES p = (PKBUGCHECK_ADD_PAGES)ReasonSpecificData;

			if (sizeof(*p) != ReasonSpecificDataLength) return;

			if (p->Context == 0) {
				rp->v = 0x4aUL + Reason + p->BugCheckCode;
				p->Flags |= KB_ADD_PAGES_FLAG_ADDITIONAL_RANGES_EXIST;
				p->Context = rp;
				return;
			}

			rp->v++;
		}

		TEST_METHOD(DdkErrorAddPages)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckReasonCallback(
				&reason.record, BugCheckAddPages, KbCallbackAddPages,
				(PUCHAR)"test"));

			TEST_EXPECT_ASSERT(KeBugCheck(NO_MORE_IRP_STACK_LOCATIONS));

			Assert::AreEqual(
				0x4aUL + KbCallbackAddPages + NO_MORE_IRP_STACK_LOCATIONS + 1,
				reason.v);

			Assert::AreEqual((BOOLEAN)TRUE,
				KeDeregisterBugCheckReasonCallback(&reason.record));
		}

		/*
		 *	Check BugCheck RemovePages
		 */
		static void BugCheckRemovePages(KBUGCHECK_CALLBACK_REASON Reason,
			PKBUGCHECK_REASON_CALLBACK_RECORD Record, PVOID ReasonSpecificData,
			ULONG ReasonSpecificDataLength)
		{
			struct reason *rp = CONTAINING_RECORD(Record, struct reason, record);
			PKBUGCHECK_REMOVE_PAGES p = (PKBUGCHECK_REMOVE_PAGES)ReasonSpecificData;

			if (sizeof(*p) != ReasonSpecificDataLength) return;

			if (p->Context == 0) {
				rp->v = 0x7aUL + Reason + p->BugCheckCode;
				p->Flags |= KB_REMOVE_PAGES_FLAG_ADDITIONAL_RANGES_EXIST;
				p->Context = rp;
				return;
			}

			rp->v++;
		}

		TEST_METHOD(DdkErrorRemovePages)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckReasonCallback(
				&reason.record, BugCheckRemovePages, KbCallbackRemovePages,
				(PUCHAR)"test"));

			TEST_EXPECT_ASSERT(KeBugCheck(NO_MORE_IRP_STACK_LOCATIONS));

			Assert::AreEqual(
				0x7aUL + KbCallbackRemovePages + NO_MORE_IRP_STACK_LOCATIONS + 1,
				reason.v);

			Assert::AreEqual((BOOLEAN)TRUE,
				KeDeregisterBugCheckReasonCallback(&reason.record));
		}

		/*
		 *	Check BugCheck TriageDumpData
		 */
		static void BugCheckTriage(KBUGCHECK_CALLBACK_REASON Reason,
			PKBUGCHECK_REASON_CALLBACK_RECORD Record, PVOID ReasonSpecificData,
			ULONG ReasonSpecificDataLength)
		{
			struct reason *rp = CONTAINING_RECORD(Record, struct reason, record);
			PKBUGCHECK_TRIAGE_DUMP_DATA p = (PKBUGCHECK_TRIAGE_DUMP_DATA)ReasonSpecificData;

			if (sizeof(*p) != ReasonSpecificDataLength) return;

			rp->v = 0x5aUL + Reason + p->BugCheckCode +
				(ULONG)p->BugCheckParameter1 + (ULONG)p->BugCheckParameter2 +
				(ULONG)p->BugCheckParameter3 + (ULONG)p->BugCheckParameter4;
		}

		TEST_METHOD(DdkErrorTriageDumpData)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckReasonCallback(
				&reason.record, BugCheckTriage, KbCallbackTriageDumpData,
				(PUCHAR)"test"));

			TEST_EXPECT_ASSERT(
				KeBugCheckEx(NO_MORE_IRP_STACK_LOCATIONS, 1, 2, 3, 4));

			Assert::AreEqual(
				0x5aUL + KbCallbackTriageDumpData +
				NO_MORE_IRP_STACK_LOCATIONS + 1 + 2 + 3 + 4, reason.v);

			Assert::AreEqual((BOOLEAN)TRUE,
				KeDeregisterBugCheckReasonCallback(&reason.record));
		}

		/*
		 *	Check BugCheck Reason Register Multiple
		 */
		TEST_METHOD(DdkErrorReasonRegisterMultiple)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckReasonCallback(
				&reason.record, BugCheckAddPages, KbCallbackAddPages,
				(PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckReasonCallback(
				&reason2.record, BugCheckRemovePages, KbCallbackRemovePages,
				(PUCHAR)"test"));

			TEST_EXPECT_ASSERT(KeBugCheck(NO_MORE_IRP_STACK_LOCATIONS));

			Assert::AreEqual(
				0x4aUL + KbCallbackAddPages + NO_MORE_IRP_STACK_LOCATIONS + 1,
				reason.v);

			Assert::AreEqual(
				0x7aUL + KbCallbackRemovePages + NO_MORE_IRP_STACK_LOCATIONS + 1,
				reason2.v);

			Assert::AreEqual((BOOLEAN)TRUE,
				KeDeregisterBugCheckReasonCallback(&reason.record));

			Assert::AreEqual((BOOLEAN)TRUE,
				KeDeregisterBugCheckReasonCallback(&reason2.record));
		}

		/*
		 *	Check BugCheck Reason Deregister Multiple
		 */
		TEST_METHOD(DdkErrorReasonDeregisterMultiple)
		{
			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckReasonCallback(
				&reason.record, BugCheckAddPages, KbCallbackAddPages,
				(PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)TRUE, KeRegisterBugCheckReasonCallback(
				&reason2.record, BugCheckRemovePages, KbCallbackRemovePages,
				(PUCHAR)"test"));

			Assert::AreEqual((BOOLEAN)TRUE,
				KeDeregisterBugCheckReasonCallback(&reason.record));

			TEST_EXPECT_ASSERT(KeBugCheck(NO_MORE_IRP_STACK_LOCATIONS));

			Assert::AreEqual(0UL, reason.v);

			Assert::AreEqual(
				0x7aUL + KbCallbackRemovePages + NO_MORE_IRP_STACK_LOCATIONS + 1,
				reason2.v);

			Assert::AreEqual((BOOLEAN)FALSE,
				KeDeregisterBugCheckReasonCallback(&reason.record));

			Assert::AreEqual((BOOLEAN)TRUE,
				KeDeregisterBugCheckReasonCallback(&reason2.record));
		}
	};
}
