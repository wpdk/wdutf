/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2022, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

 /*
  * Detours Routines.
  */

#include "stdafx.h"
#include "detours.h"


/*
 *	Architure specific definitions.
 */

#if defined(_X86_) || defined(_AMD64_)
#define WRAPPER_LEN	16
#else
#define WRAPPER_LEN 24
#endif

#define MAXIMUM_JUMP 0x80000000U


/*
 *	Detours is designed to attach a single, essentially static, intercept to a
 *	function. In a Unit Test environment, multiple short-lived tests, requiring
 *	different intercepts, can be running concurrently.
 *
 *	A list of active intercepts is maintained and a wrapper selects the
 *	appropriate intercept for the current thread. The intercept is identified
 *	by a unique identity in case the compiler/linker merges common code.
 */

typedef struct _INTERCEPT {
	struct _INTERCEPT *next;
	struct _DETOUR *detour;		// Detour
	PVOID intercept;			// Intercept function
	PVOID identity;				// Identity of intercept
	PVOID instance;				// Instance of intercept
	PVOID thread;				// Thread to intercept
} INTERCEPT;


/*
 *	Each detour starts with the position-independent code located in wrapper[]
 *	that preserves the arguments and 'this' pointer, calls DdkGetIntercept to
 *	identify the appropriate detour and then executes it.
 */

typedef struct _DETOUR {
	char wrapper[WRAPPER_LEN];	// Wrapper code (must be first element)
	struct _REGION *region;		// Region
	PVOID function;				// Original function
	PVOID trampoline;			// Trampoline
	SRWLOCK lock;				// Intercept lock
	INTERCEPT *intercepts;		// Intercept list
} DETOUR;


/*
 *	Detours are created in regions of memory located near to the target
 *	function so that the wrapper can be reached with a signed 32-bit offset.
 */

typedef struct _REGION {
	struct _REGION *next;
	DWORD size;					// Region size
	ULONG refcount;				// Reference count
	size_t count;				// Detour count
	size_t freecount;			// Free detours
	size_t hwm;					// High water mark
	DETOUR detour[1];			// Detour array
} REGION;


static REGION *regionlist;
static SRWLOCK Lock = SRWLOCK_INIT;

__declspec(thread) PVOID realpointer = NULL;

extern "C" void _DdkWrapper(), _DdkWrapperEnd();


NTSTATUS DdkAllocateIntercept(PVOID pIntercept,
	PVOID pIdentity, PVOID pInstance, PVOID pThread, INTERCEPT **ppEntry);
NTSTATUS DdkFreeIntercept(INTERCEPT *pEntry);
DETOUR *DdkFindDetour(PVOID pFunction);
NTSTATUS DdkAllocateDetour(PVOID pFunction, DETOUR **ppDetour);
NTSTATUS DdkFreeDetour(DETOUR *pDetour);
NTSTATUS DdkAttachDetour(DETOUR *pDetour);
NTSTATUS DdkDetachDetour(DETOUR *pDetour);
NTSTATUS DdkUpdateDetour(DETOUR *pDetour, bool enable);
NTSTATUS DdkAllocateRegion(PVOID pFunction, REGION **ppRegion);
void DdkFreeRegion(REGION *pRegion);
void DdkAddReference(REGION *pRegion);
void DdkDeleteReference(REGION *pRegion);
NTSTATUS DdkGetDetourStatus(LONG rc);


PVOID DdkCodeFromPointer(PVOID pAddr)
{
	return DetourCodeFromPointer(pAddr, NULL);
}


