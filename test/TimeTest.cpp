/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Time Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkTimeTest)
	{
	public:
		TEST_METHOD_INITIALIZE(DdkTimeTestInit)
		{
			DdkThreadInit();
		}

		TEST_METHOD(DdkTimeTickCount)
		{
			for (int i = 0; true; i++) {
				ULONGLONG x = GetTickCount64(), v;

				KeQueryTickCount(&v);
				v = (v * KeQueryTimeIncrement()) / 10000;

				if (x == v) break;
				Assert::IsTrue(i < 10);
			}
		}

		/*
		 * Convert LARGE_INTEGER to TIME_FIELDS
		 */
		TEST_METHOD(DdkTimeToTimeFields)
		{
			LARGE_INTEGER time;
			time.QuadPart = 123479190430320000;
			const TIME_FIELDS expectedTimeFields = { 1992, 4, 16, 17, 30, 43, 32, 4 };
			TIME_FIELDS timeFields;

			RtlTimeToTimeFields(&time, &timeFields);

			Assert::IsTrue(memcmp(&expectedTimeFields, &timeFields, sizeof(expectedTimeFields)) == 0);
		}

		/*
		 * Convert LARGE_INTEGER to TIME_FIELDS at zero day
		 */
		TEST_METHOD(DdkTimeToTimeFieldsZero)
		{
			TIME_FIELDS timeFields;
			LARGE_INTEGER time = { 0 }; // January 1st 1601
			const TIME_FIELDS expectedTimeFields = { 1601, 1, 1, 0, 0, 0, 0, 1 };

			RtlTimeToTimeFields(&time, &timeFields);

			Assert::IsTrue(memcmp(&expectedTimeFields, &timeFields, sizeof(expectedTimeFields)) == 0);
		}

		/*
		 * Convert TIME_FIELDS to LARGE_INTEGER
		 */
		TEST_METHOD(DdkTimeFieldsToTime)
		{			
			LARGE_INTEGER time, expectedTime;
			TIME_FIELDS timeFields = { 1992, 4, 16, 17, 30, 43, 32, 4 };

			expectedTime.QuadPart = 123479190430320000;

			BOOLEAN returnValue = RtlTimeFieldsToTime(&timeFields, &time);

			Assert::IsTrue(time.QuadPart == expectedTime.QuadPart);
			Assert::IsTrue(returnValue == TRUE);
		}

		/*
		 * Convert TIME_FIELDS to LARGE_INTEGER at zero time
		 */
		TEST_METHOD(DdkTimeFieldsToTimeZero)
		{
			const LARGE_INTEGER expectedTime = { 0 }; // January 1st 1601

			LARGE_INTEGER time;
			TIME_FIELDS timeFields = { 1601, 1, 1, 0, 0, 0, 0, 1 };
			
			BOOLEAN returnValue = RtlTimeFieldsToTime(&timeFields, &time);

			Assert::IsTrue(time.QuadPart == expectedTime.QuadPart);
			Assert::IsTrue(returnValue == TRUE);
		}

		/*
		 * Retrieve the current system time as a Large_Integer
		 */
		TEST_METHOD(DdkTimeQuerySystemTime)
		{
			LARGE_INTEGER li1, li2;
			FILETIME ft;
			
			GetSystemTimeAsFileTime(&ft);
			KeQuerySystemTime(&li1);
			KeQuerySystemTimePrecise(&li2);

			FILETIME kernelTime1, kernelTime2;
			kernelTime1.dwHighDateTime = li1.HighPart;
			kernelTime1.dwLowDateTime = li1.LowPart;
			
			kernelTime2.dwHighDateTime = li2.HighPart;
			kernelTime2.dwLowDateTime = li2.LowPart;

			Assert::IsTrue(CompareFileTime(&ft, &kernelTime1) <= 0);
			Assert::IsTrue(CompareFileTime(&kernelTime1, &kernelTime2) <= 0);
		}

		/*
		 * Retrieve the current precise time as a Large_Integer
		 */
		TEST_METHOD(DdkTimeQuerySystemTimePrecise)
		{
			LARGE_INTEGER li1, li2;
			FILETIME ft;

			GetSystemTimeAsFileTime(&ft);
			KeQuerySystemTimePrecise(&li1);
			KeQuerySystemTimePrecise(&li2);

			FILETIME kernelTime1, kernelTime2;
			kernelTime1.dwHighDateTime = li1.HighPart;
			kernelTime1.dwLowDateTime = li1.LowPart;

			kernelTime2.dwHighDateTime = li2.HighPart;
			kernelTime2.dwLowDateTime = li2.LowPart;

			Assert::IsTrue(CompareFileTime(&ft, &kernelTime1) <= 0);
			Assert::IsTrue(CompareFileTime(&kernelTime1, &kernelTime2) <= 0);
		}

	};
}
