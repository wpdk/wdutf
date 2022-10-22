/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Data Definitions
 */

#include "stdddk.h"


extern "C" {

#undef MmBadPointer
PVOID MmBadPointer = 0;
PVOID pMmBadPointer = (PVOID)&MmBadPointer;

#undef KeNumberProcessors
volatile CCHAR KeNumberProcessors;
volatile CCHAR _KeNumberProcessors;

#undef NlsMbCodePageTag
BOOLEAN NlsMbCodePageTag = FALSE;
BOOLEAN *pNlsMbCodePageTag = &NlsMbCodePageTag;

#undef NlsMbOemCodePageTag
BOOLEAN NlsMbOemCodePageTag = FALSE;
BOOLEAN *pNlsMbOemCodePageTag = &NlsMbOemCodePageTag;


/*
 *	Object Type Values
 */

#undef CmKeyObjectType
static OBJECT_TYPE _CmKeyType = { CmKeyType };
POBJECT_TYPE CmKeyObjectType = &_CmKeyType;
POBJECT_TYPE *pCmKeyObjectType = &CmKeyObjectType;

#undef IoFileObjectType
static OBJECT_TYPE _IoFileType = { IoFileType };
POBJECT_TYPE IoFileObjectType = &_IoFileType;
POBJECT_TYPE *pIoFileObjectType = &IoFileObjectType;

#undef ExEventObjectType
static OBJECT_TYPE _ExEventType = { EventType };
POBJECT_TYPE ExEventObjectType = &_ExEventType;
POBJECT_TYPE *pExEventObjectType = &ExEventObjectType;

#undef ExSemaphoreObjectType
static OBJECT_TYPE _ExSemaphoreType = { SemaphoreType };
POBJECT_TYPE ExSemaphoreObjectType = &_ExSemaphoreType;
POBJECT_TYPE *pExSemaphoreObjectType = &ExSemaphoreObjectType;

#undef TmTransactionManagerObjectType
static OBJECT_TYPE _TmTransactionManagerType = { TransactionManagerType };
POBJECT_TYPE TmTransactionManagerObjectType = &_TmTransactionManagerType;
POBJECT_TYPE *pTmTransactionManagerObjectType = &TmTransactionManagerObjectType;

#undef TmResourceManagerObjectType
static OBJECT_TYPE _TmResourceManagerType = { ResourceManagerType };
POBJECT_TYPE TmResourceManagerObjectType = &_TmResourceManagerType;
POBJECT_TYPE *pTmResourceManagerObjectType = &TmResourceManagerObjectType;

#undef TmEnlistmentObjectType
static OBJECT_TYPE _TmEnlistmentType = { EnlistmentType };
POBJECT_TYPE TmEnlistmentObjectType = &_TmEnlistmentType;
POBJECT_TYPE *pTmEnlistmentObjectType = &TmEnlistmentObjectType;

#undef TmTransactionObjectType
static OBJECT_TYPE _TmTransactionType = { TransactionType };
POBJECT_TYPE TmTransactionObjectType = &_TmTransactionType;
POBJECT_TYPE *pTmTransactionObjectType = &TmTransactionObjectType;

#undef PsProcessType
static OBJECT_TYPE _PsProcessType = { ProcessType };
POBJECT_TYPE PsProcessType = &_PsProcessType;
POBJECT_TYPE *pPsProcessType = &PsProcessType;

#undef PsThreadType
static OBJECT_TYPE _PsThreadType = { ThreadType };
POBJECT_TYPE PsThreadType = &_PsThreadType;
POBJECT_TYPE *pPsThreadType = &PsThreadType;

#undef SeTokenObjectType
static OBJECT_TYPE _SeTokenType = { SecurityTokenType };
POBJECT_TYPE SeTokenObjectType = &_SeTokenType;
POBJECT_TYPE *pSeTokenObjectType = &SeTokenObjectType;

};
