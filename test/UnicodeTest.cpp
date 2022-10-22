/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2016, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	String Tests
 */

#include "stdafx.h"
#include "initguid.h"


// The maximum MaximumLength for a UNICODE_STRING 
#define MAX_USTRING_WCHARS (MAXUSHORT / sizeof(WCHAR))			// 0x7FFF  WCHARs


namespace DdkUnitTest
{
	DEFINE_GUID(UNICODE_TEST_GUID, 0x6B29FC40, 0xCA47, 0x1067, 0xB3, 0x1D, 0x00, 0xDD, 0x01, 0x06, 0x62, 0xDA);

	TEST_CLASS(DdkUnicodeTest)
	{
	public:

		/*
		 * Initialise a short string 
		 */
		TEST_METHOD(DdkUnicodeInitStringShort)
		{
			UNICODE_STRING unicodeString;
			const WCHAR* wideString = L"abc";
			RtlInitUnicodeString(&unicodeString, wideString);
			Assert::IsTrue(unicodeString.Buffer == wideString);
			Assert::IsTrue(unicodeString.Length == wcslen(wideString) * sizeof(WCHAR));
			Assert::IsTrue(unicodeString.MaximumLength == (wcslen(wideString) * sizeof(WCHAR)) + sizeof(WCHAR));
		}

		/*
		 * Initialize an empty string
		 */
		TEST_METHOD(DdkUnicodeInitStringEmpty)
		{
			UNICODE_STRING unicodeString;
			const WCHAR* wideString = NULL;
			RtlInitUnicodeString(&unicodeString, wideString);
			Assert::IsTrue(unicodeString.Buffer == NULL);
			Assert::IsTrue(unicodeString.Length == 0);
			Assert::IsTrue(unicodeString.MaximumLength == 0);
		}

		/*
		 * Pass a maximum-length string (MAXUSHORT -1) to RtlInitUnicodeString;
		 */
		TEST_METHOD(DdkUnicodeInitStringLong)
		{
			const LONG64 STRING_LEN = MAX_USTRING_WCHARS;	// 0x7FFF
			UNICODE_STRING unicodeString;
			WCHAR* wideString = (WCHAR*)malloc((STRING_LEN) * sizeof(WCHAR));	// allocate space for string + a null terminator
			Assert::IsTrue(wideString != NULL);
			wmemset(wideString, L'A', STRING_LEN-1);
			wideString[STRING_LEN-1] = UNICODE_NULL;
			RtlInitUnicodeString(&unicodeString, wideString);

			Assert::IsTrue(unicodeString.Buffer == wideString);
			Assert::IsTrue(unicodeString.Length == wcslen(wideString) * sizeof(WCHAR));
			Assert::IsTrue(unicodeString.MaximumLength == (wcslen(wideString) * sizeof(WCHAR)) + sizeof(WCHAR));

			free(wideString);
		}

		/*
		 * Pass a string which is too long (> MAXUSHORT -1) to RtlInitUnicodeString; 
		 * RtlInitUnicodeString should call ddkfail when this happens
		 */
		TEST_METHOD(DdkUnicodeInitStringTooLong)
		{
			const LONG64 STRING_LEN = MAX_USTRING_WCHARS + 1;	// 0x8000
			UNICODE_STRING unicodeString;
			WCHAR* wideString = (WCHAR*) malloc((STRING_LEN) * sizeof(WCHAR));	// allocate space for string + a null terminator
			Assert::IsTrue(wideString != NULL);
			wmemset(wideString, L'A', STRING_LEN-1);
			wideString[STRING_LEN-1] = 0;
			
			TEST_DDK_FAIL(RtlInitUnicodeString(&unicodeString, wideString));

			free(wideString);
		}

		/*
		 * Copy a short unicode string to a destination unicode string with sufficient buffer space
		 */
		TEST_METHOD(DdkUnicodeCopyStringShort)
		{
			UNICODE_STRING unicodeStringSrc;
			const WCHAR* wideStringSrc = L"abc";
			RtlInitUnicodeString(&unicodeStringSrc, wideStringSrc);

			const int DestStringLen = 4;
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, DestStringLen);

			RtlCopyUnicodeString(&unicodeStringDst, &unicodeStringSrc);

			Assert::IsTrue(unicodeStringDst.Buffer == unicodeStringDst_buffer);					// dest buffer should be unmodified
			Assert::IsTrue(unicodeStringDst.MaximumLength == sizeof(unicodeStringDst_buffer));	// dest buffer size should be unmodified
			Assert::IsTrue(unicodeStringDst.Length == unicodeStringSrc.Length);
			Assert::IsTrue(wcsncmp(unicodeStringDst.Buffer, unicodeStringSrc.Buffer, unicodeStringSrc.Length / sizeof(WCHAR)) == 0);
		}

		/*
		 * Copy a short unicode string to a destination unicode string with insufficient buffer space
		 */
		TEST_METHOD(DdkUnicodeCopyStringTruncated)
		{
			UNICODE_STRING unicodeStringSrc;
			const WCHAR* wideStringSrc = L"abc";
			RtlInitUnicodeString(&unicodeStringSrc, wideStringSrc);

			const int DestStringLen = 2;
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, DestStringLen);

			RtlCopyUnicodeString(&unicodeStringDst, &unicodeStringSrc);

