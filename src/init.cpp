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
void DdkCpuInit();
void DdkTimeInit();
void DdkNameInit();
void DdkLoadInit();
void DdkTestInit();
void DdkWaitInit();
void DdkPnpInit();
void DdkExceptionInit();
void DdkExceptionDeinit();

static bool initialised = false;
static struct _DdkInit { _DdkInit() { DdkInit(); } } _DdkInit;


void DdkInit()
{
	if (!initialised) {
		DdkExceptionInit();
		DdkCpuInit();
		DdkTimeInit();
		DdkNameInit();
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
	DdkExceptionDeinit();
}


extern "C"
int DllMain(HANDLE hModule, DWORD reason, PVOID lpReserved)
{
    switch (reason)
	{
		case DLL_PROCESS_ATTACH:
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
