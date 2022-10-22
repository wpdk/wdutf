/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2014, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * CRT Routines.
 */

#include "stdafx.h"


void DdkForceReference()
{
	ddkfail("DdkForceReference called");

	(void)isdigit(0);
	_purecall();
	__C_specific_handler(0,0,0,0);
}
