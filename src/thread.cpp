/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Thread Routines
 */

#include "stdafx.h"
#include <intrin.h>


typedef struct PROCESS : public OBJECT {
	KPRIORITY			BasePriority;
} *PPROCESS;

typedef struct _THREAD : public OBJECT {
	PKSTART_ROUTINE		Start;
	PVOID				Context;
	PPROCESS			process;
	KPRIORITY			Priority;
	KPRIORITY			BasePriority;
	DWORD				ThreadId;
	struct _THREAD		*next;
} THREAD, *PTHREAD;


void DdkInsertThread();


__declspec(thread) PTHREAD DdkCurrentThread = 0;
__declspec(thread) PPROCESS DdkCurrentProcess = 0;

static PROCESS SystemProcess;
DDKAPI PEPROCESS PsInitialSystemProcess = 0;

static const int DefaultThreadPriority = 8;
static const int DefaultBaseThreadPriority = 16;

static THREAD *threadlist;
static SRWLOCK Lock = SRWLOCK_INIT;


static DWORD StartThread(PVOID arg)
{
	THREAD *pThread = (THREAD *)arg;

	while (pThread->h == NULL) Sleep(1);

	DdkThreadDeinit();
	DdkCurrentThread = pThread;
	DdkCurrentProcess = pThread->process;
	DdkThreadInit();
	DdkInsertThread();

	(*pThread->Start)(pThread->Context);

	PsTerminateSystemThread(0);
	return 0;
}


/*
 *	VOID DdkThreadInit()
 *
 *	Initialise the thread in preparation for DDK calls.
 */

void DdkThreadInit()
{
	static const DWORD TebCurrentThread = 0x188;

	if (SystemProcess.type != ProcessType) {
		SystemProcess.type = ProcessType;
		SystemProcess.h = GetCurrentProcess();
		SystemProcess.BasePriority = DefaultBaseThreadPriority;
		InterlockedIncrement(&SystemProcess.refcount);
		PsInitialSystemProcess = (PEPROCESS)ToPointer(&SystemProcess);
	}

	if (!DdkCurrentProcess)
		DdkCurrentProcess = &SystemProcess;

	if (!DdkCurrentThread) {
		DdkCurrentThread = (THREAD *)DdkAllocObject(sizeof(THREAD), ThreadType);
		DdkCurrentThread->process = &SystemProcess;
		DdkCurrentThread->Priority = DefaultThreadPriority;
		DdkCurrentThread->BasePriority = DefaultBaseThreadPriority;

		if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
				&DdkCurrentThread->h, 0, FALSE, DUPLICATE_SAME_ACCESS))
			ddkfail("Failed to duplicate thread handle");

		DdkInsertThread();
	}

	// Update the Thread Environment Block with a pointer to the thread,
	// allowing the inline version of KeGetCurrentThread() to run unchanged.
	// The area is named SoftFpcr and seems to be unused.

	__writegsqword(TebCurrentThread, (DWORD64)DdkCurrentThread);
}


void DdkInsertThread()
{
	DdkThreadLock();
	DdkCurrentThread->ThreadId = GetCurrentThreadId();

	for (THREAD *pThread = threadlist; pThread; pThread = pThread->next)
		if (pThread->ThreadId == DdkCurrentThread->ThreadId)
			ddkfail("Thread already in list");

	DdkCurrentThread->next = threadlist;
	threadlist = DdkCurrentThread;
	DdkThreadUnlock();
}


void DdkThreadDeinit()
{
	if (DdkCurrentThread) {
		DdkDetachIntercept(NULL, DdkGetCurrentThread());
		DdkThreadLock();

		for (THREAD **pp = &threadlist; *pp; pp = &(*pp)->next)
			if (*pp == DdkCurrentThread) {
				*pp = DdkCurrentThread->next;
				DdkCurrentThread->next = NULL;
				break;
			}

		DdkDereferenceObject(DdkCurrentThread);
		DdkCurrentThread = NULL;
		DdkThreadUnlock();
	}
}


LONG DdkInvokeForAllThreads(LONG (*func)(HANDLE))
{
	for (THREAD *pThread = threadlist; pThread; pThread = pThread->next) {
		LONG rc = (*func)((pThread == DdkCurrentThread)
			? GetCurrentThread() : pThread->h);

		if (rc != NO_ERROR)
			return rc;
	}

	return NO_ERROR;
}


