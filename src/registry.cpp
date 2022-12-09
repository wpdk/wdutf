/*****************************************************************************
 *                                                                           *
 *                        NOTICE                                             *
 *                                                                           *
 *           COPYRIGHT (c) 2022 DATACORE SOFTWARE CORPORATION                *
 *                                                                           *
 *                    ALL RIGHTS RESERVED                                    *
 *                                                                           *
 *                                                                           *
 *    This Computer Program is CONFIDENTIAL and a TRADE SECRET of            *
 *    DATACORE SOFTWARE CORPORATION. Any reproduction of this program        *
 *    without the express written consent of DATACORE SOFTWARE CORPORATION   *
 *    is a violation of the copyright laws and may subject you to criminal   *
 *    prosecution.                                                           *
 *                                                                           *
 *****************************************************************************/


/*
 * Registry Routines.
 */

#include "stdddk.h"
#include <winreg.h>


typedef struct KEY : public OBJECT {
	HKEY			Key;
} KEY, *PKEY;


NTSTATUS DdkQueryEnumerateValues(HKEY key,
	RTL_QUERY_REGISTRY_TABLE *qp, PVOID Context, PVOID Environment);

NTSTATUS DdkQueryValue(HKEY key, RTL_QUERY_REGISTRY_TABLE *qp,
	PVOID Context, PVOID Environment);

NTSTATUS DdkQueryCallback(RTL_QUERY_REGISTRY_TABLE *qp,
	DWORD type, PVOID data, DWORD bytes, PVOID Context, PVOID Environment);

NTSTATUS DdkQueryDirect(RTL_QUERY_REGISTRY_TABLE *qp, DWORD type,
	PVOID data, DWORD bytes, PVOID Context, PVOID Environment);

void DdkGetDefaultValue(RTL_QUERY_REGISTRY_TABLE *qp, DWORD *pType,
	PVOID *pData, DWORD *pBytes);

NTSTATUS DdkGetRegistryObject(PKEY *ppKey,
	POBJECT_ATTRIBUTES ObjectAttributes, ULONG *pDisposition);

void DdkExpandString(PVOID *pString, ULONG *pBytes, PVOID Environment);
void DdkCreateRegistryKey(ULONG RelativeTo, PWSTR Path);
WCHAR *DdkGetRelativePath(ULONG RelativeTo);
bool DdkCheckKeyExists(ULONG RelativeTo, PCWSTR Path, bool parent);
NTSTATUS DdkGetRegistryKey(HKEY *pHkey, ULONG RelativeTo, PCWSTR Path, bool create);
NTSTATUS DdkGetRegistryStatus(LSTATUS rc);


static HKEY hive = NULL;


void DdkRegistryInit()
{
	char path[MAX_PATH+1];

	DdkGetLocalPath(path, sizeof(path), "\\Windows\\System32\\Config", "hive.dat");

	if (RegLoadAppKeyA(path, &hive, KEY_ALL_ACCESS,
			REG_PROCESS_APPKEY, NULL) != ERROR_SUCCESS)
		ddkfail("Unable to create registry hive");

	DdkCreateRegistryKey(RTL_REGISTRY_CONTROL, L"");
	DdkCreateRegistryKey(RTL_REGISTRY_DEVICEMAP, L"");

	DdkCreateRegistryKey(RTL_REGISTRY_SERVICES, L"");
	DdkCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Disk");
	DdkCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Tcpip\\Parameters");

	RtlWriteRegistryValue(RTL_REGISTRY_SERVICES,
		L"Tcpip\\Parameters", L"Hostname", REG_SZ, L"UnitTest", sizeof(WCHAR) * 9);

	DdkCreateRegistryKey(RTL_REGISTRY_WINDOWS_NT, L"");
	DdkCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, L"\\Registry\\User\\.Default");
	DdkCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, L"\\Registry\\User\\CurrentUser");
	DdkCreateRegistryKey(RTL_REGISTRY_ABSOLUTE,
		L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName");

	RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
		L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName",
		L"ComputerName", REG_SZ, L"UnitTest", sizeof(WCHAR) * 9);

	RtlWriteRegistryValue(RTL_REGISTRY_WINDOWS_NT, L"",
		L"SystemRoot", REG_SZ, L"C:\\Windows", sizeof(WCHAR) * 11);

	DWORD major = 10, minor = 0;

	RtlWriteRegistryValue(RTL_REGISTRY_WINDOWS_NT, L"",
		L"CurrentMajorVersionNumber", REG_DWORD, &major, sizeof(major));

	RtlWriteRegistryValue(RTL_REGISTRY_WINDOWS_NT, L"",
		L"CurrentMinorVersionNumber", REG_DWORD, &minor, sizeof(minor));
}


void DdkRegistryDeinit()
{
	if (hive != NULL)
		RegCloseKey(hive);
}


