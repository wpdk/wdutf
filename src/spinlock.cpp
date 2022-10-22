/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	SpinLock Routines
 */

#include "stdafx.h"


typedef struct _LOCKHANDLE {
	PKSPIN_LOCK		lock;
	KIRQL			irql;
} LOCKHANDLE, *PLOCKHANDLE;


static ULONG lockcount = 1;
static const int maxv = 4096;

static SRWLOCK *lockv[4096];
static SRWLOCK Lock = SRWLOCK_INIT;


DDKAPI
VOID KeInitializeSpinLock(PKSPIN_LOCK SpinLock)
{
	if (sizeof(SRWLOCK) > sizeof(KSPIN_LOCK))
		ddkfail("spinlock is too small to initialize");

	InitializeSRWLock((SRWLOCK *)SpinLock);
}


DDKAPI
BOOLEAN KeTestSpinLock(PKSPIN_LOCK SpinLock)
{
	return (((SRWLOCK *)SpinLock)->Ptr == 0) ? TRUE : FALSE;
}


static
KIRQL AcquireSpinLock(PKSPIN_LOCK SpinLock, KIRQL NewIrql)
{
	KIRQL rc;
	KeRaiseIrql(NewIrql, &rc);
	AcquireSRWLockExclusive((SRWLOCK *)SpinLock);
	return rc;
}


DDKAPI
KIRQL KeAcquireSpinLockRaiseToDpc(PKSPIN_LOCK SpinLock)
{
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	return AcquireSpinLock(SpinLock, DISPATCH_LEVEL);
}


DDKAPI
KIRQL KeAcquireSpinLockForDpc(PKSPIN_LOCK SpinLock)
{
	return AcquireSpinLock(SpinLock, DISPATCH_LEVEL);
}


DDKAPI
VOID KeAcquireSpinLockAtDpcLevel(PKSPIN_LOCK SpinLock)
{
	DDKASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
	AcquireSpinLock(SpinLock, KeGetCurrentIrql());
}


DDKAPI
BOOLEAN KeTryToAcquireSpinLockAtDpcLevel(PKSPIN_LOCK SpinLock)
{
	DDKASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
	return (TryAcquireSRWLockExclusive((SRWLOCK *)SpinLock) != 0) ? TRUE : FALSE;
}


DDKAPI
VOID KeReleaseSpinLock(PKSPIN_LOCK SpinLock, KIRQL NewIrql)
{
	DDKASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
	ReleaseSRWLockExclusive((SRWLOCK *)SpinLock);
	KeLowerIrql(NewIrql);
}


DDKAPI
VOID KeReleaseSpinLockFromDpcLevel(PKSPIN_LOCK SpinLock)
{
	KeReleaseSpinLock(SpinLock, KeGetCurrentIrql());
}


DDKAPI
VOID KeReleaseSpinLockForDpc(PKSPIN_LOCK SpinLock, KIRQL OldIrql)
{
	KeReleaseSpinLock(SpinLock, OldIrql);
}


DDKAPI
VOID KeAcquireInStackQueuedSpinLock(PKSPIN_LOCK SpinLock, PKLOCK_QUEUE_HANDLE LockHandle)
{
	PLOCKHANDLE h = (PLOCKHANDLE)LockHandle;
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	h->irql = AcquireSpinLock(SpinLock, DISPATCH_LEVEL);
	h->lock = SpinLock;
}


DDKAPI
VOID KeAcquireInStackQueuedSpinLockAtDpcLevel(PKSPIN_LOCK SpinLock, PKLOCK_QUEUE_HANDLE LockHandle)
{
	PLOCKHANDLE h = (PLOCKHANDLE)LockHandle;
	DDKASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
	h->irql = AcquireSpinLock(SpinLock, KeGetCurrentIrql());
	h->lock = SpinLock;
}


DDKAPI
VOID KeReleaseInStackQueuedSpinLock(PKLOCK_QUEUE_HANDLE LockHandle)
{
	PLOCKHANDLE h = (PLOCKHANDLE)LockHandle;
	DDKASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
	KeReleaseSpinLock(h->lock, h->irql);
}


