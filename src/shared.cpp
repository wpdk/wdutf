/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2014, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Shared User Data.
 */

#include "stdddk.h"


#define SHARED_DATA	0x7FFE0000;


ULONG DdkGetTickCountMultiplier()
{
	PKUSER_SHARED_DATA pData = (PKUSER_SHARED_DATA)SHARED_DATA;
	return pData->TickCountMultiplier;
}


ULONG64 DdkGetTickCount()
{
	PKUSER_SHARED_DATA pData = (PKUSER_SHARED_DATA)SHARED_DATA;
	return pData->TickCountQuad;
}

