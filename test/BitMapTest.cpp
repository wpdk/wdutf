/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2016, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	BitMap Tests
 */

#include "stdafx.h"


namespace DdkUnitTest
{
	TEST_CLASS(DdkBitMapTest)
	{
		static const int bitcount = 1000;
		static const int bufcount = (bitcount - 1) / (sizeof(ULONG) * 8);

		ULONG buffer[bufcount+1];
		RTL_BITMAP map;

		static const ULONG NUM_BITS_PER_ARRAY_INDEX = (sizeof(ULONG)* 8);
		static const ULONG ALL_BITS_SET = 0xFFFFFFFF;

	public:

		TEST_METHOD_INITIALIZE(DdkBitMapInit)
		{
			memset(buffer, 0, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);
		}


#pragma region RtlInitializeBitMapTests

		// Check initialisation to zero
		TEST_METHOD(DdkBitMapInitToZero)
		{
			// Check bit map is initialised to 0 in "TEST_METHOD_INITIALIZE"
			for (int buffer_index = 0; buffer_index <= bufcount; buffer_index++)
			{
				Assert::IsTrue(0 == map.Buffer[buffer_index]);
			}
		}

		// Check initialisation to one
		TEST_METHOD(DdkBitMapInitToOne)
		{
			memset(buffer, 0xff, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			// Check bit map is initialised to 1
			for (int buffer_index = 0; buffer_index <= bufcount; buffer_index++)
			{
				Assert::IsTrue(ALL_BITS_SET == map.Buffer[buffer_index]);
			}
		}

#pragma endregion


#pragma region RtlClearBitTests
		
		TEST_METHOD(DdkBitMapClearBitNullPtr)
		{
			TEST_DDK_FAIL(RtlClearBit(NULL, 0));
		}

		TEST_METHOD(DdkBitMapClearBitExceedBufSize)
		{
			TEST_DDK_FAIL(RtlClearBit(&map, (bitcount + 1)));
		}

		TEST_METHOD(DdkBitMapClearBit)
		{
			// Initialise buffer to all ones
			memset(buffer, 0xff, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			// Specify bit to be cleared and call clear bit function
			ULONG bit_to_clear = 652;
			RtlClearBit(&map, bit_to_clear);

			// Set up comparison for Assert check
			int array_index = bit_to_clear / NUM_BITS_PER_ARRAY_INDEX;
			int shift_number = bit_to_clear - (array_index * NUM_BITS_PER_ARRAY_INDEX);
			
			ULONG comparison_num = ALL_BITS_SET & ~(1 << shift_number);

			Assert::IsTrue(comparison_num == map.Buffer[array_index]);
		}

#pragma endregion
		

#pragma region RtlSetBitTests

		TEST_METHOD(DdkBitMapSetBitNullPtr)
		{
			TEST_DDK_FAIL(RtlSetBit(NULL, 0));
		}

		TEST_METHOD(DdkBitMapSetBitExceedBufSize)
		{
			TEST_DDK_FAIL(RtlSetBit(&map, (bitcount + 1)));
		}

		TEST_METHOD(DdkBitMapSetBit)
		{
			// Initialise buffer to all zeros
			memset(buffer, 0, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			// Specify bit to be set and call set bit function
			ULONG bit_to_clear = 67;
			RtlSetBit(&map, bit_to_clear);

			// Set up comparison for Assert check
			int array_index = bit_to_clear / NUM_BITS_PER_ARRAY_INDEX;
			int shift_number = bit_to_clear - (array_index * NUM_BITS_PER_ARRAY_INDEX);

			ULONG comparison_num = 0x0 | (1 << shift_number);

			Assert::IsTrue(comparison_num == map.Buffer[array_index]);
		}

#pragma endregion


#pragma region RtlClearAllBitsTests

		TEST_METHOD(DdkBitMapClearAllBitsNullPtr)
		{
			TEST_DDK_FAIL(RtlClearAllBits(NULL));
		}

		TEST_METHOD(DdkBitMapClearAllBits)
		{
			// Initialise buffer to all ones
			memset(buffer, 0xff, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			// Clear all the bits in the bit map
			RtlClearAllBits(&map);

			unsigned last_buffer_index = (map.SizeOfBitMap - 1) / NUM_BITS_PER_ARRAY_INDEX;

			// Check that all the buffer elements up to the last buffer are 0.
			for (unsigned buffer_index = 0; buffer_index < last_buffer_index; buffer_index++)
			{
				Assert::IsTrue(0 == map.Buffer[buffer_index]);
			}

			// Check that the last buffer element is 0.
			unsigned last_bit_position_in_array_element = (bitcount - 1) - (last_buffer_index * NUM_BITS_PER_ARRAY_INDEX);

			for (unsigned bit_position = 0; bit_position <= last_bit_position_in_array_element; bit_position++)
			{
				ULONG bit_set = map.Buffer[last_buffer_index] & (1 << bit_position);
				Assert::IsTrue(0 == bit_set);
			}
		}

		TEST_METHOD(DdkBitMapClearAllBitsOneArrayElement)
		{
			ULONG buf[1] = { 0xFF };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			// Clear all the bits in the bit map
			RtlClearAllBits(&map);
			Assert::IsTrue(0 == map.Buffer[0]);
		}

#pragma endregion

		
#pragma region RtlSetAllBitsTests

		TEST_METHOD(DdkBitMapSetAllBitsNullPtr)
		{
			TEST_DDK_FAIL(RtlSetAllBits(NULL));
		}

		TEST_METHOD(DdkBitMapSetAllBits)
		{
			// Initialise buffer to all zeros
			memset(buffer, 0, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			// Set all the bits in the bit map
			RtlSetAllBits(&map);

			unsigned last_buffer_index = (map.SizeOfBitMap - 1) / NUM_BITS_PER_ARRAY_INDEX;

			// Check that all the buffer elements up to the last buffer are 1.
			for (unsigned buffer_index = 0; buffer_index < last_buffer_index; buffer_index++)
			{
				Assert::IsTrue(ALL_BITS_SET == map.Buffer[buffer_index]);
			}

			unsigned last_bit_position_in_array_element = (bitcount - 1) - (last_buffer_index * NUM_BITS_PER_ARRAY_INDEX);

			// Check that the bits in the last element are 1.
			// This may not be 2^n boundary aligned.
			for (unsigned bit_position = 0; bit_position <= last_bit_position_in_array_element; bit_position++)
			{
				ULONG bit_set = map.Buffer[last_buffer_index] & (1 << bit_position);
				Assert::IsTrue(bit_set != 0);
			}
		}

		TEST_METHOD(DdkBitMapSetAllBitsOneArrayElement)
		{
			ULONG buf[1] = { 0X0 };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			// Set all the bits in the bit map
			RtlSetAllBits(&map);
			Assert::IsTrue(ALL_BITS_SET == map.Buffer[0]);
		}

#pragma endregion


#pragma region RtlClearBitsTests

		TEST_METHOD(DdkBitMapClearBitsNullPtr)
		{
			ULONG starting_index = 0;
			ULONG num_to_clear = 10;

			TEST_DDK_FAIL(RtlClearBits(NULL, starting_index, num_to_clear));
		}

		TEST_METHOD(DdkBitMapClearBitsStartIndexExceedsBufSize)
		{
			ULONG starting_index = bitcount + 1;
			ULONG num_to_clear = 10;

			TEST_DDK_FAIL(RtlClearBits(&map, starting_index, num_to_clear));
		}

		TEST_METHOD(DdkBitMapClearBitsNumToClearExceedsBufSize)
		{
			ULONG starting_index = 0;
			ULONG num_to_clear = bitcount + 1;

			TEST_DDK_FAIL(RtlClearBits(&map, starting_index, num_to_clear));
		}

		TEST_METHOD(DdkBitMapClearBits)
		{
			// Initialise buffer to all ones
			memset(buffer, 0xff, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			ULONG starting_index = 67;
			ULONG number_to_clear = 100;

			RtlClearBits(&map, starting_index, number_to_clear);

			ULONG start_buffer_index = starting_index / NUM_BITS_PER_ARRAY_INDEX;
			unsigned start_bit_position = starting_index - (start_buffer_index * NUM_BITS_PER_ARRAY_INDEX);

			ULONG last_buffer_index = (starting_index + number_to_clear - 1) / NUM_BITS_PER_ARRAY_INDEX;
			unsigned last_bit_position = (starting_index + number_to_clear - 1) - (last_buffer_index * NUM_BITS_PER_ARRAY_INDEX);

			// Check the first buffer element
			for (ULONG bit_position = start_bit_position; bit_position < NUM_BITS_PER_ARRAY_INDEX; bit_position++)
			{
				ULONG bit_set = map.Buffer[start_buffer_index] & (1 << bit_position);
				Assert::IsTrue(0 == bit_set);
			}

			// Check buffer elements between start and last
			for (ULONG buffer_index = (start_buffer_index + 1); buffer_index < last_buffer_index; buffer_index++)
			{
				Assert::IsTrue(0 == map.Buffer[buffer_index]);
			}

			// Check last buffer element
			for (ULONG bit_position = 0; bit_position <= last_bit_position; bit_position++)
			{
				ULONG bit_set = map.Buffer[last_buffer_index] & (1 << bit_position);
				Assert::IsTrue(0 == bit_set);
			}
		}

		TEST_METHOD(DdkBitMapClearBitsSameStartEndIndex)
		{
			// Initialise buffer to all ones
			memset(buffer, 0xff, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			ULONG starting_index = 1;
			ULONG num_to_clear = 8;

			RtlClearBits(&map, starting_index, num_to_clear);
			Assert::IsTrue(0xFFFFFE01 == map.Buffer[0]);

			for (int i = 1; i <= bufcount; i++)
				Assert::IsTrue(ALL_BITS_SET == map.Buffer[i]);
		}

#pragma endregion


#pragma region RtlSetBitsTests
		
		TEST_METHOD(DdkBitMapSetBitsNullPtr)
		{
			ULONG starting_index = 0;
			ULONG num_to_set = 10;

			TEST_DDK_FAIL(RtlSetBits(NULL, starting_index, num_to_set));
		}

		TEST_METHOD(DdkBitMapSetBitsStartIndexExceedsBufSize)
		{
			ULONG starting_index = bitcount + 1;
			ULONG num_to_set = 10;

			TEST_DDK_FAIL(RtlSetBits(&map, starting_index, num_to_set));
		}

		TEST_METHOD(DdkBitMapSetBitsNumToClearExceedsBufSize)
		{
			ULONG starting_index = 0;
			ULONG num_to_set = bitcount + 1;

			TEST_DDK_FAIL(RtlSetBits(&map, starting_index, num_to_set));
		}

		TEST_METHOD(DdkBitMapSetBits)
		{
			// Initialise buffer to all ones
			memset(buffer, 0, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			ULONG starting_index = 67;
			ULONG number_to_set = 100;

			RtlSetBits(&map, starting_index, number_to_set);

			ULONG start_buffer_index = starting_index / NUM_BITS_PER_ARRAY_INDEX;
			unsigned start_bit_position = starting_index - (start_buffer_index * NUM_BITS_PER_ARRAY_INDEX);

			ULONG last_buffer_index = (starting_index + number_to_set - 1) / NUM_BITS_PER_ARRAY_INDEX;
			unsigned last_bit_position = (starting_index + number_to_set - 1) - (last_buffer_index * NUM_BITS_PER_ARRAY_INDEX);

			// Check the first buffer element
			for (ULONG bit_position = start_bit_position; bit_position < NUM_BITS_PER_ARRAY_INDEX; bit_position++)
			{
				ULONG bit_set = map.Buffer[start_buffer_index] & (1 << bit_position);
				Assert::IsTrue(bit_set != 0);
			}

			// Check buffer elements between start and last
			for (ULONG buffer_index = (start_buffer_index + 1); buffer_index < last_buffer_index; buffer_index++)
			{
				Assert::IsTrue(ALL_BITS_SET == map.Buffer[buffer_index]);
			}

			// Check last buffer element
			for (ULONG bit_position = 0; bit_position <= last_bit_position; bit_position++)
			{
				ULONG bit_set = map.Buffer[last_buffer_index] & (1 << bit_position);
				Assert::IsTrue(bit_set != 0);
			}
		}

		TEST_METHOD(DdkBitMapSetBitsSameStartEndIndex)
		{
			// Initialise buffer to all ones
			memset(buffer, 0x00, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			ULONG starting_index = 1;
			ULONG num_to_set = 8;

			RtlSetBits(&map, starting_index, num_to_set);
			Assert::IsTrue(0x000001FE == map.Buffer[0]);

			for (int i = 1; i <= bufcount; i++)
				Assert::IsTrue(0x0 == map.Buffer[i]);
		}
	
#pragma endregion


#pragma region RtlFindNextForwardRunClearTests

		TEST_METHOD(DdkBitMapFindNextForwardRunClearBufNullPtr)
		{
			ULONG from_index = 0;
			ULONG first_clear_bit_in_run = 0;

			TEST_DDK_FAIL(RtlFindNextForwardRunClear(NULL, from_index, &first_clear_bit_in_run));
		}

		TEST_METHOD(DdkBitMapFindNextForwardRunClearFirstClearBitNullPtr)
		{
			ULONG from_index = 0;

			TEST_DDK_FAIL(RtlFindNextForwardRunClear(&map, from_index, NULL));
		}

		TEST_METHOD(DdkBitMapFindNextForwardRunClearFromIndexExceedsBufSize)
		{
			ULONG from_index = bitcount + 1;
			ULONG first_clear_bit_in_run = 0;

			TEST_DDK_FAIL(RtlFindNextForwardRunClear(&map, from_index, &first_clear_bit_in_run));
		}

		TEST_METHOD(DdkBitMapFindNextForwardRunClear)
		{
			// Initialise buffer to all ones
			memset(buffer, 0xff, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			// Set one element in the bit map to a run of zeros
			unsigned set_run_index = 10;
			map.Buffer[set_run_index] = 0x000000ff;

			unsigned set_run_index2 = 11;
			map.Buffer[set_run_index2] = 0xff000000;

			const ULONG from_index = 320;
			ULONG first_clear_bit_in_run = 0;
			ULONG run_count = RtlFindNextForwardRunClear(&map, from_index, &first_clear_bit_in_run);

			Assert::IsTrue(48 == run_count);

			ULONG expected_first_bit_in_run = (set_run_index * NUM_BITS_PER_ARRAY_INDEX) + 8;
			Assert::IsTrue(expected_first_bit_in_run == first_clear_bit_in_run);
		}

		TEST_METHOD(DdkBitMapFindNextForwardRunClearSameStartEndIndex)
		{
			ULONG buf[1] = { 0x000000FF };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			const ULONG from_index = 2;
			ULONG first_clear_bit_in_run = 0;
			ULONG run_count = RtlFindNextForwardRunClear(&map, from_index, &first_clear_bit_in_run);
			Assert::IsTrue(24 == run_count);
		}

		TEST_METHOD(DdkBitMapFindNextForwardRunClearSameStartEndIndexNoRun)
		{
			ULONG buf[1] = { ALL_BITS_SET };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			const ULONG from_index = 16;
			ULONG first_clear_bit_in_run = 0;
			ULONG run_count = RtlFindNextForwardRunClear(&map, from_index, &first_clear_bit_in_run);
			Assert::IsTrue(0 == run_count);
		}

#pragma endregion


#pragma region RtlAreBitsClearTests

		TEST_METHOD(DdkBitMapAreBitsClearNullPtr)
		{
			ULONG starting_index = 8;
			ULONG length = 64;

			TEST_DDK_FAIL(RtlAreBitsClear(NULL, starting_index, length));
		}

		TEST_METHOD(DdkBitMapAreBitsClearStartIndexExceedBufSize)
		{
			ULONG starting_index = bitcount + 1;
			ULONG length = 64;

			Assert::IsTrue(RtlAreBitsClear(&map, starting_index, length) == FALSE);
		}

		TEST_METHOD(DdkBitMapAreBitsClearLengthExceedBufSize)
		{
			ULONG starting_index = 8;
			ULONG length = bitcount + 1;

			Assert::IsTrue(RtlAreBitsClear(&map, starting_index, length) == FALSE);
		}

		TEST_METHOD(DdkBitMapAreBitsClearLengthZero)
		{
			ULONG starting_index = 8;
			ULONG length = 0;

			Assert::IsTrue(RtlAreBitsClear(&map, starting_index, length) == FALSE);
		}

		TEST_METHOD(DdkBitMapAreBitsClearExpectReturnTrue)
		{
			// Initialise buffer to all ones
			memset(buffer, 0xff, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			ULONG starting_index = 8;
			ULONG length = 64;

			// Clear 64 bits in the bit map, starting at the starting index
			// before carrying out the check
			map.Buffer[0] = 0x000000ff;
			map.Buffer[1] = 0x0;
			map.Buffer[2] = 0xffffff00;

			// Expected to return TRUE because the starting index starts at the clear run
			BOOLEAN bits_clear = RtlAreBitsClear(&map, starting_index, length);

			Assert::IsTrue(TRUE == bits_clear);
		}

		TEST_METHOD(DdkBitMapAreBitsClearExpectReturnFalse)
		{
			// Initialise buffer to all ones
			memset(buffer, 0xff, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			ULONG starting_index = 0;
			ULONG length = 64;

			// Clear 64 bits in the bit map, starting at the starting index
			// before carrying out the check
			map.Buffer[0] = 0x000000ff;
			map.Buffer[1] = 0x0;
			map.Buffer[2] = 0xffffff00;

			// Expected to return FALSE because the starting index starts before the clear run
			BOOLEAN bits_clear = RtlAreBitsClear(&map, starting_index, length);

			Assert::IsTrue(FALSE == bits_clear);
		}

		TEST_METHOD(DdkBitMapAreBitsClearSameStartEndIndex)
		{
			ULONG buf[1] = { 0xFF00000F };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);
			ULONG starting_index = 4;
			ULONG length = 20;

			BOOLEAN bits_clear = RtlAreBitsClear(&map, starting_index, length);
			Assert::IsTrue(TRUE == bits_clear);
		}

		TEST_METHOD(DdkBitMapAreBitsClearSameStartEndIndexReturnFalse)
		{
			ULONG buf[1] = { 0xFF00000F };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);
			ULONG starting_index = 12;
			ULONG length = 20;

			BOOLEAN bits_clear = RtlAreBitsClear(&map, starting_index, length);
			Assert::IsTrue(FALSE == bits_clear);
		}

#pragma endregion


#pragma region RtlAreBitsSetTests

		TEST_METHOD(DdkBitMapAreBitsSetNullPtr)
		{
			ULONG starting_index = 8;
			ULONG length = 64;

			TEST_DDK_FAIL(RtlAreBitsSet(NULL, starting_index, length));
		}

		TEST_METHOD(DdkBitMapAreBitsSetStartIndexExceedBufSize)
		{
			ULONG starting_index = bitcount + 1;
			ULONG length = 64;

			Assert::IsTrue(RtlAreBitsSet(&map, starting_index, length) == FALSE);
		}

		TEST_METHOD(DdkBitMapAreBitsSetLengthExceedBufSize)
		{
			ULONG starting_index = 8;
			ULONG length = bitcount + 1;

			Assert::IsTrue(RtlAreBitsSet(&map, starting_index, length) == FALSE);
		}

		TEST_METHOD(DdkBitMapAreBitsSetLengthZero)
		{
			ULONG starting_index = 8;
			ULONG length = 0;

			Assert::IsTrue(RtlAreBitsSet(&map, starting_index, length) == FALSE);
		}

		TEST_METHOD(DdkBitMapAreBitsSetExpectReturnTrue)
		{
			// Initialise buffer to all zeros
			memset(buffer, 0, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			// Set 64 bits in the bit map, starting at the starting index
			// before carrying out the check
			map.Buffer[0] = 0xffffff00;
			map.Buffer[1] = 0x00ffffff;

			ULONG starting_index = 8;
			ULONG length = 48;

			// Expected to return TRUE because starting index is at the start of set run
			BOOLEAN bits_set = RtlAreBitsSet(&map, starting_index, length);

			Assert::IsTrue(TRUE == bits_set);
		}

		TEST_METHOD(DdkBitMapAreBitsSetExpectReturnFalse)
		{
			// Initialise buffer to all zeros
			memset(buffer, 0, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			// Set 64 bits in the bit map, starting at the starting index
			// before carrying out the check
			map.Buffer[0] = 0xffffff00;
			map.Buffer[1] = 0x00ffffff;

			ULONG starting_index = 0;
			ULONG length = 48;

			BOOLEAN bits_set = RtlAreBitsSet(&map, starting_index, length);

			// Expected to return FALSE because starting index is after the start of set run
			Assert::IsTrue(FALSE == bits_set);
		}

		TEST_METHOD(DdkBitMapAreBitsSetSameStartEndIndex)
		{
			ULONG buf[1] = { 0x00FFFFF0 };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);
			ULONG starting_index = 4;
			ULONG length = 20;

			BOOLEAN bits_set = RtlAreBitsSet(&map, starting_index, length);
			Assert::IsTrue(TRUE == bits_set);
		}

		TEST_METHOD(DdkBitMapAreBitsSetSameStartEndIndexReturnFalse)
		{
			ULONG buf[1] = { 0x00FFFFF0 };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);
			ULONG starting_index = 12;
			ULONG length = 20;

			BOOLEAN bits_set = RtlAreBitsSet(&map, starting_index, length);
			Assert::IsTrue(FALSE == bits_set);
		}

#pragma endregion


#pragma region RtlFindClearBitsTests

		TEST_METHOD(DdkBitMapFindClearBitsNullPtr)
		{
			ULONG number_to_find = 32;
			ULONG hint_index = 0;

			TEST_DDK_FAIL(RtlFindClearBits(NULL, number_to_find, hint_index));
		}

		TEST_METHOD(DdkBitMapFindClearBitsNumToFindExceedBufSize)
		{
			ULONG number_to_find = bitcount + 1;
			ULONG hint_index = 0;

			Assert::IsTrue(ALL_BITS_SET == RtlFindClearBits(&map, number_to_find, hint_index));
		}

		TEST_METHOD(DdkBitMapFindClearBitsExpectFindClearBits)
		{
			// Initialise buffer to all ones
			memset(buffer, 0xff, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			int run_index = 28;
			map.Buffer[run_index] = 0x000000ff;
			map.Buffer[run_index + 1] = 0;
			
			ULONG number_to_find = 56;
			ULONG hint_index = (run_index * NUM_BITS_PER_ARRAY_INDEX) + (NUM_BITS_PER_ARRAY_INDEX / 2); // Start in the middle of the run

			ULONG bit_starting_run = RtlFindClearBits(&map, number_to_find, hint_index);

			ULONG expected_answer = (run_index * NUM_BITS_PER_ARRAY_INDEX) + 8;

			Assert::IsTrue(expected_answer == bit_starting_run);
		}

		TEST_METHOD(DdkBitMapFindClearBitsExpectDontFindClearBits)
		{
			// Initialise buffer to all ones
			memset(buffer, 0xff, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			int run_index = 28;

			ULONG number_to_find = 56;
			ULONG hint_index = (run_index * NUM_BITS_PER_ARRAY_INDEX) + (NUM_BITS_PER_ARRAY_INDEX / 2); // Start in the middle of the run

			ULONG bit_starting_run = RtlFindClearBits(&map, number_to_find, hint_index);

			Assert::IsTrue(ALL_BITS_SET == bit_starting_run);
		}

		TEST_METHOD(DdkBitMapFindClearBitsSameStartEndIndex)
		{
			ULONG buf[1] = { 0xFFF000FF };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);
			
			ULONG number_to_find = 12;
			ULONG hint_index = 6;
			ULONG bit_starting_run = RtlFindClearBits(&map, number_to_find, hint_index);
			Assert::IsTrue(8 == bit_starting_run);
		}

		TEST_METHOD(DdkBitMapFindClearBitsSameStartEndIndexBitsNotFound)
		{
			ULONG buf[1] = { ALL_BITS_SET };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			ULONG number_to_find = 12;
			ULONG hint_index = 6;
			ULONG bit_starting_run = RtlFindClearBits(&map, number_to_find, hint_index);
			Assert::IsTrue(ALL_BITS_SET == bit_starting_run);
		}

		TEST_METHOD(DdkBitMapFindClearBitsSameStartEndIndexLoopTwice)
		{
			ULONG buf[1] = { 0xFFF000FF };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			ULONG number_to_find = 12;
			ULONG hint_index = 16;
			ULONG bit_starting_run = RtlFindClearBits(&map, number_to_find, hint_index);
			Assert::IsTrue(8 == bit_starting_run);
		}

		TEST_METHOD(DdkBitMapFindClearBitsTwoClearBitRuns)
		{
			ULONG buf[1] = { 0x0000FF00 };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			ULONG number_to_find = 16;
			ULONG hint_index = 0;
			ULONG bit_starting_run = RtlFindClearBits(&map, number_to_find, hint_index);
			Assert::IsTrue(16 == bit_starting_run);
		}

#pragma endregion


#pragma region RtlFindSetBitsTests

		TEST_METHOD(DdkBitMapFindSetBitsNullPtr)
		{
			ULONG number_to_find = 32;
			ULONG hint_index = 0;

			TEST_DDK_FAIL(RtlFindSetBits(NULL, number_to_find, hint_index));
		}

		TEST_METHOD(DdkBitMapFindSetBitsNumToFindExceedBufSize)
		{
			ULONG number_to_find = bitcount + 1;
			ULONG hint_index = 0;

			Assert::IsTrue(ALL_BITS_SET == RtlFindSetBits(&map, number_to_find, hint_index));
		}

		TEST_METHOD(DdkBitMapFindSetBitsExpectFindSetBits)
		{
			// Initialise buffer to all zeros
			memset(buffer, 0, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			int run_index = 0;
			map.Buffer[run_index] = 0xffffff00;
			map.Buffer[run_index + 1] = 0x000000ff;

			ULONG number_to_find = 32;
			ULONG hint_index = (run_index * NUM_BITS_PER_ARRAY_INDEX) + (NUM_BITS_PER_ARRAY_INDEX / 2); // Start in the middle of the run

			ULONG bit_starting_run = RtlFindSetBits(&map, number_to_find, hint_index);

			ULONG expected_answer = (run_index * NUM_BITS_PER_ARRAY_INDEX) + 8;

			Assert::IsTrue(expected_answer == bit_starting_run);
		}

		TEST_METHOD(DdkBitMapFindSetBitsExpectDontFindSetBits)
		{
			// Initialise buffer to all zeros
			memset(buffer, 0, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			int run_index = 0;

			ULONG number_to_find = 32;
			ULONG hint_index = (run_index * NUM_BITS_PER_ARRAY_INDEX) + (NUM_BITS_PER_ARRAY_INDEX / 2); // Start in the middle of the run

			ULONG bit_starting_run = RtlFindSetBits(&map, number_to_find, hint_index);

			Assert::IsTrue(ALL_BITS_SET == bit_starting_run);
		}

		TEST_METHOD(DdkBitMapFindSetBitsSameStartEndIndex)
		{
			ULONG buf[1] = { 0x000FFF00 };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			ULONG number_to_find = 12;
			ULONG hint_index = 6;
			ULONG bit_starting_run = RtlFindSetBits(&map, number_to_find, hint_index);
			Assert::IsTrue(8 == bit_starting_run);
		}

		TEST_METHOD(DdkBitMapFindSetBitsSameStartEndIndexBitsNotFound)
		{
			ULONG buf[1] = { 0x0 };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			ULONG number_to_find = 12;
			ULONG hint_index = 6;
			ULONG bit_starting_run = RtlFindSetBits(&map, number_to_find, hint_index);
			Assert::IsTrue(ALL_BITS_SET == bit_starting_run);
		}

		TEST_METHOD(DdkBitMapFindSetBitsSameStartEndIndexLoopTwice)
		{
			ULONG buf[1] = { 0x000FFF00 };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			ULONG number_to_find = 12;
			ULONG hint_index = 16;
			ULONG bit_starting_run = RtlFindSetBits(&map, number_to_find, hint_index);
			Assert::IsTrue(8 == bit_starting_run);
		}

		TEST_METHOD(DdkBitMapFindSetBitsTwoSetBitRuns)
		{
			ULONG buf[1] = { 0xFFFF00FF };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			ULONG number_to_find = 16;
			ULONG hint_index = 0;
			ULONG bit_starting_run = RtlFindSetBits(&map, number_to_find, hint_index);
			Assert::IsTrue(16 == bit_starting_run);
		}

#pragma endregion


#pragma region RtlNumberOfSetBitsTests

		TEST_METHOD(DdkBitMapNumberOfSetBitsNullPtr)
		{
			TEST_DDK_FAIL(RtlNumberOfSetBits(NULL));
		}

		TEST_METHOD(DdkBitMapNumberOfSetBitsAreSet)
		{
			// Initialise buffer to all zeros
			memset(buffer, 0x0, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			map.Buffer[0] = 0xffffff00;
			map.Buffer[1] = ALL_BITS_SET;
			map.Buffer[2] = ALL_BITS_SET;

			ULONG number_of_set_bits = RtlNumberOfSetBits(&map);

			Assert::IsTrue(88 == number_of_set_bits);
		}

		TEST_METHOD(DdkBitMapNumberOfSetBitsNotSet)
		{
			// Initialise buffer to all zeros
			memset(buffer, 0x0, sizeof(buffer));
			RtlInitializeBitMap(&map, buffer, bitcount);

			ULONG number_of_set_bits = RtlNumberOfSetBits(&map);

			Assert::IsTrue(0 == number_of_set_bits);
		}

		TEST_METHOD(DdkBitMapNumberOfSetBitsSameStartEndIndex)
		{
			ULONG buf[1] = { 0xFFFF0000 };
			RtlInitializeBitMap(&map, buf, NUM_BITS_PER_ARRAY_INDEX);

			ULONG number_of_set_bits = RtlNumberOfSetBits(&map);
			Assert::IsTrue(16 == number_of_set_bits);
		}

#pragma endregion

#pragma region RtlFindClearBitsAndSetTests

		TEST_METHOD(DdkBitMapFindClearBitsAndSetWithHint)
		{
			const ULONG arrayIndex = 2, hintBitIndex = arrayIndex * NUM_BITS_PER_ARRAY_INDEX;

			memset(buffer, ALL_BITS_SET, sizeof(buffer));

			buffer[arrayIndex] = 0x00000000;

			ULONG startBitIndex = RtlFindClearBitsAndSet(&map, NUM_BITS_PER_ARRAY_INDEX, hintBitIndex);

			Assert::IsTrue(hintBitIndex == startBitIndex);

			for (int i = 0; i < bufcount + 1; i++)
				Assert::IsTrue(ALL_BITS_SET == buffer[i]);
		}

		TEST_METHOD(DdkBitMapFindClearBitsAndSetNoHint)
		{
			const ULONG arrayIndex = 2, hintBitIndex = arrayIndex * NUM_BITS_PER_ARRAY_INDEX;

			memset(buffer, ALL_BITS_SET, sizeof(buffer));

			buffer[arrayIndex] = 0x00000000;

			ULONG startBitIndex = RtlFindClearBitsAndSet(&map, NUM_BITS_PER_ARRAY_INDEX, 0);

			Assert::IsTrue(hintBitIndex == startBitIndex);

			for (int i = 0; i < bufcount + 1; i++)
				Assert::IsTrue(ALL_BITS_SET == buffer[i]);
		}

		TEST_METHOD(DdkBitMapFindClearBitsAndSetOddBit)
		{
			const ULONG arrayIndex = 2, hintBitIndex = arrayIndex * NUM_BITS_PER_ARRAY_INDEX;

			memset(buffer, ALL_BITS_SET, sizeof(buffer));
			buffer[2] = 0x00100000;

			ULONG startBitIndex = RtlFindClearBitsAndSet(&map, NUM_BITS_PER_ARRAY_INDEX, hintBitIndex);

			Assert::IsTrue(ALL_BITS_SET == startBitIndex);

			for (int i = 0; i < bufcount + 1; i++)
				if (i == arrayIndex)
					Assert::IsTrue(0x00100000 == buffer[i]);
				else
					Assert::IsTrue(ALL_BITS_SET == buffer[i]);
		}

		TEST_METHOD(DdkBitMapFindClearBitsAndSetHalfRun)
		{
			const ULONG arrayIndex = 2, hintBitIndex = arrayIndex * NUM_BITS_PER_ARRAY_INDEX;

			memset(buffer, ALL_BITS_SET, sizeof(buffer));

			buffer[arrayIndex] = buffer[arrayIndex + 1] = 0x00000000;

			ULONG startBitIndex = RtlFindClearBitsAndSet(&map, NUM_BITS_PER_ARRAY_INDEX, hintBitIndex);

			Assert::IsTrue(hintBitIndex == startBitIndex);

			for (int i = 0; i < bufcount + 1; i++)
				if (i == arrayIndex + 1)
					Assert::IsTrue(0x00000000 == buffer[i]);
				else
					Assert::IsTrue(ALL_BITS_SET == buffer[i]);
			
		}

		TEST_METHOD(DdkBitMapFindClearBitsAndSetEndToEnd)
		{
			const ULONG firstElement = 0xFFFFFF00, lastElement = 0x000000FF;

			memset(&buffer[1], ALL_BITS_SET, sizeof(buffer) - (2 * sizeof(buffer[0])));

			buffer[0] = firstElement;
			buffer[bufcount] = lastElement;

			ULONG startBitIndex = RtlFindClearBitsAndSet(&map, NUM_BITS_PER_ARRAY_INDEX, 0);

			Assert::IsTrue(ALL_BITS_SET == startBitIndex);

			for (int i = 1; i < bufcount; i++)
				Assert::IsTrue(ALL_BITS_SET == buffer[i]);

			Assert::IsTrue(buffer[0] == firstElement);
			Assert::IsTrue(buffer[bufcount] == lastElement);
		}

		TEST_METHOD(DdkBitMapFindClearBitsAndSetSecondGroup)
		{
			const ULONG arrayIndex = bufcount, hintBitIndex = arrayIndex * NUM_BITS_PER_ARRAY_INDEX;

			memset(buffer, ALL_BITS_SET, sizeof(buffer));

			buffer[0] = buffer[arrayIndex] = 0x00000000;

			ULONG startBitIndex = RtlFindClearBitsAndSet(&map, 8, hintBitIndex);

			Assert::IsTrue(hintBitIndex == startBitIndex);

			for (int i = 1; i < bufcount; i++)
				Assert::IsTrue(ALL_BITS_SET == buffer[i]);
			
			Assert::IsTrue(0x00000000 == buffer[0]);
		}

		TEST_METHOD(DdkBitMapFindClearBitsAndSetSpanningTwoArrayElements)
		{
			const ULONG firstElement = 0x000000FF, lastElement = 0xFFFFFF00;
			const ULONG arrayIndex = 2, hintBitIndex = (arrayIndex * NUM_BITS_PER_ARRAY_INDEX) + 8;

			memset(buffer, ALL_BITS_SET, sizeof(buffer));

			buffer[arrayIndex] = firstElement;
			buffer[arrayIndex + 1] = lastElement;

			ULONG startBitIndex = RtlFindClearBitsAndSet(&map, NUM_BITS_PER_ARRAY_INDEX, hintBitIndex);

			Assert::IsTrue(hintBitIndex == startBitIndex);

			for (int i = 1; i < bufcount; i++)
				Assert::IsTrue(ALL_BITS_SET == buffer[i]);
		}

		TEST_METHOD(DdkBitMapFindClearBitsAndSetSpanningTwoArrayElementsIncorrectHint)
		{
			const ULONG firstElement = 0x000000FF, lastElement = 0xFFFFFF00;

			const ULONG arrayIndex = 2;
			const ULONG runStartBit = (arrayIndex * NUM_BITS_PER_ARRAY_INDEX) + 8, hintBitIndex = runStartBit + 1;

			memset(buffer, ALL_BITS_SET, sizeof(buffer));

			buffer[arrayIndex] = firstElement;
			buffer[arrayIndex + 1] = lastElement;

			ULONG startBitIndex = RtlFindClearBitsAndSet(&map, NUM_BITS_PER_ARRAY_INDEX, hintBitIndex);

			Assert::IsTrue(runStartBit == startBitIndex);

			for (int i = 1; i < bufcount; i++)
				Assert::IsTrue(ALL_BITS_SET == buffer[i]);
		}

#pragma endregion

#pragma region RtlFindSetBitsAndClearTests

		TEST_METHOD(DdkBitMapFindSetBitsAndClearWithHint)
		{
			const ULONG arrayIndex = 2, hintBitIndex = arrayIndex * NUM_BITS_PER_ARRAY_INDEX;

			buffer[arrayIndex] = ALL_BITS_SET;

			ULONG startBitIndex = RtlFindSetBitsAndClear(&map, NUM_BITS_PER_ARRAY_INDEX, hintBitIndex);

			Assert::IsTrue(hintBitIndex == startBitIndex);

			for (int i = 0; i < bufcount + 1; i++)
				Assert::IsTrue(0x00000000 == buffer[i]);
		}

		TEST_METHOD(DdkBitMapFindSetBitsAndClearNoHint)
		{
			const ULONG arrayIndex = 2, hintBitIndex = arrayIndex * NUM_BITS_PER_ARRAY_INDEX;

			buffer[arrayIndex] = ALL_BITS_SET;

			ULONG startBitIndex = RtlFindSetBitsAndClear(&map, NUM_BITS_PER_ARRAY_INDEX, 0);

			Assert::IsTrue(hintBitIndex == startBitIndex);

			for (int i = 0; i < bufcount + 1; i++)
				Assert::IsTrue(0x00000000 == buffer[i]);
		}

		TEST_METHOD(DdkBitMapFindSetBitsAndClearOddBit)
		{
			const ULONG arrayIndex = 2, hintBitIndex = arrayIndex * NUM_BITS_PER_ARRAY_INDEX;
			const ULONG brokenSetBit = 0xFFF0FFFF;

			buffer[arrayIndex] = brokenSetBit;

			ULONG startBitIndex = RtlFindSetBitsAndClear(&map, NUM_BITS_PER_ARRAY_INDEX, hintBitIndex);

			Assert::IsTrue(ALL_BITS_SET == startBitIndex);

			for (int i = 0; i < bufcount + 1; i++)
				if (i == arrayIndex)
					Assert::IsTrue(brokenSetBit == buffer[i]);
				else
					Assert::IsTrue(0x00000000 == buffer[i]);			
		}

		TEST_METHOD(DdkBitMapFindSetBitsAndClearHalfRun)
		{
			const ULONG arrayIndex = 2, hintBitIndex = arrayIndex * NUM_BITS_PER_ARRAY_INDEX;

			buffer[arrayIndex] = buffer[arrayIndex + 1] = ALL_BITS_SET;

			ULONG startBitIndex = RtlFindSetBitsAndClear(&map, NUM_BITS_PER_ARRAY_INDEX, hintBitIndex);

			Assert::IsTrue(hintBitIndex == startBitIndex);

			for (int i = 0; i < bufcount + 1; i++)
				if (i == arrayIndex + 1)
					Assert::IsTrue(ALL_BITS_SET == buffer[i]);
				else
					Assert::IsTrue(0x00000000 == buffer[i]);

		}
		
		TEST_METHOD(DdkBitMapFindSetBitsAndClearEndToEnd)
		{
			const ULONG firstElement = 0x000000FF, lastElement = 0xFFFFFF00;

			buffer[0] = firstElement;
			buffer[bufcount] = lastElement;

			ULONG startBitIndex = RtlFindSetBitsAndClear(&map, NUM_BITS_PER_ARRAY_INDEX, 0);

			Assert::IsTrue(ALL_BITS_SET == startBitIndex);

			for (int i = 1; i < bufcount; i++)
				Assert::IsTrue(0x00000000 == buffer[i]);

			Assert::IsTrue(firstElement == buffer[0]);
			Assert::IsTrue(lastElement == buffer[bufcount]);
		}

		TEST_METHOD(DdkBitMapFindSetBitsAndClearSecondGroup)
		{
			const ULONG arrayIndex = bufcount, hintBitIndex = arrayIndex * NUM_BITS_PER_ARRAY_INDEX;

			buffer[0] = buffer[arrayIndex] = ALL_BITS_SET;

			ULONG startBitIndex = RtlFindSetBitsAndClear(&map, 8, hintBitIndex);

			Assert::IsTrue(bufcount * NUM_BITS_PER_ARRAY_INDEX == startBitIndex);

			for (int i = 1; i < bufcount; i++)
				Assert::IsTrue(0x00000000 == buffer[i]);

			Assert::IsTrue(ALL_BITS_SET == buffer[0]);
		}

		TEST_METHOD(DdkBitMapFindSetBitsAndClearSpanningTwoArrayElements)
		{
			const ULONG firstElement = 0xFFFFFF00, lastElement = 0x000000FF;
			const ULONG arrayIndex = 2, hintBitIndex = (arrayIndex * NUM_BITS_PER_ARRAY_INDEX) + 8;
			
			buffer[arrayIndex] = firstElement;
			buffer[arrayIndex + 1] = lastElement;

			ULONG startBitIndex = RtlFindSetBitsAndClear(&map, NUM_BITS_PER_ARRAY_INDEX, hintBitIndex);

			Assert::IsTrue(hintBitIndex == startBitIndex);

			for (int i = 1; i < bufcount; i++)
				Assert::IsTrue(0x00000000 == buffer[i]);
		}

		TEST_METHOD(DdkBitMapFindSetBitsAndClearSpanningTwoArrayElementsIncorrectHint)
		{
			const ULONG firstElement = 0xFFFFFF00, lastElement = 0x000000FF;

			const ULONG arrayIndex = 2;
			const ULONG runStartBit = (arrayIndex * NUM_BITS_PER_ARRAY_INDEX) + 8, hintBitIndex = runStartBit + 1;

			buffer[arrayIndex] = firstElement;
			buffer[arrayIndex + 1] = lastElement;

			ULONG startBitIndex = RtlFindSetBitsAndClear(&map, NUM_BITS_PER_ARRAY_INDEX, hintBitIndex);

			Assert::IsTrue(runStartBit == startBitIndex);

			for (int i = 1; i < bufcount; i++)
				Assert::IsTrue(0x00000000 == buffer[i]);
		}

#pragma endregion

	};
}
