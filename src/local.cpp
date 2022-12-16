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
 * Local Resource Routines.
 */

#include "stdafx.h"


void DdkDeleteDirectory(char *pDir);

static HANDLE TempLock;
static char TempBase[MAX_PATH+1], TempDir[MAX_PATH+1];


void DdkLocalInit()
{
	if (!GetTempPath(sizeof(TempBase), TempBase)
			|| strlen(TempBase) > sizeof(TempBase) - 40)
		ddkfail("Unable to get temporary file path");

	strcat(TempBase, "ddk\\");
	sprintf(TempDir, "%s%x\\", TempBase, GetCurrentProcessId());

	// Cleanup temporary files, holding global mutex

	if (!(TempLock = CreateMutex(NULL, TRUE, "Global\\DDKTempDir")))
		ddkfail("Unable to create temporary file mutex");

	if (GetLastError() == ERROR_SUCCESS)
		DdkDeleteDirectory(TempBase);

	else WaitForSingleObject(TempLock, INFINITE);

	DdkDeleteDirectory(TempDir);

	// Create temporary directory

	CreateDirectory(TempBase, NULL);
	BOOL rc = CreateDirectory(TempDir, NULL);

	ReleaseMutex(TempLock);

	if (!rc)
		ddkfail("Unable to create temporary directory");
}


void DdkLocalDeinit()
{
	if (TempLock == NULL) return;

	WaitForSingleObject(TempLock, INFINITE);
	DdkDeleteDirectory(TempDir);
	RemoveDirectory(TempBase);
	ReleaseMutex(TempLock);
}


static void DdkDeleteDirectory(char *pDir)
{
	char path[MAX_PATH+1];
	WIN32_FIND_DATA find;

	size_t n = snprintf(path, sizeof(path), "%s\\*.*", pDir);
	if (n > sizeof(path) - 1) return;

	HANDLE h = FindFirstFile(path, &find);
	if (h == INVALID_HANDLE_VALUE) return;

	char *cp = strrchr(path, '\\') + 1;

	do {
		if (strcmp(find.cFileName, ".") && strcmp(find.cFileName, "..")) {
			size_t i = snprintf(cp, sizeof(path) - n, "%s", find.cFileName);
			if (i > sizeof(path) - n - 1) continue;

			if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				DdkDeleteDirectory(path);
				RemoveDirectory(path);
			}

			else DeleteFile(path);
		}
	} while (FindNextFile(h, &find));

	FindClose(h);
	RemoveDirectory(pDir);
}


DWORD DdkGetFileDisposition(ULONG Disposition, ULONG *pInfo)
{
	switch (Disposition) {
	case FILE_SUPERSEDE:
		*pInfo = FILE_SUPERSEDED;
		return CREATE_ALWAYS;

	case FILE_CREATE:
		*pInfo = FILE_CREATED;
		return CREATE_NEW;

	case FILE_OPEN:
		*pInfo = FILE_OPENED;
		return OPEN_EXISTING;

	case FILE_OPEN_IF:
		*pInfo = FILE_OPENED;
		return OPEN_ALWAYS;

	case FILE_OVERWRITE:
		*pInfo = FILE_OVERWRITTEN;
		return TRUNCATE_EXISTING;

	case FILE_OVERWRITE_IF:
		*pInfo = FILE_OVERWRITTEN;
		return CREATE_ALWAYS;
	}

	return (DWORD)-1;
}


