/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2017, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
* Debug Routines.
*/

#include "stdafx.h"
#include <dpfilter.h>
#include <stdio.h>
#include <stddef.h>


DDKAPI
ULONG vDbgPrintEx(ULONG ComponentId, ULONG Level, PCCH Format, va_list arglist)
{
	UNREFERENCED_PARAMETER(ComponentId);
	UNREFERENCED_PARAMETER(Level);

	if (IsDebuggerPresent()) {
		char s[2048];
		int n = vsnprintf(s, sizeof(s) - 1, Format, arglist);

		if (n > 0 && n < sizeof(s) - 1 && s[n - 1] != '\n')
			strcpy(s + n, "\n");

		OutputDebugString(s);
	}

	return 0;
}


extern "C"
DDKUSEDECL
ULONG __cdecl DbgPrint(PCSTR Format, ...)
{
	va_list args;
	va_start(args, Format);
	vDbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, Format, args);
	va_end(args);
	return 0;
}


DDKAPI
ULONG DbgPrintEx(ULONG ComponentId, ULONG Level, PCSTR Format, ...)
{
	va_list args;
	va_start(args, Format);
	vDbgPrintEx(ComponentId, Level, Format, args);
	va_end(args);
	return 0;
}
