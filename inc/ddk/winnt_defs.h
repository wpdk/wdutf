/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2013-2016, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Windows Type Definitions.
 */

#ifndef _DDK_WINNT_DEFS_H_
#define _DDK_WINNT_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif


/*
 *	Basic Types
 */

typedef DWORD TP_WAIT_RESULT;


/*
 *	Structure Declarations
 */

typedef struct _RTL_SRWLOCK RTL_SRWLOCK, *PRTL_SRWLOCK;
typedef struct _RTL_CONDITION_VARIABLE RTL_CONDITION_VARIABLE, *PRTL_CONDITION_VARIABLE;
typedef struct _RTL_BARRIER RTL_BARRIER, *PRTL_BARRIER;
typedef struct _RTL_CRITICAL_SECTION_DEBUG RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG;
typedef struct _HEAP_INFORMATION_CLASS HEAP_INFORMATION_CLASS, *PHEAP_INFORMATION_CLASS;
typedef struct _MEMORY_BASIC_INFORMATION MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
typedef struct _JOB_SET_ARRAY JOB_SET_ARRAY, *PJOB_SET_ARRAY;
typedef struct _PERFORMANCE_DATA PERFORMANCE_DATA, *PPERFORMANCE_DATA;
typedef struct _WOW64_CONTEXT WOW64_CONTEXT, *PWOW64_CONTEXT;
typedef struct _WOW64_LDT_ENTRY WOW64_LDT_ENTRY, *PWOW64_LDT_ENTRY;
typedef struct _TP_IO TP_IO, *PTP_IO;
typedef struct _TP_POOL TP_POOL, *PTP_POOL;
typedef struct _TP_WORK TP_WORK, *PTP_WORK;
typedef struct _TP_TIMER TP_TIMER, *PTP_TIMER;
typedef struct _TP_WAIT TP_WAIT, *PTP_WAIT;
typedef struct _TP_CALLBACK_INSTANCE TP_CALLBACK_INSTANCE, *PTP_CALLBACK_INSTANCE;
typedef struct _TP_POOL_STACK_INFORMATION TP_POOL_STACK_INFORMATION, *PTP_POOL_STACK_INFORMATION;
typedef struct _TP_CLEANUP_GROUP TP_CLEANUP_GROUP, *PTP_CLEANUP_GROUP;
typedef struct _TP_CALLBACK_ENVIRON TP_CALLBACK_ENVIRON, *PTP_CALLBACK_ENVIRON;
typedef struct _SYSTEM_PROCESSOR_CYCLE_TIME_INFORMATION SYSTEM_PROCESSOR_CYCLE_TIME_INFORMATION,
	*PSYSTEM_PROCESSOR_CYCLE_TIME_INFORMATION;
typedef struct _IMAGE_DOS_HEADER IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;
typedef struct _RTL_CRITICAL_SECTION { __int64 x[5]; } RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;
typedef struct _CFG_CALL_TARGET_INFO CFG_CALL_TARGET_INFO, *PCFG_CALL_TARGET_INFO;
typedef struct MEM_EXTENDED_PARAMETER MEM_EXTENDED_PARAMETER, *PMEM_EXTENDED_PARAMETER;
typedef struct _PROCESS_DYNAMIC_ENFORCED_ADDRESS_RANGE PROCESS_DYNAMIC_ENFORCED_ADDRESS_RANGE,
	* PPROCESS_DYNAMIC_ENFORCED_ADDRESS_RANGE;


/*
 *	Enumeration Declarations
 */

typedef enum _TP_CALLBACK_PRIORITY TP_CALLBACK_PRIORITY;
typedef enum _FIRMWARE_TYPE FIRMWARE_TYPE, *PFIRMWARE_TYPE;
typedef enum _JOBOBJECTINFOCLASS JOBOBJECTINFOCLASS;
typedef enum _ACL_INFORMATION_CLASS ACL_INFORMATION_CLASS;
typedef enum _RTL_UMS_SCHEDULER_REASON UMS_SCHEDULER_REASON;
typedef enum _RTL_UMS_SCHEDULER_REASON RTL_UMS_SCHEDULER_REASON, *PRTL_UMS_SCHEDULER_REASON;


/*
 *	Macro Definitions
 */

#define DLL_PROCESS_ATTACH   1
#define DLL_PROCESS_DETACH   0


/*
 *	Callback Declarations
 */

typedef VOID(*PTP_SIMPLE_CALLBACK)(PTP_CALLBACK_INSTANCE, PVOID);
typedef VOID(*WAITORTIMERCALLBACK)(PVOID, BOOLEAN);
typedef VOID(*PAPCFUNC)(ULONG_PTR);
typedef LONG(*PVECTORED_EXCEPTION_HANDLER)(struct _EXCEPTION_POINTERS *);
typedef VOID(*PFLS_CALLBACK_FUNCTION)(PVOID);
typedef VOID(*PTP_CLEANUP_GROUP_CANCEL_CALLBACK)(PVOID, PVOID);
typedef VOID(*PTP_WORK_CALLBACK)(PTP_CALLBACK_INSTANCE, PVOID, PTP_WORK);
typedef VOID(*PTP_TIMER_CALLBACK)(PTP_CALLBACK_INSTANCE, PVOID, PTP_TIMER);
typedef VOID(*PTP_WAIT_CALLBACK)(PTP_CALLBACK_INSTANCE, PVOID, PTP_WAIT, TP_WAIT_RESULT);
typedef BOOLEAN(*PSECURE_MEMORY_CACHE_CALLBACK)(PVOID, SIZE_T);
typedef VOID RTL_UMS_SCHEDULER_ENTRY_POINT(RTL_UMS_SCHEDULER_REASON, ULONG_PTR, PVOID);
typedef RTL_UMS_SCHEDULER_ENTRY_POINT *PRTL_UMS_SCHEDULER_ENTRY_POINT;
typedef LONG (*PVECTORED_EXCEPTION_HANDLER)(struct _EXCEPTION_POINTERS *);


/*
 *	Function Declarations
 */

VOID TpSetCallbackThreadpool(PTP_CALLBACK_ENVIRON, PTP_POOL);
VOID TpSetCallbackCleanupGroup(PTP_CALLBACK_ENVIRON, PTP_CLEANUP_GROUP, PTP_CLEANUP_GROUP_CANCEL_CALLBACK);
VOID TpInitializeCallbackEnviron(PTP_CALLBACK_ENVIRON);
VOID TpSetCallbackLongFunction(PTP_CALLBACK_ENVIRON);
VOID TpSetCallbackRaceWithDll(PTP_CALLBACK_ENVIRON, PVOID);
VOID TpSetCallbackFinalizationCallback(PTP_CALLBACK_ENVIRON, PTP_SIMPLE_CALLBACK);
VOID TpSetCallbackPriority(PTP_CALLBACK_ENVIRON, TP_CALLBACK_PRIORITY);
VOID TpSetCallbackPersistent(PTP_CALLBACK_ENVIRON);
VOID TpDestroyCallbackEnviron(PTP_CALLBACK_ENVIRON);

VOID RtlRestoreContext(PCONTEXT ContextRecord, struct _EXCEPTION_RECORD *ExceptionRecord);


#ifdef __cplusplus
};
#endif

#endif /* _DDK_WINNT_DEFS_H_ */
