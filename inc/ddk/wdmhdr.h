/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	WDM header.
 */

#ifndef _WDMHDR_H_
#define _WDMHDR_H_

#include <ntdef.h>


/*
 *	Disable some inlining
 */

#define KeGetCurrentThread _KeGetCurrentThread
#define PsGetCurrentThread _PsGetCurrentThread
#define KeGetCurrentIrql _KeGetCurrentIrql
#ifndef NTDDI_WINBLUE
#define KeLowerIrql _KeLowerIrql
#define KfRaiseIrql _KfRaiseIrql
#endif


#include <../km/wdm.h>


/*
 *	Disable PAGED_CODE()
 */

#undef PAGED_CODE
#define PAGED_CODE()


/*
 *	Define Annotations
 */

#ifndef _IRQL_requires_max_
#define _IRQL_requires_max_(x) __drv_maxIRQL(x)
#endif

#ifndef _IRQL_raises_
#define _IRQL_raises_(x) __drv_raisesIRQL(x)
#endif

#ifndef _IRQL_saves_
#define _IRQL_saves_ __drv_savesIRQL
#endif

#ifndef _Notliteral_
#define _Notliteral_ __drv_nonConstant 
#endif

#ifndef _IRQL_restores_
#define _IRQL_restores_ __drv_restoresIRQL
#endif


/*
 *	Non-inlined Declarations
 */

#undef KeGetCurrentThread
#undef PsGetCurrentThread
#undef KeGetCurrentIrql
#undef KeLowerIrql
#undef KfRaiseIrql

NTSYSAPI PKTHREAD NTAPI KeGetCurrentThread(VOID);
_IRQL_requires_max_(DISPATCH_LEVEL) NTSYSAPI PETHREAD PsGetCurrentThread(VOID);
_IRQL_requires_max_(HIGH_LEVEL) _IRQL_saves_ NTSYSAPI KIRQL NTAPI KeGetCurrentIrql(VOID);
_IRQL_requires_max_(HIGH_LEVEL) NTSYSAPI VOID KeLowerIrql(_In_ _Notliteral_ _IRQL_restores_ KIRQL NewIrql);
_IRQL_requires_max_(HIGH_LEVEL) _IRQL_raises_(NewIrql) _IRQL_saves_ NTSYSAPI KIRQL KfRaiseIrql(_In_ KIRQL NewIrql);

#define KeRaiseIrql(a,b) *(b) = KfRaiseIrql(a)

#endif	/* _WDMHDR_H_ */
