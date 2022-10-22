/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2018, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Encryption Routines
 */

#include "stdafx.h"

typedef PVOID BCRYPT_HANDLE;
typedef PVOID BCRYPT_ALG_HANDLE;
typedef PVOID BCRYPT_KEY_HANDLE;


DDKAPI
NTSTATUS BCryptOpenAlgorithmProvider(
	_Out_       BCRYPT_ALG_HANDLE   *phAlgorithm,
	_In_        LPCWSTR pszAlgId,
	_In_opt_    LPCWSTR pszImplementation,
	_In_        ULONG   dwFlags)
{
	return STATUS_NOT_IMPLEMENTED;
}


DDKAPI
NTSTATUS
BCryptCloseAlgorithmProvider(
	_Inout_ BCRYPT_ALG_HANDLE   hAlgorithm,
	_In_    ULONG   dwFlags)
{
	return STATUS_NOT_IMPLEMENTED;
}


DDKAPI
NTSTATUS
BCryptSetProperty(
	_Inout_                 BCRYPT_HANDLE   hObject,
	_In_                    LPCWSTR pszProperty,
	_In_reads_bytes_(cbInput)    PUCHAR   pbInput,
	_In_                    ULONG   cbInput,
	_In_                    ULONG   dwFlags)
{
	return STATUS_NOT_IMPLEMENTED;
}


DDKAPI
NTSTATUS
BCryptGetProperty(
	_In_                                        BCRYPT_HANDLE   hObject,
	_In_                                        LPCWSTR pszProperty,
	_Out_writes_bytes_to_opt_(cbOutput, *pcbResult) PUCHAR   pbOutput,
	_In_                                        ULONG   cbOutput,
	_Out_                                       ULONG   *pcbResult,
	_In_                                        ULONG   dwFlags)
{
	return STATUS_NOT_IMPLEMENTED;
}


DDKAPI
NTSTATUS
BCryptGenerateSymmetricKey(
	_Inout_                             BCRYPT_ALG_HANDLE   hAlgorithm,
	_Out_                               BCRYPT_KEY_HANDLE   *phKey,
	_Out_writes_bytes_all_opt_(cbKeyObject)  PUCHAR   pbKeyObject,
	_In_                                ULONG   cbKeyObject,
	_In_reads_bytes_(cbSecret)               PUCHAR   pbSecret,
	_In_                                ULONG   cbSecret,
	_In_                                ULONG   dwFlags)
{
	return STATUS_NOT_IMPLEMENTED;
}


DDKAPI
NTSTATUS
BCryptDestroyKey(
	_Inout_ BCRYPT_KEY_HANDLE   hKey)
{
	return STATUS_NOT_IMPLEMENTED;
}


DDKAPI
NTSTATUS
BCryptEncrypt(
	_Inout_                                     BCRYPT_KEY_HANDLE hKey,
	_In_reads_bytes_opt_(cbInput)                    PUCHAR   pbInput,
	_In_                                        ULONG   cbInput,
	_In_opt_                                    VOID    *pPaddingInfo,
	_Inout_updates_bytes_opt_(cbIV)                    PUCHAR   pbIV,
	_In_                                        ULONG   cbIV,
	_Out_writes_bytes_to_opt_(cbOutput, *pcbResult) PUCHAR   pbOutput,
	_In_                                        ULONG   cbOutput,
	_Out_                                       ULONG   *pcbResult,
	_In_                                        ULONG   dwFlags)
{
	return STATUS_NOT_IMPLEMENTED;
}


DDKAPI
NTSTATUS
BCryptDecrypt(
	_Inout_                                     BCRYPT_KEY_HANDLE   hKey,
	_In_reads_bytes_opt_(cbInput)                    PUCHAR   pbInput,
	_In_                                        ULONG   cbInput,
	_In_opt_                                    VOID    *pPaddingInfo,
	_Inout_updates_bytes_opt_(cbIV)                    PUCHAR   pbIV,
	_In_                                        ULONG   cbIV,
	_Out_writes_bytes_to_opt_(cbOutput, *pcbResult) PUCHAR   pbOutput,
	_In_                                        ULONG   cbOutput,
	_Out_                                       ULONG   *pcbResult,
	_In_                                        ULONG   dwFlags)
{
	return STATUS_NOT_IMPLEMENTED;
}


DDKAPI
NTSTATUS
BCryptGenRandom(
	_In_opt_                        BCRYPT_ALG_HANDLE   hAlgorithm,
	_Out_writes_bytes_(cbBuffer)    PUCHAR  pbBuffer,
	_In_                            ULONG   cbBuffer,
	_In_                            ULONG   dwFlags)
{
	return STATUS_NOT_IMPLEMENTED;
}

