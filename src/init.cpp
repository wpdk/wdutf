/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Initialization Routines.
 */

#include "stdafx.h"


void DdkInit();
void DdkInitError();
void DdkCpuInit();
void DdkTimeInit();
void DdkNameInit();
void DdkLoadInit();
void DdkLocalInit();
void DdkRegistryInit();
void DdkTestInit();
void DdkWaitInit();
void DdkPnpInit();
void DdkRegistryDeinit();
void DdkLocalDeinit();
void DdkExceptionInit();
void DdkExceptionDeinit();

static bool initialised = false;
static struct _DdkInit { _DdkInit() { DdkInit(); } } _DdkInit;

static HMODULE DdkModule;


void DdkInit()
{
	if (!DdkModule)
		DdkModule = GetModuleHandle(NULL);

	if (!initialised) {
		DdkExceptionInit();
		DdkInitError();
		DdkCpuInit();
		DdkTimeInit();
		DdkNameInit();
		DdkLocalInit();
		DdkRegistryInit();
		DdkLoadInit();
		DdkTestInit();
		DdkWaitInit();
		DdkPnpInit();
		initialised = true;
	}

	DdkThreadInit();
}


void DdkDeinit()
{
	DdkDetachIntercept(NULL, NULL);
	DdkRegistryDeinit();
	DdkLocalDeinit();
	DdkExceptionDeinit();
}


extern "C"
int DllMain(HINSTANCE hModule, DWORD reason, PVOID lpReserved)
{
    switch (reason)
	{
		case DLL_PROCESS_ATTACH:
			DdkModule = hModule;
			DdkInit();
			break;

		case DLL_THREAD_ATTACH:
			DdkThreadInit();
			break;

		case DLL_THREAD_DETACH:
			DdkThreadDeinit();
			break;

		case DLL_PROCESS_DETACH:
			DdkDeinit();
			break;
    }

    return TRUE;
}


PVOID
DdkGetProcAddress(char *name)
{
	return GetProcAddress(DdkModule, name);
}