DWORD DdkGetFileFlags(ULONG CreateOptions)
{
	DWORD flags = 0;

	if (CreateOptions & FILE_WRITE_THROUGH)
		flags |= FILE_FLAG_WRITE_THROUGH;

	if (CreateOptions & FILE_NO_INTERMEDIATE_BUFFERING)
		flags |= FILE_FLAG_NO_BUFFERING;

	if (CreateOptions & FILE_DELETE_ON_CLOSE)
		flags |= FILE_FLAG_DELETE_ON_CLOSE;

	if (CreateOptions & FILE_DIRECTORY_FILE)
		flags |= FILE_FLAG_BACKUP_SEMANTICS;

#if 0
	// TODO: Add asynchronous support
	if ((CreateOptions & FILE_SYNCHRONOUS_IO_ALERT) == 0 &&
			(CreateOptions & FILE_SYNCHRONOUS_IO_NONALERT) == 0)
		flags |= FILE_FLAG_OVERLAPPED;
#endif

	return flags;
}


BOOL DdkGetFileInfo(HANDLE h, ULONG FileInformationClass,
	PVOID buffer, ULONG Length, size_t InfoSize, DWORD *pBytesCopied)
{
	// The DDK file information definitions are not in scope, so use the documented
	// integer numbers. The DDK and user space structures have different names, but
	// are identical. As a sanity check assert that the sizes match.

	BOOL rc = 0;

	switch (FileInformationClass) {
	case 4:	// FileBasicInformation
	{
		DDKASSERT(sizeof(FILE_BASIC_INFO) == InfoSize);
		rc = GetFileInformationByHandleEx(h, FileBasicInfo, buffer, Length);
		break;
	}

	case 5:	// FileStandardInformation
	{
		DDKASSERT(sizeof(FILE_STANDARD_INFO) == InfoSize);
		rc = GetFileInformationByHandleEx(h, FileStandardInfo, buffer, Length);
		break;
	}

	default:
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	if (rc) *pBytesCopied = InfoSize;
	return rc;
}


bool DdkIsFilePath(PUNICODE_STRING path)
{
	WCHAR *wp = path->Buffer;
	WCHAR *ep = wp + path->Length / sizeof(WCHAR);

	while (wp < ep && *wp == L'\\') wp++;
	if (wp + 2 < ep && wcsncmp(wp, L"??\\", 3) == 0) wp += 3;

	return (wp + 1 < ep && wp[1] == L':');
}


void
DdkGetLocalPath(WCHAR *buffer, int len, UNICODE_STRING *path, bool create)
{
	// Remove drive letter and path separator

	WCHAR *wp = path->Buffer;
	WCHAR *ep = wp + path->Length / sizeof(WCHAR);

	while (wp < ep && *wp == L'\\') wp++;
	if (wp + 2 < ep && wcsncmp(wp, L"??\\", 3) == 0) wp += 3;
	if (wp + 1 < ep && wp[1] == L':') wp += 2;
	while (wp < ep && (*wp == L'\\' || *wp == L'/')) wp++;

	// Construct path to local file

	int n = _snwprintf(buffer, len, L"%S%.*s", TempDir, (int)(ep - wp), wp);

	if (n > len - 1) n = len - 1;

	// Create intermediate directories

	for (int i = strlen(TempDir); create && i < n &&
			(i += wcscspn(&buffer[i], L"\\/")) < n; i++) {
		buffer[i] = L'\0';
		CreateDirectoryW(buffer, NULL);
		buffer[i] = L'\\';
	}
}


void
DdkGetLocalPath(char *buffer, size_t len, char *path, char *file, char *suffix)
{
	// Remove drive letter and path separator

	while (*path == '\\') path++;
	if (strncmp(path, "??\\", 3) == 0) path += 3;
	if (*path && path[1] == ':') path += 2;

	while (*path == '\\' || *path == '/') path++;

	size_t n = snprintf(buffer, len, "%s%s%s%s%s", TempDir, path,
		(*file) ? "\\" : "", file, suffix);

	if (n > len - 1) n = len - 1;

	// Create intermediate directories

	for (size_t i = strlen(TempDir); i < n &&
			(i += strcspn(&buffer[i], "\\/")) < n; i++) {
		buffer[i] = 0;
		CreateDirectory(buffer, NULL);
		buffer[i] = '\\';
	}
}
