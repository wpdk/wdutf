/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Time Routines
 */

#include "stdafx.h"


ULONG KeMaximumIncrement;

typedef VOID (*PGETPRECISE)(LPFILETIME);
static PGETPRECISE pGetSystemTimePreciseAsFileTime;


void DdkTimeInit()
{
	ULONGLONG v = 10000I64 * DdkGetTickCountMultiplier();
	v += ((1 << 23) - 1);		// round up before shift as this is what the kernel seems to do
	KeMaximumIncrement = (ULONG)(v >> 24);

	pGetSystemTimePreciseAsFileTime = (PGETPRECISE)GetProcAddress(
		GetModuleHandle("kernel32.dll"), "GetSystemTimePreciseAsFileTime");

	if (!pGetSystemTimePreciseAsFileTime)
		pGetSystemTimePreciseAsFileTime = &GetSystemTimeAsFileTime;
}


DDKAPI
VOID KeQueryTickCount(PLARGE_INTEGER CurrentCount)
{
	CurrentCount->QuadPart = (LONGLONG)DdkGetTickCount();
}


DDKAPI
ULONG KeQueryTimeIncrement()
{
	return KeMaximumIncrement;
}


DWORD DdkGetWaitTime(LARGE_INTEGER *pTimeout)
{
	if (!pTimeout) return INFINITE;

	if (pTimeout->QuadPart <= 0)
		return (DWORD)((-pTimeout->QuadPart) / 10000);

	LARGE_INTEGER v;
	FILETIME now;

	GetSystemTimeAsFileTime(&now);
	
	v.HighPart = now.dwHighDateTime;
	v.LowPart = now.dwLowDateTime;

	if (v.QuadPart > pTimeout->QuadPart)
		return 0;

	v.QuadPart = (pTimeout->QuadPart - v.QuadPart) / 10000;

	if (v.HighPart)
		ddkfail("Wait timeout is too large");

	return v.LowPart;
}


DWORD DdkGetDelayTime(LARGE_INTEGER *pTimeout)
{
	if (!pTimeout)
		ddkfail("Timeout is zero pointer");

	return DdkGetWaitTime(pTimeout);
}


FILETIME DdkGetDueTime(LARGE_INTEGER Timeout)
{
	FILETIME x;
	x.dwHighDateTime = Timeout.HighPart;
	x.dwLowDateTime = Timeout.LowPart;
	return x;
}


DDKAPI
VOID RtlTimeToTimeFields(PLARGE_INTEGER Time, PTIME_FIELDS TimeFields)
{
	FILETIME ft;
	SYSTEMTIME st;

	ft.dwLowDateTime = Time->LowPart;
	ft.dwHighDateTime = Time->HighPart;

	if (!FileTimeToSystemTime(&ft, &st))
		ddkfail("Could not convert time structure");

	TimeFields->Year = st.wYear;
	TimeFields->Month = st.wMonth;
	TimeFields->Day = st.wDay;
	TimeFields->Hour = st.wHour;
	TimeFields->Minute = st.wMinute;
	TimeFields->Second = st.wSecond;
	TimeFields->Milliseconds = st.wMilliseconds;
	TimeFields->Weekday = st.wDayOfWeek;
}


DDKAPI
BOOLEAN RtlTimeFieldsToTime(PTIME_FIELDS TimeFields, PLARGE_INTEGER Time)
{
	FILETIME ft;
	SYSTEMTIME st;

	st.wYear = TimeFields->Year;
	st.wMonth = TimeFields->Month;
	st.wDay = TimeFields->Day;
	st.wHour = TimeFields->Hour;
	st.wMinute = TimeFields->Minute;
	st.wSecond = TimeFields->Second;
	st.wMilliseconds = TimeFields->Milliseconds;
	st.wDayOfWeek = TimeFields->Weekday;

	if (!SystemTimeToFileTime(&st, &ft))
		return FALSE;

	Time->LowPart = ft.dwLowDateTime;
	Time->HighPart = ft.dwHighDateTime;

	return TRUE;
}


DDKAPI
VOID ExSystemTimeToLocalTime(PLARGE_INTEGER SystemTime, PLARGE_INTEGER LocalTime)
{
	FILETIME SysTime;
	FILETIME localTime;

	SysTime.dwLowDateTime = SystemTime->LowPart;
	SysTime.dwHighDateTime = SystemTime->HighPart;

	if (!FileTimeToLocalFileTime(&SysTime, &localTime))
		ddkfail("Could not convert system time to a valid local time");

	LocalTime->LowPart = localTime.dwLowDateTime;
	LocalTime->HighPart = localTime.dwHighDateTime;
}


#undef KeQuerySystemTime
DDKAPI
VOID KeQuerySystemTime(PLARGE_INTEGER CurrentTime)
{
	FILETIME ft;

	GetSystemTimeAsFileTime(&ft);

	CurrentTime->LowPart = ft.dwLowDateTime;
	CurrentTime->HighPart = ft.dwHighDateTime;
}


DDKAPI
VOID KeQuerySystemTimePrecise(PLARGE_INTEGER CurrentTime)
{
	FILETIME ft;

	(*pGetSystemTimePreciseAsFileTime)(&ft);

	CurrentTime->LowPart = ft.dwLowDateTime;
	CurrentTime->HighPart = ft.dwHighDateTime;
}
