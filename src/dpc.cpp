/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2014, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	DPC Routines
 */

#include "stdafx.h"


typedef struct _DPC {
	PTP_WORK Work;
	PCRITICAL_SECTION Lock;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PVOID DeferredContext;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
	volatile LONG queued;
	volatile LONG cancel;
    UCHAR Importance;
    bool threaded;
} DPC, *PDPC;


__declspec(thread) bool DdkDpcActive = false;
static volatile LONGLONG DpcsQueued, DpcsCompleted;


static VOID DdkDpcCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	DPC *pDpc = (DPC *)Context;
	KIRQL irql = (pDpc->threaded ? PASSIVE_LEVEL : DISPATCH_LEVEL);
	KIRQL oldirql;

	DdkThreadInit();
	DdkDpcActive = true;
	InterlockedExchange(&pDpc->queued, 0);
	DisassociateCurrentThreadFromCallback(Instance);
	KeLowerIrql(PASSIVE_LEVEL);
	KeRaiseIrql(irql, &oldirql);

	(*pDpc->DeferredRoutine)((PRKDPC)pDpc,
		pDpc->DeferredContext, pDpc->SystemArgument1, pDpc->SystemArgument2);

	DDKASSERT(KeGetCurrentIrql() == irql);

	InterlockedIncrement64(&DpcsCompleted);
	KeLowerIrql(PASSIVE_LEVEL);
	DdkDpcActive = false;
}


DDKAPI
VOID KeInitializeDpc(PRKDPC Dpc, PKDEFERRED_ROUTINE DeferredRoutine, PVOID DeferredContext)
{
	DPC *pDpc = (DPC *)Dpc;

	if (sizeof(DPC) > _SizeofDpc_)
		ddkfail("DPC is too small to initialize");

	memset(Dpc, 0, _SizeofDpc_);

	DdkThreadInit();
	pDpc->Lock = (PCRITICAL_SECTION)calloc(1, sizeof(CRITICAL_SECTION));
	pDpc->Work = CreateThreadpoolWork(DdkDpcCallback, (PVOID)pDpc, NULL);

	if (!pDpc->Work || !pDpc->Lock)
		ddkfail("Unable to initialize DPC");

	if (!InitializeCriticalSectionAndSpinCount(pDpc->Lock, 4000))
		ddkfail("KeInitializeDpc failed to initialize spinlock");

	pDpc->DeferredRoutine = DeferredRoutine;
	pDpc->DeferredContext = DeferredContext;
	pDpc->Importance = MediumImportance;
}


DDKAPI
VOID KeInitializeThreadedDpc(PRKDPC Dpc, PKDEFERRED_ROUTINE DeferredRoutine, PVOID DeferredContext)
{
	KeInitializeDpc(Dpc, DeferredRoutine, DeferredContext);
	((PDPC)Dpc)->threaded = true;
}


DDKAPI
BOOLEAN KeInsertQueueDpc(PRKDPC Dpc, PVOID SystemArgument1, PVOID SystemArgument2)
{
	DPC *pDpc = (DPC *)Dpc;

	EnterCriticalSection(pDpc->Lock);

	if (pDpc->queued || pDpc->cancel) {
		LeaveCriticalSection(pDpc->Lock);
		return FALSE;
	}

	pDpc->SystemArgument1 = SystemArgument1;
	pDpc->SystemArgument2 = SystemArgument2;
	pDpc->queued = 1;

	InterlockedIncrement64(&DpcsQueued);
	SubmitThreadpoolWork(pDpc->Work);
	LeaveCriticalSection(pDpc->Lock);
	return TRUE;
}


DDKAPI
BOOLEAN KeRemoveQueueDpc(PRKDPC Dpc)
{
	DPC *pDpc = (DPC *)Dpc;

	EnterCriticalSection(pDpc->Lock);

	if (!pDpc->queued || pDpc->cancel) {
		LeaveCriticalSection(pDpc->Lock);
		return FALSE;
	}

	pDpc->cancel = 1;
	LeaveCriticalSection(pDpc->Lock);
	WaitForThreadpoolWorkCallbacks(pDpc->Work, TRUE);

	BOOLEAN rc = (InterlockedExchange(&pDpc->queued, 0) != 0);
	if (rc) InterlockedIncrement64(&DpcsCompleted);
	InterlockedExchange(&pDpc->cancel, 0);
	return rc;
}


DDKAPI
VOID KeFlushQueuedDpcs()
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	LONGLONG queued = DpcsQueued;
	while (DpcsCompleted < queued) Sleep(10);
}


BOOLEAN DdkIsDpc()
{
	return (DdkDpcActive != false);
}