DDKAPI
void DdkCreateRegistryKey(ULONG RelativeTo, PWSTR Path)
{
	HKEY key = NULL;
	DdkGetRegistryKey(&key, RelativeTo, Path, true);

	if (key && key != hive)
		RegCloseKey(key);
}


DDKAPI
void DdkDeleteRegistryKey(ULONG RelativeTo, PWSTR Path)
{
	HKEY key = NULL;
	DdkGetRegistryKey(&key, RelativeTo, Path, false);

	if (key) RegDeleteKeyW(key, L"");

	if ((RelativeTo & RTL_REGISTRY_HANDLE) == 0)
		if (key && key != hive) RegCloseKey(key);
}


DDKAPI
NTSTATUS ZwOpenKey(PHANDLE KeyHandle,
	ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes)
{
	return ZwOpenKeyEx(KeyHandle, DesiredAccess, ObjectAttributes, 0);
}


DDKAPI
NTSTATUS ZwOpenKeyEx(PHANDLE KeyHandle,
	ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, ULONG OpenOptions)
{
	UNREFERENCED_PARAMETER(DesiredAccess);
	UNREFERENCED_PARAMETER(OpenOptions);

	if (!KeyHandle)
		return STATUS_INVALID_PARAMETER;

	PKEY pKey = NULL;
	NTSTATUS status = DdkGetRegistryObject(&pKey, ObjectAttributes, NULL);

	if (NT_SUCCESS(status))
		*KeyHandle = ToHandle(pKey);

	return status;
}


DDKAPI
NTSTATUS ZwCreateKey(PHANDLE KeyHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, ULONG TitleIndex, PUNICODE_STRING Class,
    ULONG CreateOptions, PULONG Disposition)
{
	UNREFERENCED_PARAMETER(DesiredAccess);
	UNREFERENCED_PARAMETER(TitleIndex);
	UNREFERENCED_PARAMETER(Class);
	UNREFERENCED_PARAMETER(CreateOptions);

	if (!KeyHandle)
		return STATUS_INVALID_PARAMETER;

	PKEY pKey = NULL;
	ULONG disposition = 0;

	NTSTATUS status = DdkGetRegistryObject(&pKey, ObjectAttributes, &disposition);

	if (NT_SUCCESS(status)) {
		*KeyHandle = ToHandle(pKey);

		if (Disposition)
			*Disposition = disposition;
	}

	return status;
}


DDKAPI
NTSTATUS ZwQueryValueKey(HANDLE KeyHandle, PUNICODE_STRING ValueName,
	KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, PVOID KeyValueInformation,
    ULONG Length, PULONG ResultLength)
{
	PKEY pKey = (PKEY)FromHandle(KeyHandle);

	if (!pKey || !pKey->Key)
		return STATUS_INVALID_HANDLE;

	WCHAR *name = DdkUnicodeToString(ValueName, L'\\');
	ULONG namelen = wcslen(name);
	DWORD type, datalen;

	LSTATUS rc = RegQueryValueExW(pKey->Key, name, NULL, &type, NULL, &datalen);

	if (rc != ERROR_SUCCESS)
		return DdkGetRegistryStatus(rc);

	switch (KeyValueInformationClass) {
	case KeyValueBasicInformation:
	{
		KEY_VALUE_BASIC_INFORMATION *pInfo = (KEY_VALUE_BASIC_INFORMATION *)KeyValueInformation;

		*ResultLength = (ULONG)((char *)&pInfo->Name[namelen] - (char *)pInfo);

		if (!pInfo) break;
		if (Length < *ResultLength) return STATUS_BUFFER_TOO_SMALL;

		pInfo->TitleIndex = 0;
		pInfo->Type = type;
		pInfo->NameLength = namelen * sizeof(WCHAR);
		memcpy(pInfo->Name, name, pInfo->NameLength);
		break;
	}

	case KeyValueFullInformation:
	{
		KEY_VALUE_FULL_INFORMATION *pInfo = (KEY_VALUE_FULL_INFORMATION *)KeyValueInformation;

		*ResultLength = (ULONG)((char *)&pInfo->Name[namelen] - (char *)pInfo) + datalen;

		if (!pInfo) break;
		if (Length < *ResultLength) return STATUS_BUFFER_TOO_SMALL;

		pInfo->TitleIndex = 0;
		pInfo->Type = type;
		pInfo->NameLength = namelen * sizeof(WCHAR);
		pInfo->DataOffset = (ULONG)((char *)&pInfo->Name[namelen] - (char *)pInfo);
		memcpy(pInfo->Name, name, pInfo->NameLength);

		rc = RegQueryValueExW(pKey->Key, name, NULL, NULL,
			(PBYTE)&pInfo->Name + pInfo->NameLength, &datalen);
			
		pInfo->DataLength = datalen;
		break;
	}

	case KeyValuePartialInformation:
	{
		KEY_VALUE_PARTIAL_INFORMATION *pInfo = (KEY_VALUE_PARTIAL_INFORMATION *)KeyValueInformation;

		*ResultLength = (ULONG)((char *)&pInfo->Data[datalen] - (char *)pInfo);

		if (!pInfo) break;
		if (Length < *ResultLength) return STATUS_BUFFER_TOO_SMALL;

		pInfo->TitleIndex = 0;
		pInfo->Type = type;

		rc = RegQueryValueExW(pKey->Key, name, NULL,
			NULL, (PBYTE)pInfo->Data, &datalen);
			
		pInfo->DataLength = datalen;
		break;
	}

	default:
		return STATUS_INVALID_PARAMETER;
	}

	return DdkGetRegistryStatus(rc);
}


