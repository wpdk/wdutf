/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Semaphore Routines
 */

#include "stdafx.h"


typedef struct _SEMAPHORE : public OBJECT {
	DWORD			threadid;
	volatile LONG	count;
} SEMAPHORE, *PSEMAPHORE;


DDKAPI
VOID KeInitializeSemaphore(PRKSEMAPHORE Semaphore, LONG Count, LONG Limit)
{
	SEMAPHORE *pSema = (SEMAPHORE *)Semaphore;

	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	DdkInitializeObject(pSema, sizeof(SEMAPHORE), _SizeofSemaphore_);

	pSema->h = CreateSemaphore(NULL, Count, Limit, NULL);
	pSema->type = SemaphoreType;
	pSema->count = Count;

	if (!pSema->h) ddkfail("Unable to initialize Semaphore");
}


DDKAPI
LONG KeReleaseSemaphore(PRKSEMAPHORE Semaphore,
	KPRIORITY Increment, LONG Adjustment, BOOLEAN Wait)
{
	SEMAPHORE *pSema = (SEMAPHORE *)Semaphore;
	LONG prev;

	DDKASSERT(KeGetCurrentIrql() <= (Wait ? PASSIVE_LEVEL : DISPATCH_LEVEL));

	InterlockedAdd(&pSema->count, Adjustment);

	if (!ReleaseSemaphore(pSema->h, Adjustment, &prev))
		KeBugCheck(STATUS_SEMAPHORE_LIMIT_EXCEEDED);

	return (prev != 0);
}


DDKAPI
LONG KeReadStateSemaphore(PRKMUTEX Semaphore)
{
	SEMAPHORE *pSema = (SEMAPHORE *)Semaphore;
	return (pSema->count != 0);
}


void DdkUpdateSemaphore(OBJECT *pObj)
{
	SEMAPHORE *pSema = (SEMAPHORE *)pObj;
	InterlockedDecrement(&pSema->count);
}

