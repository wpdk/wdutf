/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Mutex Routines
 */

#include "stdafx.h"


typedef struct _MUTEX : public OBJECT {
	DWORD			threadid;
	volatile LONG	count;
} MUTEX, *PMUTEX;


DDKAPI
VOID KeInitializeMutex(PRKMUTEX Mutex, ULONG Level)
{
	MUTEX *pMutex = (MUTEX *)Mutex;

	DdkInitializeObject(pMutex, sizeof(MUTEX), _SizeofMutex_);

	pMutex->h = CreateMutex(NULL, FALSE, NULL);
	pMutex->type = MutexType;

	if (!pMutex->h) ddkfail("Unable to initialize Mutex");
}


DDKAPI
LONG KeReleaseMutex(PRKMUTEX Mutex, BOOLEAN Wait)
{
	MUTEX *pMutex = (MUTEX *)Mutex;
	LONG rc = 1;

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	if (pMutex->threadid == GetCurrentThreadId())
		if (!--pMutex->count) {
			pMutex->threadid = 0;
			rc = 0;
		}

	if (!ReleaseMutex(pMutex->h)) return 1;
	return rc;
}


DDKAPI
LONG KeReadStateMutex(PRKMUTEX Mutex)
{
	MUTEX *pMutex = (MUTEX *)Mutex;

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	return (pMutex->count == 0);
}


void DdkUpdateMutex(OBJECT *pObj)
{
	MUTEX *pMutex = (MUTEX *)pObj;
	DWORD id = GetCurrentThreadId();

	if (pMutex->threadid != id) {
		pMutex->threadid = id;
		pMutex->count = 1;
	}

	else pMutex->count++;
}

