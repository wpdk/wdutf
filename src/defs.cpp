/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 *  Copyright (c) 1998-2017, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

 /*
  *	Share DDK Definitions with Windows Implementation.
  */

#include "stdddk.h"


size_t _SizeofEvent_ = sizeof(KEVENT);
size_t _SizeofMutex_ = sizeof(KMUTEX);
size_t _SizeofTimer_ = sizeof(KTIMER);
size_t _SizeofSemaphore_ = sizeof(KSEMAPHORE);
size_t _SizeofDpc_ = sizeof(KDPC);

ULONG ThreadWaitObjects = THREAD_WAIT_OBJECTS;