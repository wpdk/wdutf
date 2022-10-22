/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * RTL Routines.
 */

#include "stdddk.h"


#undef RtlZeroMemory


/*
 * Memory Routines
 */

DDKAPI_NODECL
VOID RtlZeroMemory(PVOID Destination, SIZE_T Length)
{
	DDKASSERT(Destination != NULL);

	memset(Destination, 0, Length);
}


DDKAPI
SIZE_T RtlCompareMemory(const VOID *Source1, const VOID *Source2, SIZE_T Length)
{
	DDKASSERT(Source1 != NULL && Source2 != NULL);

	char *s1 = (char *)Source1, *s2 = (char *)Source2;

	for (SIZE_T i = 0; i < Length; i++)
		if (s1[i] != s2[i]) return i;

	return Length;
}

DDKAPI
NTSTATUS RtlGetVersion(PRTL_OSVERSIONINFOW OsVersionInfo)
{
	DDKASSERT(OsVersionInfo != NULL);

	if (OsVersionInfo->dwOSVersionInfoSize == sizeof(RTL_OSVERSIONINFOEXW)) {
		PRTL_OSVERSIONINFOEXW OsVersionInfoEx = (PRTL_OSVERSIONINFOEXW)OsVersionInfo;

		OsVersionInfoEx->wServicePackMajor = 0x0;
		OsVersionInfoEx->wServicePackMinor = 0x0;
		OsVersionInfoEx->wSuiteMask = 0x110;
		OsVersionInfoEx->wProductType = 0x03;
		OsVersionInfoEx->wReserved = 0x00;
	}
	else
		OsVersionInfo->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

	OsVersionInfo->dwMajorVersion = 0x6;
	OsVersionInfo->dwMinorVersion = 0x3;
	OsVersionInfo->dwBuildNumber = 0x2580;
	OsVersionInfo->dwPlatformId = 0x2;

	return STATUS_SUCCESS;
}


#undef RtlUshortByteSwap
DDKAPI
USHORT RtlUshortByteSwap(USHORT Source)
{
	return _byteswap_ushort(Source);
}


#undef RtlUlongByteSwap
DDKAPI
ULONG RtlUlongByteSwap(ULONG Source)
{
	return _byteswap_ulong(Source);
}


#undef RtlUlonglongByteSwap
DDKAPI
ULONGLONG RtlUlonglongByteSwap(ULONGLONG Source)
{
	return _byteswap_uint64(Source);
}
