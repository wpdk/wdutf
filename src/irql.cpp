/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	IRQL Routines
 */

#include "stdafx.h"


#undef KeRaiseIrql
#undef KeGetCurrentIrql

__declspec(thread) KIRQL DdkCurrentIrql = PASSIVE_LEVEL;


#ifndef _DDKINLINE_
DDKAPI
KIRQL KeGetCurrentIrql()
{
	return DdkCurrentIrql;
}
#endif


DDKAPI
KIRQL DdkGetCurrentIrql()
{
	return DdkCurrentIrql;
}


ULONG64 DdkReadCR8()
{
	return (ULONG64)DdkCurrentIrql;
}


VOID DdkWriteCR8(ULONG64 v)
{
	DdkCurrentIrql = (KIRQL)v;
}


DDKAPI
KIRQL KfRaiseIrql(KIRQL NewIrql)
{
    KIRQL irql = DdkCurrentIrql;

	DDKASSERT(KeGetCurrentIrql() <= NewIrql);

	DdkCurrentIrql = NewIrql;
    return irql;
}


DDKAPI
VOID KeRaiseIrql(KIRQL NewIrql, PKIRQL OldIrql)
{
	*OldIrql = KfRaiseIrql(NewIrql);
}


DDKAPI
VOID KeLowerIrql(KIRQL NewIrql)
{
	DDKASSERT(KeGetCurrentIrql() >= NewIrql);
	DdkCurrentIrql = NewIrql;
}


DDKAPI
VOID KfLowerIrql(KIRQL NewIrql)
{
	DDKASSERT(KeGetCurrentIrql() >= NewIrql);
	DdkCurrentIrql = NewIrql;
}


DDKAPI
KIRQL DdkRaiseIrqlToDpcLevel()
{
	return KfRaiseIrql(DISPATCH_LEVEL);
}