PKTHREAD DdkGetCurrentThread()
{
	if (!DdkCurrentThread) DdkThreadInit();
	return (PKTHREAD)ToPointer(DdkCurrentThread);
}


static PPROCESS GetProcess(OBJECT *pObj)
{
	if (!pObj || pObj->type != ProcessType)
		ddkfail("Invalid process specified");

	return static_cast<PROCESS *>(pObj);
}


static PTHREAD GetThread(OBJECT *pObj)
{
	if (!pObj || pObj->type != ThreadType)
		ddkfail("Invalid thread specified");

	return static_cast<THREAD *>(pObj);
}


DDKAPI
NTSTATUS PsCreateSystemThread(PHANDLE ThreadHandle, ULONG DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle, PCLIENT_ID ClientId,
    PKSTART_ROUTINE StartRoutine, PVOID StartContext)
{
	DdkThreadInit();
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	PROCESS *pProcess = (!ProcessHandle) ? &SystemProcess : GetProcess(FromHandle(ProcessHandle));
	THREAD *pThread = (THREAD *)DdkAllocObject(sizeof(THREAD), ThreadType, true);

	if (!pThread) return STATUS_INSUFFICIENT_RESOURCES;

	pThread->Start = StartRoutine;
	pThread->Context = StartContext;
	pThread->process = pProcess;
	pThread->Priority = DefaultThreadPriority;
	pThread->BasePriority = DefaultBaseThreadPriority;

	DdkReferenceObject(pThread);
	pThread->h = CreateThread(NULL, 0, StartThread, pThread, 0, NULL);

	if (pThread->h == NULL) {
		free(pThread);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	*ThreadHandle = ToHandle(pThread);
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS PsTerminateSystemThread(NTSTATUS ExitStatus)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	DdkThreadDeinit();

	ExitThread(ExitStatus);
	return STATUS_UNSUCCESSFUL;
}


DDKAPI
PEPROCESS IoGetCurrentProcess()
{
	DdkThreadInit();
	return (PEPROCESS)ToPointer(DdkCurrentProcess);
}


DDKAPI
HANDLE PsGetCurrentProcessId()
{
	DdkThreadInit();
	return ToHandle(DdkCurrentProcess);
}


DDKAPI
KPRIORITY KeQueryPriorityThread(PKTHREAD Thread)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	
	return GetThread(FromPointer(Thread))->Priority;
}


DDKAPI
KPRIORITY KeSetPriorityThread(PKTHREAD Thread, KPRIORITY Priority)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	KPRIORITY oldPriority = KeQueryPriorityThread(Thread);
	KPRIORITY newPriority = Priority < 0 ? 0 : Priority > 31 ? 31 : Priority;
		
	GetThread(FromPointer(Thread))->Priority = newPriority;
	return oldPriority;
}


DDKAPI
LONG KeSetBasePriorityThread(PKTHREAD Thread, LONG Increment)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	PTHREAD pThread = GetThread(FromPointer(Thread));
	KPRIORITY oldBase = pThread->BasePriority;
	KPRIORITY newBase = pThread->process->BasePriority + Increment;

	if (newBase > 31) newBase = 31;
	else if (newBase < 0) newBase = 0;

	pThread->BasePriority = newBase;
	return oldBase - pThread->process->BasePriority;
}


DDKAPI
HANDLE PsGetProcessId(PEPROCESS Process)
{
	DdkThreadInit();
	return ToHandle(GetProcess(FromPointer(Process)));
}


#ifndef _DDKINLINE_
DDKAPI
PKTHREAD KeGetCurrentThread()
{
	DdkThreadInit();
	return (PKTHREAD)ToPointer(DdkCurrentThread);	
}


DDKAPI
PKTHREAD PsGetCurrentThread()
{
	DdkThreadInit();
	return (PKTHREAD)ToPointer(DdkCurrentThread);	
}
#endif


DDKAPI
HANDLE PsGetCurrentThreadId()
{
	DdkThreadInit();
	return ToHandle(DdkCurrentThread);
}


DDKAPI
BOOLEAN PsIsSystemThread(PETHREAD Thread)
{
	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	return (GetThread(FromPointer(Thread))->process == &SystemProcess);
}


void DdkThreadLock() {
	AcquireSRWLockExclusive(&Lock);
}


void DdkThreadUnlock() {
	ReleaseSRWLockExclusive(&Lock);
}