PVOID DdkFindFunction(const char *pName)
{
	char *program, *system;
	PVOID pSysAddr = NULL;

	if (!pName) return NULL;

	size_t slen = (system = getenv("SystemRoot")) ? strlen(system) : 0;
	size_t plen = (program = getenv("ProgramFiles")) ? strlen(program) : 0;

	const char *cp = strchr((char *)pName, '!');
	const char *symbol = cp ? cp + 1 : pName;
	size_t len = cp ? (cp - pName) : 0;

	for (HMODULE h = NULL; (h = DetourEnumerateModules(h)); ) {
		char path[MAX_PATH] = { 0 };
		if (!GetModuleFileNameA(h, path, sizeof(path) - 1)) continue;

		// Limit search to specific module (module!symbol)

		if (len) {
			if (!(cp = strrchr(path, '\\'))) cp = strrchr(path, '/');
			cp = cp ? cp + 1 : path;
			if (_strnicmp(cp, pName, len) || (cp[len] && cp[len] != '.'))
				continue;
		}

		// Deprioritise non-driver modules in system paths

		if (!(cp = strrchr(path, '.')) || _stricmp(cp, ".sys"))
			if ((slen && _strnicmp(path, system, slen) == 0) ||
					(plen && _strnicmp(path, program, plen) == 0)) {
				if (!pSysAddr)
					pSysAddr = DetourFindFunction(path, symbol);
				continue;
			}

		PVOID v = DetourFindFunction(path, symbol);
		if (v) return v;
	}

	return pSysAddr;
}


HMODULE DdkFindModule(PVOID pAddr)
{
	ULONG_PTR addr = (ULONG_PTR)pAddr;

	for (HMODULE h = NULL; (h = DetourEnumerateModules(h)); ) {
		char path[MAX_PATH] = { 0 };
		if (!GetModuleFileNameA(h, path, sizeof(path) - 1)) continue;

		IMAGE_OPTIONAL_HEADER64 *pOpt = &((PIMAGE_NT_HEADERS64)((ULONG_PTR)h
			+ ((PIMAGE_DOS_HEADER)h)->e_lfanew))->OptionalHeader;

		if (pOpt->ImageBase <= addr && addr < pOpt->ImageBase + pOpt->SizeOfImage)
			return h;
	}

	return NULL;
}


NTSTATUS DdkAttachIntercept(PVOID pFunction, PVOID pIntercept,
	PVOID pIdentity, PVOID pInstance, PVOID pThread)
{
	INTERCEPT *pEntry;

	if (!pFunction || !pIntercept)
		return STATUS_INVALID_PARAMETER;

	NTSTATUS status = DdkAllocateIntercept(pIntercept,
						pIdentity, pInstance, pThread, &pEntry);

	if (!NT_SUCCESS(status))
		return status;

	AcquireSRWLockExclusive(&Lock);
	DETOUR *pDetour = DdkFindDetour(pFunction);

	if (!pDetour) {
		status = DdkAllocateDetour(pFunction, &pDetour);

		if (!NT_SUCCESS(status)) {
			DdkFreeIntercept(pEntry);
			ReleaseSRWLockExclusive(&Lock);
			return status;
		}
	}

	pEntry->detour = pDetour;

	AcquireSRWLockExclusive(&pDetour->lock);
	pEntry->next = pDetour->intercepts;
	pDetour->intercepts = pEntry;
	ReleaseSRWLockExclusive(&pDetour->lock);

	ReleaseSRWLockExclusive(&Lock);
	return STATUS_SUCCESS;
}


NTSTATUS DdkDetachIntercept(PVOID pId, PVOID pThread)
{
	if (!regionlist)
		return STATUS_SUCCESS;

	NTSTATUS status = STATUS_SUCCESS;

	AcquireSRWLockExclusive(&Lock);

	for (REGION *pRegion = regionlist, *pNext; pRegion; pRegion = pNext) {
		DdkAddReference(pRegion);

		for (size_t i = 0; i < pRegion->hwm; i++)
			for (INTERCEPT *p = pRegion->detour[i].intercepts, *next; p; p = next) {
				next = p->next;

				if ((!pThread || p->thread == pThread) &&
						(!pId || p->identity == pId || p->instance == pId)) {
					NTSTATUS rc = DdkFreeIntercept(p);
					if (!NT_SUCCESS(rc)) status = rc;
				}
			}

		pNext = pRegion->next;
		DdkDeleteReference(pRegion);
	}

	ReleaseSRWLockExclusive(&Lock);
	return status;
}


