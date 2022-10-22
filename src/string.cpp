/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2013-2017, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	String Routines
 */

#include "stdddk.h"
#include <stringapiset.h>


char *DdkAllocAnsiBuffer(size_t len)
{
	void *s = ExAllocatePool(NonPagedPool, len);
	if (!s) ddkfail("Unable to allocate memory");
	return (char *)s;
}


DDKAPI
VOID RtlInitAnsiString(PANSI_STRING a, PCSZ s)
{
	// The kernel's version of RtlInitAnsiString() truncates the string if it is too long
	// but we will fail instead as no DataCore driver should be using strings this long ...
	DDKASSERT(!s || strlen(s) <= (MAXUSHORT - 1));

	a->Buffer = (char *)s;
	a->Length = (s) ? (USHORT) strlen(s) : 0;
	a->MaximumLength = (s) ? a->Length + 1 : 0;
}


DDKAPI
VOID RtlInitString(PSTRING DestinationString, PCSZ SourceString)
{
	RtlInitAnsiString(DestinationString, SourceString);
}


DDKAPI
ULONG RtlxAnsiStringToUnicodeSize(PCANSI_STRING AnsiString)
{
	size_t len = mbstowcs(NULL, AnsiString->Buffer, AnsiString->Length);
	return ((__int64)len >= 0) ? (ULONG)((len + sizeof(ANSI_NULL)) * sizeof(WCHAR)) : 0;
}


DDKAPI 
LONG RtlCompareString(const STRING *s1, const STRING *s2, BOOLEAN ign)
{
	int l = (s1->Length < s2->Length) ? s1->Length : s2->Length;

	int rc = (l) ? (ign ? _strnicmp(s1->Buffer, s2->Buffer, l)
		: strncmp(s1->Buffer, s2->Buffer, l)) : 0;

	if (rc != 0) return rc;
	return s1->Length - s2->Length;
}


DDKAPI
VOID RtlFreeAnsiString(PANSI_STRING AnsiString)
{
	DDKASSERT(KeGetCurrentIrql() <= APC_LEVEL);
	
	if (AnsiString->Buffer != NULL)
		ExFreePool(AnsiString->Buffer);

	AnsiString->Buffer = NULL;
}


DDKAPI
VOID RtlCopyString(PSTRING DestinationString, const STRING *SourceString)
{	
	DDKASSERT(DestinationString != NULL);
	
	if (SourceString == NULL){
		DestinationString->Length = 0;
		return;
	}

	const USHORT len = SourceString->Length < DestinationString->MaximumLength ? SourceString->Length : DestinationString->MaximumLength;

	DDKASSERT(DestinationString->Buffer != NULL);
	
	memcpy(DestinationString->Buffer, SourceString->Buffer, len);
	DestinationString->Length = len;
}


DDKAPI
BOOLEAN RtlEqualString(const STRING *s1, const STRING *s2, BOOLEAN CaseInSensitive)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	return RtlCompareString(s1, s2, CaseInSensitive) == 0;
}


DDKAPI
VOID RtlUpperString(PSTRING DestinationString, const STRING *SourceString)
{
	DDKASSERT(KeGetCurrentIrql() <= APC_LEVEL);
	DDKASSERT(DestinationString != NULL || SourceString != NULL);

	USHORT len = SourceString->Length < DestinationString->MaximumLength ? SourceString->Length : DestinationString->MaximumLength;

	// convert to uppercase one char at a time
	// NB we don't use _strupr_s() as this uses null-termination ...
	char ch;
	for (USHORT i = 0; i < len; i++) {
		ch = SourceString->Buffer[i];
		if (islower(ch))
			ch = (char)toupper(ch);
		DestinationString->Buffer[i] = ch;
	}
	
	DestinationString->Length = len;
}
