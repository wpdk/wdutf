/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Unicode String Routines
 */

#include "stdddk.h"
#include <stringapiset.h>


WCHAR *DdkAllocUnicodeBuffer(size_t len)
{
	void *s = ExAllocatePool(NonPagedPool, len);
	if (!s) ddkfail("Unable to allocate memory");
	return (WCHAR *)s;
}


DDKAPI
VOID RtlInitUnicodeString(PUNICODE_STRING u, PCWSTR s)
{
	// The kernel's version of RtlInitUnicodeString() truncates the string if it is too long
	// but we will fail instead as no DataCore driver should be using strings this long ...
	DDKASSERT(!s || ((wcslen(s) + 1) * sizeof(WCHAR)) <= (MAXUSHORT - 1));

	u->Buffer = (WCHAR *)s;
	u->Length = (s) ? (USHORT)(wcslen(s) * sizeof(WCHAR)) : 0;
	u->MaximumLength = (s) ? u->Length + sizeof(UNICODE_NULL) : 0;
}


DDKAPI
VOID RtlCopyUnicodeString(PUNICODE_STRING u, PCUNICODE_STRING s)
{
	int i = 0;

	if (s != NULL) {
		for (; i < u->MaximumLength && i < s->Length; i++)
			((char *)u->Buffer)[i] = ((char *)s->Buffer)[i];
	}

	u->Length = (USHORT) i;
}


DDKAPI
NTSTATUS RtlIntegerToUnicodeString(ULONG val, ULONG base, PUNICODE_STRING u)
{
	DDKASSERT(u != NULL);
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	switch (base) {
		case 0:				// 0 or 10 => decimal
			base = 10; 
		case 10:
			break;
		case 2:				// binary
		case 8:				// octal
		case 16:			// hex
			break;
		default:
			return STATUS_INVALID_PARAMETER;
	}

	// convert into a local buffer to avoid C runtime library error handling behaviour (e.g. if buffer is too small etc) - 
	// which by default seems to involve displaying a dialog box & aborting the process
	const int BufLen = 64+1;	// max for a binary 64-bit integer + a null terminator
	WCHAR buf[BufLen];	

	errno_t err = _ui64tow_s((unsigned __int64)val, buf, BufLen, (int)base);
	DDKASSERT(err == 0);

	// buffer overflow ?
	size_t lenInBytes = wcslen(buf) * sizeof(WCHAR);
	if (lenInBytes > u->MaximumLength)
		return STATUS_BUFFER_OVERFLOW;

	memcpy(u->Buffer, buf, lenInBytes);

	u->Length = (USHORT) lenInBytes;
	return STATUS_SUCCESS;
}


DDKAPI
LONG RtlCompareUnicodeStrings(PCWCH String1, 
	SIZE_T String1Length, PCWCH String2, SIZE_T String2Length, BOOLEAN ign)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	SIZE_T l = (String1Length < String2Length) ? String1Length : String2Length;
	int rc = (l) ? (ign ? _wcsnicmp(String1, String2, l / sizeof(WCHAR))
			            : wcsncmp(String1, String2, l / sizeof(WCHAR))) : 0;

	if (rc != 0) return rc;
	return (ULONG)(String1Length - String2Length);
}


DDKAPI
LONG RtlCompareUnicodeString(PCUNICODE_STRING u1, PCUNICODE_STRING u2, BOOLEAN ign)
{
	return RtlCompareUnicodeStrings(u1->Buffer, u1->Length, u2->Buffer, u2->Length, ign);
}


