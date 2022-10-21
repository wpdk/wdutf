/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2013-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	DDK Definitions.
 */

#ifndef _DDK_NTDDK_DEFS_H_
#define _DDK_NTDDK_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif


/*
 *	Basic Types (must match DDK)
 */

typedef LONG KPRIORITY;
typedef CCHAR KPROCESSOR_MODE;
typedef unsigned char KIRQL, *PKIRQL;
typedef ULONG_PTR KSPIN_LOCK, *PKSPIN_LOCK;
typedef volatile LONG EX_SPIN_LOCK, *PEX_SPIN_LOCK;
typedef struct _OBJECT_TYPE OBJECT_TYPE, *POBJECT_TYPE;


/*
 *	Enumeration Definitions (must match DDK)
 */

typedef enum { WaitAll, WaitAny } WAIT_TYPE;
typedef enum { NotificationEvent, SynchronizationEvent } EVENT_TYPE;
typedef enum { NotificationTimer, SynchronizationTimer } TIMER_TYPE;
typedef enum { LowImportance, MediumImportance, HighImportance, MediumHighImportance } KDPC_IMPORTANCE;
typedef enum { CriticalWorkQueue, DelayedWorkQueue, HyperCriticalWorkQueue, MaximumWorkQueue } WORK_QUEUE_TYPE;
typedef enum { Executive, Suspended = 5, UserRequest, } KWAIT_REASON;
typedef enum { NonPagedPool, PagedPool } POOL_TYPE;
typedef enum { LowPoolPriority, NormalPoolPriority = 16 } EX_POOL_PRIORITY;
typedef enum { KernelMode, UserMode } MODE;
typedef enum { MmNonCached = FALSE, MmCached = TRUE } MEMORY_CACHING_TYPE;


/*
 *	Structure Declarations
 */

typedef struct _IRP IRP, *PIRP;
typedef struct _MDL MDL, *PMDL;
typedef struct _IO_STACK_LOCATION IO_STACK_LOCATION, *PIO_STACK_LOCATION;
typedef struct _KDPC KDPC, *PKDPC, *PRKDPC;
typedef struct _KEVENT KEVENT, *PKEVENT, *PRKEVENT;
typedef struct _KMUTANT KMUTANT, *PKMUTANT, *PRKMUTANT, KMUTEX, *PKMUTEX, *PRKMUTEX;
typedef struct _KSEMAPHORE KSEMAPHORE, *PKSEMAPHORE, *PRKSEMAPHORE;
typedef struct _KWAIT_BLOCK KWAIT_BLOCK, *PKWAIT_BLOCK, *PRKWAIT_BLOCK;
typedef struct _CLIENT_ID CLIENT_ID, *PCLIENT_ID;
typedef struct _KTIMER KTIMER, *PKTIMER, *PRKTIMER;
typedef struct _DEVICE_OBJECT DEVICE_OBJECT, *PDEVICE_OBJECT;
typedef struct _DRIVER_OBJECT DRIVER_OBJECT, *PDRIVER_OBJECT;
typedef struct _FILE_OBJECT FILE_OBJECT, *PFILE_OBJECT;
typedef struct _IO_WORKITEM IO_WORKITEM, *PIO_WORKITEM;
typedef struct _KLOCK_QUEUE_HANDLE KLOCK_QUEUE_HANDLE, *PKLOCK_QUEUE_HANDLE;
typedef struct _KTHREAD *PETHREAD, *PKTHREAD, *PRKTHREAD;
typedef struct _KPROCESS *PKPROCESS, *PRKPROCESS, *PEPROCESS;
typedef struct _OBJECT_ATTRIBUTES OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES, *PCOBJECT_ATTRIBUTES;
typedef struct _OBJECT_HANDLE_INFORMATION OBJECT_HANDLE_INFORMATION, *POBJECT_HANDLE_INFORMATION;
typedef struct _KEY_VALUE_PARTIAL_INFORMATION KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;
typedef struct _PCI_COMMON_CONFIG PCI_COMMON_CONFIG, *PPCI_COMMON_CONFIG;
typedef struct _CM_RESOURCE_LIST CM_RESOURCE_LIST, *PCM_RESOURCE_LIST;


/*
 *	Callback Declarations (must match DDK)
 */

typedef VOID KSTART_ROUTINE(PVOID);
typedef KSTART_ROUTINE *PKSTART_ROUTINE;
typedef VOID KDEFERRED_ROUTINE(KDPC *, PVOID, PVOID, PVOID);
typedef KDEFERRED_ROUTINE *PKDEFERRED_ROUTINE;
typedef VOID IO_WORKITEM_ROUTINE(PDEVICE_OBJECT, PVOID);
typedef IO_WORKITEM_ROUTINE *PIO_WORKITEM_ROUTINE;
typedef VOID IO_WORKITEM_ROUTINE_EX(PVOID, PVOID, PIO_WORKITEM);
typedef IO_WORKITEM_ROUTINE_EX *PIO_WORKITEM_ROUTINE_EX;
typedef NTSTATUS DRIVER_INITIALIZE(PDRIVER_OBJECT, PUNICODE_STRING);
typedef DRIVER_INITIALIZE *PDRIVER_INITIALIZE;


/*
 *	Macro Definitions (must match DDK)
 */

#define IO_NO_INCREMENT 0

#define KeWaitForMutexObject KeWaitForSingleObject


/*
 *	Function Declarations
 */

DDKAPI KIRQL KeGetCurrentIrql();
DDKAPI VOID KeRaiseIrql(KIRQL NewIrql, PKIRQL OldIrql);
DDKAPI VOID KeLowerIrql(KIRQL NewIrql);

