/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Timer Routines
 */

#include "stdafx.h"


DDKAPI BOOLEAN KeInsertQueueDpc(PRKDPC Dpc, PVOID SystemArgument1, PVOID SystemArgument2);
DDKAPI BOOLEAN KeRemoveQueueDpc(PRKDPC Dpc);


typedef struct _TIMER : public OBJECT {
	PTP_TIMER		Clock;
	PKDPC			Dpc;
	FILETIME		DueTime;
	LONG			Period;
	volatile LONG	state;
	bool			notify;
} TIMER, *PTIMER;

enum { TimerIdle, TimerActive, TimerDone };


static VOID DdkTimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer)
{
	TIMER *pTimer = (PTIMER)Context;

	if (pTimer->Period || InterlockedCompareExchange(
			&pTimer->state, TimerDone, TimerActive) == TimerActive) {
		SetEvent(pTimer->h);
		if (pTimer->Dpc) KeInsertQueueDpc(pTimer->Dpc, 0, 0);
	}
}


DDKAPI
VOID KeInitializeTimerEx(PKTIMER Timer, TIMER_TYPE Type)
{
	TIMER *pTimer = (TIMER *)Timer;

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	DdkInitializeObject(pTimer, sizeof(TIMER), _SizeofTimer_);

	pTimer->h = CreateEvent(NULL, (Type == NotificationTimer), FALSE, NULL);
	pTimer->Clock = CreateThreadpoolTimer(DdkTimerCallback, (PVOID)Timer, NULL);
	pTimer->type = TimerType;
	pTimer->state = TimerIdle;
	pTimer->notify = (Type == NotificationTimer);

	if (!pTimer->h || !pTimer->Clock)
		ddkfail("Unable to initialize Timer");
}


DDKAPI
VOID KeInitializeTimer(PKTIMER Timer)
{
	KeInitializeTimerEx(Timer, NotificationTimer);
}


DDKAPI
BOOLEAN KeCancelTimer(PKTIMER Timer)
{
	TIMER *pTimer = (TIMER *)Timer;
	LONG rc = InterlockedExchange(&pTimer->state, TimerIdle);

	SetThreadpoolTimer(pTimer->Clock, NULL, 0, 0);
	WaitForThreadpoolTimerCallbacks(pTimer->Clock, TRUE);
	return (rc == TimerActive);
}


DDKAPI
BOOLEAN KeSetTimerEx(PKTIMER Timer, LARGE_INTEGER DueTime, LONG Period, PKDPC Dpc)
{
	DDKASSERT(Period <= MAXLONG);
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	TIMER *pTimer = (TIMER *)Timer;
	LONG rc = InterlockedExchange(&pTimer->state, TimerIdle);

	SetThreadpoolTimer(pTimer->Clock, NULL, 0, 0);
	WaitForThreadpoolTimerCallbacks(pTimer->Clock, TRUE);

	pTimer->Dpc = Dpc;
	pTimer->Period = Period;
	pTimer->DueTime = DdkGetDueTime(DueTime);
	pTimer->state = TimerActive;

	ResetEvent(pTimer->h);
	SetThreadpoolTimer(pTimer->Clock, &pTimer->DueTime, Period, 0);
	return (rc == TimerActive);
}


DDKAPI
BOOLEAN KeSetTimer(PKTIMER Timer, LARGE_INTEGER DueTime, PKDPC Dpc)
{
	return KeSetTimerEx(Timer, DueTime, 0, Dpc);
}


DDKAPI
BOOLEAN KeReadStateTimer(PKTIMER Timer)
{
	TIMER *pTimer = (TIMER *)Timer;

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	DWORD rc = WaitForSingleObject(pTimer->h, 0);

	if (rc == WAIT_OBJECT_0) {
		if (!pTimer->notify) SetEvent(pTimer->h);
		return TRUE;
	}

	return FALSE;
}
