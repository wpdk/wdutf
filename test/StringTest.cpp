/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	String Tests
 */

#include "stdafx.h"


#define MAX_STRING_LEN  0xFFFF


namespace DdkUnitTest
{
	TEST_CLASS(DdkStringTest)
	{
	public:

		/*
		 * Initialise a short string 
		 */		
		TEST_METHOD(DdkStringInitShort)
		{
			const PCSZ s = "abc";
			STRING s1;
			RtlInitString(&s1, s);
			Assert::IsTrue(s1.Buffer == s);
			Assert::IsTrue(s1.Length == strlen(s));
			Assert::IsTrue(s1.MaximumLength == strlen(s) + 1);
		}

		/*
		 * Initialize an empty string
		 */
		TEST_METHOD(DdkStringInitNull)
		{
			const PCSZ s = NULL;
			STRING s1;
			RtlInitString(&s1, s);
			Assert::IsTrue(s1.Buffer == NULL);
			Assert::IsTrue(s1.Length == 0);
			Assert::IsTrue(s1.MaximumLength == 0);
		}

		/*
		 * Initialize a maximum-length string
		 */
		TEST_METHOD(DdkStringInitLong)
		{
			const LONG STRING_LEN = MAX_STRING_LEN; 
			STRING s1;
			char *s = (char*)malloc((STRING_LEN) * sizeof(char)); 
			Assert::IsTrue(s != NULL);
			memset(s, L'A', STRING_LEN - 1);
			s[STRING_LEN - 1] = 0;
			RtlInitString(&s1, s);

			Assert::IsTrue(s1.Buffer == s);
			Assert::IsTrue(s1.Length == strlen(s));
			Assert::IsTrue(s1.MaximumLength == strlen(s) + 1);
			
			free(s);			
		}

		/*
		 * Initialize a string which is too long
		 */
		TEST_METHOD(DdkStringInitTooLong)
		{
			const LONG STRING_LEN = MAX_STRING_LEN + 1;
			STRING s1;
			char *s = (char*)malloc((STRING_LEN) * sizeof(char));
			Assert::IsTrue(s != NULL);
			memset(s, L'A', STRING_LEN - 1);
			s[STRING_LEN - 1] = 0;

			TEST_DDK_FAIL(RtlInitString(&s1, s));
			
			free(s);
		}

		/*
		 * Compare 2 identical strings with the same case
		 */
		TEST_METHOD(DdkStringCompareIdentical)
		{
			STRING str1 = RTL_CONSTANT_STRING("abcde");
			STRING str2 = RTL_CONSTANT_STRING("abcde");
			Assert::IsTrue(RtlCompareString(&str1, &str2, TRUE) == 0);
			Assert::IsTrue(RtlCompareString(&str1, &str2, FALSE) == 0);
		}

		/*
		 * Compare 2 different strings
		 */
		TEST_METHOD(DdkStringCompareDiff)
		{
			STRING str1 = RTL_CONSTANT_STRING("abcde1");
			STRING str2 = RTL_CONSTANT_STRING("abcde2");
			Assert::IsTrue(RtlCompareString(&str1, &str2, TRUE) < 0);
			Assert::IsTrue(RtlCompareString(&str1, &str2, FALSE) < 0);
		}

		/*
		 * Compare 2 strings, which differ only by case
		 */
		TEST_METHOD(DdkStringCompareDiffCase)
		{
			STRING str1 = RTL_CONSTANT_STRING("Abcde");
			STRING str2 = RTL_CONSTANT_STRING("abcde");
			Assert::IsTrue(RtlCompareString(&str1, &str2, TRUE) == 0);
			Assert::IsTrue(RtlCompareString(&str1, &str2, FALSE) != 0);
		}		

		TEST_METHOD(DdkStringCompareLess)
		{
			STRING str1 = RTL_CONSTANT_STRING("Firs");
			STRING str2 = RTL_CONSTANT_STRING("first");
			Assert::IsTrue(RtlCompareString(&str1, &str2, TRUE) < 0);
		}

		TEST_METHOD(DdkStringCompareGreater)
		{
			STRING str1 = RTL_CONSTANT_STRING("First12");
			STRING str2 = RTL_CONSTANT_STRING("first");
			Assert::IsTrue(RtlCompareString(&str1, &str2, TRUE) > 0);
		}

		TEST_METHOD(DdkStringCompareUnequal)
		{
			STRING str1 = RTL_CONSTANT_STRING("First");
			STRING str2 = RTL_CONSTANT_STRING("Last");
			Assert::IsFalse(RtlCompareString(&str1, &str2, TRUE) == 0);
		}
				
		/* 
		 * Copy string from one buffer to another
		 */
		TEST_METHOD(DdkStringCopy)
		{
			const STRING sourceStr = RTL_CONSTANT_STRING("abcde");
			const USHORT maxLength = 6;

			STRING destinationStr;
			destinationStr.MaximumLength = maxLength;
			destinationStr.Buffer = new char[maxLength];

			RtlCopyString(&destinationStr, &sourceStr);

			Assert::IsTrue(destinationStr.Length == sourceStr.Length);
			Assert::IsTrue(memcmp(destinationStr.Buffer, sourceStr.Buffer, sourceStr.Length) == 0);

			delete[] destinationStr.Buffer;
			destinationStr.Buffer = NULL;
		}

		/*
		* Attempt to copy a string from one buffer to a smaller buffer
		*/
		TEST_METHOD(DdkStringCopyShortDestination)
		{
			const STRING sourceStr = RTL_CONSTANT_STRING("abcde");
			const USHORT maxLength = 3;

			STRING destinationStr;
			destinationStr.MaximumLength = maxLength;
			destinationStr.Buffer = new char[maxLength];

			RtlCopyString(&destinationStr, &sourceStr);

			Assert::IsTrue(destinationStr.Length == maxLength);
			Assert::IsTrue(memcmp(destinationStr.Buffer, sourceStr.Buffer, destinationStr.MaximumLength) == 0);

			delete[] destinationStr.Buffer;
			destinationStr.Buffer = NULL;
		}

		/*
		 * Attempt to copy a NULL string
		 */
		TEST_METHOD(DdkStringCopyNullSource)
		{
			const USHORT maxLength = 3;

			STRING destinationStr;
			destinationStr.MaximumLength = maxLength;
			destinationStr.Buffer = new char[maxLength];

			RtlCopyString(&destinationStr, NULL);

			Assert::IsTrue(destinationStr.Length == 0);

			delete[] destinationStr.Buffer;
			destinationStr.Buffer = NULL;
		}

		TEST_METHOD(DdkStringCopyNullDestination)
		{
			const STRING s = RTL_CONSTANT_STRING("AbCdE");

			TEST_DDK_FAIL(RtlCopyString(NULL, &s));
		}

		TEST_METHOD(DdkStringEqualCaseSensitiveSucceed)
		{
			const STRING s1 = RTL_CONSTANT_STRING("aBcDe");
			const STRING s2 = RTL_CONSTANT_STRING("aBcDe");

			Assert::IsTrue(RtlEqualString(&s1, &s2, FALSE) == TRUE);
		}

		/*
		 * Compare two strings which differ only by case
		 */
		TEST_METHOD(DdkStringEqualCaseSensitiveFail)
		{
			const STRING s1 = RTL_CONSTANT_STRING("aBcDe");
			const STRING s2 = RTL_CONSTANT_STRING("AbCdE");

			Assert::IsFalse(RtlEqualString(&s1, &s2, FALSE) == TRUE);
		}

		TEST_METHOD(DdkStringEqualCaseInSensitive)
		{
			const STRING s1 = RTL_CONSTANT_STRING("abcde");
			const STRING s2 = RTL_CONSTANT_STRING("aBcDE");

			Assert::IsTrue(RtlEqualString(&s1, &s2, TRUE) == TRUE);
		}

		/*
		 * Compare two strings of different length
		 */
		TEST_METHOD(DdkStringUnequal)
		{
			const STRING s1 = RTL_CONSTANT_STRING("abcde");
			const STRING s2 = RTL_CONSTANT_STRING("abc");

			Assert::IsFalse(RtlEqualString(&s1, &s2, TRUE) == TRUE);
		}

		/*
		 * Convert lower case string to upper case
		 */
		TEST_METHOD(DdkStringLowerToUpper)
		{
			const STRING expectedStr = RTL_CONSTANT_STRING("ABCDE");
			const STRING sourceStr = RTL_CONSTANT_STRING("abcde");
			const USHORT maxLength = 6;

			STRING destinationStr;
			destinationStr.MaximumLength = maxLength;
			destinationStr.Buffer = new char[maxLength];

			RtlUpperString(&destinationStr, &sourceStr);

			Assert::IsTrue(destinationStr.Length == expectedStr.Length);
			Assert::IsTrue(memcmp(destinationStr.Buffer, expectedStr.Buffer, expectedStr.Length) == 0);

			delete[] destinationStr.Buffer;
			destinationStr.Buffer = NULL;
		}

		TEST_METHOD(DdkStringMixedToUpper)
		{
			const STRING expectedStr = RTL_CONSTANT_STRING("ABCDE");
			const STRING sourceStr = RTL_CONSTANT_STRING("aBcDe");
			const USHORT maxLength = 6;

			STRING destinationStr;
			destinationStr.MaximumLength = maxLength;
			destinationStr.Buffer = new char[maxLength];

			RtlUpperString(&destinationStr, &sourceStr);

			Assert::IsTrue(destinationStr.Length == expectedStr.Length);
			Assert::IsTrue(memcmp(destinationStr.Buffer, expectedStr.Buffer, expectedStr.Length) == 0);
		}
		
		TEST_METHOD(DdkStringUpperToUpper)
		{
			const STRING expectedStr = RTL_CONSTANT_STRING("ABCDE");
			const STRING sourceStr = RTL_CONSTANT_STRING("ABCDE");
			const USHORT maxLength = 6;

			STRING destinationStr;
			destinationStr.MaximumLength = maxLength;
			destinationStr.Buffer = new char[maxLength];

			RtlUpperString(&destinationStr, &sourceStr);

			Assert::IsTrue(destinationStr.Length == expectedStr.Length);
			Assert::IsTrue(memcmp(destinationStr.Buffer, expectedStr.Buffer, expectedStr.Length) == 0);
		}
	};
}
