/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Device Driver Definitions.
 */

#ifndef _DDK_WDM_H_
#define _DDK_WDM_H_


#ifdef _DDKINLINE_
#pragma include_alias(<_ddkhdr_>,<../km/wdm.h>)
#else
#pragma include_alias(<_ddkhdr_>,<ddk/wdmhdr.h>)
#endif

#include <ddk/ddkhdr.h>


#endif /* _DDK_WDM_H_ */
