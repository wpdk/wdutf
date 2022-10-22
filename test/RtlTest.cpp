/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Rtl Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkRtlTest)
	{
	public:
		static const int BUF_LEN = 10;

		TEST_METHOD_INITIALIZE(DdkRtlTestInit)
		{
			DdkThreadInit();
		}

		/*
		 * Zero All bytes within a buffer
		 */
		TEST_METHOD(DdkRtlZeroMemory)
		{
			UCHAR TestBuf[BUF_LEN];
			memset(&TestBuf, 0xff, BUF_LEN);

			RtlZeroMemory(&TestBuf, BUF_LEN);

			for (int i = 0; i < BUF_LEN; i++)
				Assert::IsTrue(TestBuf[i] == 0);
		}

		/*
		 * Zero Half the bytes within a buffer
		 */
		TEST_METHOD(DdkRtlZeroMemoryHalf)
		{
			UCHAR TestBuf[BUF_LEN];
			memset(&TestBuf, 0xff, BUF_LEN);

			RtlZeroMemory(&TestBuf, (BUF_LEN / 2));

			for (int i = 0; i < (BUF_LEN / 2); i++)
				Assert::IsTrue(TestBuf[i] == 0);
			
			for (int i = (BUF_LEN / 2); i < BUF_LEN; i++)
				Assert::IsTrue(TestBuf[i] == 0xff);			
		}

		/*
		 * Compare two matching buffers
		 */
		TEST_METHOD(DdkRtlCompareMemory)
		{
			UCHAR TestBuf[BUF_LEN];
			UCHAR TestBuf2[BUF_LEN];
			memset(&TestBuf, 0xff, BUF_LEN);
			memset(&TestBuf2, 0xff, BUF_LEN);

			SIZE_T result = RtlCompareMemory(&TestBuf, &TestBuf2, BUF_LEN);
			Assert::IsTrue(result == BUF_LEN);
		}

		/*
		 * Compare with no matches
		 */
		TEST_METHOD(DdkRtlCompareMemoryExpectZero)
		{
			UCHAR TestBuf[BUF_LEN];
			UCHAR TestBuf2[BUF_LEN];
			memset(&TestBuf, 0xff, BUF_LEN);
			memset(&TestBuf2[1], 0xff, BUF_LEN - sizeof(UCHAR));
			TestBuf2[0] = 0;

			SIZE_T result = RtlCompareMemory(&TestBuf, &TestBuf2, BUF_LEN);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Compare with only half the bytes matching
		 */
		TEST_METHOD(DdkRtlCompareMemoryExpectHalf)
		{
			UCHAR TestBuf[BUF_LEN];
			UCHAR TestBuf2[BUF_LEN];
			memset(&TestBuf, 0xff, BUF_LEN);
			memset(&TestBuf2, 0xff, (BUF_LEN / 2));

			memset(&TestBuf2[BUF_LEN / 2], 0, (BUF_LEN / 2));

			SIZE_T result = RtlCompareMemory(&TestBuf, &TestBuf2, BUF_LEN);
			Assert::IsTrue(result == (BUF_LEN / 2));
		}

		/*
		 * Compare with size 0
		 */
		TEST_METHOD(DdkRtlCompareMemorySizeZero)
		{
			UCHAR TestBuf[BUF_LEN];
			UCHAR TestBuf2[BUF_LEN];
			memset(&TestBuf, 0xff, BUF_LEN);
			memset(&TestBuf2, 0xff, BUF_LEN);

			SIZE_T result = RtlCompareMemory(&TestBuf, &TestBuf2, 0);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Compare with size 1
		 */
		TEST_METHOD(DdkRtlCompareMemorySizeOne)
		{
			UCHAR TestBuf[BUF_LEN];
			UCHAR TestBuf2[BUF_LEN];
			memset(&TestBuf, 0xff, BUF_LEN);
			memset(&TestBuf2, 0xff, BUF_LEN);

			SIZE_T result = RtlCompareMemory(&TestBuf, &TestBuf2, 1);
			Assert::IsTrue(result == 1);
		}

		/*
		 * Compare with one byte not matching
		 */
		TEST_METHOD(DdkRtlCompareMemoryOneDifferentByte)
		{
			UCHAR TestBuf[BUF_LEN];
			UCHAR TestBuf2[BUF_LEN];
			memset(&TestBuf, 0xff, BUF_LEN);
			memset(&TestBuf2, 0xff, BUF_LEN);

			TestBuf2[BUF_LEN / 2] = 0;

			SIZE_T result = RtlCompareMemory(&TestBuf, &TestBuf2, BUF_LEN);
			Assert::IsTrue(result == (BUF_LEN / 2));
		}

		/*
		 * Compare two larger buffers that match
		 */
		TEST_METHOD(DdkRtlCompareMemoryLargeBuffer)
		{
			UCHAR largeBuffer[1024];
			memset(&largeBuffer, 0xff, sizeof(largeBuffer));
			UCHAR largeBuffer2[1024];
			memset(&largeBuffer2, 0xff, sizeof(largeBuffer2));

			SIZE_T result = RtlCompareMemory(&largeBuffer, &largeBuffer2, sizeof(largeBuffer));
			Assert::IsTrue(result == sizeof(largeBuffer));
		}
		
		/*
		 * Compare two larger buffers with last UCHAR not matching
		 */
		TEST_METHOD(DdkRtlCompareMemoryLargeBufferZeroLast)
		{
			UCHAR largeBuffer[1024];
			memset(&largeBuffer, 0xff, sizeof(largeBuffer));
			UCHAR largeBuffer2[1024];
			memset(&largeBuffer2, 0xff, (sizeof(largeBuffer2)-sizeof(UCHAR)));
			largeBuffer2[1023] = 0;

			SIZE_T result = RtlCompareMemory(&largeBuffer, &largeBuffer2, sizeof(largeBuffer));
			Assert::IsTrue(result == (sizeof(largeBuffer) - sizeof(UCHAR)));
		}

		/*
		 * Compare two larger buffers first UCHAR not matching
		 */
		TEST_METHOD(DdkRtlCompareMemoryLargeBufferZeroFirst)
		{
			UCHAR largeBuffer[1024];
			memset(&largeBuffer, 0xff, sizeof(largeBuffer));
			UCHAR largeBuffer2[1024];
			memset(&largeBuffer2, 0xff, sizeof(largeBuffer2));
			largeBuffer2[0] = 0;

			SIZE_T result = RtlCompareMemory(&largeBuffer, &largeBuffer2, sizeof(largeBuffer));
			Assert::IsTrue(result == 0);
		}

		/*
		 * Compare two bytes
		 */
		TEST_METHOD(DdkRtlCompareMemorySmallBuffer)
		{
			unsigned char smallBuffer = 0xff;
			unsigned char smallBuffer2 = 0xff;

			SIZE_T result = RtlCompareMemory(&smallBuffer, &smallBuffer2, sizeof(smallBuffer));
			Assert::IsTrue(result == 1);
		}

		/*
		 * Compare two zero'd bytes
		 */
		TEST_METHOD(DdkRtlCompareMemoryMatchingZeroBytes)
		{
			unsigned char smallBuffer = 0;
			unsigned char smallBuffer2 = 0;

			SIZE_T result = RtlCompareMemory(&smallBuffer, &smallBuffer2, sizeof(smallBuffer));
			Assert::IsTrue(result == 1);
		}		

		/*
		* Compare two non matching bytes
		*/
		TEST_METHOD(DdkRtlCompareMemoryNonMatchingBytes)
		{
			unsigned char smallBuffer = 0;
			unsigned char smallBuffer2 = 0xff;

			SIZE_T result = RtlCompareMemory(&smallBuffer, &smallBuffer2, sizeof(smallBuffer));
			Assert::IsTrue(result == 0);
		}

		TEST_METHOD(DdkRtlGetVersionStandardInfo)
		{
			RTL_OSVERSIONINFOW osInfo;
			osInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

			NTSTATUS returnStatus = RtlGetVersion(&osInfo);

			Assert::IsTrue(returnStatus == STATUS_SUCCESS);

			Assert::IsTrue(osInfo.dwOSVersionInfoSize == sizeof(RTL_OSVERSIONINFOW));
			Assert::IsTrue(osInfo.dwMajorVersion == 0x6); 
			Assert::IsTrue(osInfo.dwMinorVersion == 0x3);
			Assert::IsTrue(osInfo.dwBuildNumber == 0x2580);
			Assert::IsTrue(osInfo.dwPlatformId == 0x2);
		}


		TEST_METHOD(DdkRtlGetVersionStandardInfoNoSize)
		{
			RTL_OSVERSIONINFOW osInfo;

			NTSTATUS returnStatus = RtlGetVersion(&osInfo);

			Assert::IsTrue(returnStatus == STATUS_SUCCESS);

			Assert::IsTrue(osInfo.dwOSVersionInfoSize == sizeof(RTL_OSVERSIONINFOW));
			Assert::IsTrue(osInfo.dwMajorVersion == 0x6);
			Assert::IsTrue(osInfo.dwMinorVersion == 0x3);
			Assert::IsTrue(osInfo.dwBuildNumber == 0x2580);
			Assert::IsTrue(osInfo.dwPlatformId == 0x2);
		}

		TEST_METHOD(DdkRtlGetVersionExtendedInfo)
		{
			RTL_OSVERSIONINFOEXW osInfo;
			osInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

			NTSTATUS returnStatus = RtlGetVersion((PRTL_OSVERSIONINFOW)&osInfo);

			Assert::IsTrue(returnStatus == STATUS_SUCCESS);

			Assert::IsTrue(osInfo.dwOSVersionInfoSize == sizeof(RTL_OSVERSIONINFOEXW));
			Assert::IsTrue(osInfo.dwMajorVersion == 0x6);
			Assert::IsTrue(osInfo.dwMinorVersion == 0x3);
			Assert::IsTrue(osInfo.dwBuildNumber == 0x2580);
			Assert::IsTrue(osInfo.dwPlatformId == 0x2);

			Assert::IsTrue(osInfo.wServicePackMajor == 0x0);
			Assert::IsTrue(osInfo.wServicePackMinor == 0x0);
			Assert::IsTrue(osInfo.wSuiteMask == 0x110);
			Assert::IsTrue(osInfo.wProductType == 0x03);
			Assert::IsTrue(osInfo.wReserved == 0x00);
		}
		
		TEST_METHOD(DdkByteSwapUshort)
		{
			const USHORT source = 0x0123;

			USHORT result = RtlUshortByteSwap(source);

			Assert::IsTrue(0x2301 == result);
		}

		TEST_METHOD(DdkByteSwapUshortNull)
		{
			USHORT result = RtlUshortByteSwap(NULL);

			Assert::IsTrue(0x0000 == result);
		}

		TEST_METHOD(DdkByteSwapUlong)
		{
			const ULONG source = 0x01234567;

			ULONG result = RtlUlongByteSwap(source);

			Assert::IsTrue(0x67452301 == result);
		}

		TEST_METHOD(DdkByteSwapUlongNull)
		{
			ULONG result = RtlUlongByteSwap(NULL);

			Assert::IsTrue(0x00000000 == result);
		}

		TEST_METHOD(DdkByteSwapUlonglong)
		{
			const ULONGLONG source = 0x0123456789ABCDEF;

			ULONGLONG result = RtlUlonglongByteSwap(source);

			Assert::IsTrue(0xEFCDAB8967452301 == result);
		}

		TEST_METHOD(DdkByteSwapUlonglongNull)
		{
			ULONGLONG result = RtlUlonglongByteSwap(NULL);

			Assert::IsTrue(0x0000000000000000 == result);
		}
	};
}