NTSTATUS DdkDetachAddressRange(PVOID pAddr, size_t len, HMODULE module)
{
	PVOID pModuleAddr = NULL, pModuleEnd = NULL;
	PVOID pEnd = (PVOID)((char *)pAddr + len);

	if (!regionlist || !len)
		return STATUS_SUCCESS;

	if (module) {
		IMAGE_OPTIONAL_HEADER64 *pOpt = &((PIMAGE_NT_HEADERS64)((ULONG_PTR)module
				+ ((PIMAGE_DOS_HEADER)module)->e_lfanew))->OptionalHeader;

		pModuleAddr = (PVOID)pOpt->ImageBase;
		pModuleEnd = (PVOID)((char *)pOpt->ImageBase + pOpt->SizeOfImage);
	}

	NTSTATUS status = STATUS_SUCCESS;

	AcquireSRWLockExclusive(&Lock);

	for (REGION *pRegion = regionlist, *pNext; pRegion; pRegion = pNext) {
		DdkAddReference(pRegion);

		for (size_t i = 0; i < pRegion->hwm; i++) {
			DETOUR *pDetour = &pRegion->detour[i];

			if (!pDetour->function) continue;

			if (pAddr <= pDetour->function && pDetour->function < pEnd)
				for (INTERCEPT *p = pDetour->intercepts, *next; p; p = next) {
					next = p->next;

					if (!module || (pModuleAddr <= p->intercept
							&& p->intercept < pModuleEnd)) {
						NTSTATUS rc = DdkFreeIntercept(p);
						if (!NT_SUCCESS(rc)) status = rc;
					}
				}
		}

		pNext = pRegion->next;
		DdkDeleteReference(pRegion);
	}

	ReleaseSRWLockExclusive(&Lock);
	return status;
}


DDKAPI
PVOID DdkGetIntercept(DETOUR *pDetour)
{
	if (!pDetour || !pDetour->function)
		ddkfail("Invalid detour");

	AcquireSRWLockShared(&pDetour->lock);
	PVOID pIntercept = pDetour->trampoline;

	for (INTERCEPT *ep = pDetour->intercepts; ep; ep = ep->next) {
		if (ep->thread == DdkGetCurrentThread()) {
			pIntercept = ep->intercept;
			break;
		}

		if (ep->thread == NULL)
			pIntercept = ep->intercept;
	}

	if (!pIntercept)
		pIntercept = pDetour->function;

	realpointer = pDetour->trampoline;

	ReleaseSRWLockShared(&pDetour->lock);
	return pIntercept;
}


PVOID DdkGetRealPointer()
{
	PVOID v = realpointer;

	if (!v)
		ddkfail("Real pointer not found");

	realpointer = NULL;
	return v;
}


static NTSTATUS DdkAllocateIntercept(PVOID pIntercept,
	PVOID pIdentity, PVOID pInstance, PVOID pThread, INTERCEPT **ppEntry)
{
	INTERCEPT *pEntry = (INTERCEPT *)calloc(1, sizeof(INTERCEPT));

	if (!pEntry)
		return STATUS_INSUFFICIENT_RESOURCES;

	pEntry->intercept = pIntercept;
	pEntry->identity = pIdentity;
	pEntry->instance = pInstance;
	pEntry->thread = pThread;
	*ppEntry = pEntry;
	return STATUS_SUCCESS;
}


