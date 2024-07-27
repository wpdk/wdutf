/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 *  Copyright (c) 1998-2017, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Error Routines
 */

#include "stdddk.h"


#define BUGCHECK_CHECKSUM(cb) \
	((ULONG_PTR)cb ^ (ULONG_PTR)cb->CallbackRoutine & (ULONG_PTR)cb->Component)


char *BugCheckString(ULONG code);
void BugCheckCallbacks(ULONG code, ULONG_PTR param1,
	ULONG_PTR param2, ULONG_PTR param3, ULONG_PTR param4);


static KSPIN_LOCK Lock;
static LIST_ENTRY List, ReasonList;


void DdkInitError()
{
	KeInitializeSpinLock(&Lock);
	InitializeListHead(&List);
	InitializeListHead(&ReasonList);
}


DDKAPI
VOID KeBugCheck(ULONG BugCheckCode)
{
	KeBugCheckEx(BugCheckCode, 0, 0, 0, 0);
}


DDKAPI
VOID KeBugCheckEx(ULONG BugCheckCode, ULONG_PTR BugCheckParameter1,
	ULONG_PTR BugCheckParameter2, ULONG_PTR BugCheckParameter3,
	ULONG_PTR BugCheckParameter4)
{
	WCHAR s[1024];

	BugCheckCallbacks(BugCheckCode, BugCheckParameter1,
		BugCheckParameter2, BugCheckParameter3, BugCheckParameter4);

	swprintf(s, sizeof(s) / sizeof(WCHAR),
		L"BugCheck Code 0x%x (%S), 0x%I64x, 0x%I64x, 0x%I64x, 0x%I64x",
		BugCheckCode, BugCheckString(BugCheckCode), BugCheckParameter1,
		BugCheckParameter2, BugCheckParameter3, BugCheckParameter4);

	DdkFail(s);
}


static
void BugCheckCallbacks(ULONG code, ULONG_PTR param1,
	ULONG_PTR param2, ULONG_PTR param3, ULONG_PTR param4)
{
	KIRQL irql;
	KeRaiseIrql(HIGH_LEVEL, &irql);
	KeAcquireSpinLockAtDpcLevel(&Lock);

	for (LIST_ENTRY *ep = ReasonList.Flink; ep != &ReasonList; ep = ep->Flink) {
		PKBUGCHECK_REASON_CALLBACK_RECORD rp =
			CONTAINING_RECORD(ep, KBUGCHECK_REASON_CALLBACK_RECORD, Entry);

		if (rp->Checksum != BUGCHECK_CHECKSUM(rp)) continue;

		switch (rp->Reason) {
		case KbCallbackAddPages:
		{
			KBUGCHECK_ADD_PAGES data = { 0 };
			do {
				data.Flags = 0;
				data.BugCheckCode = code;
				rp->CallbackRoutine(rp->Reason, rp, &data, sizeof(data));
			} while (data.Flags & KB_ADD_PAGES_FLAG_ADDITIONAL_RANGES_EXIST);
			break;
		}

		case KbCallbackRemovePages:
		{
			KBUGCHECK_REMOVE_PAGES data = { 0 };
			do {
				data.Flags = 0;
				data.BugCheckCode = code;
				rp->CallbackRoutine(rp->Reason, rp, &data, sizeof(data));
			} while (data.Flags & KB_REMOVE_PAGES_FLAG_ADDITIONAL_RANGES_EXIST);
			break;
		}

		case KbCallbackTriageDumpData:
		{
			KBUGCHECK_TRIAGE_DUMP_DATA data = { 0 };

			data.Flags = KB_TRIAGE_DUMP_DATA_FLAG_BUGCHECK_ACTIVE;
			data.BugCheckCode = code;
			data.BugCheckParameter1 = param1;
			data.BugCheckParameter2 = param2;
			data.BugCheckParameter3 = param3;
			data.BugCheckParameter4 = param4;
			data.MaxVirtMemSize = 1024 * 1024 * 1024I64;

			rp->CallbackRoutine(rp->Reason, rp, &data, sizeof(data));
			break;
		}

		case KbCallbackSecondaryDumpData:
		case KbCallbackSecondaryMultiPartDumpData:
		case KbCallbackDumpIo:
			break;
		}
	}

	for (LIST_ENTRY *ep = List.Flink; ep != &List; ep = ep->Flink) {
		PKBUGCHECK_CALLBACK_RECORD rp =
			CONTAINING_RECORD(ep, KBUGCHECK_CALLBACK_RECORD, Entry);

		if (rp->Checksum == BUGCHECK_CHECKSUM(rp))
			rp->CallbackRoutine(rp->Buffer, rp->Length);
	}

	KeReleaseSpinLockFromDpcLevel(&Lock);
	KeLowerIrql(irql);
}


