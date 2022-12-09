/*****************************************************************************
 *                                                                           *
 *                        NOTICE                                             *
 *                                                                           *
 *           COPYRIGHT (c) 2022 DATACORE SOFTWARE CORPORATION                *
 *                                                                           *
 *                    ALL RIGHTS RESERVED                                    *
 *                                                                           *
 *                                                                           *
 *    This Computer Program is CONFIDENTIAL and a TRADE SECRET of            *
 *    DATACORE SOFTWARE CORPORATION. Any reproduction of this program        *
 *    without the express written consent of DATACORE SOFTWARE CORPORATION   *
 *    is a violation of the copyright laws and may subject you to criminal   *
 *    prosecution.                                                           *
 *                                                                           *
 *****************************************************************************/


/*
 *	Registry Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkRegistryTest)
	{
	public:
		TEST_METHOD_INITIALIZE(DdkRegistryTestInit)
		{
			DdkThreadInit();
		}

		/*
		 *	Check predefined 'RelativeTo' keys
		 */
		TEST_METHOD(DdkRegistryCheckRelative)
		{
			NTSTATUS status;

			status = RtlCheckRegistryKey(RTL_REGISTRY_CONTROL, L"");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlCheckRegistryKey(RTL_REGISTRY_DEVICEMAP, L"");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlCheckRegistryKey(RTL_REGISTRY_SERVICES, L"");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlCheckRegistryKey(RTL_REGISTRY_USER, L"");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlCheckRegistryKey(RTL_REGISTRY_WINDOWS_NT, L"");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwOpenKey for an existing key
		 */
		TEST_METHOD(DdkRegistryZwOpenKey)
		{
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKey(&h, GENERIC_ALL, &attr);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwOpenKey for a non-existant key
		 */
		TEST_METHOD(DdkRegistryZwOpenKeyNotPresent)
		{
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\NotFound");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKey(&h, GENERIC_ALL, &attr);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((HANDLE)NULL, h);
		}

		/*
		 *	Check ZwOpenKeyEx for an existing key
		 */
		TEST_METHOD(DdkRegistryZwOpenKeyEx)
		{
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwOpenKeyEx for a non-existant key
		 */
		TEST_METHOD(DdkRegistryZwOpenKeyExNotPresent)
		{
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\NotFound");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((HANDLE)NULL, h);
		}

		/*
		 *	Check ZwOpenKeyEx relative for an existing key
		 */
		TEST_METHOD(DdkRegistryZwOpenKeyExRelative)
		{
			HANDLE h = NULL, base = NULL;
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&base, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, base);

			RtlInitUnicodeString(&u, L"CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, base, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwClose(base);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwOpenKeyEx relative for a non-existant key
		 */
		TEST_METHOD(DdkRegistryZwOpenKeyExRelativeNotPresent)
		{
			HANDLE h = NULL, base = NULL;
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&base, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, base);

			RtlInitUnicodeString(&u, L"CurrentControlSet\\Services\\NotFound");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, base, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((HANDLE)NULL, h);
		}

		/*
		 *	Check ZwCreateKey creating a key
		 */
		TEST_METHOD(DdkRegistryZwCreateKey)
		{
			OBJECT_ATTRIBUTES attr;
			DWORD disposition = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Test2001");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwCreateKey(&h, GENERIC_ALL, &attr, 0, NULL,
				REG_OPTION_NON_VOLATILE, &disposition);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)REG_CREATED_NEW_KEY, disposition);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = ZwDeleteKey(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwCreateKey opening a key
		 */
		TEST_METHOD(DdkRegistryZwCreateKeyExists)
		{
			OBJECT_ATTRIBUTES attr;
			DWORD disposition = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwCreateKey(&h, GENERIC_ALL, &attr, 0, NULL,
				REG_OPTION_NON_VOLATILE, &disposition);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)REG_OPENED_EXISTING_KEY, disposition);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwCreateKey creating a key with a missing path component
		 */
		TEST_METHOD(DdkRegistryZwCreateKeyMissingPath)
		{
			OBJECT_ATTRIBUTES attr;
			DWORD disposition = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Test2002\\Test2002");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwCreateKey(&h, GENERIC_ALL, &attr, 0, NULL,
				REG_OPTION_NON_VOLATILE, &disposition);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)REG_CREATED_NEW_KEY, disposition);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = ZwDeleteKey(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwDeleteKey
		 */
		TEST_METHOD(DdkRegistryZwDeleteKey)
		{
			OBJECT_ATTRIBUTES attr;
			DWORD disposition = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Test2003");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwCreateKey(&h, GENERIC_ALL, &attr, 0, NULL,
				REG_OPTION_NON_VOLATILE, &disposition);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)REG_CREATED_NEW_KEY, disposition);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = ZwDeleteKey(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			h = NULL;
			status = ZwOpenKey(&h, GENERIC_ALL, &attr);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((HANDLE)NULL, h);
		}

		/*
		 *	Check ZwSetValueKey writing a String to the default value
		 */
		TEST_METHOD(DdkRegistryZwSetValueKeyDefault)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678";
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Test1901");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwCreateKey(&h, KEY_ALL_ACCESS, &attr, 0, NULL,
				REG_OPTION_NON_VOLATILE, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			RtlInitUnicodeString(&u, L"");
			status = ZwSetValueKey(h, &u, 0, REG_SZ, data, sizeof(WCHAR) * 9);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Test1901", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 8), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"Test1901", L"");
			Assert::AreEqual(STATUS_SUCCESS, status);

			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test1901");
		}

		/*
		 *	Check ZwSetValueKey creating a DWORD value
		 */
		TEST_METHOD(DdkRegistryZwSetValueKeyInteger)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			DWORD data = 0x12345678;
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			RtlInitUnicodeString(&u, L"Test1401");
			status = ZwSetValueKey(h, &u, 0, REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test1401";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test1401");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwSetValueKey modifying a DWORD value
		 */
		TEST_METHOD(DdkRegistryZwSetValueKeyIntegerRewrite)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			DWORD data = 0x23456781;
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			RtlInitUnicodeString(&u, L"Test1402");
			status = ZwSetValueKey(h, &u, 0, REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			data = 0x12345678;
			status = ZwSetValueKey(h, &u, 0, REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test1402";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test1402");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwSetValueKey writing a QWORD value
		 */
		TEST_METHOD(DdkRegistryZwSetValueKeyQuad)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			__int64 data = 0x12345678;
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			RtlInitUnicodeString(&u, L"Test1411");
			status = ZwSetValueKey(h, &u, 0, REG_QWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));
			*(LONG *)&data = (-(LONG)sizeof(data));


			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test1411";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((__int64)0x12345678, data);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test1411");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwSetValueKey writing a BINARY value
		 */
		TEST_METHOD(DdkRegistryZwSetValueKeyBinary)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			struct { int a, b, c; } data;
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			data.a = 0x12345678;
			data.b = 0x23456789;
			data.c = 0x34567890;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			RtlInitUnicodeString(&u, L"Test1412");
			status = ZwSetValueKey(h, &u, 0, REG_BINARY, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));
			*(LONG *)&data = (-(LONG)sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test1412";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(0x12345678, data.a);
			Assert::AreEqual(0x23456789, data.b);
			Assert::AreEqual(0x34567890, data.c);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test1412");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwSetValueKey writing a String value
		 */
		TEST_METHOD(DdkRegistryZwSetValueKeyString)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678";
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			RtlInitUnicodeString(&u, L"Test1601");
			status = ZwSetValueKey(h, &u, 0, REG_SZ, data, sizeof(WCHAR) * 9);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test1601";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 8), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test1601");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwSetValueKey modifying a String value
		 */
		TEST_METHOD(DdkRegistryZwSetValueKeyStringRewrite)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"3456789012";
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			RtlInitUnicodeString(&u, L"Test1641");
			status = ZwSetValueKey(h, &u, 0, REG_SZ, data, sizeof(WCHAR) * 9);
			Assert::AreEqual(STATUS_SUCCESS, status);

			data = L"12345678";
			status = ZwSetValueKey(h, &u, 0, REG_SZ, data, sizeof(WCHAR) * 9);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test1641";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 8), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test1641");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwSetValueKey writing a Multi-String value
		 */
		TEST_METHOD(DdkRegistryZwSetValueKeyMultiString)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678\0" L"23456789\0";
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			RtlInitUnicodeString(&u, L"Test1651");
			status = ZwSetValueKey(h, &u, 0, REG_SZ, data, sizeof(WCHAR) * 19);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED |
							 RTL_QUERY_REGISTRY_NOEXPAND;
			table[0].Name = L"Test1651";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 18), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual(0, wcsncmp(u.Buffer + sizeof(WCHAR) * 9, data + sizeof(WCHAR) * 9, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[17]);
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[18]);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test1651");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwQueryValueKey for basic info
		 */
		TEST_METHOD(DdkRegistryZwQueryValueKeyBasic)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			OBJECT_ATTRIBUTES attr;
			DWORD i = 0, match = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			struct { KEY_VALUE_BASIC_INFORMATION info; WCHAR v[1024]; } data;
			DWORD resultlen = 0;

			RtlInitUnicodeString(&u, L"SystemRoot");

			status = ZwQueryValueKey(h, &u, KeyValueBasicInformation,
				(PVOID)&data.info, sizeof(data), &resultlen);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)REG_SZ, data.info.Type);
			Assert::AreEqual((ULONG)20, data.info.NameLength);
			Assert::AreEqual((ULONG)(((char *)&data.info.Name - (char *)&data)
				+ data.info.NameLength), resultlen);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwQueryValueKey for full info
		 */
		TEST_METHOD(DdkRegistryZwQueryValueKeyFull)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			OBJECT_ATTRIBUTES attr;
			DWORD i = 0, match = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			struct { KEY_VALUE_FULL_INFORMATION info; WCHAR v[1024]; } data;
			DWORD resultlen = 0;

			RtlInitUnicodeString(&u, L"SystemRoot");

			status = ZwQueryValueKey(h, &u, KeyValueFullInformation,
				(PVOID)&data.info, sizeof(data), &resultlen);

			Assert::AreEqual((ULONG)REG_SZ, data.info.Type);
			Assert::AreEqual((ULONG)20, data.info.NameLength);
			Assert::AreEqual((ULONG)22, data.info.DataLength);
			Assert::AreEqual(0, wcsncmp((WCHAR*)((char *)&data + data.info.DataOffset),
				L"C:\\Windows", 10));
			Assert::AreEqual((ULONG)(((char *)&data.info.Name - (char *)&data)
				+ data.info.NameLength + data.info.DataLength), resultlen);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwQueryValueKey for partial info
		 */
		TEST_METHOD(DdkRegistryZwQueryValueKeyPartial)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			OBJECT_ATTRIBUTES attr;
			DWORD i = 0, match = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			struct { KEY_VALUE_PARTIAL_INFORMATION info; WCHAR v[1024]; } data;
			DWORD resultlen = 0;

			RtlInitUnicodeString(&u, L"SystemRoot");

			status = ZwQueryValueKey(h, &u, KeyValuePartialInformation,
				(PVOID)&data.info, sizeof(data), &resultlen);

			Assert::AreEqual((ULONG)REG_SZ, data.info.Type);
			Assert::AreEqual(0, wcsncmp((WCHAR*)data.info.Data, L"C:\\Windows", 10));
			Assert::AreEqual((ULONG)(((char *)&data.info.Data - (char *)&data)
				+ data.info.DataLength), resultlen);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwEnumerateKey for basic info
		 */
		TEST_METHOD(DdkRegistryZwEnumerateKeyBasic)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			OBJECT_ATTRIBUTES attr;
			DWORD i = 0, match = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			for (; true; i++) {
				struct { KEY_BASIC_INFORMATION info; WCHAR v[1024]; } data;
				DWORD resultlen = 0;
			
				NTSTATUS status = ZwEnumerateKey(h, i, KeyBasicInformation,
					(PVOID)&data.info, sizeof(data), &resultlen);

				if (status == STATUS_NO_MORE_ENTRIES) break;

				Assert::AreEqual(STATUS_SUCCESS, status);

				if (wcsncmp(data.info.Name, L"Control", 7) == 0) {
					Assert::AreEqual((ULONG)14, data.info.NameLength);
					Assert::AreEqual((ULONG)(((char *)&data.info.Name - (char *)&data)
						+ data.info.NameLength), resultlen);
					match++;
				}
			}

			Assert::IsTrue(i >= 2 && match);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwEnumerateKey for node info
		 */
		TEST_METHOD(DdkRegistryZwEnumerateKeyNode)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			OBJECT_ATTRIBUTES attr;
			DWORD i = 0, match = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			for (; true; i++) {
				struct { KEY_NODE_INFORMATION info; WCHAR v[1024]; } data;
				DWORD resultlen = 0;
			
				NTSTATUS status = ZwEnumerateKey(h, i, KeyNodeInformation,
					(PVOID)&data.info, sizeof(data), &resultlen);

				if (status == STATUS_NO_MORE_ENTRIES) break;

				Assert::AreEqual(STATUS_SUCCESS, status);

				if (wcsncmp(data.info.Name, L"Control", 7) == 0) {
					Assert::AreEqual((ULONG)14, data.info.NameLength);
					Assert::AreEqual((ULONG)0, data.info.ClassLength);
					Assert::AreEqual((char *)&data.info.Name + data.info.NameLength,
						(char *)&data + data.info.ClassOffset);
					Assert::AreEqual((ULONG)(((char *)&data.info.Name - (char *)&data)
						+ data.info.NameLength + data.info.ClassLength), resultlen);
					match++;
				}
			}

			Assert::IsTrue(i >= 2 && match);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwEnumerateKey for full info
		 */
		TEST_METHOD(DdkRegistryZwEnumerateKeyFull)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			OBJECT_ATTRIBUTES attr;
			DWORD i = 0, match = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			for (; true; i++) {
				struct { KEY_FULL_INFORMATION info; WCHAR v[1024]; } data;
				DWORD resultlen = 0;
			
				NTSTATUS status = ZwEnumerateKey(h, i, KeyFullInformation,
					(PVOID)&data.info, sizeof(data), &resultlen);

				if (status == STATUS_NO_MORE_ENTRIES) break;

				Assert::AreEqual(STATUS_SUCCESS, status);

				if (data.info.SubKeys > 0) {
					Assert::IsTrue(data.info.MaxNameLen < 256);
					Assert::IsTrue(data.info.MaxValueNameLen < 256);
					Assert::AreEqual((char *)data.info.Class, (char *)&data + data.info.ClassOffset);
					Assert::AreEqual((ULONG)(((char *)&data.info.Class - (char *)&data)
						+ data.info.ClassLength), resultlen);
					match++;
				}
			}

			Assert::IsTrue(i >= 2 && match);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwEnumerateValueKey for basic info
		 */
		TEST_METHOD(DdkRegistryZwEnumerateValueKeyBasic)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			OBJECT_ATTRIBUTES attr;
			DWORD i = 0, match = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			for (; true; i++) {
				struct { KEY_VALUE_BASIC_INFORMATION info; WCHAR v[1024]; } data;
				DWORD resultlen = 0;
			
				NTSTATUS status = ZwEnumerateValueKey(h, i, KeyValueBasicInformation,
					(PVOID)&data.info, sizeof(data), &resultlen);

				if (status == STATUS_NO_MORE_ENTRIES) break;

				Assert::AreEqual(STATUS_SUCCESS, status);

				if (wcsncmp(data.info.Name, L"SystemRoot", 10) == 0) {
					Assert::AreEqual((ULONG)REG_SZ, data.info.Type);
					Assert::AreEqual((ULONG)20, data.info.NameLength);
					Assert::AreEqual((ULONG)(((char *)&data.info.Name - (char *)&data)
						+ data.info.NameLength), resultlen);
					match++;
				}
			}

			Assert::IsTrue(i >= 3 && match);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwEnumerateValueKey for full info
		 */
		TEST_METHOD(DdkRegistryZwEnumerateValueKeyFull)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			OBJECT_ATTRIBUTES attr;
			DWORD i = 0, match = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			for (; true; i++) {
				struct { KEY_VALUE_FULL_INFORMATION info; WCHAR v[1024]; } data;
				DWORD resultlen = 0;
			
				NTSTATUS status = ZwEnumerateValueKey(h, i, KeyValueFullInformation,
					(PVOID)&data.info, sizeof(data), &resultlen);

				if (status == STATUS_NO_MORE_ENTRIES) break;

				Assert::AreEqual(STATUS_SUCCESS, status);

				if (wcsncmp(data.info.Name, L"SystemRoot", 10) == 0) {
					Assert::AreEqual((ULONG)REG_SZ, data.info.Type);
					Assert::AreEqual((ULONG)20, data.info.NameLength);
					Assert::AreEqual((ULONG)22, data.info.DataLength);
					Assert::AreEqual(0, wcsncmp((WCHAR*)((char *)&data + data.info.DataOffset),
						L"C:\\Windows", 10));
					Assert::AreEqual((ULONG)(((char *)&data.info.Name - (char *)&data)
						+ data.info.NameLength + data.info.DataLength), resultlen);
					match++;
				}
			}

			Assert::IsTrue(i >= 3 && match);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwEnumerateValueKey for partial info
		 */
		TEST_METHOD(DdkRegistryZwEnumerateValueKeyPartial)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			OBJECT_ATTRIBUTES attr;
			DWORD i = 0, match = 0;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKeyEx(&h, GENERIC_ALL, &attr, 0);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			for (; true; i++) {
				struct { KEY_VALUE_PARTIAL_INFORMATION info; WCHAR v[1024]; } data;
				DWORD resultlen = 0;
			
				NTSTATUS status = ZwEnumerateValueKey(h, i, KeyValuePartialInformation,
					(PVOID)&data.info, sizeof(data), &resultlen);

				if (status == STATUS_NO_MORE_ENTRIES) break;

				Assert::AreEqual(STATUS_SUCCESS, status);

				if (data.info.DataLength == 22) {
					Assert::AreEqual((ULONG)REG_SZ, data.info.Type);
					Assert::AreEqual(0, wcsncmp((WCHAR*)data.info.Data, L"C:\\Windows", 10));
					Assert::AreEqual((ULONG)(((char *)&data.info.Data - (char *)&data)
						+ data.info.DataLength), resultlen);
					match++;
				}
			}

			Assert::IsTrue(i >= 3 && match);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwFlushKey
		 */
		TEST_METHOD(DdkRegistryZwFlushKey)
		{
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKey(&h, GENERIC_ALL, &attr);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = ZwFlushKey(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwDeleteValueKey on a value that exists
		 */
		TEST_METHOD(DdkRegistryZwDeleteValueKey)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			DWORD data = 0x12345678;
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test1311",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKey(&h, GENERIC_ALL, &attr);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test1311";
			table[0].EntryContext = &data;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data);

			RtlInitUnicodeString(&u, L"Test1311");
			status = ZwDeleteValueKey(h, &u);
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);

			data = 0;
			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((DWORD)0, data);
		}

		/*
		 *	Check ZwDeleteValueKey on a value that doesn't exist
		 */
		TEST_METHOD(DdkRegistryZwDeleteValueKeyNonExistantValue)
		{
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKey(&h, GENERIC_ALL, &attr);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			RtlInitUnicodeString(&u, L"Test1312");
			status = ZwDeleteValueKey(h, &u);
			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwClose for a valid key
		 */
		TEST_METHOD(DdkRegistryZwClose)
		{
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Control");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKey(&h, GENERIC_ALL, &attr);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check ZwClose for a NULL key
		 */
		TEST_METHOD(DdkRegistryZwCloseNull)
		{
			NTSTATUS status = ZwClose(NULL);
			Assert::AreEqual(STATUS_INVALID_HANDLE, status);
		}

		/*
		 *	Check whether a key that exists is identified
		 */
		TEST_METHOD(DdkRegistryCheckKey)
		{
			NTSTATUS status;

			status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Control");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check user keys
		 */
		TEST_METHOD(DdkRegistryCheckUserKeys)
		{
			NTSTATUS status;

			status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE,
				L"\\Registry\\User\\CurrentUser");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE,
				L"\\Registry\\User\\.Default");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check that non-existant keys return an error
		 */
		TEST_METHOD(DdkRegistryCheckKeyNotPresent)
		{
			NTSTATUS status;

			status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\xx");
			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			
			status = RtlCheckRegistryKey(RTL_REGISTRY_CONTROL, L"xx");
			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
		}

		/*
		 *	Check creating a key
		 */
		TEST_METHOD(DdkRegistryCreateKey)
		{
			NTSTATUS status;

			status = RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Test101");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test102");
			Assert::AreEqual(STATUS_SUCCESS, status);

			DdkDeleteRegistryKey(RTL_REGISTRY_ABSOLUTE,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Test101");

			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test102");
		}

		/*
		 *	Check creating a key with a missing path component
		 */
		TEST_METHOD(DdkRegistryCreateKeyMissingPath)
		{
			NTSTATUS status;

			status = RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\xx\\Test201");
			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);

			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"xx\\Test202");
			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
		}

		/*
		 *	Check creating a key that exists
		 */
		TEST_METHOD(DdkRegistryRecreateKey)
		{
			NTSTATUS status;

			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test301");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test301");
			Assert::AreEqual(STATUS_SUCCESS, status);

			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test301");
		}

		/*
		 *	Check deleting a value that exists
		 */
		TEST_METHOD(DdkRegistryDeleteValue)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			DWORD data = 0x12345678;
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test311",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test311";
			table[0].EntryContext = &data;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test311");
			Assert::AreEqual(STATUS_SUCCESS, status);

			data = 0;
			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((DWORD)0, data);
		}

		/*
		 *	Check deleting a value that doesn't exist
		 */
		TEST_METHOD(DdkRegistryDeleteValueNonExistant)
		{
			NTSTATUS status;

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test312");
			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
		}

		/*
		 *	Check deleting a value using a handle
		 */
		TEST_METHOD(DdkRegistryDeleteValueHandle)
		{
			DWORD data = 0x12345678;
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test317",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKey(&h, GENERIC_ALL, &attr);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_HANDLE, (WCHAR *)h, L"Test317");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check deleting a value using a null handle
		 */
		TEST_METHOD(DdkRegistryDeleteValueNullHandle)
		{
			DWORD data = 0x12345678;
			NTSTATUS status;
			HANDLE h = NULL;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test318",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_HANDLE, (WCHAR *)h, L"Test318");
			Assert::AreEqual(STATUS_INVALID_HANDLE, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test318");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check writing a String to default value
		 */
		TEST_METHOD(DdkRegistryWriteDefault)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678";
			UNICODE_STRING u;
			NTSTATUS status;

			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test901");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"Test901", L"",
				REG_SZ, data, sizeof(WCHAR) * 9);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Test901", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 8), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"Test901", L"");
			Assert::AreEqual(STATUS_SUCCESS, status);

			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test901");
		}

		/*
		 *	Check creating a DWORD value
		 */
		TEST_METHOD(DdkRegistryWriteInteger)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			DWORD data = 0x12345678;
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test401",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test401";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test401");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check modifying a DWORD value
		 */
		TEST_METHOD(DdkRegistryWriteIntegerRewrite)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			DWORD data = 0x23456781;
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test501",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			data = 0x12345678;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test501",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test501";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test501");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check creating a DWORD value using a handle
		 */
		TEST_METHOD(DdkRegistryWriteIntegerHandle)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			DWORD data = 0x12345678;
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKey(&h, GENERIC_ALL, &attr);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			status = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE, (WCHAR *)h, L"Test408",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test408";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_HANDLE, (WCHAR *)h, L"Test408");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check creating a DWORD value using a null handle
		 */
		TEST_METHOD(DdkRegistryWriteIntegerNullHandle)
		{
			DWORD data = 0x12345678;
			NTSTATUS status;
			HANDLE h = NULL;

			status = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE, (WCHAR *)h, L"Test408",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_INVALID_HANDLE, status);
		}

		/*
		 *	Check writing a QWORD value
		 */
		TEST_METHOD(DdkRegistryWriteQuad)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			__int64 data = 0x12345678;
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test411",
				REG_QWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));
			*(LONG *)&data = (-(LONG)sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test411";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((__int64)0x12345678, data);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test411");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check writing a BINARY value
		 */
		TEST_METHOD(DdkRegistryWriteBinary)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			struct { int a, b, c; } data;
			NTSTATUS status;

			data.a = 0x12345678;
			data.b = 0x23456789;
			data.c = 0x34567890;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test412",
				REG_BINARY, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));
			*(LONG *)&data = (-(LONG)sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test412";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(0x12345678, data.a);
			Assert::AreEqual(0x23456789, data.b);
			Assert::AreEqual(0x34567890, data.c);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test412");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check writing a String value
		 */
		TEST_METHOD(DdkRegistryWriteString)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678";
			UNICODE_STRING u;
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test601",
				REG_SZ, data, sizeof(WCHAR) * 9);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test601";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 8), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test601");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check modifying a String value
		 */
		TEST_METHOD(DdkRegistryWriteStringRewrite)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"3456789012";
			UNICODE_STRING u;
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test641",
				REG_SZ, data, sizeof(WCHAR) * 11);
			Assert::AreEqual(STATUS_SUCCESS, status);

			data = L"12345678";
			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test641",
				REG_SZ, data, sizeof(WCHAR) * 9);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test641";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 8), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test641");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check writing a Multi-String value
		 */
		TEST_METHOD(DdkRegistryWriteMultiString)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678\0" L"23456789\0";
			UNICODE_STRING u;
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test651",
				REG_MULTI_SZ, data, sizeof(WCHAR) * 19);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED |
							 RTL_QUERY_REGISTRY_NOEXPAND;
			table[0].Name = L"Test651";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 18), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual(0, wcsncmp(u.Buffer + sizeof(WCHAR) * 9, data + sizeof(WCHAR) * 9, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[17]);
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[18]);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test651");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying an non-existant value
		 */
		TEST_METHOD(DdkRegistryQueryNotFound)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			DWORD data;

			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Hostname101";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Tcpip\\Parameters", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
		}

		/*
		 *	Check querying an non-existant subkey
		 */
		TEST_METHOD(DdkRegistryQuerySubkeyNotFound)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];

			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
			table[0].Name = L"NotThere";
			table[0].DefaultType = REG_NONE;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
		}

		/*
		 *	Check querying an entry with NOVALUE
		 */
		static NTSTATUS QueryNoValueRoutine(PWSTR ValueName, ULONG ValueType,
			PVOID ValueData, ULONG ValueLength, PVOID Context, PVOID EntryContext)
		{
			Assert::AreEqual(REG_NONE, ValueType);
			Assert::AreEqual((PVOID)NULL, ValueData);
			Assert::AreEqual((ULONG)0, ValueLength);
			Assert::AreEqual((PVOID)0x1234, EntryContext);
			Assert::AreEqual((PVOID)0x5678, Context);
			return STATUS_SUCCESS;
		}

		TEST_METHOD(DdkRegistryQueryNoValue)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];

			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_NOVALUE;
			table[0].Name = NULL;
			table[0].QueryRoutine = QueryNoValueRoutine;
			table[0].EntryContext = (PVOID)0x1234;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], (PVOID)0x5678, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying with correct type
		 */
		TEST_METHOD(DdkRegistryQueryTypeOk)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			UNICODE_STRING u;

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED |
							 RTL_QUERY_REGISTRY_TYPECHECK;
			table[0].Name = L"Hostname";
			table[0].EntryContext = &u;
			table[0].DefaultType = (REG_SZ << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_NONE;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Tcpip\\Parameters", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 8), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, L"UnitTest", 8));
		}

		/*
		 *	Check querying with incorrect type
		 */
		TEST_METHOD(DdkRegistryQueryTypeFail)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];

			memset(table, 0, sizeof(table));
			DWORD data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED |
							 RTL_QUERY_REGISTRY_TYPECHECK;
			table[0].Name = L"Hostname";
			table[0].EntryContext = &data;
			table[0].DefaultType = (REG_DWORD << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_NONE;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Tcpip\\Parameters", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_OBJECT_TYPE_MISMATCH, status);
		}

		/*
		 *	Check querying with spurious routine
		 */
		TEST_METHOD(DdkRegistryQuerySpuriousRoutine)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];

			memset(table, 0, sizeof(table));
			DWORD data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"CurrentMajorVersionNumber";
			table[0].EntryContext = &data;
			table[0].QueryRoutine = QueryNoValueRoutine;
			table[0].DefaultType = REG_NONE;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_INVALID_PARAMETER, status);
		}

		/*
		 *	Check querying with missing routine
		 */
		TEST_METHOD(DdkRegistryQueryMissingRoutine)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];

			memset(table, 0, sizeof(table));
			DWORD data = 0;

			table[0].Flags =RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"CurrentMajorVersionNumber";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_INVALID_PARAMETER, status);
		}

		/*
		 *	Check querying and delete value
		 */
		TEST_METHOD(DdkRegistryQueryDelete)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			DWORD data = 0x12345678;
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test111",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED |
							 RTL_QUERY_REGISTRY_DELETE;
			table[0].Name = L"Test111";
			table[0].EntryContext = &data;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data);

			memset(table, 0, sizeof(table));
			data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test111";
			table[0].EntryContext = &data;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((DWORD)0, data);
		}

		/*
		 *	Check querying a null name using a routine
		 */
		static NTSTATUS QueryNullRoutine(PWSTR ValueName, ULONG ValueType,
			PVOID ValueData, ULONG ValueLength, PVOID Context, PVOID EntryContext)
		{
			if (wcscmp(ValueName, L"CurrentMajorVersionNumber") == 0) {
				Assert::AreEqual(REG_DWORD, ValueType);
				Assert::AreEqual((DWORD)10, *(DWORD *)ValueData);
				Assert::AreEqual((ULONG)sizeof(DWORD), ValueLength);
				(*(DWORD *)EntryContext)++;
			}
			(*(DWORD *)Context)++;
			return STATUS_SUCCESS;
		}

		TEST_METHOD(DdkRegistryQueryNull)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			ULONG count = 0, match = 0;
			NTSTATUS status;
 
			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = NULL;
			table[0].QueryRoutine = QueryNullRoutine;
			table[0].EntryContext = (PVOID)&match;

			status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], &count, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)1, match);
			Assert::IsTrue(count > match);
		}

		/*
		 *	Check querying a null name on an empty key using a routine
		 */
		static NTSTATUS QueryNullEmptyRoutine(PWSTR ValueName, ULONG ValueType,
			PVOID ValueData, ULONG ValueLength, PVOID Context, PVOID EntryContext)
		{
			(*(DWORD *)Context)++;
			return STATUS_SUCCESS;
		}

		TEST_METHOD(DdkRegistryQueryNullEmpty)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			ULONG count = 0;
			NTSTATUS status;
 
			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test121");
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));

			table[0].Flags = 0;
			table[0].Name = NULL;
			table[0].QueryRoutine = QueryNullRoutine;
			table[0].EntryContext = (PVOID)0x1234;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Test121", &table[0], &count, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)0, count);

			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test121");
		}

		/*
		 *	Check querying a required null name on an empty key using a routine
		 */
		TEST_METHOD(DdkRegistryQueryNullEmptyRequired)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			ULONG count = 0;
			NTSTATUS status;
 
			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test122");
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = NULL;
			table[0].QueryRoutine = QueryNullRoutine;
			table[0].EntryContext = (PVOID)0x1234;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Test121", &table[0], &count, NULL);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((ULONG)0, count);

			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test122");
		}

		/*
		 *	Check querying a null name on a subkey using a routine
		 */
		TEST_METHOD(DdkRegistryQueryNullSubkey)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			ULONG count = 0;
			NTSTATUS status;
 
			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test123");
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			DWORD data = 0x12345678;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"Test123", L"Test123",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_SUBKEY;
			table[0].Name = L"Test123";
			table[0].QueryRoutine = QueryNullRoutine;
			table[0].EntryContext = (PVOID)0x1234;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], &count, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)1, count);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"Test123", L"Test123");
			Assert::AreEqual(STATUS_SUCCESS, status);

			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test123");
		}

		/*
		 *	Check querying a null name with delete using a routine
		 */
		TEST_METHOD(DdkRegistryQueryNullDelete)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			ULONG count = 0;
			NTSTATUS status;
 
			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test124");
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			DWORD data = 0x12345678;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"Test124", L"Test124",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"Test124", L"Test125",
				REG_DWORD, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_DELETE;
			table[0].Name = NULL;
			table[0].QueryRoutine = QueryNullEmptyRoutine;
			table[0].EntryContext = (PVOID)0x1234;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Test124", &table[0], &count, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)2, count);

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Test124", &table[0], &count, NULL);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);

			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test124");
		}

		/*
		 *	Check querying a required value
		 */
		TEST_METHOD(DdkRegistryQueryRequiredPresent)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			NTSTATUS status;

			memset(table, 0, sizeof(table));
			DWORD data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"CurrentMajorVersionNumber";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)10, data);
		}

		/*
		 *	Check querying a non present required value
		 */
		TEST_METHOD(DdkRegistryQueryRequiredNotPresent)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			NTSTATUS status;

			memset(table, 0, sizeof(table));
			DWORD data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"NotFound";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((DWORD)0, data);
		}

		/*
		 *	Check querying a required value with a routine
		 */
		TEST_METHOD(DdkRegistryQueryRequiredPresentRoutine)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			NTSTATUS status;
			DWORD count = 0;

			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"CurrentMajorVersionNumber";
			table[0].QueryRoutine = QueryNullEmptyRoutine;

			status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], &count, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)1, count);
		}

		/*
		 *	Check querying a non-present required value with a routine
		 */
		TEST_METHOD(DdkRegistryQueryRequiredNotPresentRoutine)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			NTSTATUS status;
			DWORD count = 0;

			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"NotFound";
			table[0].QueryRoutine = QueryNullEmptyRoutine;

			status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], &count, NULL);

			Assert::AreEqual(STATUS_OBJECT_NAME_NOT_FOUND, status);
			Assert::AreEqual((DWORD)0, count);
		}

		/*
		 *	Check querying multiple values
		 */
		TEST_METHOD(DdkRegistryQueryMultiple)
		{
			DWORD data1 = 0x12345678, data2 = 0x34567890;
			RTL_QUERY_REGISTRY_TABLE table[3];
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test471",
				REG_DWORD, &data1, sizeof(data1));
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test472",
				REG_DWORD, &data2, sizeof(data2));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			data1 = data2 = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test471";
			table[0].EntryContext = &data1;
			table[0].DefaultType = REG_NONE;
			table[1].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[1].Name = L"Test472";
			table[1].EntryContext = &data2;
			table[1].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data1);
			Assert::AreEqual((DWORD)0x34567890, data2);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test471");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test472");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying multiple values with subkey
		 */
		TEST_METHOD(DdkRegistryQueryMultipleSubkey)
		{
			DWORD data1 = 0x12345678, data2 = 0x34567890, data3 = 0x56789012;
			RTL_QUERY_REGISTRY_TABLE table[6];
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test481",
				REG_DWORD, &data1, sizeof(data1));
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test482Key");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"Test482Key", L"Test482",
				REG_DWORD, &data2, sizeof(data2));
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test483Key");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"Test483Key", L"Test483",
				REG_DWORD, &data3, sizeof(data3));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			data1 = data2 = data3 = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test481";
			table[0].EntryContext = &data1;
			table[0].DefaultType = REG_NONE;
			table[1].Flags = RTL_QUERY_REGISTRY_SUBKEY;
			table[1].Name = L"Test482Key";
			table[2].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[2].Name = L"Test482";
			table[2].EntryContext = &data2;
			table[2].DefaultType = REG_NONE;
			table[3].Flags = RTL_QUERY_REGISTRY_SUBKEY;
			table[3].Name = L"Test483Key";
			table[4].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[4].Name = L"Test483";
			table[4].EntryContext = &data3;
			table[4].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data1);
			Assert::AreEqual((DWORD)0x34567890, data2);
			Assert::AreEqual((DWORD)0x56789012, data3);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test481");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"Test482Key", L"Test482");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"Test483Key", L"Test483");
			Assert::AreEqual(STATUS_SUCCESS, status);

			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test482Key");
			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test483Key");
		}

		/*
		 *	Check querying multiple values with topkey
		 */
		TEST_METHOD(DdkRegistryQueryMultipleTopkey)
		{
			DWORD data1 = 0x12345678, data2 = 0x34567890, data3 = 0x56789012;
			RTL_QUERY_REGISTRY_TABLE table[5];
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test491",
				REG_DWORD, &data1, sizeof(data1));
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlCreateRegistryKey(RTL_REGISTRY_SERVICES, L"Test492Key");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"Test492Key", L"Test492",
				REG_DWORD, &data2, sizeof(data2));
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test493",
				REG_DWORD, &data3, sizeof(data3));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			data1 = data2 = data3 = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test491";
			table[0].EntryContext = &data1;
			table[0].DefaultType = REG_NONE;
			table[1].Flags = RTL_QUERY_REGISTRY_SUBKEY;
			table[1].Name = L"Test492Key";
			table[2].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[2].Name = L"Test492";
			table[2].EntryContext = &data2;
			table[2].DefaultType = REG_NONE;
			table[3].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED |
							 RTL_QUERY_REGISTRY_TOPKEY;
			table[3].Name = L"Test493";
			table[3].EntryContext = &data3;
			table[3].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x12345678, data1);
			Assert::AreEqual((DWORD)0x34567890, data2);
			Assert::AreEqual((DWORD)0x56789012, data3);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test491");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"Test492Key", L"Test492");
			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test493");
			Assert::AreEqual(STATUS_SUCCESS, status);

			DdkDeleteRegistryKey(RTL_REGISTRY_SERVICES, L"Test492Key");
		}

		/*
		 *	Check querying a String value
		 */
		TEST_METHOD(DdkRegistryQueryString)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			UNICODE_STRING u;

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Hostname";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Tcpip\\Parameters", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 8), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, L"UnitTest", 8));
		}

		/*
		 *	Check querying a String value using a routine
		 */
		static NTSTATUS QueryStringRoutine(PWSTR ValueName, ULONG ValueType,
			PVOID ValueData, ULONG ValueLength, PVOID Context, PVOID EntryContext)
		{
			Assert::AreEqual(REG_SZ, ValueType);
			Assert::AreEqual(0, wcscmp((WCHAR *)ValueData, L"UnitTest"));
			Assert::AreEqual((ULONG)(sizeof(WCHAR) * 9), ValueLength);
			Assert::AreEqual((PVOID)0x1234, EntryContext);
			Assert::AreEqual((PVOID)0x5678, Context);
			return STATUS_SUCCESS;
		}

		TEST_METHOD(DdkRegistryQueryStringRoutine)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];

			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Hostname";
			table[0].QueryRoutine = QueryStringRoutine;
			table[0].EntryContext = (PVOID)0x1234;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Tcpip\\Parameters", &table[0], (PVOID)0x5678, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a non-existant String value returns the default
		 */
		TEST_METHOD(DdkRegistryQueryStringDefault)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			UNICODE_STRING u;

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
			table[0].Name = L"Hostname101";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_SZ;
			table[0].DefaultData = L"NotThere";
			table[0].DefaultLength = sizeof(WCHAR) * 9;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Tcpip\\Parameters", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 8), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, L"NotThere", 8));
		}

		/*
		 *	Check querying a non-existant String value returns a calculated
		 *	default string length
		 */
		TEST_METHOD(DdkRegistryQueryStringDefaultNoLength)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			UNICODE_STRING u;

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
			table[0].Name = L"Hostname101";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_SZ;
			table[0].DefaultData = L"NotThere";
			table[0].DefaultLength = 0;

			NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"Tcpip\\Parameters", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 8), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, L"NotThere", 8));
		}

		/*
		 *	Check querying a Multi-String value
		 */
		TEST_METHOD(DdkRegistryQueryMultiString)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678\0" L"23456789\0";
			UNICODE_STRING u;
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test671",
				REG_MULTI_SZ, data, sizeof(WCHAR) * 19);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED |
							 RTL_QUERY_REGISTRY_NOEXPAND;
			table[0].Name = L"Test671";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 18), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual(0, wcsncmp(u.Buffer + sizeof(WCHAR) * 9, data + sizeof(WCHAR) * 9, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[17]);
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[18]);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test671");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a Multi-String value using a routine
		 */
		static NTSTATUS QueryMultiStringRoutine(PWSTR ValueName, ULONG ValueType,
			PVOID ValueData, ULONG ValueLength, PVOID Context, PVOID EntryContext)
		{
			Assert::AreEqual(REG_MULTI_SZ, ValueType);
			Assert::AreEqual((ULONG)sizeof(WCHAR) * 19, ValueLength);
			Assert::AreEqual(0, wcsncmp((WCHAR *)ValueData, L"12345678", 8));
			Assert::AreEqual(0, wcsncmp((WCHAR *)ValueData + 9, L"23456789", 8));
			Assert::AreEqual((WCHAR)L'\0', ((WCHAR *)ValueData)[8]);
			Assert::AreEqual((WCHAR)L'\0', ((WCHAR *)ValueData)[17]);
			Assert::AreEqual((WCHAR)L'\0', ((WCHAR *)ValueData)[18]);
			Assert::AreEqual((PVOID)0x1234, EntryContext);
			Assert::AreEqual((PVOID)0x5678, Context);
			return STATUS_SUCCESS;
		}

		TEST_METHOD(DdkRegistryQueryMultiStringRoutine)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678\0" L"23456789\0";
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test671",
				REG_MULTI_SZ, data, sizeof(WCHAR) * 19);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
			table[0].Name = L"Test671";
			table[0].QueryRoutine = QueryMultiStringRoutine;
			table[0].EntryContext = (PVOID)0x1234;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], (PVOID)0x5678, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test671");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying an expanded Multi-String value using a routine
		 */
		static NTSTATUS QueryMultiStringExpandRoutine(PWSTR ValueName, ULONG ValueType,
			PVOID ValueData, ULONG ValueLength, PVOID Context, PVOID EntryContext)
		{
			Assert::AreEqual(REG_SZ, ValueType);
			Assert::AreEqual((ULONG)sizeof(WCHAR) * 8, ValueLength);

			if ((*(DWORD *)Context) == 0) {
				Assert::AreEqual(0, wcsncmp((WCHAR *)ValueData, L"12345678", 8));
				Assert::AreEqual((WCHAR)L'\0', ((WCHAR *)ValueData)[8]);
				Assert::AreEqual((PVOID)0x1234, EntryContext);
			} else {
				Assert::AreEqual(0, wcsncmp((WCHAR *)ValueData, L"23456789", 8));
				Assert::AreEqual((WCHAR)L'\0', ((WCHAR *)ValueData)[8]);
				Assert::AreEqual((PVOID)0x1234, EntryContext);
			}

			(*(DWORD *)Context)++;
			return STATUS_SUCCESS;
		}

		TEST_METHOD(DdkRegistryQueryMultiStringExpandRoutine)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678\0" L"23456789\0";
			NTSTATUS status;
			DWORD index = 0;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test681",
				REG_MULTI_SZ, data, sizeof(WCHAR) * 19);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test681";
			table[0].QueryRoutine = QueryMultiStringExpandRoutine;
			table[0].EntryContext = (PVOID)0x1234;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], &index, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)2, index);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test681");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a Multi-String value without NOEXPAND
		 */
		TEST_METHOD(DdkRegistryQueryMultiStringExpand)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678\0" L"23456789\0";
			UNICODE_STRING u;
			NTSTATUS status;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test672",
				REG_MULTI_SZ, data, sizeof(WCHAR) * 19);
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test672";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreNotEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((USHORT)0, u.Length);
			Assert::AreEqual((USHORT)0, u.MaximumLength);
			Assert::AreEqual((WCHAR *)0, u.Buffer);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test672");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a Multi-String value with a default
		 */
		TEST_METHOD(DdkRegistryQueryMultiStringDefault)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678\0" L"23456789\0";
			UNICODE_STRING u;
			NTSTATUS status;

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED |
							 RTL_QUERY_REGISTRY_NOEXPAND;
			table[0].Name = L"Test679";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_MULTI_SZ;
			table[0].DefaultData = (PVOID)data;
			table[0].DefaultLength = 19 * sizeof(WCHAR);

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 18), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual(0, wcsncmp(u.Buffer + sizeof(WCHAR) * 9, data + sizeof(WCHAR) * 9, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[17]);
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[18]);
		}

		/*
		 *	Check querying a Multi-String value with a default and no length
		 */
		TEST_METHOD(DdkRegistryQueryMultiStringDefaultNoLength)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678\0" L"23456789\0";
			UNICODE_STRING u;
			NTSTATUS status;

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED |
							 RTL_QUERY_REGISTRY_NOEXPAND;
			table[0].Name = L"Test679";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_MULTI_SZ;
			table[0].DefaultData = (PVOID)data;
			table[0].DefaultLength = 0;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((sizeof(WCHAR) * 18), (size_t)u.Length);
			Assert::AreEqual(0, wcsncmp(u.Buffer, data, 8));
			Assert::AreEqual(0, wcsncmp(u.Buffer + sizeof(WCHAR) * 9, data + sizeof(WCHAR) * 9, 8));
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[8]);
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[17]);
			Assert::AreEqual((WCHAR)L'\0', (WCHAR)u.Buffer[18]);
		}

		/*
		 *	Check querying a Multi-String value without a default
		 */
		TEST_METHOD(DdkRegistryQueryMultiStringNoDefault)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			WCHAR *data = L"12345678\0" L"23456789\0";
			UNICODE_STRING u;
			NTSTATUS status;

			memset(table, 0, sizeof(table));
			RtlInitUnicodeString(&u, NULL);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
			table[0].Name = L"Test677";
			table[0].EntryContext = &u;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((USHORT)0, u.Length);
			Assert::AreEqual((USHORT)0, u.MaximumLength);
			Assert::AreEqual((WCHAR *)0, u.Buffer);
		}

		/*
		 *	Check querying a DWORD value
		 */
		TEST_METHOD(DdkRegistryQueryInteger)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			NTSTATUS status;

			memset(table, 0, sizeof(table));
			DWORD data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"CurrentMajorVersionNumber";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)10, data);
		}

		/*
		 *	Check querying a non-existant DWORD value returns the default
		 */
		TEST_METHOD(DdkRegistryQueryIntegerDefault)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			NTSTATUS status;

			memset(table, 0, sizeof(table));
			DWORD data = 0, val = 0x1234567;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test611";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_DWORD;
			table[0].DefaultData = &val;
			table[0].DefaultLength = (ULONG)sizeof(val);

			status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0x1234567, data);
		}

		/*
		 *	Check querying an optional DWORD value with no default
		 */
		TEST_METHOD(DdkRegistryQueryIntegerNoDefault)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			NTSTATUS status;

			memset(table, 0, sizeof(table));
			DWORD data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
			table[0].Name = L"Test691";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)0, data);
		}

		/*
		 *	Check querying a DWORD value using a routine
		 */
		static NTSTATUS QueryIntegerRoutine(PWSTR ValueName, ULONG ValueType,
			PVOID ValueData, ULONG ValueLength, PVOID Context, PVOID EntryContext)
		{
			Assert::AreEqual(REG_DWORD, ValueType);
			Assert::AreEqual((DWORD)10, *(DWORD *)ValueData);
			Assert::AreEqual((ULONG)sizeof(DWORD), ValueLength);
			Assert::AreEqual((PVOID)0x1234, EntryContext);
			Assert::AreEqual((PVOID)0x5678, Context);
			return STATUS_SUCCESS;
		}

		TEST_METHOD(DdkRegistryQueryIntegerRoutine)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			NTSTATUS status;

			memset(table, 0, sizeof(table));
			DWORD data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"CurrentMajorVersionNumber";
			table[0].QueryRoutine = QueryIntegerRoutine;
			table[0].EntryContext = (PVOID)0x1234;

			status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], (PVOID)0x5678, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a DWORD value using a handle
		 */
		TEST_METHOD(DdkRegistryQueryIntegerHandle)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			OBJECT_ATTRIBUTES attr;
			UNICODE_STRING u;
			NTSTATUS status;
			HANDLE h = NULL;

			RtlInitUnicodeString(&u,
				L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion");

			InitializeObjectAttributes(&attr, &u, OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenKey(&h, GENERIC_ALL, &attr);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreNotEqual((HANDLE)NULL, h);

			memset(table, 0, sizeof(table));
			DWORD data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"CurrentMajorVersionNumber";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((DWORD)10, data);

			status = ZwClose(h);
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a DWORD value using a null handle
		 */
		TEST_METHOD(DdkRegistryQueryIntegerNullHandle)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			NTSTATUS status;
			HANDLE h = NULL;

			memset(table, 0, sizeof(table));
			DWORD data = 0;

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"CurrentMajorVersionNumber";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE, (WCHAR *)h,
				&table[0], NULL, NULL);

			Assert::AreEqual(STATUS_INVALID_HANDLE, status);
			Assert::AreEqual((DWORD)0, data);
		}

		/*
		 *	Check querying a BINARY value
		 */
		typedef struct { int a, b, c; } QueryBinary_t;
	
		TEST_METHOD(DdkRegistryQueryBinary)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			QueryBinary_t data;
			NTSTATUS status;

			data.a = 0x12345678;
			data.b = 0x23456789;
			data.c = 0x34567890;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test811",
				REG_BINARY, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&data, 0, sizeof(data));
			*(LONG *)&data = (-(LONG)sizeof(data));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test811";
			table[0].EntryContext = &data;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual(0x12345678, data.a);
			Assert::AreEqual(0x23456789, data.b);
			Assert::AreEqual(0x34567890, data.c);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test811");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a BINARY value returns size and type
		 */
		TEST_METHOD(DdkRegistryQueryBinaryWithInfo)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			struct { ULONG size, type; QueryBinary_t data; } buffer;
			NTSTATUS status;

			buffer.data.a = 0x12345678;
			buffer.data.b = 0x23456789;
			buffer.data.c = 0x34567890;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test821",
				REG_BINARY, &buffer.data, sizeof(buffer.data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&buffer, 0, sizeof(buffer));
			*(ULONG *)&buffer = (ULONG)sizeof(buffer);

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test821";
			table[0].EntryContext = &buffer;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)sizeof(buffer.data), buffer.size);
			Assert::AreEqual(REG_BINARY, buffer.type);
			Assert::AreEqual(0x12345678, buffer.data.a);
			Assert::AreEqual(0x23456789, buffer.data.b);
			Assert::AreEqual(0x34567890, buffer.data.c);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test821");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a BINARY value with a buffer that's too small
		 *	for all the data
		 */
		TEST_METHOD(DdkRegistryQueryBinaryWithInfoPartial)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			struct { ULONG size, type; QueryBinary_t data; } buffer;
			NTSTATUS status;

			buffer.data.a = 0x12345678;
			buffer.data.b = 0x23456789;
			buffer.data.c = 0x34567890;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test831",
				REG_BINARY, &buffer.data, sizeof(buffer.data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&buffer, 0, sizeof(buffer));
			*(ULONG *)&buffer = (ULONG)(sizeof(buffer) - sizeof(int));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test831";
			table[0].EntryContext = &buffer;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)sizeof(buffer.data), buffer.size);
			Assert::AreEqual(REG_BINARY, buffer.type);
			Assert::AreEqual(0x12345678, buffer.data.a);
			Assert::AreEqual(0x23456789, buffer.data.b);
			Assert::AreEqual(0, buffer.data.c);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test831");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a BINARY value with room for the size only
		 */
		TEST_METHOD(DdkRegistryQueryBinaryWithInfoSize)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			struct { ULONG size, type; QueryBinary_t data; } buffer;
			NTSTATUS status;

			buffer.data.a = 0x12345678;
			buffer.data.b = 0x23456789;
			buffer.data.c = 0x34567890;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test842",
				REG_BINARY, &buffer.data, sizeof(buffer.data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&buffer, 0, sizeof(buffer));
			*(ULONG *)&buffer = (ULONG)(sizeof(buffer.size));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test842";
			table[0].EntryContext = &buffer;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)sizeof(buffer.data), buffer.size);
			Assert::AreEqual((ULONG)0, buffer.type);
			Assert::AreEqual(0, buffer.data.a);
			Assert::AreEqual(0, buffer.data.b);
			Assert::AreEqual(0, buffer.data.c);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test842");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a BINARY value with room for the size and type only
		 */
		TEST_METHOD(DdkRegistryQueryBinaryWithInfoType)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			struct { ULONG size, type; QueryBinary_t data; } buffer;
			NTSTATUS status;

			buffer.data.a = 0x12345678;
			buffer.data.b = 0x23456789;
			buffer.data.c = 0x34567890;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test843",
				REG_BINARY, &buffer.data, sizeof(buffer.data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));
			memset(&buffer, 0, sizeof(buffer));
			*(ULONG *)&buffer = (ULONG)(sizeof(buffer.size) + sizeof(buffer.type));

			table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test843";
			table[0].EntryContext = &buffer;
			table[0].DefaultType = REG_NONE;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], NULL, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);
			Assert::AreEqual((ULONG)sizeof(buffer.data), buffer.size);
			Assert::AreEqual(REG_BINARY, buffer.type);
			Assert::AreEqual(0, buffer.data.a);
			Assert::AreEqual(0, buffer.data.b);
			Assert::AreEqual(0, buffer.data.c);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test843");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}

		/*
		 *	Check querying a BINARY value with a routine
		 */
		static NTSTATUS QueryBinaryRoutine(PWSTR ValueName, ULONG ValueType,
			PVOID ValueData, ULONG ValueLength, PVOID Context, PVOID EntryContext)
		{
			QueryBinary_t *ptr = (QueryBinary_t *)ValueData;
			Assert::AreEqual(REG_BINARY, ValueType);
			Assert::AreEqual(0x12345678, ptr->a);
			Assert::AreEqual(0x23456789, ptr->b);
			Assert::AreEqual(0x34567890, ptr->c);
			Assert::AreEqual((ULONG)sizeof(*ptr), ValueLength);
			Assert::AreEqual((PVOID)0x1234, EntryContext);
			Assert::AreEqual((PVOID)0x5678, Context);
			return STATUS_SUCCESS;
		}

		TEST_METHOD(DdkRegistryQueryBinaryRoutine)
		{
			RTL_QUERY_REGISTRY_TABLE table[2];
			QueryBinary_t data;
			NTSTATUS status;

			data.a = 0x12345678;
			data.b = 0x23456789;
			data.c = 0x34567890;

			status = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test911",
				REG_BINARY, &data, sizeof(data));
			Assert::AreEqual(STATUS_SUCCESS, status);

			memset(table, 0, sizeof(table));

			table[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
			table[0].Name = L"Test911";
			table[0].QueryRoutine = QueryBinaryRoutine;
			table[0].EntryContext = (PVOID)0x1234;

			status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
				L"", &table[0], (PVOID)0x5678, NULL);

			Assert::AreEqual(STATUS_SUCCESS, status);

			status = RtlDeleteRegistryValue(RTL_REGISTRY_SERVICES, L"", L"Test911");
			Assert::AreEqual(STATUS_SUCCESS, status);
		}
	};
}