static NTSTATUS DdkFreeIntercept(INTERCEPT *pEntry)
{
	if (!pEntry) return STATUS_SUCCESS;

	NTSTATUS status = STATUS_SUCCESS;
	DETOUR *pDetour = pEntry->detour;

	if (pDetour) {
		AcquireSRWLockExclusive(&pDetour->lock);

		for (INTERCEPT **epp = &pDetour->intercepts; *epp; epp = &(*epp)->next)
			if (*epp == pEntry) {
				*epp = pEntry->next;
				pEntry->next = NULL;
				break;
			}

		ReleaseSRWLockExclusive(&pDetour->lock);
		status = DdkFreeDetour(pDetour);
	}

	free(pEntry);
	return status;
}


static bool DdkIsRegionWithinJumpBounds(REGION *pRegion,
	PVOID pFunction, int slack = 0)
{
	unsigned int maxjump = MAXIMUM_JUMP - slack;
	char *pAddr = (char *)pFunction;

	if (pAddr >= (char *)pRegion
		&& pAddr - (char *)pRegion <= maxjump) return true;

	if ((char *)pRegion + pRegion->size > pAddr
		&& (char *)pRegion + pRegion->size - pAddr <= maxjump) return true;

	return false;
}


static DETOUR *DdkFindDetour(PVOID pFunction)
{
	for (REGION *pRegion = regionlist; pRegion; pRegion = pRegion->next) {
		if (!DdkIsRegionWithinJumpBounds(pRegion, pFunction)) continue;

		for (size_t i = 0; i < pRegion->hwm; i++)
			if (pRegion->detour[i].function == pFunction)
				return &pRegion->detour[i];
	}

	return NULL;
}


static NTSTATUS DdkAllocateDetour(PVOID pFunction, DETOUR **ppDetour)
{
	REGION *pRegion;

	// Allocate from first available region within jump bounds

	for (pRegion = regionlist; pRegion; pRegion = pRegion->next)
		if (pRegion->freecount && DdkIsRegionWithinJumpBounds(pRegion,
			pFunction, 0x100)) break;

	// Allocate a new region if necessary

	if (!pRegion) {
		NTSTATUS status = DdkAllocateRegion(pFunction, &pRegion);

		if (!NT_SUCCESS(status))
			return status;
	}

	// Find free detour

	DETOUR *pDetour = NULL;

	for (size_t i = 0; i < pRegion->count; i++)
		if (!pRegion->detour[i].function) {
			pDetour = &pRegion->detour[i];
			if (i >= pRegion->hwm)
				pRegion->hwm = i + 1;
			break;
		}

	if (!pDetour)
		ddkfail("Region has no free detours");

	// Initialise detour

	pDetour->function = pFunction;
	pDetour->intercepts = NULL;
	pDetour->trampoline = NULL;
	pRegion->freecount--;

	// Attach detour to function

	NTSTATUS status = DdkAttachDetour(pDetour);

	if (!NT_SUCCESS(status)) {
		DdkFreeDetour(pDetour);
		return status;
	}

	*ppDetour = pDetour;
	return STATUS_SUCCESS;
}


static NTSTATUS DdkFreeDetour(DETOUR *pDetour)
{
	if (!pDetour || pDetour->intercepts)
		return STATUS_SUCCESS;

	REGION *pRegion = pDetour->region;

	NTSTATUS status = DdkDetachDetour(pDetour);

	if (!pDetour->trampoline && pDetour->function) {
		pDetour->function = NULL;
		pRegion->freecount++;
	}

	for (; pRegion->hwm; pRegion->hwm--)
		if (pRegion->detour[pRegion->hwm - 1].function) break;

	DdkFreeRegion(pRegion);
	return status;
}


static NTSTATUS DdkAttachDetour(DETOUR *pDetour)
{
	if (!pDetour || pDetour->trampoline)
		return STATUS_SUCCESS;

	pDetour->trampoline = pDetour->function;

	NTSTATUS status = DdkUpdateDetour(pDetour, true);

	if (!NT_SUCCESS(status))
		pDetour->trampoline = NULL;

	return status;
}


