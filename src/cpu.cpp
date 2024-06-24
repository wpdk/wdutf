/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * CPU Routines.
 */

#include "stdafx.h"


#undef KeNumberProcessors

extern "C" {
#if defined(_X86_) || defined(_AMD64_)
extern volatile CCHAR KeNumberProcessors;
extern volatile CCHAR _KeNumberProcessors;
#else
extern CCHAR KeNumberProcessors;
extern CCHAR _KeNumberProcessors;
#endif
};


void DdkCpuInit()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    
	KeNumberProcessors = (CCHAR) sysinfo.dwNumberOfProcessors;
	_KeNumberProcessors = (CCHAR) sysinfo.dwNumberOfProcessors;
}
