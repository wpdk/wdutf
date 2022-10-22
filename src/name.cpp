/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Name Handling Routines
 */

#include "stdafx.h"


typedef struct _NAME {
	struct _NAME	*Next;
	POBJECT			Object;
	UNICODE_STRING	Name;
	WCHAR			Buffer[1];
} NAME, *PNAME;


NAME *DdkNameList;
static CRITICAL_SECTION Lock;


void DdkNameInit()
{
	if (!InitializeCriticalSectionAndSpinCount(&Lock, 4000))
		ddkfail("DdkNameInit failed to initialize spinlock");
}


void DdkCreatePath(PUNICODE_STRING Out, PUNICODE_STRING Name, PWCH Dir, PWCH Prefix)
{
	size_t d = (Dir && *Dir && Name->Buffer[0] != '\\') ? wcslen(Dir) + 1 : 0;
	size_t p = (Prefix && *Prefix && Name->Buffer[0] != '\\') ? wcslen(Prefix) : 0;
	PWCH s = DdkAllocUnicodeBuffer(Name->Length + (d + p + 1) * sizeof(WCHAR));
	s[0] = 0;

	if (d) {
		wcscpy(s, Dir);
		if (Dir[wcslen(Dir)-1] != '\\') wcscat(s, L"\\");
	}

	if (p) wcscat(s, Prefix);

	wcsncat(s, Name->Buffer, Name->Length / sizeof(WCHAR));
	RtlInitUnicodeString(Out, s);
}


static POBJECT DdkFindName(PUNICODE_STRING Path, PWCH *pPath = 0)
{
	for (PNAME pEntry = DdkNameList; pEntry; pEntry = pEntry->Next)
		if (RtlCompareUnicodeString(Path, &pEntry->Name, TRUE) == 0)
			if (pEntry->Object->refcount) {
				if (pPath) *pPath = pEntry->Buffer;
				return pEntry->Object;
			}

	return 0;
}


POBJECT DdkCreateName(PUNICODE_STRING Name, POBJECT pObj, PWCH Dir, PWCH *pPath)
{
	DDKASSERT(Name && pObj);
	DDKASSERT(Name->Length >= sizeof(WCHAR));

	UNICODE_STRING u;
	DdkCreatePath(&u, Name, Dir);

	NAME *pEntry = (NAME *)calloc(1, sizeof(NAME) + u.Length);
	if (!pEntry) ddkfail("Unable to create name");

	wcsncpy(pEntry->Buffer, u.Buffer, u.Length / sizeof(WCHAR));
	RtlInitUnicodeString(&pEntry->Name, pEntry->Buffer);
	RtlFreeUnicodeString(&u);

	EnterCriticalSection(&Lock);
	POBJECT pLookup = DdkFindName(&pEntry->Name, pPath);

	if (pLookup) {
		DdkReferenceObject(pLookup);
		DdkDereferenceObject(pObj);
		LeaveCriticalSection(&Lock);
		free(pEntry);
		return pLookup;
	}

	pEntry->Object = pObj;
	pEntry->Next = DdkNameList;
	DdkNameList = pEntry;
	DdkReferenceObject(pObj);
	if (pPath) *pPath = pEntry->Buffer;
	LeaveCriticalSection(&Lock);
	return pObj;
}


BOOLEAN DdkInsertName(PUNICODE_STRING Name, POBJECT pObj, PWCH Dir, PWCH *pPath)
{
	POBJECT pResult = DdkCreateName(Name, pObj, Dir, pPath);

	if (pResult != pObj) {
		DdkDereferenceObject(pResult);
		return FALSE;
	}

	return TRUE;
}


VOID DdkRemoveName(PUNICODE_STRING Name)
{
	EnterCriticalSection(&Lock);
	PNAME *ppEntry = &DdkNameList, *ppNext;

	for (PNAME pEntry; (pEntry = *ppEntry) != 0; ppEntry = ppNext) {
		ppNext = &(*ppEntry)->Next;

		if (RtlCompareUnicodeString(Name, &pEntry->Name, TRUE) == 0) {
			*ppEntry = pEntry->Next;
			ppNext = ppEntry;
			DdkDereferenceObject(pEntry->Object);
			free(pEntry);
		}
	}

	LeaveCriticalSection(&Lock);
}


VOID DdkRemoveObjectName(POBJECT Object)
{
	EnterCriticalSection(&Lock);
	PNAME *ppEntry = &DdkNameList, *ppNext;

	for (PNAME pEntry; (pEntry = *ppEntry) != 0; ppEntry = ppNext) {
		ppNext = &(*ppEntry)->Next;

		if (pEntry->Object == Object) {
			*ppEntry = pEntry->Next;
			ppNext = ppEntry;
			DdkDereferenceObject(pEntry->Object);
			free(pEntry);
		}
	}

	LeaveCriticalSection(&Lock);
}


POBJECT DdkLookupName(PUNICODE_STRING Name, PWCH Dir)
{
	UNICODE_STRING u;
	DdkCreatePath(&u, Name, Dir);
	EnterCriticalSection(&Lock);

	OBJECT *pObj = DdkFindName(&u);
	if (pObj) DdkReferenceObject(pObj);

	LeaveCriticalSection(&Lock);
	RtlFreeUnicodeString(&u);
	return pObj;
}


POBJECT DdkLookupName(PWCH Name, PWCH Dir)
{
	UNICODE_STRING u;
	RtlInitUnicodeString(&u, Name);
	return DdkLookupName(&u, Dir);
}


POBJECT DdkLookupName(char *Name, PWCH Dir)
{
	WCHAR wname[256];
	swprintf(wname, sizeof(wname), L"%S", Name);
	return DdkLookupName(wname, Dir);
}

