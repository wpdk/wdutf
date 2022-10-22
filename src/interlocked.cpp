/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Interlocked Routines
 */

#include "stdafx.h"


DDKAPI
LARGE_INTEGER ExInterlockedAddLargeInteger(PLARGE_INTEGER Addend,
    LARGE_INTEGER Increment, PKSPIN_LOCK Lock)
{
	LARGE_INTEGER x;
	x.QuadPart = InterlockedExchangeAdd64(&Addend->QuadPart, Increment.QuadPart);
	return x;
}