			Assert::IsTrue(unicodeStringDst.Buffer == unicodeStringDst_buffer);					// dest buffer should be unmodified
			Assert::IsTrue(unicodeStringDst.MaximumLength == sizeof(unicodeStringDst_buffer));	// dest buffer size should be unmodified
			Assert::IsTrue(unicodeStringDst.Length == DestStringLen * sizeof(WCHAR));
			Assert::IsTrue(wcsncmp(unicodeStringDst.Buffer, unicodeStringSrc.Buffer, DestStringLen) == 0);
		}

		/*
		 * Copy an empty unicode string
		 */
		TEST_METHOD(DdkUnicodeCopyStringEmpty)
		{
			const int DestStringLen = 2;
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, DestStringLen);

			RtlCopyUnicodeString(&unicodeStringDst, NULL);

			Assert::IsTrue(unicodeStringDst.Buffer == unicodeStringDst_buffer);					// dest buffer should be unmodified
			Assert::IsTrue(unicodeStringDst.MaximumLength == sizeof(unicodeStringDst_buffer));	// dest buffer size should be unmodified
			Assert::IsTrue(unicodeStringDst.Length == 0);
		}

		/*
		 * Convert an unsigned integer to a string
		 */
		TEST_METHOD(DdkUnicodeIntegerToString)
		{
			ULONG base = 10;
			const unsigned int num = 2147483647;
			const WCHAR valStr[] = L"2147483647";

			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, (sizeof(valStr) / sizeof(WCHAR)));

			NTSTATUS status = RtlIntegerToUnicodeString(num, base, &unicodeStringDst);
			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(unicodeStringDst.Length == wcslen(valStr) * sizeof(WCHAR));
			Assert::IsTrue(wcsncmp(unicodeStringDst_buffer, valStr, unicodeStringDst.Length / sizeof(WCHAR)) == 0);
		}

		/*
		 * Convert an integer to a hex string
		 */
		TEST_METHOD(DdkUnicodeIntegerToStringHex)
		{
			ULONG base = 16;
			const unsigned int num = 0x12345678;
			const WCHAR valStr[] = L"12345678";

			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, (sizeof(valStr) / sizeof(WCHAR)));

			NTSTATUS status = RtlIntegerToUnicodeString(num, base, &unicodeStringDst);
			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(unicodeStringDst.Length == wcslen(valStr) * sizeof(WCHAR));
			Assert::IsTrue(wcsncmp(unicodeStringDst_buffer, valStr, unicodeStringDst.Length / sizeof(WCHAR)) == 0);
		}

		/*
		 * Convert an integer to a binary string
		 */
		TEST_METHOD(DdkUnicodeIntegerToStringBinary)
		{
			ULONG base = 2;
			const unsigned int num = 0xFFFFFFFE;
			const WCHAR valStr[] = L"11111111111111111111111111111110";

			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, (sizeof(valStr) / sizeof(WCHAR)));

			NTSTATUS status = RtlIntegerToUnicodeString(num, base, &unicodeStringDst);
			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(unicodeStringDst.Length == wcslen(valStr) * sizeof(WCHAR));
			Assert::IsTrue(wcsncmp(unicodeStringDst_buffer, valStr, unicodeStringDst.Length / sizeof(WCHAR)) == 0);
		}

		/*
		 * Convert an integer to a string where the string buffer supplied is too small
		 */
		TEST_METHOD(DdkUnicodeIntegerToStringOverflow)
		{
			ULONG base = 10;
			const unsigned int num = 1234567890;
			const WCHAR valStr[] = L"1234567890";

			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, 2);	// declare a 2 char buffer

			NTSTATUS status = RtlIntegerToUnicodeString(num, base, &unicodeStringDst);
			Assert::IsTrue(status == STATUS_BUFFER_OVERFLOW);
		}

		/*
		 * Compare 2 identical strings with the same case
		 */
		TEST_METHOD(DdkUnicodeCompareStringsIdentical)
		{
			UNICODE_STRING str1 = RTL_CONSTANT_STRING(L"abcde");
			UNICODE_STRING str2 = RTL_CONSTANT_STRING(L"abcde");
			LONG res = RtlCompareUnicodeString(&str1, &str2, true);
			Assert::IsTrue(res == 0);
			res = RtlCompareUnicodeString(&str1, &str2, false);
			Assert::IsTrue(res == 0);
		}

		/*
		 * Compare 2 different strings
		 */
		TEST_METHOD(DdkUnicodeCompareStringsDiff)
		{
			UNICODE_STRING str1 = RTL_CONSTANT_STRING(L"abcde1");
			UNICODE_STRING str2 = RTL_CONSTANT_STRING(L"abcde2");
			LONG res = RtlCompareUnicodeString(&str1, &str2, true);
			Assert::IsTrue(res < 0);
			res = RtlCompareUnicodeString(&str1, &str2, false);
			Assert::IsTrue(res < 0);
		}

		/*
		 * Compare 2 strings, which differ only by case
		 */
		TEST_METHOD(DdkUnicodeCompareStringsDiffCase)
		{
			UNICODE_STRING str1 = RTL_CONSTANT_STRING(L"Abcde");
			UNICODE_STRING str2 = RTL_CONSTANT_STRING(L"abcde");
			LONG res = RtlCompareUnicodeString(&str1, &str2, true);
			Assert::IsTrue(res == 0);
			res = RtlCompareUnicodeString(&str1, &str2, false);
			Assert::IsTrue(res != 0);
		}

		/*
		 * Concatenates a null-terminated unicode string to a buffered Unicode string.
		 */
		TEST_METHOD(DdkUnicodeAppendToString)
		{
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, 10);
			const WCHAR unicodeStringSrc[] = L"67890";
			const WCHAR expectedString[] = L"1234567890";

			RtlAppendUnicodeToString(&unicodeStringDst, L"12345");
			NTSTATUS status = RtlAppendUnicodeToString(&unicodeStringDst, unicodeStringSrc);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(unicodeStringDst.Length == wcslen(expectedString) * sizeof(WCHAR));
			Assert::IsTrue(wcsncmp(unicodeStringDst.Buffer, expectedString, wcslen(expectedString)) == 0);
		}

		/*
		 * Concatenates a null-terminated unicode string to a buffered Unicode string which is too small
		 */
		TEST_METHOD(DdkUnicodeAppendToStringTooSmall)
		{
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, 9);
			const WCHAR unicodeStringSrc[] = L"67890";
			const WCHAR expectedString[] = L"1234567890";

			RtlAppendUnicodeToString(&unicodeStringDst, L"12345");
			NTSTATUS status = RtlAppendUnicodeToString(&unicodeStringDst, unicodeStringSrc);

			Assert::IsTrue(status == STATUS_BUFFER_TOO_SMALL);
		}

		/*
		 * Concatenates 2 unicode strings
		 */
		TEST_METHOD(DdkUnicodeAppendStringToString)
		{
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, 10);
			UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"67890");
			UNICODE_STRING unicodeStringExpected = RTL_CONSTANT_STRING(L"1234567890");

			RtlAppendUnicodeToString(&unicodeStringDst, L"12345");
			NTSTATUS status = RtlAppendUnicodeStringToString(&unicodeStringDst, &unicodeStringSrc);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(unicodeStringDst.Length == unicodeStringExpected.Length);
			Assert::IsTrue(wcsncmp(unicodeStringDst.Buffer, unicodeStringExpected.Buffer, unicodeStringExpected.Length / sizeof(WCHAR)) == 0);
		}

		/*
		 * Concatenates one unicode string to another which is too short
		 */
		TEST_METHOD(DdkUnicodeAppendStringToStringTooSmall)
		{
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, 9);
			UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"67890");

			RtlAppendUnicodeToString(&unicodeStringDst, L"12345");
			NTSTATUS status = RtlAppendUnicodeStringToString(&unicodeStringDst, &unicodeStringSrc);

			Assert::IsTrue(status == STATUS_BUFFER_TOO_SMALL);
		}

		/*
		 * Convert a unicode string to upper case
		 */
		TEST_METHOD(DdkUnicodeUpcaseString)
		{
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, 24);
			UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"My name is Michael Caine");
			UNICODE_STRING unicodeStringExpected = RTL_CONSTANT_STRING(L"MY NAME IS MICHAEL CAINE");
			NTSTATUS status = RtlUpcaseUnicodeString(&unicodeStringDst, &unicodeStringSrc, false);
			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(unicodeStringDst.Length == unicodeStringExpected.Length);
			Assert::IsTrue(wcsncmp(unicodeStringDst.Buffer, unicodeStringExpected.Buffer, unicodeStringExpected.Length / sizeof(WCHAR)) == 0);
		}

		/*
		 * Convert a unicode string to upper case, allocating the destination buffer
		 */
		TEST_METHOD(DdkUnicodeUpcaseStringAlloc)
		{
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, 24);
			UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"My name is Michael Caine");
			UNICODE_STRING unicodeStringExpected = RTL_CONSTANT_STRING(L"MY NAME IS MICHAEL CAINE");
			NTSTATUS status = RtlUpcaseUnicodeString(&unicodeStringDst, &unicodeStringSrc, true);
			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(unicodeStringDst.Length == unicodeStringExpected.Length);
			Assert::IsTrue(unicodeStringDst.Buffer != unicodeStringDst_buffer);
			Assert::IsTrue(wcsncmp(unicodeStringDst.Buffer, unicodeStringExpected.Buffer, unicodeStringExpected.Length / sizeof(WCHAR)) == 0);
			
			RtlFreeUnicodeString(&unicodeStringDst);
			Assert::IsTrue(unicodeStringDst.Buffer == NULL);
		}

		/*
		 * Convert a unicode string containing a decimal number to an integer, passing a base
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalWithBase)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L" 214748364ze5h7");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 10, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 214748364);
		}

		/*
		 * Convert a unicode string containing a decimal number to an integer, passing a base but no excess data terminating the number or preceding spaces
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalClean)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"214748364");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 10, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 214748364);
		}


		/*
		 * Convert a null terminated unicode string containing a decimal number to an integer, passing a base
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalWithNullTerminator)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L" 214748364\0");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 10, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 214748364);
		}

		/*
		 * Convert a unicode string containing a decimal number to an integer, passing no base
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalNoBase)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	0214748364ze5h7");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 214748364);
		}

		/*
		 * Convert a unicode string containing a negative decimal number to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalNegative)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	-214748364ze5h7");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 10, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == -214748364);
		}

		/*
		 * Convert a unicode string containing 0 with no base
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalZero)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"0");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Convert a unicode string containing a decimal number with a series of preceding zeros
		 * Passing in a base and no prefix in the string
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalPrecedingZeros)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"-0000214748364ze5h7");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 10, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == -214748364);
		}

		/*
		 * Convert a unicode string containing a decimal number with a series of preceding zero
		 * no base and no prefix in the string
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalPrecedingZerosNoBase)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"00001010");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 1010);
		}

		/*
		 * Convert a unicode string containing the highest possible positive hex number
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexMax)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"+0xFFFFFFFF");
			ULONG result = 0;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0xFFFFFFFF);
		}

		/*
		 * Convert a unicode string containing the lowest possible negative decimal number
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalMaxNegative)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"-2147483648");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0x80000000);
		}

		/*
		 * Convert a unicode string containing decimal value greater than MAX_ULONG 
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalOverflow)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"1836873016182535765326452654254231");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 10, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 3815155863);
		}

		/*
		 * Convert a unicode string containing negative decimal value less than MIN_LONG
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerDecimalNegativeOverflow)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"-1836873016182535765326452654254231");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 10, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 479811433);
		}

		/*
		 * Convert a unicode string containing a hexadecimal number, passing base but no prefix to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexBaseNoPrefix)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	+123456FDzt4g");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 16, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0x123456FD);
		}

		/*
		 * Convert a unicode string containing a hexadecimal number, passing prefix but no base to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexPrefixNoBase)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"   0x023567cfu7");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0x023567cf);
		}

		/*
		 * Convert a unicode string containing a hexadecimal number, passing the base and a prefix to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexPrefixAndBase)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"   +0x234567cfu7");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 16, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Convert a unicode string containing a hexadecimal number, passing the base and an incorrect prefix to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexBaseAndIncorrectPrefix)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"   +0o234567cfu7");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 16, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Convert a unicode string containing a hexadecimal number with a series of preceding zeros
		 * passing the base and a prefix to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexPrecedingZeros)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"   +0x000002367cfu7");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0x000002367cf);
		}

		/*
		 * Convert a unicode string containing a negative hexadecimal number passing a prefix
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexNegative)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"   -0x834567cfu7");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0x7cba9831);
		}
		
		/*
		 * Convert a unicode string containing a negative hexadecimal number with an invalid prefix
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexUpperCasePrefixNoBase)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"   -0X234567cfu7");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Convert a unicode string containing a negative hexadecimal number, passing the base and a prefix to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexUpperCase)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"0xABCD");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0xABCD);
		}

		/*
		 * Convert a unicode string containing a Hex value greater than MAX_ULONG
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexOverflow)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"123456789ABCDE0123456789ABCDE");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 16, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 2023406814);
		}

		/*
		 * Convert a unicode string containing a negative Hex value less than MIN_LONG
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerHexNegativeOverflow)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"-123456789ABCDE0123456789ABCDE");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 16, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 2271560482);
		}

		/*
		 * Convert a unicode string containing a binary number, passing both a prefix but no base to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerBinaryPrefixNoBase)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	0b10102g5e4z");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 10);
		}

		/*
		 * Convert a unicode string containing a binary number, passing the base and no prefix to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerBinaryBaseNoPrefix)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	1010g5e4z");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 2, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 10);
		}

		/*
		 * Convert a unicode string containing a binary number, passing both a prefix and the base to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerBinaryBaseAndPrefix)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	0b1010g5e4z");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 2, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Convert a unicode string containing a binary number, passing both a prefix and the base to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerBinaryBaseAndIncorrectPrefix)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	0x1010g5e4z");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 2, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0);
		}
		
		/*
		 * Convert a unicode string containing a negative binary number
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerBinaryNegative)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	-0101010g45ez");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 2, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == (ULONG)-42);
		}

		/*
		 * Convert a unicode string containing a binary number with preceding zeros
		 * Passing in the base parameter and no prefix in the string
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerBinaryWithPreceedingZeros)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	00000101010g45ez");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 2, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == (ULONG)42);
		}

		/*
		* Convert a unicode string containing negative binary value greater than MAX_ULONG
		*/
		TEST_METHOD(DdkUnicodeStringToIntegerBinaryNegativeOverflow)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"-10000010100001001000011010001000100010101000110010001110101010101010101010");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 2, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 3452261718);
		}

		/*
		 * Convert a unicode string containing binary value greater than MAX_ULONG
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerBinaryOverflow)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"10000010100001001000011010001000100010101000110010001110101010101010101010");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 2, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 842705578);
		}

		/*
		 * Convert a unicode string containing a octal number, passing the base but no prefix to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerOctalBaseNoPrefix)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	+705137248hnjgf432");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 8, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 070513724);
		}
		
		/*
		 * Convert a unicode string containing a octal number, passing a prefix but no base to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerOctalPrefixNoBase)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	0o70513724hnjgf432");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 0, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 070513724);
		}

		/*
		 * Convert a unicode string containing a octal number, passing both a prefix and the base to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerOctalBaseAndPrefix)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	0o70513724hnjgf432");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 8, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Convert a unicode string containing a octal number, passing both the base an incorrect prefix to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerOctalBaseAndIncorrectPrefix)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	0x70513724hnjgf432");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 8, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Convert a unicode string containing a octal number with preceding zeros to an integer
		 * passing the base with no prefix in the string
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerOctalPrecedingZeros)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	000705137hnjgf432");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 8, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 000705137);
		}

		/*
		 * Convert a unicode string containing a negative octal number
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerOctalNegative)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"	-070513724hnjgf432");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 8, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == -070513724);
		}

		/*
		 * Convert a unicode string containing an octal value greater than MAX_ULONG
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerOctalOverflow)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"10120441504212431654235642564307");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 8, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 4002080967);
		}

		/*
		 * Convert a unicode string containing a negative octal value greater than MAX_ULONG
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerOctalNegativeOverflow)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"-10120441504212431654235642564307");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 8, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 292886329);
		}

		/*
		 * Attempt to convert an empty unicode string to an integer
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerEmptyString)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"");			
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 10, &result);

			Assert::IsTrue(status == STATUS_INVALID_PARAMETER);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Attempt to convert a unicode string to an integer passing in an invalid base value.
		 */
		TEST_METHOD(DdkUnicodeStringToIntegerInvalidBase)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"0x123ABC");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 42, &result);

			Assert::IsTrue(status == STATUS_INVALID_PARAMETER);
			Assert::IsTrue(result == 0);
		}

		/*
		* Attempt to convert a string with no valid characters passing no base
		*/
		TEST_METHOD(DdkUnicodeStringToIntegerInvalidString)
		{
			const UNICODE_STRING unicodeStringSrc = RTL_CONSTANT_STRING(L"y");
			ULONG result = -1;

			NTSTATUS status = RtlUnicodeStringToInteger(&unicodeStringSrc, 10, &result);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Compare 2 identical strings for equality
		 */
		TEST_METHOD(DdkUnicodeEqualString)
		{
			UNICODE_STRING str1 = RTL_CONSTANT_STRING(L"ABCde");
			UNICODE_STRING str2 = RTL_CONSTANT_STRING(L"abcde");
			BOOLEAN res = RtlEqualUnicodeString(&str1, &str2, true);
			Assert::IsTrue(res == TRUE);
			res = RtlEqualUnicodeString(&str1, &str2, false);
			Assert::IsTrue(res == FALSE);
		}

		/*
		 * Convert an Ansi String To Unicode
		 */
		TEST_METHOD(DdkUnicodeFromAnsiString)
		{
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, 24);
			ANSI_STRING ansiStringSrc = RTL_CONSTANT_STRING("My name is Michael Caine");
			UNICODE_STRING unicodeStringExpected = RTL_CONSTANT_STRING(L"My name is Michael Caine");

			NTSTATUS status = RtlAnsiStringToUnicodeString(&unicodeStringDst, &ansiStringSrc, false);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(unicodeStringDst.Length == unicodeStringExpected.Length);
			Assert::IsTrue(unicodeStringDst.Buffer != NULL);
			Assert::IsTrue(wcsncmp(unicodeStringDst.Buffer, unicodeStringExpected.Buffer, unicodeStringExpected.Length / sizeof(WCHAR)) == 0);
		}

		/*
		 * Convert an Ansi String To Unicode, allocating the buffer
		 */
		TEST_METHOD(DdkUnicodeFromAnsiStringAlloc)
		{
			DECLARE_UNICODE_STRING_SIZE(unicodeStringDst, 24);
			ANSI_STRING ansiStringSrc = RTL_CONSTANT_STRING("My name is Michael Caine");
			UNICODE_STRING unicodeStringExpected = RTL_CONSTANT_STRING(L"My name is Michael Caine");

			NTSTATUS status = RtlAnsiStringToUnicodeString(&unicodeStringDst, &ansiStringSrc, true);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(unicodeStringDst.Length == unicodeStringExpected.Length);
			Assert::IsTrue(unicodeStringDst.Buffer != unicodeStringDst_buffer);
			Assert::IsTrue(wcsncmp(unicodeStringDst.Buffer, unicodeStringExpected.Buffer, unicodeStringExpected.Length / sizeof(WCHAR)) == 0);

			RtlFreeUnicodeString(&unicodeStringDst);
			Assert::IsTrue(unicodeStringDst.Buffer == NULL);
		}

		/*
		 * Get the ansi-size of a unicode string
		 */
		TEST_METHOD(DdkUnicodeStringToAnsiSize)
		{
			UNICODE_STRING unicodeString = RTL_CONSTANT_STRING(L"abcde");
			int len = RtlxUnicodeStringToAnsiSize(&unicodeString);
			Assert::IsTrue(len == 6);
		}

		/*
		 * Convert character to uppercase
		 */
		TEST_METHOD(DdkUnicodeCharToUpper)
		{
			WCHAR chLower = 'a';
			WCHAR chUpper = RtlUpcaseUnicodeChar(chLower);
			Assert::IsTrue(chUpper == L'A');
		}

		/*
		 * Convert character to lowercase
		 */
		TEST_METHOD(DdkUnicodeCharToLower)
		{
			WCHAR chUpper = 'Z';
			WCHAR chLower = RtlDowncaseUnicodeChar(chUpper);
			Assert::IsTrue(chLower == L'z');
		}

		/*
		 * Test Default Hash == X65599 hash of a string.
		 */
		TEST_METHOD(DdkUnicodeHashDefault)
		{
			UNICODE_STRING unicodeString = RTL_CONSTANT_STRING(L"Thi$ Is @ T3ST $tr1ng 4_tH3 D3F@uLt HasH!");
			ULONG defaultResult, x65599Result;

			NTSTATUS defaultStatus = RtlHashUnicodeString(&unicodeString, FALSE, HASH_STRING_ALGORITHM_DEFAULT, &defaultResult);
			NTSTATUS x65599Status = RtlHashUnicodeString(&unicodeString, FALSE, HASH_STRING_ALGORITHM_X65599, &x65599Result);
			Assert::IsTrue(defaultStatus == x65599Status);
			Assert::IsTrue(defaultResult == x65599Result);
		}

		/*
		 * X65599 Hash of lower case string
		 */
		TEST_METHOD(DdkUnicodeHashLower)
		{
			UNICODE_STRING unicodeString = RTL_CONSTANT_STRING(L"test");
			ULONG result;
			const ULONG caseInsensitiveHash = 1720303922UL;
			const ULONG lowerHash = 1195757874UL;

			NTSTATUS returnStatus = RtlHashUnicodeString(&unicodeString, TRUE, HASH_STRING_ALGORITHM_X65599, &result);
			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(result == caseInsensitiveHash);

			returnStatus = RtlHashUnicodeString(&unicodeString, FALSE, HASH_STRING_ALGORITHM_X65599, &result);
			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(result == lowerHash);
		}

		/*
		 * X65599 Hash of upper case string
		 */
		TEST_METHOD(DdkUnicodeHashUpper)
		{
			UNICODE_STRING unicodeString = RTL_CONSTANT_STRING(L"TEST");
			ULONG result;
			const ULONG caseInsensitiveHash = 1720303922UL;
						
			NTSTATUS returnStatus = RtlHashUnicodeString(&unicodeString, TRUE, HASH_STRING_ALGORITHM_X65599, &result);
			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(result == caseInsensitiveHash);

			returnStatus = RtlHashUnicodeString(&unicodeString, FALSE, HASH_STRING_ALGORITHM_X65599, &result);
			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(result == caseInsensitiveHash);
		}

		/*
		 * X65599 Hash of mixed case string
		 */
		TEST_METHOD(DdkUnicodeHashMixed)
		{
			UNICODE_STRING unicodeString = RTL_CONSTANT_STRING(L"TEst");
			ULONG result;

			const ULONG caseInsensitiveHash = 1720303922UL;
			const ULONG mixHash = 1722403122UL;
			
			NTSTATUS returnStatus = RtlHashUnicodeString(&unicodeString, TRUE, HASH_STRING_ALGORITHM_X65599, &result);
			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(result == caseInsensitiveHash);

			returnStatus = RtlHashUnicodeString(&unicodeString, FALSE, HASH_STRING_ALGORITHM_X65599, &result);
			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(result == mixHash);
		}

		/*
		 * Hash of an empty string
		 */
		TEST_METHOD(DdkUnicodeHashEmpty)
		{
			UNICODE_STRING unicodeString = RTL_CONSTANT_STRING(L"");
			ULONG result;

			long returnStatus = RtlHashUnicodeString(&unicodeString, TRUE, HASH_STRING_ALGORITHM_DEFAULT, &result);
			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(result == 0);
		}

		/*
		 * Hash of a null pointer
		 */
		TEST_METHOD(DdkUnicodeHashNULL)
		{
			ULONG result;
			NTSTATUS returnStatus = RtlHashUnicodeString(nullptr, TRUE, HASH_STRING_ALGORITHM_DEFAULT, &result);
			Assert::IsTrue(returnStatus == STATUS_INVALID_PARAMETER);
		}

		/*
		 * Create a Unicode String from a GUID object
		 */
		TEST_METHOD(DdkUnicodeGuidToString)
		{
			UNICODE_STRING guidStr;
			UNICODE_STRING expectedGuidStr = RTL_CONSTANT_STRING(L"{6B29FC40-CA47-1067-B31D-00DD010662DA}");

			NTSTATUS returnStatus = RtlStringFromGUID(UNICODE_TEST_GUID, &guidStr);
			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			
			LONG compareResult = RtlCompareUnicodeString(&guidStr, &expectedGuidStr, false);
			Assert::IsTrue(compareResult == 0);

			RtlFreeUnicodeString(&guidStr);
		}

		/*
		 * Create a GUID object from a Unicode String
		 */
		TEST_METHOD(DdkUnicodeGuidFromString)
		{
			GUID guid;
			UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"{6B29FC40-CA47-1067-B31D-00DD010662DA}");
			const unsigned char expectedData4[8] = { 0xB3, 0x1D, 0x00, 0xDD, 0x01, 0x06, 0x62, 0xDA };

			NTSTATUS returnStatus = RtlGUIDFromString(&unicodeStr, &guid);
			
			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(guid.Data1 == 0x6B29FC40);
			Assert::IsTrue(guid.Data2 == 0xCA47);
			Assert::IsTrue(guid.Data3 == 0x1067);
			for (unsigned int i = 0; i < 8; i++)
				Assert::IsTrue(guid.Data4[i] == expectedData4[i]);
		}		

		/*
		 * Attempt to create a GUID object from a Unicode String without the curly braces
		 */
		TEST_METHOD(DdkUnicodeGuidFromStringMissingBraces)
		{
			GUID guid;
			UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"6B29FC40-CA47-1067-B31D-00DD010662DA");

			NTSTATUS returnStatus = RtlGUIDFromString(&unicodeStr, &guid);

			Assert::IsTrue(returnStatus == STATUS_INVALID_PARAMETER);
		}

		/*
		 * Attempt to create a GUID object from a Unicode String without dashes in the string
		 */
		TEST_METHOD(DdkUnicodeGuidFromStringMissingDashes)
		{
			GUID guid;
			UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"{6B29FC40CA471067B31D00DD010662DA}");

			NTSTATUS returnStatus = RtlGUIDFromString(&unicodeStr, &guid);

			Assert::IsTrue(returnStatus == STATUS_INVALID_PARAMETER);
		}

		/*
		 * Attempt to create a GUID object from a Unicode String swapping the closing brace for a L'.'
		 */
		TEST_METHOD(DdkUnicodeGuidMissingClosingBrace)
		{
			GUID guid;
			UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"{6B29FC40-CA47-1067-B31D-00DD010662DA.");

			NTSTATUS returnStatus = RtlGUIDFromString(&unicodeStr, &guid);

			Assert::IsTrue(returnStatus == STATUS_INVALID_PARAMETER);
		}

		/*
		 * Convert a Unicode string to the respective UTF8 String
		 */
		TEST_METHOD(DdkUnicodeToUTF8)
		{
			const UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"My N\u00c3m\u04d9 is Mi\u08a2ha\u20acl C\u0917in\u20ac\u2122\0");
			const unsigned char expectedUtf8Str[] = { 0x4d, 0x79, 0x20, 0x4e, 0xc3, 0x83, 0x6d, 0xd3, 0x99, 0x20, 0x69, 0x73,
				0x20, 0x4d, 0x69, 0xe0, 0xa2, 0xa2, 0x68, 0x61, 0xe2, 0x82, 0xac, 0x6c, 0x20, 0x43, 0xe0, 0xa4, 0x97, 0x69,
				0x6e, 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2, 0x00 };
			const ULONG utf8StrMaxByteCount = 38;

			char utf8StrDst[utf8StrMaxByteCount];
			ULONG utf8StrActualByteCount;

			NTSTATUS returnStatus = RtlUnicodeToUTF8N(utf8StrDst, utf8StrMaxByteCount, &utf8StrActualByteCount, unicodeStr.Buffer, unicodeStr.Length);
			
			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(utf8StrActualByteCount == utf8StrMaxByteCount);
			Assert::IsTrue(memcmp(&expectedUtf8Str, &utf8StrDst, utf8StrMaxByteCount) == 0);
		}

		/*
		 * Convert an empty Unicode string to a UTF8 String
		 */
		TEST_METHOD(DdkUnicodeToUTF8EmptySourceString)
		{
			UNICODE_STRING *unicodeStr = new UNICODE_STRING();
			unicodeStr->Length = 0;
			unicodeStr->MaximumLength = 0;
			unicodeStr->Buffer = new WCHAR[1];

			const ULONG utf8StrMaxByteCount = 1;

			char utf8StrDst[utf8StrMaxByteCount];
			ULONG utf8StrActualByteCount = 0;

			utf8StrDst[0] = '\0';  // This is just a place holder as NULL value would cause a STATUS_INVALID_PARAMETER

			NTSTATUS returnStatus = RtlUnicodeToUTF8N(utf8StrDst, utf8StrMaxByteCount, &utf8StrActualByteCount, unicodeStr->Buffer, unicodeStr->Length);

			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(utf8StrActualByteCount == 0);

			delete unicodeStr;
		}

		/*
		 * Attempt to convert a Unicode string to a UTF8 string with NULL utf8ActualByteCount
		 */
		TEST_METHOD(DdkUnicodeToUTF8NullActualByteCount)
		{
			const UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"My N\u00c3m\u04d9 is Mi\u08a2ha\u20acl C\u0917in\u20ac\u2122\0");
			const unsigned char expectedUtf8Str[] = { 0x4d, 0x79, 0x20, 0x4e, 0xc3, 0x83, 0x6d, 0xd3, 0x99, 0x20, 0x69, 0x73,
				0x20, 0x4d, 0x69, 0xe0, 0xa2, 0xa2, 0x68, 0x61, 0xe2, 0x82, 0xac, 0x6c, 0x20, 0x43, 0xe0, 0xa4, 0x97, 0x69,
				0x6e, 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2, 0x00 };
			const ULONG utf8StrMaxByteCount = 38;

			char utf8StrDst[utf8StrMaxByteCount];

			NTSTATUS returnStatus = RtlUnicodeToUTF8N(utf8StrDst, utf8StrMaxByteCount, NULL, unicodeStr.Buffer, unicodeStr.Length);

			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(memcmp(&expectedUtf8Str, &utf8StrDst, utf8StrMaxByteCount) == 0);
		}

		/*
		 * Attempt to convert a Unicode string to a UTF8 string, with a NULL destination pointer
		 */
		TEST_METHOD(DdkUnicodeToUTF8NullDestination)
		{
			const UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"My N\u00c3m\u04d9 is Mi\u08a2ha\u20acl C\u0917in\u20ac\u2122\0");
			const ULONG utf8StrMaxByteCount = 38;

			ULONG utf8StrActualByteCount;

			NTSTATUS returnStatus = RtlUnicodeToUTF8N(NULL, utf8StrMaxByteCount, &utf8StrActualByteCount, unicodeStr.Buffer, unicodeStr.Length);

			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(utf8StrActualByteCount == utf8StrMaxByteCount);
		}

		/*
		 * Attempt to convert a Unicode string to a UTF8 string, with a NULL destination pointer and UTF8StringActualByteCount
		 */
		TEST_METHOD(DdkUnicodeToUTF8NullDestinationAndActualByteCount)
		{
			const UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"My N\u00c3m\u04d9 is Mi\u08a2ha\u20acl C\u0917in\u20ac\u2122\0");
			const ULONG utf8StrMaxByteCount = 38;

			NTSTATUS returnStatus = RtlUnicodeToUTF8N(NULL, utf8StrMaxByteCount, NULL, unicodeStr.Buffer, unicodeStr.Length);

			Assert::IsTrue(returnStatus == STATUS_INVALID_PARAMETER);
		}

		/*
		 * Attempt to convert a NULL Unicode string to a UTF8 string
		 */
		TEST_METHOD(DdkUnicodeToUTF8NullSource)
		{
			const ULONG utf8StrMaxByteCount = 2;

			char utf8StrDst[utf8StrMaxByteCount];
			ULONG utf8StrActualByteCount;

			NTSTATUS returnStatus = RtlUnicodeToUTF8N(utf8StrDst, utf8StrMaxByteCount, &utf8StrActualByteCount, NULL, 0);

			Assert::IsTrue(returnStatus == STATUS_INVALID_PARAMETER_4);
		}

		/*
		 * Attempt to convert a Unicode String to UTF8 string passing the same source and destination pointers.
		 */
		TEST_METHOD(DdkUnicodeToUTF8SamePointer)
		{
			const UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"My N\u00c3m\u04d9 is Mi\u08a2ha\u20acl C\u0917in\u20ac\u2122\0");
			const ULONG utf8StrMaxByteCount = 38;

			ULONG utf8StrActualByteCount;

			NTSTATUS returnStatus = RtlUnicodeToUTF8N((PCHAR)unicodeStr.Buffer, utf8StrMaxByteCount, &utf8StrActualByteCount, unicodeStr.Buffer, unicodeStr.Length);

			Assert::IsTrue(returnStatus == STATUS_INVALID_PARAMETER);
		}
		
		/*
		 * Attempt to convert a Unicode string to the respective UTF8 String with a buffer too small
		 */
		TEST_METHOD(DdkUnicodeToUTF8SmallBuffer)
		{
			const UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"My N\u00c3m\u04d9 is Mi\u08a2ha\u20acl C\u0917in\u20ac\u2122\0");
			const unsigned char expectedUtf8Str[] = { 0x4d, 0x79 };
			const ULONG utf8StrMaxByteCount = 2;

			char utf8StrDst[utf8StrMaxByteCount];
			ULONG utf8StrActualByteCount;

			NTSTATUS returnStatus = RtlUnicodeToUTF8N(utf8StrDst, utf8StrMaxByteCount, &utf8StrActualByteCount, unicodeStr.Buffer, unicodeStr.Length);

			Assert::IsTrue(returnStatus == STATUS_BUFFER_TOO_SMALL);
			Assert::IsTrue(utf8StrActualByteCount == utf8StrMaxByteCount);
			Assert::IsTrue(memcmp(&expectedUtf8Str, &utf8StrDst, utf8StrActualByteCount) == 0);
		}

		/*
		 * Convert a UTF8 encoded string the equivelent Unicode string
		 */
		TEST_METHOD(DdkUTF8ToUnicode)
		{
			const unsigned char utf8Str[] = { 0x4d, 0x79, 0x20, 0x4e, 0xc3, 0x83, 0x6d, 0xd3, 0x99, 0x20, 0x69, 0x73,
				0x20, 0x4d, 0x69, 0xe0, 0xa2, 0xa2, 0x68, 0x61, 0xe2, 0x82, 0xac, 0x6c, 0x20, 0x43, 0xe0, 0xa4, 0x97,
				0x69, 0x6e, 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2, 0x00 };
			const ULONG utf8MaxStrByteCount = 38;
			const ULONG unicodeStrMaxByteCount = 52;

			ULONG unicodeActualByteCount;
			wchar_t unicodeStrDst[unicodeStrMaxByteCount / sizeof(wchar_t)];

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN(unicodeStrDst, unicodeStrMaxByteCount, &unicodeActualByteCount, (char *)utf8Str, utf8MaxStrByteCount);

			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(unicodeActualByteCount == unicodeStrMaxByteCount);
		}

		/*
		 * Attempt to convert an empty UTF8 string to a Unicode string
		 */
		TEST_METHOD(DdkUTF8ToUnicodeEmptySourceString)
		{
			const unsigned char utf8Str[] = { '\0' }; // This is just a place holder as NULL value would cause a STATUS_INVALID_PARAMETER
			const ULONG utf8MaxStrByteCount = 1;
			const ULONG unicodeStrMaxByteCount = 2;

			ULONG unicodeActualByteCount = 0;
			wchar_t unicodeStrDst[unicodeStrMaxByteCount / sizeof(wchar_t)];

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN(unicodeStrDst, unicodeStrMaxByteCount, &unicodeActualByteCount, (char *)utf8Str, 0);

			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(unicodeActualByteCount == 0);
		}

		/*
		 * Attempt to convert a UTF8 string to a Unicode string with NULL unicodeActualByteCount
		 */
		TEST_METHOD(DdkUTF8ToUnicodeNullActualByteCount)
		{
			const UNICODE_STRING expectedUnicodeStr = RTL_CONSTANT_STRING(L"My N\u00c3m\u04d9 is Mi\u08a2ha\u20acl C\u0917in\u20ac\u2122\0");
			const unsigned char utf8Str[] = { 0x4d, 0x79, 0x20, 0x4e, 0xc3, 0x83, 0x6d, 0xd3, 0x99, 0x20, 0x69, 0x73,
				0x20, 0x4d, 0x69, 0xe0, 0xa2, 0xa2, 0x68, 0x61, 0xe2, 0x82, 0xac, 0x6c, 0x20, 0x43, 0xe0, 0xa4, 0x97,
				0x69, 0x6e, 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2, 0x00 };
			const ULONG utf8StrMaxByteCount = 38, unicodeStrMaxByteCount = 54;

			wchar_t unicodeStrDst[unicodeStrMaxByteCount / sizeof(wchar_t)];

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN(unicodeStrDst, unicodeStrMaxByteCount, NULL, (char *)utf8Str, utf8StrMaxByteCount);

			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(memcmp(&unicodeStrDst, expectedUnicodeStr.Buffer, expectedUnicodeStr.Length) == 0);
		}

		/*
		 * Attempt to convert a UTF8 string to the equivelent unicode string with a NULL unicode string destination 
		 */
		TEST_METHOD(DdkUTF8ToUnicodeNullDestination)
		{
			const unsigned char utf8Str[] = { 0x4d, 0x79, 0x20, 0x4e, 0xc3, 0x83, 0x6d, 0xd3, 0x99, 0x20, 0x69, 0x73,
				0x20, 0x4d, 0x69, 0xe0, 0xa2, 0xa2, 0x68, 0x61, 0xe2, 0x82, 0xac, 0x6c, 0x20, 0x43, 0xe0, 0xa4, 0x97,
				0x69, 0x6e, 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2, 0x00 };
			const ULONG utf8StrMaxByteCount = 38, unicodeStrMaxByteCount = 52;

			ULONG unicodeActualByteCount = 0;

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN(NULL, unicodeStrMaxByteCount, &unicodeActualByteCount, (char *)utf8Str, utf8StrMaxByteCount);

			Assert::IsTrue(returnStatus == STATUS_SUCCESS);
			Assert::IsTrue(unicodeActualByteCount == unicodeStrMaxByteCount);
		}

		/*
		 * Attempt to convert a Unicode string to a UTF8 string with a NULL destination pointer and NULL actualByteCount.
		 */
		TEST_METHOD(DdkUTF8ToUnicodeNullDestinationAndActualByteCount)
		{
			const unsigned char utf8Str[] = { 0x4d, 0x79, 0x20, 0x4e, 0xc3, 0x83, 0x6d, 0xd3, 0x99, 0x20, 0x69, 0x73,
				0x20, 0x4d, 0x69, 0xe0, 0xa2, 0xa2, 0x68, 0x61, 0xe2, 0x82, 0xac, 0x6c, 0x20, 0x43, 0xe0, 0xa4, 0x97,
				0x69, 0x6e, 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2 };
			const ULONG utf8StrMaxByteCount = 38, unicodeStrMaxByteCount = 54;

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN(NULL, unicodeStrMaxByteCount, NULL, (char *)utf8Str, utf8StrMaxByteCount);

			Assert::IsTrue(returnStatus == STATUS_INVALID_PARAMETER);
		}

		/*
		 * Attempt to convert a NULL UTF8 encoded string to the equivelent unicode string
		 */
		TEST_METHOD(DdkUTF8ToUnicodeNullSource)
		{
			const ULONG utf8StrByteCount = 2, unicodeStrMaxByteCount = 50;

			ULONG unicodeActualByteCount;
			wchar_t unicodeStrDst[unicodeStrMaxByteCount / sizeof(wchar_t)];

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN(unicodeStrDst, unicodeStrMaxByteCount, &unicodeActualByteCount, NULL, utf8StrByteCount);

			Assert::IsTrue(returnStatus == STATUS_INVALID_PARAMETER_4);
		}

		/*
		 * Attempt to convert a Unicode String to UTF8 string passing the same source and destination pointers.
		 */
		TEST_METHOD(DdkUTF8ToUnicodeSamePointer)
		{
			const unsigned char utf8Str[] = { 0x4d, 0x79, 0x20, 0x4e, 0xc3, 0x83, 0x6d, 0xd3, 0x99, 0x20, 0x69, 0x73,
				0x20, 0x4d, 0x69, 0xe0, 0xa2, 0xa2, 0x68, 0x61, 0xe2, 0x82, 0xac, 0x6c, 0x20, 0x43, 0xe0, 0xa4, 0x97,
				0x69, 0x6e, 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2, 0x00 };
			const ULONG utf8StrByteCount = 39, unicodeStrMaxByteCount = 56;

			ULONG unicodeActualByteCount;

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN((PWCHAR)utf8Str, unicodeStrMaxByteCount, &unicodeActualByteCount, (char *)utf8Str, utf8StrByteCount);

			Assert::IsTrue(returnStatus == STATUS_INVALID_PARAMETER);
		}

		/*
		 * Attempt to convert a UTF8 encoded string the equivelent Unicode string with a buffer too small to hold the result
		 */
		TEST_METHOD(DdkUTF8ToUnicodeSmallBuffer)
		{
			const UNICODE_STRING expectedUnicodeStr = RTL_CONSTANT_STRING(L"My N\u00c3m\u04d9 is");
			const unsigned char utf8Str[] = { 0x4d, 0x79, 0x20, 0x4e, 0xc3, 0x83, 0x6d, 0xd3, 0x99, 0x20, 0x69, 0x73,
				0x20, 0x4d, 0x69, 0xe0, 0xa2, 0xa2, 0x68, 0x61, 0xe2, 0x82, 0xac, 0x6c, 0x20, 0x43, 0xe0, 0xa4, 0x97,
				0x69, 0x6e, 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2, 0xf8 };
			const ULONG utf8StrByteCount = 32, unicodeStrMaxByteCount = 20;

			ULONG unicodeActualByteCount;
			wchar_t unicodeStrDst[unicodeStrMaxByteCount / sizeof(wchar_t)];

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN(unicodeStrDst, unicodeStrMaxByteCount, &unicodeActualByteCount, (char *)utf8Str, utf8StrByteCount);

			Assert::IsTrue(returnStatus == STATUS_BUFFER_TOO_SMALL);
			Assert::IsTrue(memcmp(expectedUnicodeStr.Buffer, &unicodeStrDst, expectedUnicodeStr.Length) == 0);
		}

		/*
		 * Attempt to convert a UTF8 encoded string expected to be retuned as a default default character unicode string
		 */
		TEST_METHOD(DdkUTF8ToUnicodeWithInvalidCharacter)
		{
			const UNICODE_STRING expectedUnicodeStr = RTL_CONSTANT_STRING(L"\uFFFD");
			const unsigned char utf8Str[] = { 0x80 };
			const ULONG utf8StrByteCount = 1, unicodeStrMaxByteCount = 2;

			ULONG unicodeActualByteCount;
			wchar_t unicodeStrDst[unicodeStrMaxByteCount / sizeof(wchar_t)];

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN(unicodeStrDst, unicodeStrMaxByteCount, &unicodeActualByteCount, (char *)utf8Str, utf8StrByteCount);

			Assert::IsTrue(returnStatus == STATUS_SOME_NOT_MAPPED);
			Assert::IsTrue(unicodeActualByteCount == expectedUnicodeStr.Length);
			Assert::IsTrue(memcmp(&unicodeStrDst, expectedUnicodeStr.Buffer, expectedUnicodeStr.Length) == 0);
		}

		/*
		 * Attempt to convert a UTF8 encoded string expected to be retuned as a default default character at the beginning of the unicode string
		 */
		TEST_METHOD(DdkUTF8ToUnicodeWithInvalidCharacterPrefix)
		{
			const UNICODE_STRING expectedUnicodeStr = RTL_CONSTANT_STRING(L"\uFFFDMy N\u00c3m\u04d9 is Mi\u08a2ha\u20acl C\u0917in\u20ac\u2122\0");
			const unsigned char utf8Str[] = { 0x80, 0x4d, 0x79, 0x20, 0x4e, 0xc3, 0x83, 0x6d, 0xd3, 0x99, 0x20, 0x69, 0x73,
				0x20, 0x4d, 0x69, 0xe0, 0xa2, 0xa2, 0x68, 0x61, 0xe2, 0x82, 0xac, 0x6c, 0x20, 0x43, 0xe0, 0xa4, 0x97, 0x69,
				0x6e, 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2, 0x00 };
			const ULONG utf8StrByteCount = 39, unicodeStrMaxByteCount = 56;

			ULONG unicodeActualByteCount;
			wchar_t unicodeStrDst[unicodeStrMaxByteCount / sizeof(wchar_t)];

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN(unicodeStrDst, unicodeStrMaxByteCount, &unicodeActualByteCount, (char *)utf8Str, utf8StrByteCount);

			Assert::IsTrue(returnStatus == STATUS_SOME_NOT_MAPPED);
			Assert::IsTrue(unicodeActualByteCount == expectedUnicodeStr.Length);
			Assert::IsTrue(memcmp(&unicodeStrDst, expectedUnicodeStr.Buffer, expectedUnicodeStr.Length) == 0);
		}

		/*
		 * Attempt to convert a UTF8 encoded string expected to be retuned as a default default character at then end of the unicode string
		 */
		TEST_METHOD(DdkUTF8ToUnicodeWithInvalidCharacterSuffix)
		{
			const UNICODE_STRING expectedUnicodeStr = RTL_CONSTANT_STRING(L"My N\u00c3m\u04d9 is Mi\u08a2ha\u20acl C\u0917in\u20ac\u2122\uFFFD\0");
			const unsigned char utf8Str[] = { 0x4d, 0x79, 0x20, 0x4e, 0xc3, 0x83, 0x6d, 0xd3, 0x99, 0x20, 0x69, 0x73,
				0x20, 0x4d, 0x69, 0xe0, 0xa2, 0xa2, 0x68, 0x61, 0xe2, 0x82, 0xac, 0x6c, 0x20, 0x43, 0xe0, 0xa4, 0x97,
				0x69, 0x6e, 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2, 0x80, 0x00 };
			const ULONG utf8StrByteCount = 39, unicodeStrMaxByteCount = 56;

			ULONG unicodeActualByteCount;
			wchar_t unicodeStrDst[unicodeStrMaxByteCount / sizeof(wchar_t)];

			NTSTATUS returnStatus = RtlUTF8ToUnicodeN(unicodeStrDst, unicodeStrMaxByteCount, &unicodeActualByteCount, (char *)utf8Str, utf8StrByteCount);

			Assert::IsTrue(returnStatus == STATUS_SOME_NOT_MAPPED);
			Assert::IsTrue(unicodeActualByteCount == expectedUnicodeStr.Length);
			Assert::IsTrue(memcmp(&unicodeStrDst, expectedUnicodeStr.Buffer, expectedUnicodeStr.Length) == 0);
		}

		/*
	 	 * Convert a unicode String to an ansi string and using caller allocated a buffer
		 */
		TEST_METHOD(DdkUnicodeToAnsi)
		{
			const UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"abcde");
			const ANSI_STRING expectedStr = RTL_CONSTANT_STRING("abcde");
			const USHORT maxLength = 6;

			ANSI_STRING ansiStr;
			ansiStr.MaximumLength = maxLength;
			ansiStr.Buffer = new char[maxLength];
			ansiStr.Length = 0;

			NTSTATUS status = RtlUnicodeStringToAnsiString(&ansiStr, &unicodeStr, FALSE);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(ansiStr.Length == expectedStr.Length);
			Assert::IsTrue(memcmp(ansiStr.Buffer, expectedStr.Buffer, expectedStr.Length) == 0);

			delete[] ansiStr.Buffer;
			ansiStr.Buffer = NULL;
		}

		/*
		 * Convert a unicode String to an ansi string using caller allocated buffer too small to fit the resultant string
		 */
		TEST_METHOD(DdkUnicodeToAnsiSmall)
		{
			const UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"abcde");
			const ANSI_STRING expectedStr = RTL_CONSTANT_STRING("abcde");
			const USHORT maxLength = 3;

			ANSI_STRING ansiStr;
			ansiStr.MaximumLength = maxLength;
			ansiStr.Buffer = new char[maxLength];
			ansiStr.Length = 0;

			NTSTATUS status = RtlUnicodeStringToAnsiString(&ansiStr, &unicodeStr, FALSE);

			Assert::IsTrue(status == STATUS_BUFFER_TOO_SMALL);

			delete[] ansiStr.Buffer;
			ansiStr.Buffer = NULL;
		}

		/*
		 * Convert a unicode string to an ansi string using function allocated buffer
		 */
		TEST_METHOD(DdkUnicodeToAnsiAllocate)
		{
			const UNICODE_STRING unicodeStr = RTL_CONSTANT_STRING(L"abcde");
			const ANSI_STRING expectedStr = RTL_CONSTANT_STRING("abcde");
			
			ANSI_STRING ansiStr;

			NTSTATUS status = RtlUnicodeStringToAnsiString(&ansiStr, &unicodeStr, TRUE);

			Assert::IsTrue(status == STATUS_SUCCESS);
			Assert::IsTrue(ansiStr.Length == expectedStr.Length);
			Assert::IsTrue(memcmp(ansiStr.Buffer, expectedStr.Buffer, expectedStr.Length) == 0);

			RtlFreeAnsiString(&ansiStr);
		}
		
	};
}
