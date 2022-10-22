/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Load Routines.
 */

#include "stdafx.h"


typedef struct _IMAGE {
	struct _IMAGE *Next;
	HMODULE h;
	PIMAGE_NT_HEADERS64 Header;
	PDRIVER_INITIALIZE DriverEntry;
	char *Data;
	char Name[1];
} IMAGE, *PIMAGE;

IMAGE *DdkImageList;


void DdkSaveImageData(PIMAGE pImage);
void DdkRestoreImageData(PIMAGE pImage);
char *DdkReadImage(char *pPath, DWORD *pSize);
void DdkUpdateImage(char *pBuffer, DWORD size, DWORD *pEntry);
bool DdkWriteImage(char *pBuffer, char *pLoad, DWORD size);
bool DdkGetWriteTime(char *pPath, FILETIME *pTime);
char *DdkGetTempDir();

static CRITICAL_SECTION Lock;


void DdkLoadInit()
{
	if (!InitializeCriticalSectionAndSpinCount(&Lock, 4000))
		ddkfail("DdkLoadInit failed to initialize lock");
}


void DdkLoadLock()
{
	EnterCriticalSection(&Lock);
}


void DdkLoadUnlock()
{
	LeaveCriticalSection(&Lock);
}


PIMAGE DdkLoadImage(char *pPath, char *pName)
{
	char tmp[MAX_PATH+1];
	DWORD size, entry;

	for (PIMAGE pEntry = DdkImageList; pEntry; pEntry = pEntry->Next)
		if (!stricmp(pEntry->Name, pName)) return pEntry;

	char *pBuffer = DdkReadImage(pPath, &size);

	if (!pBuffer)
		ddkfail("Unable to read driver file");

	DdkUpdateImage(pBuffer, size, &entry);
	sprintf(tmp, "%s%s.sys", DdkGetTempDir(), pName);

	if (!DdkWriteImage(pBuffer, tmp, size))
		ddkfail("Unable to write driver file");

	free(pBuffer);

	HMODULE h = LoadLibrary(tmp);
	if (!h) ddkfail("Unable to load driver");

	PIMAGE p = (PIMAGE)calloc(1, sizeof(IMAGE) + strlen(pName));
	if (!p) ddkfail("Unable to create image");

	p->Header = (PIMAGE_NT_HEADERS64)((ULONG_PTR)h + ((PIMAGE_DOS_HEADER)h)->e_lfanew);
	p->DriverEntry = (PDRIVER_INITIALIZE)((ULONG_PTR)h + (ULONG_PTR)entry);
	p->Next = DdkImageList;
	p->h = h;

	strcpy(p->Name, pName);
	DdkSaveImageData(p);
	DdkImageList = p;
	return p;
}


static bool isSaveData(PIMAGE_SECTION_HEADER p)
{
	return (p->Characteristics & (IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_CNT_UNINITIALIZED_DATA))
		&& (p->Characteristics & IMAGE_SCN_MEM_WRITE);
}


static void DdkSaveImageData(PIMAGE pImage)
{
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pImage->Header);
	DWORD len = 0;

	for (int i = 0; i < pImage->Header->FileHeader.NumberOfSections; i++)
		if (isSaveData(&pSection[i])) len += pSection[i].Misc.VirtualSize;

	if (!len) return;

	pImage->Data = (char *)malloc(len);
	char *cp = pImage->Data;

	if (!cp) ddkfail("Unable to save image data");

	for (int i = 0; i < pImage->Header->FileHeader.NumberOfSections; i++)
		if (isSaveData(&pSection[i])) {
			memcpy(cp, (char *)(pImage->h) + pSection[i].VirtualAddress, pSection[i].Misc.VirtualSize);
			cp += pSection[i].Misc.VirtualSize;
		}
}


static void DdkRestoreImageData(PIMAGE pImage)
{
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pImage->Header);
	char *cp = pImage->Data;

	if (!cp) return;

	for (int i = 0; i < pImage->Header->FileHeader.NumberOfSections; i++)
		if (isSaveData(&pSection[i])) {
			memcpy((char *)(pImage->h) + pSection[i].VirtualAddress, cp, pSection[i].Misc.VirtualSize);
			cp += pSection[i].Misc.VirtualSize;
		}
}


static char *DdkReadImage(char *pPath, DWORD *pSize)
{
	HANDLE h = CreateFile(pPath, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h == INVALID_HANDLE_VALUE) return 0;

	BY_HANDLE_FILE_INFORMATION info;

	if (!GetFileInformationByHandle(h, &info) || info.nFileSizeHigh) {
		CloseHandle(h);
		return 0;
	}

	char *pBuffer = (char *)malloc(info.nFileSizeLow);

	if (!pBuffer) {
		CloseHandle(h);
		return 0;
	}

	if (!ReadFile(h, pBuffer, info.nFileSizeLow, pSize, NULL)
			|| *pSize != info.nFileSizeLow) {
		CloseHandle(h);
		free(pBuffer);
		return 0;
	}

	CloseHandle(h);
	return pBuffer;
}