DDKAPI
NTSTATUS ZwEnumerateKey(HANDLE KeyHandle, ULONG Index,
    KEY_INFORMATION_CLASS KeyInformationClass, PVOID KeyInformation,
	ULONG Length, PULONG ResultLength)
{
	PKEY pKey = (PKEY)FromHandle(KeyHandle);

	if (!pKey || !pKey->Key)
		return STATUS_INVALID_HANDLE;

	WCHAR name[1024], classname[1024];
	DWORD namelen = sizeof(name) / sizeof(name[0]);
	DWORD classlen = sizeof(classname) / sizeof(classname[0]);
	FILETIME ftime;

	LSTATUS rc = RegEnumKeyExW(pKey->Key, Index, name, &namelen,
		NULL, classname, &classlen, &ftime);

	if (rc != ERROR_SUCCESS)
		return DdkGetRegistryStatus(rc);

	switch (KeyInformationClass) {
	case KeyBasicInformation:
	{
		KEY_BASIC_INFORMATION *pInfo = (KEY_BASIC_INFORMATION *)KeyInformation;

		*ResultLength = (ULONG)((char *)&pInfo->Name[namelen] - (char *)pInfo);

		if (!pInfo) break;
		if (Length < *ResultLength) return STATUS_BUFFER_TOO_SMALL;

		pInfo->LastWriteTime.HighPart = ftime.dwHighDateTime;
		pInfo->LastWriteTime.LowPart = ftime.dwLowDateTime;
		pInfo->TitleIndex = 0;
		pInfo->NameLength = namelen * sizeof(WCHAR);
		memcpy(pInfo->Name, name, pInfo->NameLength);
		break;
	}

	case KeyNodeInformation:
	{
		KEY_NODE_INFORMATION *pInfo = (KEY_NODE_INFORMATION *)KeyInformation;

		*ResultLength = (ULONG)((char *)&pInfo->Name[namelen] - (char *)pInfo) +
			classlen * sizeof(WCHAR);

		if (!pInfo) break;
		if (Length < *ResultLength) return STATUS_BUFFER_TOO_SMALL;

		pInfo->LastWriteTime.HighPart = ftime.dwHighDateTime;
		pInfo->LastWriteTime.LowPart = ftime.dwLowDateTime;
		pInfo->TitleIndex = 0;
		pInfo->ClassOffset = (ULONG)((char *)&pInfo->Name[namelen] - (char *)pInfo);
		pInfo->ClassLength = classlen * sizeof(WCHAR);
		pInfo->NameLength = namelen * sizeof(WCHAR);
		memcpy(pInfo->Name, name, pInfo->NameLength);
		memcpy(&pInfo->Name[namelen], classname, pInfo->ClassLength);
		break;
	}

	case KeyFullInformation:
	{
		KEY_FULL_INFORMATION *pInfo = (KEY_FULL_INFORMATION *)KeyInformation;
		HKEY key;

		*ResultLength = (ULONG)((char *)&pInfo->Class[classlen] - (char *)pInfo);

		if (!pInfo) break;
		if (Length < *ResultLength) return STATUS_BUFFER_TOO_SMALL;

		rc = RegOpenKeyW(pKey->Key, name, &key);
		if (rc != ERROR_SUCCESS) break;

		pInfo->LastWriteTime.HighPart = ftime.dwHighDateTime;
		pInfo->LastWriteTime.LowPart = ftime.dwLowDateTime;
		pInfo->TitleIndex = 0;
		pInfo->ClassOffset = (ULONG)((char *)&pInfo->Class - (char *)pInfo);
		pInfo->ClassLength = classlen * sizeof(WCHAR);
		memcpy(&pInfo->Class, classname, pInfo->ClassLength);

		rc = RegQueryInfoKeyW(key, NULL, NULL, NULL, &pInfo->SubKeys,
			&pInfo->MaxNameLen, &pInfo->MaxClassLen, &pInfo->Values,
			&pInfo->MaxValueNameLen, &pInfo->MaxValueDataLen, NULL, NULL);

		RegCloseKey(key);
		break;
	}

	default:
		return STATUS_INVALID_PARAMETER;
	}

	return DdkGetRegistryStatus(rc);
}


