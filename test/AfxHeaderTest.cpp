/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Afx Header Tests
 */

#define NO_WARN_MBCS_MFC_DEPRECATION
#define _AFXDLL
#define _AFX_NOFORCE_LIBS

#include <afx.h>
#include <afxwin.h>
#include <ddk.h>
#include <ntifs.h>
#include <wmilib.h>
#include "stdafx.h"


void AfxThrowMemoryException() { }
void AfxThrowOleException(LONG) { }


namespace DdkUnitTest
{
	TEST_CLASS(DdkAfxHeaderTest)
	{
	public:
		
		TEST_METHOD(DdkAfxHeader)
		{
			Assert::IsTrue(true);
		}
	};
}
