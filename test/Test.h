/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2013-2014, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Test Definitions
 */

namespace DdkUnitTest { LONG GetUniqueId(); }

#define TEST_DDK_FAIL(call) TEST_EXPECT_ASSERT(call)
