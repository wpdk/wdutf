 /*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2013, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Event Routines
 */

#include "stdafx.h"


typedef struct _EVENT : public OBJECT {
	bool	notify;
} EVENT, *PEVENT;



static void DdkInitEvent(PEVENT pEvent, EVENT_TYPE Type, BOOLEAN State)
{
	pEvent->h = CreateEvent(NULL, (Type == NotificationEvent), (State != FALSE), NULL);
	pEvent->type = EventType;
	pEvent->notify = (Type == NotificationEvent);
	if (!pEvent->h) ddkfail("Unable to initialize Event");
}


DDKAPI
VOID KeInitializeEvent(PRKEVENT Event, EVENT_TYPE Type, BOOLEAN State)
{
	EVENT *pEvent = (EVENT *)Event;

	DdkInitializeObject(pEvent, sizeof(EVENT), _SizeofEvent_);
	DdkInitEvent(pEvent, Type, State);
}


static PKEVENT DdkCreateEvent(
	PUNICODE_STRING EventName, PHANDLE EventHandle, EVENT_TYPE Type)
{
	DDKASSERT(EventHandle);
	DDKASSERT(EventName && EventName->Length);

	POBJECT pObj = DdkAllocObject(_SizeofEvent_, EventType, true);
	if (!pObj) return 0;

	DdkInitEvent((PEVENT)pObj, Type, TRUE);
	PEVENT pEvent = (PEVENT)DdkCreateName(EventName, pObj, L"\\BaseNamedObjects");

	if (pEvent == pObj) DdkDereferenceObject(pEvent);

	if (pEvent->type != EventType || pEvent->notify != (Type == NotificationEvent)) {
		DdkDereferenceObject(pEvent);
		return 0;
	}

	*EventHandle = ToHandle(pEvent);
	return (PKEVENT)ToPointer(pEvent);	
}


DDKAPI
PKEVENT IoCreateNotificationEvent(PUNICODE_STRING EventName, PHANDLE EventHandle)
{
	return DdkCreateEvent(EventName, EventHandle, NotificationEvent);
}


DDKAPI
PKEVENT IoCreateSynchronizationEvent(PUNICODE_STRING EventName, PHANDLE EventHandle)
{
	return DdkCreateEvent(EventName, EventHandle, SynchronizationEvent);
}


DDKAPI
LONG KeSetEvent(PRKEVENT Event, KPRIORITY Increment, BOOLEAN Wait)
{
	EVENT *pEvent = (EVENT *)Event;

	DDKASSERT(KeGetCurrentIrql() <= (Wait ? APC_LEVEL : DISPATCH_LEVEL));

	DWORD rc = WaitForSingleObject(pEvent->h, 0);

	if (rc == WAIT_OBJECT_0) {
		if (!pEvent->notify) SetEvent(pEvent->h);
		return 1;
	}

	SetEvent(pEvent->h);
	return 0;
}


DDKAPI
LONG KeResetEvent(PRKEVENT Event)
{
	EVENT *pEvent = (EVENT *)Event;

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	DWORD rc = WaitForSingleObject(pEvent->h, 0);

	if (rc == WAIT_OBJECT_0) {
		if (pEvent->notify) ResetEvent(pEvent->h);
		return 1;
	}

	return 0;
}


DDKAPI
VOID KeClearEvent(PRKEVENT Event)
{
	EVENT *pEvent = (EVENT *)Event;

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	ResetEvent(pEvent->h);
}


DDKAPI
LONG KeReadStateEvent(PRKEVENT Event)
{
	EVENT *pEvent = (EVENT *)Event;

	DDKASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	DWORD rc = WaitForSingleObject(pEvent->h, 0);

	if (rc == WAIT_OBJECT_0) {
		if (!pEvent->notify) SetEvent(pEvent->h);
		return TRUE;
	}

	return FALSE;
}