DDKAPI
VOID KeReleaseInStackQueuedSpinLockFromDpcLevel(PKLOCK_QUEUE_HANDLE LockHandle)
{
	PLOCKHANDLE h = (PLOCKHANDLE)LockHandle;
	DDKASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
	KeReleaseSpinLock(h->lock, h->irql);
}


static
SRWLOCK *GetSRWLock(PEX_SPIN_LOCK SpinLock)
{
	ULONG i = *(ULONG *)SpinLock;

	if (i == 0) {
		AcquireSRWLockExclusive(&Lock);

		if ((i = *(ULONG *)SpinLock) == 0) {
			i = lockcount;

			if (i / maxv >= sizeof(lockv) / sizeof(lockv[0]))
				ddkfail("Too many read write spinlocks");

			 if (!lockv[i / maxv] && !(lockv[i / maxv] = (SRWLOCK *)calloc(maxv, sizeof(SRWLOCK))))
				 ddkfail("Unable to allocate read write spinlock");

			 lockcount++;
			 InterlockedExchange((ULONG *)SpinLock, i);
		}

		ReleaseSRWLockExclusive(&Lock);
	}

	if (i >= lockcount || !lockv[i / maxv])
		ddkfail("Uninitialized read write lock");

	return &lockv[i / maxv][i % maxv];
}


static
KIRQL AcquireSpinLockShared(PEX_SPIN_LOCK SpinLock, KIRQL NewIrql)
{
	KIRQL rc;
	KeRaiseIrql(NewIrql, &rc);
	AcquireSRWLockShared(GetSRWLock(SpinLock));
	return rc;
}


DDKAPI
KIRQL ExAcquireSpinLockShared(PEX_SPIN_LOCK SpinLock)
{
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	return AcquireSpinLockShared(SpinLock, DISPATCH_LEVEL);
}


DDKAPI
VOID ExReleaseSpinLockShared(PEX_SPIN_LOCK SpinLock, KIRQL NewIrql)
{
	DDKASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
	ReleaseSRWLockShared(GetSRWLock(SpinLock));
	KeLowerIrql(NewIrql);
}


DDKAPI
VOID ExAcquireSpinLockSharedAtDpcLevel(PEX_SPIN_LOCK SpinLock)
{
	DDKASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
	AcquireSpinLockShared(SpinLock, KeGetCurrentIrql());
}


DDKAPI
VOID ExReleaseSpinLockSharedFromDpcLevel(PEX_SPIN_LOCK SpinLock)
{
	DDKASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
	ReleaseSRWLockShared(GetSRWLock(SpinLock));
}


DDKAPI
LOGICAL ExTryConvertSharedSpinLockExclusive(PEX_SPIN_LOCK SpinLock)
{
	// Not supported by SRWLOCKs
	return FALSE;
}


static
KIRQL AcquireSpinLockExclusive(PEX_SPIN_LOCK SpinLock, KIRQL NewIrql)
{
	KIRQL rc;
	KeRaiseIrql(NewIrql, &rc);
	AcquireSRWLockExclusive(GetSRWLock(SpinLock));
	return rc;
}


DDKAPI
KIRQL ExAcquireSpinLockExclusive(PEX_SPIN_LOCK SpinLock)
{
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	return AcquireSpinLockExclusive(SpinLock, DISPATCH_LEVEL);
}


DDKAPI
VOID ExReleaseSpinLockExclusive(PEX_SPIN_LOCK SpinLock, KIRQL NewIrql)
{
	DDKASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
	ReleaseSRWLockExclusive(GetSRWLock(SpinLock));
	KeLowerIrql(NewIrql);
}


DDKAPI
VOID ExAcquireSpinLockExclusiveAtDpcLevel(PEX_SPIN_LOCK SpinLock)
{
	DDKASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
	AcquireSpinLockExclusive(SpinLock, KeGetCurrentIrql());
}

DDKAPI
VOID ExReleaseSpinLockExclusiveFromDpcLevel(PEX_SPIN_LOCK SpinLock)
{
	ExReleaseSpinLockExclusive(SpinLock, KeGetCurrentIrql());
}
