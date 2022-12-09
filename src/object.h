/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2014, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Object Definition
 */

#ifndef _OBJECT_H_
#define _OBJECT_H_

typedef struct _OBJECT {
	USHORT			type;
	USHORT			alloc:1;
	USHORT			temporary:1;
	volatile LONG	refcount;
	HANDLE			h;
} OBJECT, *POBJECT;

enum _OBJECTTYPE {
	MinObjectType = 0x2dc0, EventType, MutexType, SemaphoreType, ThreadType,
	TimerType, ProcessType, SecurityTokenType, EnlistmentType,
	ResourceManagerType, TransactionManagerType, TransactionType, CmKeyType,
	IoFileType, IoDeviceType, IoDriverType, IoSymbolicLinkType, KeyType,
	MaxObjectType
};

typedef struct _OBJECT_TYPE {
	USHORT	type;
} OBJECT_TYPE, *POBJECT_TYPE;


/*
 *	Object Validation
 */

inline bool isPublicObject(OBJECT *pObj) {
	return (pObj && (pObj->type == IoFileType
		|| pObj->type == IoDeviceType || pObj->type == IoDriverType));
}

inline bool isDispatchObject(OBJECT *pObj) {
	return (pObj && (pObj->type == EventType || pObj->type == MutexType
		|| pObj->type == SemaphoreType || pObj->type == ThreadType || pObj->type == TimerType));
}

inline bool isValidObject(OBJECT *pObj) {
	return (pObj && pObj->type > MinObjectType && pObj->type < MaxObjectType);
}

inline bool isHandleObject(OBJECT *pObj) {
	return (isValidObject(pObj) && pObj->refcount > 0
		&& (pObj->type == EventType || pObj->type == SemaphoreType || pObj->type == ThreadType
		|| pObj->type == ProcessType || pObj->type == IoFileType || pObj->type == SecurityTokenType
		|| pObj->type == EnlistmentType || pObj->type == ResourceManagerType
		|| pObj->type == TransactionManagerType || pObj->type == TransactionType
		|| pObj->type == KeyType));
}


/*
 *	Convert to/from DDK handles and pointers
 */

inline HANDLE ToHandle(OBJECT *pObj) {
	return (HANDLE)(((ULONG_PTR)pObj) | 1);
}

inline OBJECT *FromHandle(HANDLE Handle) {
	OBJECT *pObj = (OBJECT *)(((ULONG_PTR)Handle) & ~1);
	return (isHandleObject(pObj) ? pObj : 0);
}

inline PVOID ToPointer(OBJECT *pObj) {
	return (PVOID)(&pObj[isPublicObject(pObj)?1:0]);
}

inline OBJECT *FromPointer(PVOID Object) {
	OBJECT *pObj = (OBJECT *)Object;
	if (pObj && !isValidObject(pObj)) pObj--;
	if (!pObj || !isValidObject(pObj) || pObj->refcount <= 0
			|| (pObj != (OBJECT *)Object && !isPublicObject(pObj)))
		ddkfail("Invalid object pointer");
	return pObj;
}

#endif /* __cplusplus */


OBJECT *DdkAllocObject(size_t size, USHORT type, bool opt = false);
void DdkInitializeObject(OBJECT *pObj, size_t size, size_t maxsize);
void DdkReferenceObject(OBJECT *pObj);
void DdkDereferenceObject(OBJECT *pObj);
