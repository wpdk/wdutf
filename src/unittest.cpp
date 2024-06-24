/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Unit Test Support Routines.
 */

#include "stdafx.h"


void DdkModuleCleanup();
void DdkTestAdd(char *pName);
bool DdkTestRemove(char *pName);


static DWORD cleanupid;
static void (*cleanupfn)();
static char *cleanupname;
static CRITICAL_SECTION Lock;

static const int maxv = 200;
static char *vec[maxv];


void DdkTestInit()
{
	if (!InitializeCriticalSectionAndSpinCount(&Lock, 4000))
		ddkfail("DdkTestInit failed to initialize lock");
}


void DdkModuleStart(char *pName, void (*cleanup)())
{
	DdkThreadInit();
	EnterCriticalSection(&Lock);

	// Improve test behaviour with Visual Studio which starts a
	// new module on the same thread without calling cleanup first.

	if (cleanupid == GetCurrentThreadId())
		if (cleanupfn && cleanupfn != cleanup) DdkModuleCleanup();

	cleanupid = GetCurrentThreadId();
	cleanupfn = cleanup;
	cleanupname = pName;
	DdkTestRemove(pName);
	LeaveCriticalSection(&Lock);
}


void DdkModuleCleanup()
{
	char *name = cleanupname;

	(*cleanupfn)();

	DdkModuleEnd(name);
	DdkTestAdd(name);
}



BOOLEAN DdkModuleEnd(char *pName)
{
	EnterCriticalSection(&Lock);
	cleanupid = 0;
	cleanupfn = NULL;
	cleanupname = NULL;
	bool rc = DdkTestRemove(pName);
	LeaveCriticalSection(&Lock);
	return (rc != true);
}


void DdkTestAdd(char *pName)
{
	if (!pName) return;

	char *name = strrchr(pName, ':');
	name = name ? name + 1 : pName;

	for (int i = 0; i < maxv; i++)
		if (!vec[i]) {
			vec[i] = _strdup(name);
			break;
		}
}


bool DdkTestRemove(char *pName)
{
	bool rc = false;
	if (!pName) return rc;

	char *name = strrchr(pName, ':');
	name = name ? name + 1 : pName;

	for (int i = 0; i < maxv; i++)
		if (vec[i] && !strcmp(vec[i], name)) {
			free(vec[i]);
			vec[i] = NULL;
			rc = true;
		}

	return rc;
}