DDKAPI
NTSTATUS ZwEnumerateValueKey(HANDLE KeyHandle, ULONG Index,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, PVOID KeyValueInformation,
	ULONG Length, PULONG ResultLength)
{
	PKEY pKey = (PKEY)FromHandle(KeyHandle);

	if (!pKey || !pKey->Key)
		return STATUS_INVALID_HANDLE;

	WCHAR name[1024];
	DWORD namelen = sizeof(name) / sizeof(name[0]);
	DWORD type, datalen;

	LSTATUS rc = RegEnumValueW(pKey->Key, Index, name, &namelen,
		NULL, &type, NULL, &datalen);

	if (rc != ERROR_SUCCESS)
		return DdkGetRegistryStatus(rc);

	switch (KeyValueInformationClass) {
	case KeyValueBasicInformation:
	{
		KEY_VALUE_BASIC_INFORMATION *pInfo = (KEY_VALUE_BASIC_INFORMATION *)KeyValueInformation;

		*ResultLength = (ULONG)((char *)&pInfo->Name[namelen] - (char *)pInfo);

		if (!pInfo) break;
		if (Length < *ResultLength) return STATUS_BUFFER_TOO_SMALL;

		pInfo->TitleIndex = 0;
		pInfo->Type = type;
		pInfo->NameLength = namelen * sizeof(WCHAR);
		memcpy(pInfo->Name, name, pInfo->NameLength);
		break;
	}

	case KeyValueFullInformation:
	{
		KEY_VALUE_FULL_INFORMATION *pInfo = (KEY_VALUE_FULL_INFORMATION *)KeyValueInformation;

		*ResultLength = (ULONG)((char *)&pInfo->Name[namelen] - (char *)pInfo) + datalen;

		if (!pInfo) break;
		if (Length < *ResultLength) return STATUS_BUFFER_TOO_SMALL;

		pInfo->TitleIndex = 0;
		pInfo->Type = type;
		pInfo->NameLength = namelen * sizeof(WCHAR);
		pInfo->DataOffset = (ULONG)((char *)&pInfo->Name[namelen] - (char *)pInfo);
		memcpy(pInfo->Name, name, pInfo->NameLength);

		namelen = sizeof(name) / sizeof(name[0]);
		rc = RegEnumValueW(pKey->Key, Index, name, &namelen, NULL,
			NULL, (PBYTE)&pInfo->Name + pInfo->NameLength, &datalen);
			
		pInfo->DataLength = datalen;
		break;
	}

	case KeyValuePartialInformation:
	{
		KEY_VALUE_PARTIAL_INFORMATION *pInfo = (KEY_VALUE_PARTIAL_INFORMATION *)KeyValueInformation;

		*ResultLength = (ULONG)((char *)&pInfo->Data[datalen] - (char *)pInfo);

		if (!pInfo) break;
		if (Length < *ResultLength) return STATUS_BUFFER_TOO_SMALL;

		pInfo->TitleIndex = 0;
		pInfo->Type = type;

		namelen = sizeof(name) / sizeof(name[0]);
		rc = RegEnumValueW(pKey->Key, Index, name, &namelen, NULL,
			NULL, (PBYTE)pInfo->Data, &datalen);
			
		pInfo->DataLength = datalen;
		break;
	}

	default:
		return STATUS_INVALID_PARAMETER;
	}

	return DdkGetRegistryStatus(rc);
}


DDKAPI
NTSTATUS ZwSetValueKey(HANDLE KeyHandle, PUNICODE_STRING ValueName,
    ULONG TitleIndex, ULONG Type, PVOID Data, ULONG DataSize)
{
	UNREFERENCED_PARAMETER(TitleIndex);

	PKEY pKey = (PKEY)FromHandle(KeyHandle);

	if (!pKey || !pKey->Key)
		return STATUS_INVALID_HANDLE;

	WCHAR *name = DdkUnicodeToString(ValueName, L'\\');

	LSTATUS rc = RegSetValueExW(pKey->Key, name, 0, Type,
			(const BYTE *)Data, DataSize);

	free(name);
	return DdkGetRegistryStatus(rc);
}


DDKAPI
NTSTATUS ZwFlushKey(HANDLE KeyHandle)
{
	PKEY pKey = (PKEY)FromHandle(KeyHandle);

	if (!pKey || !pKey->Key)
		return STATUS_INVALID_HANDLE;

	LSTATUS rc = RegFlushKey(pKey->Key);

	return DdkGetRegistryStatus(rc);
}


DDKAPI
NTSTATUS ZwDeleteValueKey(HANDLE KeyHandle, PUNICODE_STRING ValueName)
{
	PKEY pKey = (PKEY)FromHandle(KeyHandle);

	if (!pKey || !pKey->Key)
		return STATUS_INVALID_HANDLE;

	WCHAR *name =  DdkUnicodeToString(ValueName, L'\\');
	LSTATUS rc = RegDeleteValueW(pKey->Key, name);

	free(name);
	return DdkGetRegistryStatus(rc);
}


