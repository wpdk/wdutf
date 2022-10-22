/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * Data Definitions.
 */

#ifndef _DATA_H_
#define _DATA_H_

#define MmBadPointer					pMmBadPointer
#define KeNumberProcessors				_KeNumberProcessors
#define NlsMbCodePageTag				pNlsMbCodePageTag
#define NlsMbOemCodePageTag				pNlsMbOemCodePageTag

#define CmKeyObjectType					pCmKeyObjectType
#define IoFileObjectType				pIoFileObjectType
#define ExEventObjectType				pExEventObjectType
#define ExSemaphoreObjectType			pExSemaphoreObjectType
#define TmTransactionManagerObjectType	pTmTransactionManagerObjectType
#define TmResourceManagerObjectType		pTmResourceManagerObjectType
#define TmEnlistmentObjectType			pTmEnlistmentObjectType
#define TmTransactionObjectType			pTmTransactionObjectType
#define PsProcessType					pPsProcessType
#define PsThreadType					pPsThreadType
#define SeTokenObjectType				pSeTokenObjectType

#endif /* _DATA_H_ */
