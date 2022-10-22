/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Executive Routines
 */

#include "stdafx.h"
#include "Objbase.h"


DDKAPI
NTSTATUS ExUuidCreate(UUID *Uuid)
{
	DDKASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	DDKASSERT(Uuid);
		
	if (CoCreateGuid(Uuid) != S_OK)
		return STATUS_RETRY;

	return STATUS_SUCCESS;
}