DDKAPI
NTSTATUS RtlAppendUnicodeToString(PUNICODE_STRING Destination, PCWSTR Source)
{
	USHORT n = (USHORT)wcslen(Source);

	if (Destination->Length + (n * sizeof(WCHAR)) > Destination->MaximumLength)
		return STATUS_BUFFER_TOO_SMALL;

	wcsncpy(&Destination->Buffer[Destination->Length / sizeof(WCHAR)], Source, n);

	Destination->Length += (n * sizeof(WCHAR));
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS RtlAppendUnicodeStringToString(PUNICODE_STRING u, PCUNICODE_STRING s)
{
	int i = 0;

	if (u->Length + s->Length > u->MaximumLength)
		return STATUS_BUFFER_TOO_SMALL;

	while (u->Length < u->MaximumLength && i < s->Length)
		((char *)u->Buffer)[u->Length++] = ((char *)s->Buffer)[i++];

	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS RtlUpcaseUnicodeString(PUNICODE_STRING DestinationString,
    PCUNICODE_STRING SourceString, BOOLEAN AllocateDestinationString)
{
	int i = 0;

	DDKASSERT(KeGetCurrentIrql() <= APC_LEVEL);

	if (AllocateDestinationString) {
		DestinationString->Buffer = DdkAllocUnicodeBuffer(SourceString->Length);
		DestinationString->MaximumLength = SourceString->Length;
	}

	if (DestinationString->MaximumLength < SourceString->Length)
		return STATUS_BUFFER_TOO_SMALL;

	DDKASSERT(DestinationString->Buffer != NULL);

	// convert to uppercase one char at a time
	// NB we don't use _wcsupr_s() as this uses null-termination ...
	int wlen = SourceString->Length / sizeof(WCHAR);
	for (int i = 0; i < wlen; i++) {
		WCHAR ch = SourceString->Buffer[i];
		if (iswlower(ch)) 
			ch = towupper(ch);
		DestinationString->Buffer[i] = ch;
	}

	DestinationString->Length = SourceString->Length;
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS RtlUnicodeStringToInteger(PCUNICODE_STRING String, ULONG Base, PULONG Value)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	*Value = 0;

	if ((Base != 16 && Base != 10 && Base != 8 && Base != 2 && Base != 0) || String->Length == 0)
		return STATUS_INVALID_PARAMETER;
		
	const WCHAR *tail = &String->Buffer[(String->Length - 1)/sizeof(WCHAR)];
	WCHAR *head = String->Buffer;

	while (head <= tail && iswspace(*head)) head++;
	
	const WCHAR sign = (head <= tail) && (*head == L'+' || *head == L'-') ? *head++ : L'+';

	if (Base == 0 && head < tail && *head == '0'){
		Base = (*++head == L'x') ? 16 : (*head == L'o') ? 8 : (*head == L'b') ? 2 : 0;

		if (Base) head++;
	}

	Base = !Base ? 10 : Base;

	for (; head <= tail; head++){
		ULONG digit = (L'0' <= *head && *head <= L'9') ? *head - L'0' :
					  (L'a' <= *head && *head <= L'f') ? *head - L'a' + 10 :
					  (L'A' <= *head && *head <= L'F') ? *head - L'A' + 10 : ULONG_MAX;

		if (digit >= Base) break;
		*Value = (*Value * Base) + digit;
	}

	*Value = (sign == L'-') ? -(LONG)*Value : *Value;

	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS RtlAnsiStringToUnicodeString(PUNICODE_STRING DestinationString,
    PCANSI_STRING SourceString, BOOLEAN AllocateDestinationString)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	ULONG len = RtlAnsiStringToUnicodeSize(SourceString);

	if (!len) return STATUS_INVALID_PARAMETER;

	if (AllocateDestinationString) {
		DestinationString->Buffer = DdkAllocUnicodeBuffer(len);
		DestinationString->MaximumLength = (USHORT)len;
	}

	if (len > DestinationString->MaximumLength + sizeof(UNICODE_NULL))
		return STATUS_BUFFER_TOO_SMALL;

	size_t count = mbstowcs(DestinationString->Buffer,
		SourceString->Buffer, DestinationString->MaximumLength / sizeof(WCHAR));

	DestinationString->Length = (USHORT)(count * sizeof(WCHAR));
	return STATUS_SUCCESS;
}


DDKAPI
BOOLEAN RtlEqualUnicodeString(PCUNICODE_STRING String1, PCUNICODE_STRING String2, BOOLEAN CaseInSensitive)
{
	return RtlCompareUnicodeString(String1, String2, CaseInSensitive) == 0;
}


DDKAPI
VOID RtlFreeUnicodeString(PUNICODE_STRING UnicodeString)
{
	if (UnicodeString->Buffer != NULL)
		ExFreePool(UnicodeString->Buffer);

	UnicodeString->Buffer = NULL;
}


DDKAPI
ULONG RtlxUnicodeStringToAnsiSize(PCUNICODE_STRING UnicodeString)
{
	return WideCharToMultiByte(CP_ACP, 0, UnicodeString->Buffer,
		UnicodeString->Length / sizeof(WCHAR), 0, 0, NULL, NULL) + sizeof(char);
}


DDKAPI
WCHAR RtlUpcaseUnicodeChar(WCHAR SourceCharacter)
{
	DDKASSERT(KeGetCurrentIrql() <= APC_LEVEL);
	return towupper(SourceCharacter);
}


DDKAPI
WCHAR RtlDowncaseUnicodeChar(WCHAR SourceCharacter)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	return towlower(SourceCharacter);
}


DDKAPI
NTSTATUS RtlHashUnicodeString(PCUNICODE_STRING String,
	BOOLEAN CaseInSensitive, ULONG HashAlgorithm, PULONG HashValue)
{
	ULONG hash = 0;

	DDKASSERT(KeGetCurrentIrql() <= APC_LEVEL);

	if (!String || !HashValue)
		return STATUS_INVALID_PARAMETER;

	if (HashAlgorithm != HASH_STRING_ALGORITHM_DEFAULT)
		if (HashAlgorithm != HASH_STRING_ALGORITHM_X65599)
			return STATUS_INVALID_PARAMETER;

	for (USHORT i = 0; i < String->Length / sizeof(WCHAR); i++) {
		WCHAR c = (CaseInSensitive) ? towupper(String->Buffer[i]) : String->Buffer[i];
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	*HashValue = hash;
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS RtlGUIDFromString(PCUNICODE_STRING GuidString, GUID* Guid)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	DDKASSERT(GuidString != NULL);
	DDKASSERT(Guid != NULL);

	if (GuidString->Length != 38 * sizeof(WCHAR))
		return STATUS_INVALID_PARAMETER;

	if (GuidString->Buffer[0] != L'{' || GuidString->Buffer[37] != L'}')
		return STATUS_INVALID_PARAMETER;

	unsigned short tmpI[8];

	int returnValue = swscanf(GuidString->Buffer, L"{%08lX-%04hX-%04hX-%02hX%02hX-%02hX%02hX%02hX%02hX%02hX%02hX}",
		&Guid->Data1, &Guid->Data2, &Guid->Data3,
		&tmpI[0], &tmpI[1], &tmpI[2], &tmpI[3],
		&tmpI[4], &tmpI[5], &tmpI[6], &tmpI[7]);

	if (returnValue != 11)
		return STATUS_INVALID_PARAMETER;

	for (unsigned int i = 0; i < 8; i++)
		Guid->Data4[i] = (unsigned char)tmpI[i];

	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS RtlStringFromGUID(REFGUID Guid, PUNICODE_STRING GuidString)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	DDKASSERT(GuidString != NULL);

	GuidString->Buffer = DdkAllocUnicodeBuffer(39 * sizeof(WCHAR));
	GuidString->MaximumLength = 39 * sizeof(WCHAR);
	GuidString->Length = 38 * sizeof(WCHAR);

	int count = swprintf(GuidString->Buffer, 39, L"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
		Guid.Data1, Guid.Data2, Guid.Data3,
		Guid.Data4[0], Guid.Data4[1], Guid.Data4[2], Guid.Data4[3],
		Guid.Data4[4], Guid.Data4[5], Guid.Data4[6], Guid.Data4[7]);

	if (count != 38){
		RtlFreeUnicodeString(GuidString);
		return STATUS_INVALID_PARAMETER;
	}

	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS RtlUnicodeStringToAnsiString(PANSI_STRING DestinationString,
PCUNICODE_STRING SourceString, BOOLEAN AllocateDestinationString)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	ULONG len = RtlxUnicodeStringToAnsiSize(SourceString);

	if (AllocateDestinationString){
		DestinationString->MaximumLength = (USHORT)len;
		DestinationString->Buffer = DdkAllocAnsiBuffer(DestinationString->MaximumLength);
	}
	else if (len > DestinationString->MaximumLength)
		return STATUS_BUFFER_TOO_SMALL;

	size_t count = wcstombs(DestinationString->Buffer, SourceString->Buffer, DestinationString->MaximumLength);

	DestinationString->Length = (USHORT)count;
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS RtlUnicodeToUTF8N(PCHAR UTF8StringDestination, ULONG UTF8StringMaxByteCount,
	PULONG UTF8StringActualByteCount, PCWCH UnicodeStringSource, ULONG UnicodeStringByteCount)
{
	DDKASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	DDKASSERT(UTF8StringMaxByteCount < INT_MAX);
	DDKASSERT(UnicodeStringByteCount / sizeof(WCHAR) < INT_MAX);

	if ((UTF8StringDestination == NULL && UTF8StringActualByteCount == NULL) || (PCHAR)UnicodeStringSource == UTF8StringDestination)
		return STATUS_INVALID_PARAMETER;

	if (UnicodeStringSource == NULL)
		return STATUS_INVALID_PARAMETER_4;

	if ((UnicodeStringByteCount % sizeof(WCHAR)) != 0)
		return STATUS_INVALID_PARAMETER_5;
		
	if (UTF8StringDestination == NULL)
		UTF8StringMaxByteCount = 0;

	NTSTATUS status = STATUS_SUCCESS;

	if (UnicodeStringByteCount == 0)
		return status;

	ULONG actualByteCount = (ULONG)WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, UnicodeStringSource,
		UnicodeStringByteCount / sizeof(WCHAR), UTF8StringDestination, UTF8StringMaxByteCount, NULL, NULL);
	
	if (actualByteCount == 0){
		DWORD err = GetLastError();
		switch (err) {
			case ERROR_INSUFFICIENT_BUFFER:
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			case ERROR_NO_UNICODE_TRANSLATION:
				status = STATUS_SOME_NOT_MAPPED;
				actualByteCount = (ULONG)WideCharToMultiByte(CP_UTF8, 0, UnicodeStringSource,
					UnicodeStringByteCount / sizeof(WCHAR), UTF8StringDestination, UTF8StringMaxByteCount, NULL, NULL);
				break;
			default:
				DDKASSERT(0); // This should never happen and indicates a programming error;
		}
	}
	
	if (status == STATUS_BUFFER_TOO_SMALL) {
		ULONG tempUnicodeStringByteCount = UnicodeStringByteCount - sizeof(WCHAR);
		// In this situation the Windows function WideCharToMultiByte sets the actual byte count to 0
		// however a truncated string has been written to the buffer, this loop retrieves the number of
		// bytes written.
		do {
			actualByteCount = (ULONG)WideCharToMultiByte(CP_UTF8, 0, UnicodeStringSource,
				tempUnicodeStringByteCount / sizeof(WCHAR), UTF8StringDestination, UTF8StringMaxByteCount, NULL, NULL);
			tempUnicodeStringByteCount -= sizeof(WCHAR);
		} while ((actualByteCount == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER) && tempUnicodeStringByteCount != 0);
	}

	if (UTF8StringActualByteCount)
		*UTF8StringActualByteCount = actualByteCount;

	return status;
}


DDKAPI
NTSTATUS RtlUTF8ToUnicodeN(PWSTR UnicodeStringDestination, ULONG UnicodeStringMaxByteCount,
	PULONG UnicodeStringActualByteCount, PCCH UTF8StringSource, ULONG UTF8StringByteCount)
{	
	DDKASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	DDKASSERT(UTF8StringByteCount < INT_MAX);
	DDKASSERT(UnicodeStringMaxByteCount / sizeof(WCHAR) < INT_MAX);

	if ((UnicodeStringDestination == NULL && UnicodeStringActualByteCount == NULL) || ((PCCH)UnicodeStringDestination == UTF8StringSource))
		return STATUS_INVALID_PARAMETER;

	if (UTF8StringSource == NULL)
		return STATUS_INVALID_PARAMETER_4;
		
	if (UnicodeStringDestination == NULL)
		UnicodeStringMaxByteCount = 0;

	NTSTATUS status = STATUS_SUCCESS;

	if (UTF8StringByteCount == 0)
		return status;

	ULONG actualCharCount = (ULONG)MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, UTF8StringSource,
		UTF8StringByteCount, UnicodeStringDestination, UnicodeStringMaxByteCount / sizeof(WCHAR));

	if (actualCharCount == 0) {
		DWORD err = GetLastError();
		switch (err) {
			case ERROR_INSUFFICIENT_BUFFER:
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			case ERROR_NO_UNICODE_TRANSLATION:
				status = STATUS_SOME_NOT_MAPPED;
				actualCharCount = (ULONG)MultiByteToWideChar(CP_UTF8, 0, UTF8StringSource, UTF8StringByteCount,
					UnicodeStringDestination, UnicodeStringMaxByteCount / sizeof(WCHAR));
				break;
			default:
				DDKASSERT(0); // This should never happen and indicates a programming error
		}
	}

	if (status == STATUS_BUFFER_TOO_SMALL) {
		ULONG tempUtf8StringByteCount = UTF8StringByteCount - sizeof(WCHAR);
		// In this situation the Windows function MultiByteToWideChar sets the actual byte count to 0
		// however a truncated string has been written to the buffer, this loop retrieves the number of
		// bytes written.
		do {
			actualCharCount = (ULONG)MultiByteToWideChar(CP_UTF8, 0, UTF8StringSource,
				tempUtf8StringByteCount, UnicodeStringDestination, UnicodeStringMaxByteCount / sizeof(WCHAR));
			tempUtf8StringByteCount -= sizeof(char);
		} while ((actualCharCount == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER) && tempUtf8StringByteCount != 0);
	}

	if (UnicodeStringActualByteCount)
		*UnicodeStringActualByteCount = actualCharCount * sizeof(WCHAR);

	return status;
}


WCHAR *DdkUnicodeToString(UNICODE_STRING *u, WCHAR remove)
{
	if (!u) return NULL;

	USHORT start = 0;

	if (remove) {
		while (start + sizeof(WCHAR) - 1 < u->Length &&
				u->Buffer[start / sizeof(WCHAR)] == remove)
			start += sizeof(WCHAR);
	}

	char *s = (char *)malloc(u->Length + sizeof(WCHAR) - start);
	if (!s) ddkfail("Unable to allocate memory");

	if (u->Length - start)
		memcpy(s, (char *)u->Buffer + start, u->Length - start);

	memset(s + u->Length - start, 0, sizeof(WCHAR));
	return (WCHAR *)s;
}
