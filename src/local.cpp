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


void
DdkGetLocalPath(char *buffer, size_t len, char *path, char *file, char *suffix)
{
	// Remove drive letter and path separator

	if (strncmp(path, "\\??\\", 4) == 0) path += 4;
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