DDKAPI
BOOLEAN KeRegisterBugCheckCallback(
	PKBUGCHECK_CALLBACK_RECORD CallbackRecord,
	PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine,
	PVOID Buffer, ULONG Length, PUCHAR Component)
{
	BOOLEAN rc = FALSE;
	KIRQL irql;

	if (CallbackRecord == NULL)
		return FALSE;

	KeRaiseIrql(HIGH_LEVEL, &irql);
	KeAcquireSpinLockAtDpcLevel(&Lock);

	if (CallbackRecord->State == BufferEmpty) {
		CallbackRecord->CallbackRoutine = CallbackRoutine;
		CallbackRecord->Buffer = Buffer;
		CallbackRecord->Length = Length;
		CallbackRecord->Component = Component;
		CallbackRecord->State = BufferInserted;
		CallbackRecord->Checksum = BUGCHECK_CHECKSUM(CallbackRecord);
		InsertTailList(&List, &CallbackRecord->Entry);
		rc = TRUE;
	}

	KeReleaseSpinLockFromDpcLevel(&Lock);
	KeLowerIrql(irql);
	return rc;
}


DDKAPI
BOOLEAN KeDeregisterBugCheckCallback(
	PKBUGCHECK_CALLBACK_RECORD CallbackRecord)
{
	BOOLEAN rc = FALSE;
	KIRQL irql;

	if (CallbackRecord == NULL)
		return FALSE;

	KeRaiseIrql(HIGH_LEVEL, &irql);
	KeAcquireSpinLockAtDpcLevel(&Lock);

	if (CallbackRecord->State == BufferInserted) {
		RemoveEntryList(&CallbackRecord->Entry);
		CallbackRecord->State = BufferEmpty;
		rc = TRUE;
	}

	KeReleaseSpinLockFromDpcLevel(&Lock);
	KeLowerIrql(irql);
	return rc;
}


DDKAPI
BOOLEAN KeRegisterBugCheckReasonCallback(
	PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord,
	PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine,
	KBUGCHECK_CALLBACK_REASON Reason,
	PUCHAR Component)
{
	BOOLEAN rc = FALSE;
	KIRQL irql;

	if (CallbackRecord == NULL)
		return FALSE;

	KeRaiseIrql(HIGH_LEVEL, &irql);
	KeAcquireSpinLockAtDpcLevel(&Lock);

	if (CallbackRecord->State == BufferEmpty) {
		CallbackRecord->CallbackRoutine = CallbackRoutine;
		CallbackRecord->Component = Component;
		CallbackRecord->Reason = Reason;
		CallbackRecord->State = BufferInserted;
		CallbackRecord->Checksum = BUGCHECK_CHECKSUM(CallbackRecord);
		InsertTailList(&ReasonList, &CallbackRecord->Entry);
		rc = TRUE;
	}

	KeReleaseSpinLockFromDpcLevel(&Lock);
	KeLowerIrql(irql);
	return rc;
}


DDKAPI
BOOLEAN KeDeregisterBugCheckReasonCallback(
	PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord)
{
	BOOLEAN rc = FALSE;
	KIRQL irql;

	if (CallbackRecord == NULL)
		return FALSE;

	KeRaiseIrql(HIGH_LEVEL, &irql);
	KeAcquireSpinLockAtDpcLevel(&Lock);

	if (CallbackRecord->State == BufferInserted) {
		RemoveEntryList(&CallbackRecord->Entry);
		CallbackRecord->State = BufferEmpty;
		rc = TRUE;
	}

	KeReleaseSpinLockFromDpcLevel(&Lock);
	KeLowerIrql(irql);
	return rc;
}


DDKAPI
BOOLEAN IoRaiseInformationalHardError(NTSTATUS ErrorStatus,
	PUNICODE_STRING String, PKTHREAD Thread)
{
	UNREFERENCED_PARAMETER(String);
	UNREFERENCED_PARAMETER(Thread);

	fprintf(stderr, "HardError %x\n", ErrorStatus);
	return TRUE;
}


_Analysis_noreturn_
void DdkFail(wchar_t *s)
{
	ULONG_PTR v[2];

	fprintf(stderr, "%S\n", s);

	v[0] = (ULONG_PTR)s;
	v[1] = NULL;
	RaiseException(EXCEPTION_UNITTEST_ASSERTION, EXCEPTION_NONCONTINUABLE, 2, v);
	exit(0);
}


static char *BugCheckString(ULONG code)
{
	switch (code) {
	case STATUS_SEMAPHORE_LIMIT_EXCEEDED:
		return "STATUS_SEMAPHORE_LIMIT_EXCEEDED";
	case MAXIMUM_WAIT_OBJECTS_EXCEEDED:
		return "MAXIMUM_WAIT_OBJECTS_EXCEEDED";
	case MULTIPLE_IRP_COMPLETE_REQUESTS:
		return "MULTIPLE_IRP_COMPLETE_REQUESTS";
	case NO_MORE_IRP_STACK_LOCATIONS:
		return "NO_MORE_IRP_STACK_LOCATIONS";
	}

	return "UNKNOWN";
}
