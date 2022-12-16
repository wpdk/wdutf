/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2022, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	File Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkFileTest)
	{
	public:
		TEST_METHOD_INITIALIZE(DdkFileTestInit)
		{
			DdkThreadInit();
		}

		/*
		 *	Check ZwCreateFile with FILE_CREATE
		 */
		TEST_METHOD(DdkFileZwCreateCreate)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test101.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwCreateFile with FILE_CREATE for directory
		 */
		TEST_METHOD(DdkFileZwCreateCreateDirectory)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test151");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwCreateFile with FILE_CREATE and file exists
		 */
		TEST_METHOD(DdkFileZwCreateCreateExists)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test102.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_OBJECT_NAME_COLLISION, status);
			Assert::IsTrue(h == NULL);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}
		/*
		 *	Check ZwCreateFile with FILE_CREATE and directory exists
		 */
		TEST_METHOD(DdkFileZwCreateCreateExistsDirectory)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test152");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_OBJECT_NAME_COLLISION, status);
			Assert::IsTrue(h == NULL);
		}

		/*
		 *	Check ZwCreateFile with FILE_OPEN
		 */
		TEST_METHOD(DdkFileZwCreateOpen)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test201.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwCreateFile with FILE_OPEN for directory
		 */
		TEST_METHOD(DdkFileZwCreateOpenDirectory)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test251");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwCreateFile with FILE_OPEN and doesn't exist
		 */
		TEST_METHOD(DdkFileZwCreateOpenNonExistant)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test202.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((ULONG_PTR)FILE_DOES_NOT_EXIST, iostat.Information);
			Assert::IsTrue(h == NULL);
		}

		/*
		 *	Check ZwCreateFile with FILE_OPEN and directory doesn't exist
		 */
		TEST_METHOD(DdkFileZwCreateOpenNonExistantDirectory)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test252");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((ULONG_PTR)FILE_DOES_NOT_EXIST, iostat.Information);
			Assert::IsTrue(h == NULL);
		}

		/*
		 *	Check ZwCreateFile with FILE_OPEN_IF
		 */
		TEST_METHOD(DdkFileZwCreateOpenIf)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test301.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwCreateFile with FILE_OPEN_IF for directory
		 */
		TEST_METHOD(DdkFileZwCreateOpenIfDirectory)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test351");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwCreateFile with FILE_OPEN_IF and file exists
		 */
		TEST_METHOD(DdkFileZwCreateOpenIfExists)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test302.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwCreateFile with FILE_OPEN_IF and directory exists
		 */
		TEST_METHOD(DdkFileZwCreateOpenIfExistsDirectory)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test352");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwOpenFile
		 */
		TEST_METHOD(DdkFileZwOpen)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1101.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwOpenFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, 0, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwOpenFile for directory
		 */
		TEST_METHOD(DdkFileZwOpenDirectory)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1151");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwOpenFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, 0, FILE_DIRECTORY_FILE);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwOpenFile non-existant file
		 */
		TEST_METHOD(DdkFileZwOpenNonExistant)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1102.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwOpenFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, 0, 0);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::IsTrue(h == NULL);
		}

		/*
		 *	Check ZwOpenFile non-existant directory
		 */
		TEST_METHOD(DdkFileZwOpenNonExistantDirectory)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1152");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwOpenFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, 0, FILE_DIRECTORY_FILE);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::IsTrue(h == NULL);
		}

		/*
		 *	Check ZwDeleteFile
		 */
		TEST_METHOD(DdkFileZwDelete)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test901.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::IsTrue(h == NULL);
		}

		/*
		 *	Check ZwWriteFile
		 */
		TEST_METHOD(DdkFileZwWrite)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1001.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer, strlen(buffer), NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(strlen(buffer), iostat.Information);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwWriteFile with multiple writes
		 */
		TEST_METHOD(DdkFileZwWriteMultiple)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1002.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			ULONG total = 0;

			for (ULONG i = strlen(buffer); i > 0; total += i--) {
				status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer, i, NULL, NULL);
				Assert::AreEqual(STATUS_SUCCESS, status);
				Assert::AreEqual((ULONG_PTR)i, iostat.Information);
			}

			FILE_POSITION_INFORMATION info;
			status = ZwQueryInformationFile(h, &iostat, &info,
				(ULONG)sizeof(info), FilePositionInformation);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(sizeof(info), iostat.Information);
			Assert::AreEqual((__int64)total, info.CurrentByteOffset.QuadPart);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwWriteFile with overlapping 
		 */
		TEST_METHOD(DdkFileZwWriteOverlap)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1003.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			ULONG total = 0;

			for (ULONG i = strlen(buffer); i > 0; i--) {
				if (total) total -= (i - 1);

				LARGE_INTEGER offset;
				offset.QuadPart = total;
				status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer,
					strlen(buffer), &offset, NULL);

				Assert::AreEqual(STATUS_SUCCESS, status);
				Assert::AreEqual((ULONG_PTR)strlen(buffer), iostat.Information);
				total += strlen(buffer);
			}

			FILE_POSITION_INFORMATION info;
			status = ZwQueryInformationFile(h, &iostat, &info,
				(ULONG)sizeof(info), FilePositionInformation);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(sizeof(info), iostat.Information);
			Assert::AreEqual((__int64)total, info.CurrentByteOffset.QuadPart);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwWriteFile with APC completion 
		 */
		static void ZwWriteApc(void *pContext, IO_STATUS_BLOCK *ios, ULONG)
		{
			Assert::AreEqual(STATUS_SUCCESS, ios->Status);
			Assert::AreEqual((ULONG_PTR)pContext, ios->Information);
			ios->Information = (ULONG_PTR)ZwWriteApc;
		}

		TEST_METHOD(DdkFileZwWriteApc)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1001.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			status = ZwWriteFile(h, NULL, ZwWriteApc, (PVOID)strlen(buffer),
				&iostat, buffer, strlen(buffer), NULL, NULL);

			Assert::IsTrue(NT_SUCCESS(status));
			while (iostat.Information != (ULONG_PTR)ZwWriteApc) Sleep(1);
			Assert::IsTrue(NT_SUCCESS(iostat.Status));

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwReadFile
		 */
		TEST_METHOD(DdkFileZwRead)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test2001.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer, strlen(buffer), NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(strlen(buffer), iostat.Information);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char buf[200];
			status = ZwReadFile(h, NULL, NULL, NULL, &iostat, buf, sizeof(buf), NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(strlen(buffer), iostat.Information);
			Assert::IsTrue(memcmp(buffer, buf, strlen(buffer)) == 0);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwReadFile with multiple reads
		 */
		TEST_METHOD(DdkFileZwReadMultiple)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test2002.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer, strlen(buffer), NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(strlen(buffer), iostat.Information);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char buf[200];

			for (ULONG i = 0; i < strlen(buffer); i++) {
				status = ZwReadFile(h, NULL, NULL, NULL, &iostat, buf + i, 1, NULL, NULL);
				Assert::AreEqual(STATUS_SUCCESS, status);
				Assert::AreEqual((ULONG_PTR)1, iostat.Information);
			}

			status = ZwReadFile(h, NULL, NULL, NULL, &iostat,
				buf + strlen(buffer), 1, NULL, NULL);
			Assert::AreNotEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)0, iostat.Information);

			Assert::IsTrue(memcmp(buffer, buf, strlen(buffer)) == 0);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwReadFile with overlap
		 */
		TEST_METHOD(DdkFileZwReadOverlap)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test2003.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer, strlen(buffer), NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(strlen(buffer), iostat.Information);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char buf[200];

			for (ULONG i = 0; i < strlen(buffer); i++) {
				LARGE_INTEGER offset;
				offset.QuadPart = i;
				status = ZwReadFile(h, NULL, NULL, NULL, &iostat, buf + i, 5, &offset, NULL);
				Assert::AreEqual(STATUS_SUCCESS, status);
				Assert::AreEqual((ULONG_PTR)(min(5,strlen(buffer) - i)), iostat.Information);
			}

			status = ZwReadFile(h, NULL, NULL, NULL, &iostat,
				buf + strlen(buffer), 1, NULL, NULL);
			Assert::AreNotEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)0, iostat.Information);

			Assert::IsTrue(memcmp(buffer, buf, strlen(buffer)) == 0);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwReadFile with APC completion
		 */
		static void ZwReadApc(void *pContext, IO_STATUS_BLOCK *ios, ULONG)
		{
			Assert::AreEqual(STATUS_SUCCESS, ios->Status);
			Assert::AreEqual((ULONG_PTR)pContext, ios->Information);
			ios->Information = (ULONG_PTR)ZwReadApc;
		}

		TEST_METHOD(DdkFileZwReadApc)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test2004.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer, strlen(buffer), NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(strlen(buffer), iostat.Information);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_OPENED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char buf[200];
			status = ZwReadFile(h, NULL, ZwReadApc, (PVOID)strlen(buffer),
				&iostat, buf, sizeof(buf), NULL, NULL);

			Assert::IsTrue(NT_SUCCESS(status));
			while (iostat.Information != (ULONG_PTR)ZwReadApc) Sleep(1);
			Assert::IsTrue(NT_SUCCESS(iostat.Status));

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwFlushBuffersFile
		 */
		TEST_METHOD(DdkFileZwFlush)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1601.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer, strlen(buffer), NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(strlen(buffer), iostat.Information);

			status = ZwFlushBuffersFile(h, &iostat);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwQueryInformationFile basic
		 */
		TEST_METHOD(DdkFileZwQueryInfoBasic)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1701.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer, strlen(buffer), NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(strlen(buffer), iostat.Information);

			FILE_BASIC_INFORMATION info;
			status = ZwQueryInformationFile(h, &iostat, &info,
				(ULONG)sizeof(info), FileBasicInformation);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(sizeof(info), iostat.Information);
			Assert::IsTrue(info.LastWriteTime.QuadPart >= info.CreationTime.QuadPart);
			Assert::IsTrue(info.LastAccessTime.QuadPart >= info.CreationTime.QuadPart);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwQueryInformationFile basic for directory
		 */
		TEST_METHOD(DdkFileZwQueryInfoBasicDirectory)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1751");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			FILE_BASIC_INFORMATION info;
			status = ZwQueryInformationFile(h, &iostat, &info,
				(ULONG)sizeof(info), FileBasicInformation);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(sizeof(info), iostat.Information);
			Assert::IsTrue(info.LastWriteTime.QuadPart >= info.CreationTime.QuadPart);
			Assert::IsTrue(info.LastAccessTime.QuadPart >= info.CreationTime.QuadPart);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwQueryInformationFile standard
		 */
		TEST_METHOD(DdkFileZwQueryInfoStandard)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1702.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer, strlen(buffer), NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(strlen(buffer), iostat.Information);

			FILE_STANDARD_INFORMATION info;
			status = ZwQueryInformationFile(h, &iostat, &info,
				(ULONG)sizeof(info), FileStandardInformation);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(sizeof(info), iostat.Information);
			Assert::IsTrue(info.EndOfFile.QuadPart == strlen(buffer));
			Assert::AreEqual((ULONG)1, info.NumberOfLinks);
			Assert::IsFalse(info.DeletePending);
			Assert::IsFalse(info.Directory);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwQueryInformationFile standard for directory
		 */
		TEST_METHOD(DdkFileZwQueryInfoStandardDirectory)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1752");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			FILE_STANDARD_INFORMATION info;
			status = ZwQueryInformationFile(h, &iostat, &info,
				(ULONG)sizeof(info), FileStandardInformation);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(sizeof(info), iostat.Information);
			Assert::AreEqual((ULONG)1, info.NumberOfLinks);
			Assert::IsFalse(info.DeletePending);
			Assert::IsTrue(info.Directory);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwQueryInformationFile position
		 */
		TEST_METHOD(DdkFileZwQueryInfoPosition)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test1703.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			HANDLE h = NULL;
			NTSTATUS status = ZwCreateFile(&h, GENERIC_READ | GENERIC_WRITE,
				&attr, &iostat, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
				FILE_NON_DIRECTORY_FILE, NULL, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG_PTR)FILE_CREATED, iostat.Information);
			Assert::IsTrue(h != NULL);

			char *buffer = "This is a test message\r\n";
			status = ZwWriteFile(h, NULL, NULL, NULL, &iostat, buffer, strlen(buffer), NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(strlen(buffer), iostat.Information);

			FILE_POSITION_INFORMATION info;
			status = ZwQueryInformationFile(h, &iostat, &info,
				(ULONG)sizeof(info), FilePositionInformation);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(sizeof(info), iostat.Information);
			Assert::IsTrue(info.CurrentByteOffset.QuadPart == strlen(buffer));

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwDeleteFile with non-existant file
		 */
		TEST_METHOD(DdkFileZwDeleteNonExistant)
		{
			OBJECT_ATTRIBUTES attr;
			IO_STATUS_BLOCK iostat;
			UNICODE_STRING u;

			RtlInitUnicodeString(&u, L"\\??\\C:\\Test\\Test902.txt");

			InitializeObjectAttributes(&attr, &u,
				(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

			NTSTATUS status = ZwDeleteFile(&attr);
			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
		}
	};
}