DDKAPI
NTSTATUS ZwDeleteKey(HANDLE KeyHandle)
{
	PKEY pKey = (PKEY)FromHandle(KeyHandle);

	if (!pKey || !pKey->Key)
		return STATUS_INVALID_HANDLE;

	LSTATUS rc = RegDeleteKeyW(pKey->Key, L"");

	return DdkGetRegistryStatus(rc);
}


DDKAPI
NTSTATUS RtlCreateRegistryKey(ULONG RelativeTo, PWSTR Path)
{
	if (!DdkCheckKeyExists(RelativeTo, Path, true))
		return STATUS_OBJECT_NAME_NOT_FOUND;

	HKEY key = NULL;
	NTSTATUS status = DdkGetRegistryKey(&key, RelativeTo, Path, true);

	if (!NT_SUCCESS(status)) return status;

	if (key != hive)
		RegCloseKey(key);

	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS RtlCheckRegistryKey(ULONG RelativeTo, PWSTR Path)
{
	return DdkCheckKeyExists(RelativeTo, Path, false) ?
		STATUS_SUCCESS : STATUS_OBJECT_NAME_NOT_FOUND;
}


DDKAPI
NTSTATUS RtlWriteRegistryValue(ULONG RelativeTo, PCWSTR Path,
	PCWSTR ValueName, ULONG ValueType, PVOID ValueData, ULONG ValueLength)
{
	HKEY key = NULL;
	NTSTATUS status = DdkGetRegistryKey(&key, RelativeTo, Path, false);

	if (!NT_SUCCESS(status)) return status;

	LSTATUS rc = RegSetValueExW(key, ValueName, 0, ValueType,
			(const BYTE *)ValueData, ValueLength);

	if ((RelativeTo & RTL_REGISTRY_HANDLE) == 0)
		if (key != hive) RegCloseKey(key);

	return DdkGetRegistryStatus(rc);
}


#undef RtlQueryRegistryValues
DDKAPI
NTSTATUS RtlQueryRegistryValues(ULONG RelativeTo, PCWSTR Path,
	PRTL_QUERY_REGISTRY_TABLE QueryTable, PVOID Context, PVOID Environment)
{
	HKEY base = NULL;
	NTSTATUS status = DdkGetRegistryKey(&base, RelativeTo, Path, false);

	if (!NT_SUCCESS(status)) return status;

	HKEY key = base;

	for (RTL_QUERY_REGISTRY_TABLE *qp = QueryTable; NT_SUCCESS(status); qp++) {
		if (qp->Flags & (RTL_QUERY_REGISTRY_SUBKEY | RTL_QUERY_REGISTRY_TOPKEY)) {
			if (key && key != base)
				RegCloseKey(key);
			key = base;
		}

		if (qp->Flags & RTL_QUERY_REGISTRY_SUBKEY) {
			if (qp->Name == NULL) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			LSTATUS rc = RegOpenKeyW(base, qp->Name, &key);

			if (rc != ERROR_SUCCESS) {
				status = DdkGetRegistryStatus(rc);
				break;
			}

			if (!qp->QueryRoutine) continue;
		}

		if (qp->Name == NULL || (qp->Flags & RTL_QUERY_REGISTRY_SUBKEY)) {
			if (!qp->QueryRoutine) break;

			if (qp->Flags & RTL_QUERY_REGISTRY_DIRECT)
				return STATUS_INVALID_PARAMETER;
			
			if (qp->Flags & RTL_QUERY_REGISTRY_NOVALUE) {
				status = qp->QueryRoutine(qp->Name,
					REG_NONE, NULL, 0, Context, qp->EntryContext);
				continue;
			}

			status = DdkQueryEnumerateValues(key, qp, Context, Environment);
			continue;
		}

		status = DdkQueryValue(key, qp, Context, Environment);

		if (NT_SUCCESS(status) && (qp->Flags & RTL_QUERY_REGISTRY_DELETE))
			RegDeleteKeyValueW(key, L"", qp->Name);
	}

	if (key && key != base)
		RegCloseKey(key);

	if ((RelativeTo & RTL_REGISTRY_HANDLE) == 0) 
		if (base && base != hive) RegCloseKey(base);

	return status;
}


static
NTSTATUS DdkQueryEnumerateValues(HKEY key,
	RTL_QUERY_REGISTRY_TABLE *qp, PVOID Context, PVOID Environment)
{
	RTL_QUERY_REGISTRY_TABLE entry = *qp;
	NTSTATUS status = STATUS_SUCCESS;
	DWORD index = 0, count = 0;

	for (; NT_SUCCESS(status); index++, count++) {
		WCHAR name[1024];
		DWORD len = sizeof(name) / sizeof(name[0]);

		LSTATUS rc = RegEnumValueW(key, index, name, &len, NULL, NULL, NULL, NULL);

		if (rc != ERROR_SUCCESS) {
			if (rc != ERROR_NO_MORE_ITEMS)
				status = DdkGetRegistryStatus(rc);
			break;
		}

		entry.Name = name;
		status = DdkQueryValue(key, &entry, Context, Environment);

		if (NT_SUCCESS(status) && (qp->Flags & RTL_QUERY_REGISTRY_DELETE))
			if (RegDeleteKeyValueW(key, L"", name) == ERROR_SUCCESS)
				index--;
	}

	if ((qp->Flags & RTL_QUERY_REGISTRY_REQUIRED) && !count)
		return STATUS_OBJECT_NAME_NOT_FOUND;

	return status;
}


static
NTSTATUS DdkQueryValue(HKEY key,
	RTL_QUERY_REGISTRY_TABLE *qp, PVOID Context, PVOID Environment)
{
	DWORD nullbytes = sizeof(WCHAR) * 3;
	DWORD len = 0, bytes = 0, type;
	BYTE *buffer = NULL;
	LSTATUS rc;

	// Validate parameters

	if (!(qp->Flags & RTL_QUERY_REGISTRY_DIRECT) == !qp->QueryRoutine)
			return STATUS_INVALID_PARAMETER;

	// Read the registry value, ensuring the buffer is always null terminated

	do {
		len += PAGE_SIZE;
		if (bytes > len) len = bytes + nullbytes;

		if (!(buffer = (BYTE *)realloc(buffer, len)))
			return STATUS_INSUFFICIENT_RESOURCES;

		bytes = len - nullbytes;
		rc = RegQueryValueExW(key, qp->Name, 0, &type, buffer, &bytes);

		if (rc == ERROR_SUCCESS)
			memset(buffer + bytes, 0, nullbytes);
	} while (rc == ERROR_MORE_DATA);

	NTSTATUS status = STATUS_SUCCESS;
	VOID *data = buffer;

	// Return value through QueryRoutine

	if ((qp->Flags & RTL_QUERY_REGISTRY_DIRECT) == 0) {
		status = (rc != ERROR_SUCCESS) ? DdkGetRegistryStatus(rc) :
			DdkQueryCallback(qp, type, data, bytes, Context, Environment);

		free(buffer);
		return status;
	}

	// Return value as Direct data

	if (rc == ERROR_SUCCESS && (qp->Flags & RTL_QUERY_REGISTRY_TYPECHECK))
		if (type != ((qp->DefaultType >> RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) & 0xff)) {
			free(buffer);
			return STATUS_OBJECT_TYPE_MISMATCH;
		}

	if (rc == ERROR_FILE_NOT_FOUND) {
		if (qp->DefaultType == REG_NONE) {
			free(buffer);
	
			if (qp->Flags & RTL_QUERY_REGISTRY_REQUIRED)
				return STATUS_OBJECT_NAME_NOT_FOUND;

			return STATUS_SUCCESS;
		}

		DdkGetDefaultValue(qp, &type, &data, &bytes);
		rc = ERROR_SUCCESS;
	}

	status = (rc != ERROR_SUCCESS) ? DdkGetRegistryStatus(rc) :
		DdkQueryDirect(qp, type, data, bytes, Context, Environment);

	free(buffer);
	return status;
}


static
NTSTATUS DdkQueryCallback(RTL_QUERY_REGISTRY_TABLE *qp, DWORD type,
	PVOID data, DWORD bytes, PVOID Context, PVOID Environment)
{
	// Handle a callback where expansion is not required

	if ((qp->Flags & RTL_QUERY_REGISTRY_NOEXPAND) ||
			(type != REG_MULTI_SZ && type != REG_EXPAND_SZ))
		return qp->QueryRoutine(qp->Name,
			type, data, bytes, Context, qp->EntryContext);

	// Expand the buffer into individual strings

	NTSTATUS status = STATUS_SUCCESS;
	WCHAR *dp = (WCHAR *)data;
	ULONG i = 0;

	do {
		ULONG base = i;
		while (dp[i++] != L'\0') ;

		if (i == base + 1) break;

		PVOID buffer = (WCHAR *)data + base;
		ULONG len = (i - base - 1) * sizeof(WCHAR);

		// Expand environment variables

		if (type == REG_EXPAND_SZ)
			DdkExpandString(&buffer, &len, Environment);

		status = qp->QueryRoutine(qp->Name, REG_SZ,
			(WCHAR *)buffer, len, Context, qp->EntryContext);

		// Free any buffer from string expansion

		if (buffer != (WCHAR *)data + base)
			free(buffer);
	} while (NT_SUCCESS(status) && type == REG_MULTI_SZ);

	return status;
}


NTSTATUS DdkQueryDirect(RTL_QUERY_REGISTRY_TABLE *qp, DWORD type,
	PVOID data, DWORD bytes, PVOID Context, PVOID Environment)
{
	DWORD nullbytes = sizeof(WCHAR) * 3;
	NTSTATUS status = STATUS_SUCCESS;
	PVOID initialdata = data;

	if (qp->EntryContext == NULL)
		return STATUS_INVALID_PARAMETER;

	switch (type) {
	case REG_MULTI_SZ:
	case REG_EXPAND_SZ:

		if ((qp->Flags & RTL_QUERY_REGISTRY_NOEXPAND) == 0) {
			if (type == REG_MULTI_SZ) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			DdkExpandString(&data, &bytes, Environment);
		}
		/* FallThru */

	case REG_SZ:
	{
		UNICODE_STRING *u = (UNICODE_STRING *)qp->EntryContext;

		if (bytes >= sizeof(WCHAR) && memcmp((char *)data +
				bytes - sizeof(WCHAR), L"\0", sizeof(WCHAR)) == 0)
			bytes -= sizeof(WCHAR);

		if (!u->Buffer) {
			u->Buffer = DdkAllocUnicodeBuffer(bytes + nullbytes);
			u->MaximumLength = (USHORT)(bytes + nullbytes);
		}

		u->Length = (USHORT)min(u->MaximumLength, bytes);
		memcpy(u->Buffer, data, min(u->MaximumLength, bytes));

		memset((char *)u->Buffer + u->Length, 0,
			min(u->MaximumLength - u->Length, (USHORT)nullbytes));
		break;
	}

	case REG_DWORD:
	case REG_QWORD:
	case REG_BINARY:
	case REG_DWORD_BIG_ENDIAN:
	{
		if (bytes <= sizeof(ULONG)) {
			memcpy(qp->EntryContext, data, bytes);
			break;
		}

		LONG size = *(LONG *)qp->EntryContext;

		if (size < 0) {
			memcpy(qp->EntryContext, data, min(-size, (LONG)bytes));
			break;
		}

		struct { ULONG size, type; } v;

		v.size = bytes;
		v.type = type;
		memcpy(qp->EntryContext, &v, min(size, sizeof(v)));

		if (size > sizeof(v))
			memcpy((char *)qp->EntryContext + sizeof(v), data,
				min(size - sizeof(v), bytes));
		break;
	}

	default:
		ddkfail("Unsupported type for registry value");
		break;
	}

	// Free any buffer from string expansion

	if (data != initialdata)
		free(data);

	return status;
}


static
void DdkGetDefaultValue(RTL_QUERY_REGISTRY_TABLE *qp, DWORD *pType,
	PVOID *pData, DWORD *pBytes)
{
	ULONG type = (qp->DefaultType & 0xff);

	*pType = type;
	*pData = qp->DefaultData;
	*pBytes = qp->DefaultLength;

	if (*pBytes || (type != REG_SZ &&
			type != REG_EXPAND_SZ && type != REG_MULTI_SZ))
		return;

	// Calculate default length

	WCHAR *dp = (WCHAR *)qp->DefaultData;
	ULONG i = 0;

	do while (dp[i++] != L'\0') ;
	while (type == REG_MULTI_SZ && (i < 2 || dp[i-1] || dp[i-2]));

	*pBytes = (DWORD)(i * sizeof(WCHAR));
}


static
void DdkExpandString(PVOID *pString, ULONG *pBytes, PVOID Environment)
{
	UNREFERENCED_PARAMETER(pBytes);

	if (Environment != NULL)
		ddkfail("Unable to expand registry values");
}


DDKAPI
NTSTATUS RtlDeleteRegistryValue(ULONG RelativeTo, PCWSTR Path, PCWSTR ValueName)
{
	HKEY key = NULL;
	NTSTATUS status = DdkGetRegistryKey(&key, RelativeTo, Path, false);

	if (!NT_SUCCESS(status)) {
		if (status == STATUS_OBJECT_NAME_NOT_FOUND)
			return STATUS_SUCCESS;

		return status;
	}

	DWORD rc = RegDeleteValueW(key, ValueName);

	if ((RelativeTo & RTL_REGISTRY_HANDLE) == 0)
		if (key != hive) RegCloseKey(key);

	return DdkGetRegistryStatus(rc);
}


static bool
DdkCheckKeyExists(ULONG RelativeTo, PCWSTR Path, bool parent)
{
	if (RelativeTo & RTL_REGISTRY_HANDLE)
		return true;

	WCHAR *s = (WCHAR *)Path, *cp;

	if (parent && (s = wcsdup(Path))) {
		if ((cp = wcsrchr(s, L'\\'))) *cp = L'\0';
		else *s = L'\0';
	}

	HKEY key = NULL;
	if (s) DdkGetRegistryKey(&key, RelativeTo, s, false);

	if (key && key != hive)
		RegCloseKey(key);

	if (s && s != Path)
		free(s);

	return (key != NULL);
}


static
NTSTATUS DdkGetRegistryObject(PKEY *ppKey,
	POBJECT_ATTRIBUTES ObjectAttributes, ULONG *pDisposition)
{
	if (!ObjectAttributes || !ObjectAttributes->ObjectName->Buffer)
		return STATUS_INVALID_PARAMETER;

	HKEY base = hive;

	if (ObjectAttributes->RootDirectory) {
		PKEY pKey = (PKEY)FromHandle((HANDLE)ObjectAttributes->RootDirectory);

		if (!pKey || !pKey->Key)
			return STATUS_INVALID_HANDLE;

		base = pKey->Key;
	}

	WCHAR *path =  DdkUnicodeToString(ObjectAttributes->ObjectName, L'\\');
	LSTATUS rc = ERROR_SUCCESS;
	HKEY key = base;

	if (*path)
		rc = (!pDisposition) ? RegOpenKeyW(base, path, &key) :
			RegCreateKeyExW(base, path, NULL, NULL, 0, KEY_ALL_ACCESS,
				NULL, &key, pDisposition);

	if (rc != ERROR_SUCCESS) {
		free(path);
		return DdkGetRegistryStatus(rc);
	}

	free(path);

	PKEY pKey = (KEY *)DdkAllocObject(sizeof(KEY), KeyType, true);

	if (!pKey) {
		if (key != base)
			RegCloseKey(key);

		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pKey->Key = key;
	*ppKey = pKey;
	return STATUS_SUCCESS;
}


static
NTSTATUS DdkGetRegistryKey(HKEY *pHkey,
	ULONG RelativeTo, PCWSTR Path, bool create)
{
	*pHkey = NULL;

	if (RelativeTo & RTL_REGISTRY_HANDLE) {
		if (create) return STATUS_INVALID_PARAMETER;

		PKEY pKey = (PKEY)FromHandle((HANDLE)Path);

		if (!pKey || !pKey->Key)
			return STATUS_INVALID_HANDLE;

		*pHkey = pKey->Key;
		return STATUS_SUCCESS;
	}

	WCHAR *rel = DdkGetRelativePath(RelativeTo);
	HKEY base = hive;

	while (*rel == '\\') rel++;
	while (*Path == '\\') Path++;

	auto OpenKey = (create) ? RegCreateKeyW : RegOpenKeyW;

	if (*rel) {
		LSTATUS rc = OpenKey(base, rel, &base);

		if (rc != ERROR_SUCCESS)
			return DdkGetRegistryStatus(rc);
	}

	NTSTATUS status = STATUS_SUCCESS;
	HKEY key = base;

	if (*Path) {
		LSTATUS rc = OpenKey(base, Path, &key);

		if (rc != ERROR_SUCCESS) {
			status = DdkGetRegistryStatus(rc);
			key = NULL;
		}
	}

	if (base != key && base != hive)
		RegCloseKey(base);

	*pHkey = key;
	return status;
}


static WCHAR *
DdkGetRelativePath(ULONG RelativeTo)
{
	switch (RelativeTo & ~RTL_REGISTRY_OPTIONAL) {
	case RTL_REGISTRY_ABSOLUTE:
		return L"";

	case RTL_REGISTRY_CONTROL:
		return L"\\Registry\\Machine\\System\\CurrentControlSet\\Control";

	case RTL_REGISTRY_DEVICEMAP:
		return L"\\Registry\\Machine\\Hardware\\DeviceMap";

	case RTL_REGISTRY_SERVICES:
		return L"\\Registry\\Machine\\System\\CurrentControlSet\\Services";

	case RTL_REGISTRY_USER:
		return PsIsSystemThread(DdkGetCurrentThread())
			? L"\\Registry\\User\\CurrentUser" : L"\\Registry\\User\\.Default";

	case RTL_REGISTRY_WINDOWS_NT:
		return L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion";
	}

	ddkfail("Unknown RTL_REGISTRY value");
	return L"";
}


void DdkFreeKeyObject(OBJECT *pObj)
{
	PKEY pKey = (PKEY)ToPointer(pObj);

	if (pKey->Key && pKey->Key != hive)
		RegCloseKey(pKey->Key);

	pKey->Key = NULL;
}


static
NTSTATUS DdkGetRegistryStatus(LSTATUS rc)
{
	switch (rc) {
	case ERROR_SUCCESS:
		return STATUS_SUCCESS;

	case ERROR_ACCESS_DENIED:
		return STATUS_ACCESS_DENIED;

	case ERROR_NOT_ENOUGH_MEMORY:
		return STATUS_INSUFFICIENT_RESOURCES;

	case ERROR_INVALID_HANDLE:
		return STATUS_INVALID_HANDLE;

	case ERROR_FILE_NOT_FOUND:
		return STATUS_OBJECT_NAME_NOT_FOUND;

	case ERROR_MORE_DATA:
		return STATUS_BUFFER_TOO_SMALL;

	case ERROR_NO_MORE_ITEMS:
		return STATUS_NO_MORE_ENTRIES;
	}

	return STATUS_UNSUCCESSFUL;
}