static NTSTATUS DdkDetachDetour(DETOUR *pDetour)
{
	if (!pDetour || !pDetour->trampoline)
		return STATUS_SUCCESS;

	NTSTATUS status = DdkUpdateDetour(pDetour, false);

	if (NT_SUCCESS(status))
		pDetour->trampoline = NULL;

	return status;
}


static NTSTATUS DdkUpdateDetour(DETOUR *pDetour, bool enable)
{
	LONG rc = DetourTransactionBegin();
	
	if (rc != NO_ERROR)
		return DdkGetDetourStatus(rc);

	DdkThreadLock();
	rc = DdkInvokeForAllThreads(DetourUpdateThread);

	if (rc != NO_ERROR) {
		DdkThreadUnlock();
		DetourTransactionAbort();
		return DdkGetDetourStatus(rc);
	}

	rc = (enable) ? DetourAttach(&pDetour->trampoline, &pDetour->wrapper) :
					DetourDetach(&pDetour->trampoline, &pDetour->wrapper);

	if (rc != NO_ERROR) {
		DdkThreadUnlock();
		DetourTransactionAbort();
		return DdkGetDetourStatus(rc);
	}

	rc = DetourTransactionCommit();

	DdkThreadUnlock();
	return DdkGetDetourStatus(rc);
}


static NTSTATUS DdkAllocateRegion(PVOID pFunction, REGION **ppRegion)
{
	DWORD size;
	REGION *pRegion = (REGION *)DetourAllocateRegionWithinJumpBounds(pFunction, &size);

	if (!pRegion || size < sizeof(REGION)) {
		if (pRegion) VirtualFree(pRegion, 0, MEM_RELEASE);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pRegion->size = size;
	pRegion->count = (size - sizeof(REGION)) / sizeof(DETOUR) + 1;
	pRegion->freecount = pRegion->count;
	pRegion->refcount = 0;

	const size_t len = (char *)&_DdkWrapperEnd - (char *)&_DdkWrapper;

	if (sizeof(pRegion->detour[0].wrapper) < len)
		ddkfail("Wrapper array is too small");

	for (int i = 0; i < pRegion->count; i++) {
		memcpy(pRegion->detour[i].wrapper, &_DdkWrapper, len);
		pRegion->detour[i].region = pRegion;
		InitializeSRWLock(&pRegion->detour[i].lock);
	}

	pRegion->next = regionlist;
	*ppRegion = regionlist = pRegion;
	return STATUS_SUCCESS;
}


static void DdkFreeRegion(REGION *pRegion)
{
	if (!pRegion || pRegion->refcount || pRegion->freecount != pRegion->count)
		return;

	// Remove region from list

	for (REGION **pp = &regionlist; *pp; pp = &(*pp)->next)
		if (*pp == pRegion) {
			*pp = pRegion->next;
			break;
		}

	VirtualFree(pRegion, 0, MEM_RELEASE);
}


static void DdkAddReference(REGION *pRegion)
{
	pRegion->refcount++;
}


static void DdkDeleteReference(REGION *pRegion)
{
	pRegion->refcount--;
	DdkFreeRegion(pRegion);
}


static NTSTATUS DdkGetDetourStatus(LONG rc)
{
	switch (rc) {
	case NO_ERROR:
		return STATUS_SUCCESS;

	case ERROR_INVALID_OPERATION:
		return STATUS_OPERATION_IN_PROGRESS;

	case ERROR_ACCESS_DENIED:
		return STATUS_ACCESS_DENIED;

	case ERROR_NOT_ENOUGH_MEMORY:
		return STATUS_INSUFFICIENT_RESOURCES;

	case ERROR_INVALID_BLOCK:
		return STATUS_INVALID_BLOCK_LENGTH;

	case ERROR_INVALID_HANDLE:
		return STATUS_INVALID_HANDLE;

	case ERROR_INVALID_DATA:
		return STATUS_DATA_ERROR;
	}

	return STATUS_UNSUCCESSFUL;
}
