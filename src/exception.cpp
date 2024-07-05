/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2014, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Exception Handling.
 */

#include "stdafx.h"


#if defined(_X86_) || defined(_AMD64_)
#define REGISTER_COUNT	16
#else
#define REGISTER_COUNT	31
#endif


extern "C" LONG DdkExceptionHandler(EXCEPTION_POINTERS *xp);

static PVOID DdkVectoredHandle;


void DdkExceptionInit()
{
	DdkVectoredHandle = AddVectoredExceptionHandler(0, DdkExceptionHandler);

	if (!DdkVectoredHandle)
		ddkfail("Unable to register exception handler");
}


void DdkExceptionDeinit()
{
	if (DdkVectoredHandle) {
		RemoveVectoredExceptionHandler(DdkVectoredHandle);
		DdkVectoredHandle = NULL;
	}
}


static DWORD64 *DdkGetRegister(EXCEPTION_POINTERS *xp, UCHAR reg)
{
	// Assumes that the registers are stored in instruction decode
	// order which is currently true and likely to remain so.

#if defined(_X86_) || defined(_AMD64_)
	DWORD64 *pReg = &xp->ContextRecord->Rax;
	return &pReg[reg & 0xf];
#else
	DWORD64* pReg = &xp->ContextRecord->X0;
	return &pReg[reg & 0x1f];
#endif
}


static LONG DdkAccessException(EXCEPTION_POINTERS *xp)
{
	static const ULONG64 KernelShared = 0xFFFFF78000000000UI64;
	static const DWORD64 UserShared = 0x7FFE0000UI64;

	// Check for Read Access Exception

	if (xp->ExceptionRecord->NumberParameters != 2
	|| xp->ExceptionRecord->ExceptionInformation[0] != 0)
		return EXCEPTION_CONTINUE_SEARCH;

	ULONG64 addr = (ULONG64) xp->ExceptionRecord->ExceptionInformation[1];

	// Check for Read from KI_USER_SHARED_DATA

	if (addr >= KernelShared && addr < KernelShared + 0x800) {
		bool found = false;

		// The address of the shared area is in a register so
		// rather than decoding the instructions just fixup the
		// registers and continue.

		for (UCHAR i = 0; i < REGISTER_COUNT; i++) {
			DWORD64 *pReg = DdkGetRegister(xp, i);

			if ((ULONG64)(*pReg) == addr) {
				*pReg = (UserShared | (*pReg & 0x7ff));
				found = true;
			}
		}

		if (found) return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}


static LONG DdkPrivException(EXCEPTION_POINTERS *xp)
{
#if defined(_X86_) || defined(_AMD64_)
	UCHAR *op = (UCHAR *)(xp->ExceptionRecord->ExceptionAddress);

	// Check for CR8 access (irql value)

	// These are MOVs with format 0x44 0x0F 0x20 0xC0
	// The register is in bytes 0 and 3 (mask 0x01 and 0x07)
	// The direction is in byte 2 (0x20: read, 0x22 write)

	if ((op[0] & 0xf6) == 0x44 && op[1] == 0x0f
			&& (op[2] & 0xfd) == 0x20 && (op[3] & 0xf8) == 0xc0) {

		DWORD64 *pReg = DdkGetRegister(xp, ((op[0] & 1) << 3) + (op[3] & 0x7));

		// Emulate the failing instruction

		if (op[2] & 0x02) DdkWriteCR8((ULONG64)*pReg);
		else *pReg = (DWORD64)DdkReadCR8();

		// Skip over it and continue running

		xp->ContextRecord->Rip += 4;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
#endif
	return EXCEPTION_CONTINUE_SEARCH;
}


extern "C"
LONG DdkExceptionHandler(EXCEPTION_POINTERS *xp)
{
	if (xp->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		return DdkAccessException(xp);

	if (xp->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION)
		return DdkPrivException(xp);

    if (xp->ExceptionRecord->ExceptionCode == STATUS_ASSERTION_FAILURE) {
        xp->ExceptionRecord->ExceptionCode = EXCEPTION_UNITTEST_ASSERTION;
        xp->ExceptionRecord->NumberParameters = 2;
		xp->ExceptionRecord->ExceptionInformation[0] = 0;
        xp->ExceptionRecord->ExceptionInformation[1] = 0;
    }

	return EXCEPTION_CONTINUE_SEARCH;
}
