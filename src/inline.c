/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Inline Routines.
 *
 *	Note: This file is an optimisation that is compiled and linked with
 *	unit test code to try and reduce the frequency of exceptions.  It is not
 *	required for correct operation.
 *
 *	In normal circumstances the exception handler will catch privileged
 *	instructions and resolve them quickly.  When the Visual Studio Debugger
 *	is involved it gets notified about every exception which can have a
 *	significant impact on run time.
 *
 *	This file redefines the COMDAT sections for inline functions and is
 *	linked with the test code as the first object file.  The functions
 *	are declared as dllexport to ensure that the linker references the
 *	code.  There are no documented guarantees that the linker will handle
 *	this as intended, but it seems to work for Debug builds.
 *
 *	For Release builds the compiler has already inlined the code and so
 *	the generated code is unchanged.
 */

typedef unsigned char KIRQL;

__declspec(dllimport) void KeLowerIrql(KIRQL irql);
__declspec(dllimport) KIRQL KfRaiseIrql(KIRQL irql);
__declspec(dllimport) KIRQL DdkRaiseIrqlToDpcLevel();

__declspec(dllexport) __forceinline void KzLowerIrql(KIRQL irql) { KeLowerIrql(irql); }
__declspec(dllexport) __forceinline KIRQL KzRaiseIrql(KIRQL irql) { return KfRaiseIrql(irql); }
__declspec(dllexport) __forceinline KIRQL KeRaiseIrqlToDpcLevel() { return DdkRaiseIrqlToDpcLevel(); }


/*
 *	KeGetCurrentIrql isn't inline by default in WDK 10.0.20348 (which defines
 *	NTDDI_WIN10_FE), so don't generate a definition to avoid a conflict.
 */
#include <sdkddkver.h>

#if !defined(NTDDI_WIN10_FE)
__declspec(dllimport) KIRQL DdkGetCurrentIrql(); 
__declspec(dllexport) __forceinline KIRQL KeGetCurrentIrql() { return DdkGetCurrentIrql(); }
#endif