static void DdkUpdateImage(char *pBuffer, DWORD size, DWORD *pEntry)
{
	// Set DLL flag

	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pBuffer;
	PIMAGE_NT_HEADERS64 pHdr = (PIMAGE_NT_HEADERS64)(&pBuffer[pDos->e_lfanew]);

	pHdr->FileHeader.Characteristics |= IMAGE_FILE_DLL;

	// Disable DriverEntry

	*pEntry = pHdr->OptionalHeader.AddressOfEntryPoint;
	pHdr->OptionalHeader.AddressOfEntryPoint = 0;

	// Link against ddk.dll

	for (DWORD i = 0; i < size; i++) {
		if (!strnicmp(&pBuffer[i], "ntoskrnl.exe", 12))
			strcpy(&pBuffer[i], "ddk.dll");
		
		else if (!strnicmp(&pBuffer[i], "hal.dll", 7))
			strcpy(&pBuffer[i], "ddk.dll");

		else if (!strnicmp(&pBuffer[i], "wmilib.sys", 10))
			strcpy(&pBuffer[i], "ddk.dll");
	}

	// Edit references to USER_SHARED_DATA

	unsigned char from[] = { 0x00, 0x00, 0x80, 0xf7, 0xff, 0xff };
	unsigned char to[] = { 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00 };

	for (DWORD i = 0; i < size; i++)
		if (!memcmp(&pBuffer[i], from, sizeof(from)))
			if (i > 1 && (unsigned char)pBuffer[i-1] < 0x80)
				memcpy(&pBuffer[i], to, sizeof(to));
}


static bool DdkWriteImage(char *pBuffer, char *pLoad, DWORD size)
{
	HANDLE h = CreateFile(pLoad, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h == INVALID_HANDLE_VALUE) return false;

	DWORD bytesWritten;

	if (!WriteFile(h, pBuffer, size, &bytesWritten, NULL)
			|| bytesWritten != size) {
		CloseHandle(h);
		return false;
	}

	CloseHandle(h);
	return true;
}


VOID DdkUnloadImage(PIMAGE pImage)
{
	HMODULE h;

	FreeLibrary(pImage->h);

	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)pImage->h, &h)) {
		DdkRestoreImageData(pImage);
		return;
	}

	for (PIMAGE *ppEntry = &DdkImageList; *ppEntry; ppEntry = &(*ppEntry)->Next)
		if (*ppEntry == pImage) {
			*ppEntry = pImage->Next;
			break;
		}

	if (pImage->Data) free(pImage->Data);
	free(pImage);
}


PDRIVER_INITIALIZE DdkGetImageEntry(PIMAGE pImage)
{
	return pImage->DriverEntry;
}


void DdkGetDriverName(char *pFile, char *np, wchar_t *wp)
{
	char *cp = strrchr(pFile, '\\');
	if (!cp) cp = pFile;

	strcpy(np, cp);
	cp = strrchr(np, '.');
	if (cp) *cp = 0;

	do *wp++ = *np; while (*np++);
}


void DdkGetDriverPath(char *pFile, char *pPath, char *pName)
{
	char *cp = strchr(pFile, '\\');

	if (cp) {
		strcpy(pPath, pFile);
		cp = strrchr(pPath, '\\');
		sprintf(cp+1, "%s.sys", pName);
		return;
	}

	HMODULE ddk = LoadLibrary("ddk.dll");

	if (ddk && GetModuleFileName(ddk, pPath, MAX_PATH+1)) {
		if ((cp = strrchr(pPath, '\\')) != NULL) *cp = 0;

		char *dp = strrchr(pPath, '\\');
		dp = (dp) ? dp + 1 : (cp && cp > pPath) ? pPath : 0;

		sprintf((cp ? cp : pPath), (cp ? "\\%s.sys" : "%s.sys"), pName);

		if (dp) {
			char up[MAX_PATH+1];
			FILETIME t, upt;

			strcpy(up, pPath);
			sprintf(&up[dp - pPath], "%s.sys", pName);

			if (!DdkGetWriteTime(pPath, &t)
			|| (DdkGetWriteTime(up, &upt) && CompareFileTime(&t, &upt) < 0))
				strcpy(pPath, up);
		}
	}

	if (ddk) FreeLibrary(ddk);
	if (GetFileAttributes(pPath) != INVALID_FILE_ATTRIBUTES) return;

	if (GetSystemDirectory(pPath, MAX_PATH+1)) {
		sprintf(&pPath[strlen(pPath)], "\\drivers\\%s.sys", pName);
		if (GetFileAttributes(pPath) != INVALID_FILE_ATTRIBUTES) return;
	}

	sprintf(pPath, "%s.sys", pName);
}


bool DdkGetWriteTime(char *pPath, FILETIME *pTime)
{
    HANDLE h = CreateFile(pPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, 0, NULL);

    if (h == INVALID_HANDLE_VALUE || !GetFileTime(h, NULL, NULL, pTime))
		return false;
        
    CloseHandle(h);
	return true;
}


static void DdkCleanTempDir(char *pDir)
{
	char path[MAX_PATH+40];
	WIN32_FIND_DATA find;

	sprintf(path, "%s\\*.*", pDir);
	char *cp = strrchr(path, '\\') + 1;

	HANDLE h = FindFirstFile(path, &find);
	if (h == INVALID_HANDLE_VALUE) return;

	do {
		if (strcmp(find.cFileName, ".") && strcmp(find.cFileName, "..")) {
			strcpy(cp, find.cFileName);

			if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				DdkCleanTempDir(path);
				RemoveDirectory(path);
			}

			else DeleteFile(path);
		}
	} while (FindNextFile(h, &find));

	FindClose(h);
}


static char *DdkGetTempDir()
{
	static char path[MAX_PATH+40];

	if (path[0]) return path;

	if (!GetTempPath(MAX_PATH+1, path))
		ddkfail("Unable to get temporary file path");

	strcat(path, "ddk\\\0");
	HANDLE h = CreateMutex(NULL, FALSE, "Global\\DDKTempDir");

	if (h && GetLastError() == ERROR_SUCCESS)
		DdkCleanTempDir(path);

	CreateDirectory(path, NULL);
	sprintf(&path[strlen(path)], "%x\\", GetCurrentProcessId());

	if (!CreateDirectory(path, NULL))
		ddkfail("Unable to create temporary directory");

	return path;
}