DDKAPI NTSTATUS PsCreateSystemThread(PHANDLE ThreadHandle, ULONG DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE ProcessHandle, PCLIENT_ID ClientId, PKSTART_ROUTINE StartRoutine, PVOID StartContext);
DDKAPI NTSTATUS PsTerminateSystemThread(NTSTATUS ExitStatus);

DDKAPI LONG RtlCompareUnicodeString(PCUNICODE_STRING u1, PCUNICODE_STRING u2, BOOLEAN ign);
DDKAPI VOID RtlCopyUnicodeString(PUNICODE_STRING DestinationString, PCUNICODE_STRING SourceString);
DDKAPI NTSTATUS RtlIntegerToUnicodeString(ULONG Value, ULONG Base, PUNICODE_STRING String);
DDKAPI BOOLEAN RtlEqualUnicodeString(PCUNICODE_STRING String1, PCUNICODE_STRING String2, BOOLEAN CaseInSensitive);
DDKAPI NTSTATUS RtlStringFromGUID(REFGUID Guid, PUNICODE_STRING GuidString);
DDKAPI NTSTATUS RtlUpcaseUnicodeString(PUNICODE_STRING DestinationString, PCUNICODE_STRING SourceString, BOOLEAN AllocateDestinationString);

DDKAPI NTSTATUS KeWaitForSingleObject(PVOID Object, KWAIT_REASON WaitReason, 
	KPROCESSOR_MODE WaitMode, BOOLEAN Alertable, PLARGE_INTEGER Timeout);
DDKAPI NTSTATUS KeWaitForMultipleObjects(ULONG Count, PVOID Object[], WAIT_TYPE WaitType,
	KWAIT_REASON WaitReason, KPROCESSOR_MODE WaitMode, BOOLEAN Alertable, PLARGE_INTEGER Timeout,
	PKWAIT_BLOCK WaitBlockArray);

DDKAPI PVOID ExAllocatePoolWithTag(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag);
DDKAPI PVOID ExAllocatePool(POOL_TYPE PoolType, SIZE_T NumberOfBytes);
DDKAPI PVOID ExAllocatePoolWithTagPriority(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag, EX_POOL_PRIORITY Priority);
DDKAPI VOID ExFreePoolWithTag(PVOID P, ULONG Tag);
DDKAPI VOID ExFreePool(PVOID P);

DDKAPI VOID KeInitializeSpinLock(PKSPIN_LOCK SpinLock);
DDKAPI VOID KeAcquireSpinLockAtDpcLevel(PKSPIN_LOCK SpinLock);
DDKAPI VOID KeReleaseSpinLock(PKSPIN_LOCK SpinLock, KIRQL NewIrql);
DDKAPI VOID KeReleaseSpinLockFromDpcLevel(PKSPIN_LOCK SpinLock);
DDKAPI VOID KeInitializeMutex(PRKMUTEX Mutex, ULONG Level);
DDKAPI LONG KeReleaseMutex(PRKMUTEX Mutex, BOOLEAN Wait);

DDKAPI VOID KeInitializeEvent(PRKEVENT Event, EVENT_TYPE Type, BOOLEAN State);
DDKAPI VOID KeClearEvent(PRKEVENT Event);
DDKAPI LONG KeReadStateEvent(PRKEVENT Event);
DDKAPI LONG KeResetEvent(PRKEVENT Event);
DDKAPI LONG KeSetEvent(PRKEVENT Event, KPRIORITY Increment, BOOLEAN Wait);

DDKAPI VOID KeInitializeTimer(PKTIMER Timer);
DDKAPI VOID KeInitializeTimerEx(PKTIMER Timer, TIMER_TYPE Type);
DDKAPI BOOLEAN KeCancelTimer(PKTIMER Timer);
DDKAPI BOOLEAN KeReadStateTimer(PKTIMER Timer);
DDKAPI BOOLEAN KeSetTimer(PKTIMER Timer, LARGE_INTEGER DueTime, PKDPC Dpc);
DDKAPI BOOLEAN KeSetTimerEx(PKTIMER Timer, LARGE_INTEGER DueTime, LONG Period, PKDPC Dpc);

DDKAPI VOID KeInitializeDpc(PRKDPC Dpc, PKDEFERRED_ROUTINE DeferredRoutine, PVOID DeferredContext);
DDKAPI VOID KeInitializeThreadedDpc(PRKDPC Dpc, PKDEFERRED_ROUTINE DeferredRoutine, PVOID DeferredContext);
DDKAPI BOOLEAN KeInsertQueueDpc(PRKDPC Dpc, PVOID SystemArgument1, PVOID SystemArgument2);
DDKAPI BOOLEAN KeRemoveQueueDpc(PRKDPC Dpc);
DDKAPI VOID KeFlushQueuedDpcs(VOID);

DDKAPI NTSTATUS KeDelayExecutionThread(KPROCESSOR_MODE WaitMode, BOOLEAN Alertable, PLARGE_INTEGER Interval);
DDKAPI KPRIORITY KeSetPriorityThread(PKTHREAD Thread, KPRIORITY Priority);
DDKAPI KPRIORITY KeQueryPriorityThread(PKTHREAD Thread);

DDKAPI LONG_PTR ObReferenceObject(PVOID Object);
DDKAPI LONG_PTR ObReferenceObjectWithTag(PVOID Object, ULONG Tag);
DDKAPI LONG_PTR ObDereferenceObject(PVOID Object);
DDKAPI LONG_PTR ObDereferenceObjectWithTag(PVOID Object, ULONG Tag);

DDKAPI VOID KeBugCheck(ULONG BugCheckCode);


#ifdef __cplusplus
};
#endif

#endif /*_DDK_NTDDK_DEFS_H_ */
