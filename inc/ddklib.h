/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2016, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * DDK Library.
 */

#ifndef _DDKLIB_H_
#define _DDKLIB_H_

#define _DDKLIB_
#define _KERNEL_
#define _DCS_KERNEL_
#define _KERNEL_MODE


/*
 *	Ensure the Target is defined
 */

#if defined(_M_AMD64) && !defined(_AMD64_)
#define _AMD64_
#endif


/*
 *	Set the DDK definitions
 */

#define W2K
#define WIN2K
#define WXP
#define WNET


/*
 *	Define include files
 */

#pragma include_alias(<wdm.h>,<ddk/wdm.h>)
#pragma include_alias("wdm.h","ddk/wdm.h")
#pragma include_alias(<ntifs.h>,<ddk/ntifs.h>)
#pragma include_alias("ntifs.h","ddk/ntifs.h")
#pragma include_alias(<ntdef.h>,<ddk/ntdef.h>)
#pragma include_alias("ntdef.h","ddk/ntdef.h")
#pragma include_alias(<ntstatus.h>,<ddk/ntstatus.h>)
#pragma include_alias("ntstatus.h","ddk/ntstatus.h")
#pragma include_alias(<winnt.h>,<ddk/winnt.h>)
#pragma include_alias("winnt.h","ddk/winnt.h")


/*
 *	Set up include paths
 */

#pragma include_alias(<mce.h>,<../km/mce.h>)
#pragma include_alias(<ntnls.h>,<../km/ntnls.h>)
#pragma include_alias(<Ntstrsafe.h>,<ddk/ntstrsafe.h>)
#pragma include_alias(<wmilib.h>,<../km/wmilib.h>)
#pragma include_alias("wmilib.h","../km/wmilib.h")
#pragma include_alias(<mountdev.h>, <../km/mountdev.h>)
#pragma include_alias("mountdev.h", "../km/mountdev.h")
#pragma include_alias(<mountmgr.h>, <../km/mountmgr.h>)
#pragma include_alias("mountmgr.h", "../km/mountmgr.h")
#pragma include_alias(<tdikrnl.h>, <../km/tdikrnl.h>)
#pragma include_alias("tdikrnl.h", "../km/tdikrnl.h")
#pragma include_alias(<netpnp.h>, <../km/netpnp.h>)
#pragma include_alias("netpnp.h", "../km/netpnp.h")
#pragma include_alias(<ntddft.h>,<ntdskreg.h>)
#pragma include_alias("ntddft.h","ntdskreg.h")


/*
 *	Setup Linkage Definition
 */

#ifdef _DDKLIBBUILD_
#define DDKAPI __declspec(dllexport)
#else
#define DDKAPI __declspec(dllimport)
#endif

#define DDKAPI_NODECL DDKAPI


/*
 *	Include DDK header
 */

#include <ddk/ntddk.h>


/*
 *	External Definitions
 */

extern "C" {
DDKAPI NTSTATUS DdkLoadDriver(char *pFile, HRESULT (*pLoad)(const char *) = NULL);
DDKAPI NTSTATUS DdkInitDriver(char *pName, PDRIVER_INITIALIZE DriverInit);
DDKAPI VOID DdkUnloadDriver(char *pName, INT (*pUnload)(const char *) = NULL);
DDKAPI VOID DdkThreadInit();
DDKAPI VOID DdkThreadDeinit();
DDKAPI VOID DdkModuleStart(char *pName, void (*cleanup)());
DDKAPI BOOLEAN DdkModuleEnd(char *pName);
DDKAPI PKTHREAD DdkGetCurrentThread();
DDKAPI PVOID DdkFindFunction(const char *pName);
DDKAPI PVOID DdkCodeFromPointer(PVOID pAddr);
DDKAPI NTSTATUS DdkAttachIntercept(PVOID pFunction,
	PVOID pIntercept, PVOID pIdentity, PVOID pInstance, PVOID pThread);
DDKAPI NTSTATUS DdkDetachIntercept(PVOID pId, PVOID pThread);
DDKAPI PVOID DdkGetRealPointer();
};


/*
 *	Load the library
 */

#ifndef _DDKLIBBUILD_
#pragma comment(lib,"ddk.lib")
#endif

#endif /* _DDKLIB_H_ */
