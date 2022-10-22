/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2017, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	Bitmap Routines
 */

#include "stdddk.h"


static const int BITS_PER_ARRAY = (sizeof(ULONG)* 8);
static const int ALL_BITS_SET = 0xFFFFFFFF;


// Forward declarations
ULONG CalculateArrayIndexFromBitNum(ULONG bitNumber);
ULONG CalculateBitPositionInArrayElement(ULONG bitNumber, ULONG arrayIndex);
ULONG CalculateLastBufferIndex(ULONG lastBitNum);
VOID ClearArrayElements(PRTL_BITMAP bufferElement, ULONG startingIndex, ULONG endIndex);
VOID ClearBitsInRange(ULONG& bufferElement, ULONG startingIndex, ULONG endIndex);
VOID SetArrayElements(PRTL_BITMAP bufferElement, ULONG startingIndex, ULONG endIndex);
VOID SetBitsInRange(ULONG& bufferElement, ULONG startingIndex, ULONG endIndex);
ULONG CalculateBitNumberInBitMap(ULONG bufferIndex, ULONG bitPosition);
VOID CountClearRun(ULONG& bufferElement, ULONG bufferIndex, ULONG startBitPosition,
	ULONG lastBitPosition, bool& runStarted, bool& runEnded, ULONG& startBit, ULONG& runCount);
bool NumClearBitsFound(ULONG& bufferElement, ULONG bufferIndex, ULONG numToFind,
	ULONG startBitPosition, ULONG endBitPosition, bool& runStarted, ULONG& startBit, ULONG& runCount);
bool NumSetBitsFound(ULONG& bufferElement, ULONG bufferIndex, ULONG numToFind,
	ULONG startBitPosition, ULONG endBitPosition, bool& runStarted, ULONG& startBit, ULONG& runCount);

DDKAPI
VOID RtlInitializeBitMap(PRTL_BITMAP BitMapHeader, PULONG BitMapBuffer, ULONG SizeOfBitMap)
{
	BitMapHeader->Buffer = BitMapBuffer;
	BitMapHeader->SizeOfBitMap = SizeOfBitMap;
}


DDKAPI
VOID RtlClearBit(PRTL_BITMAP BitMapHeader, ULONG BitNumber)
{
	DDKASSERT(NULL != BitMapHeader);
	DDKASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

	ULONG array_index = CalculateArrayIndexFromBitNum(BitNumber);
	ClearBitsInRange(BitMapHeader->Buffer[array_index], BitNumber, BitNumber);
}


DDKAPI
VOID RtlSetBit(PRTL_BITMAP BitMapHeader, ULONG BitNumber)
{
	DDKASSERT(NULL != BitMapHeader);
	DDKASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

	ULONG array_index = CalculateArrayIndexFromBitNum(BitNumber);
	SetBitsInRange(BitMapHeader->Buffer[array_index], BitNumber, BitNumber);
}

DDKAPI
BOOLEAN RtlTestBit(PRTL_BITMAP BitMapHeader, ULONG BitNumber)
{
	DDKASSERT(NULL != BitMapHeader);
	DDKASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

	ULONG array_index = CalculateArrayIndexFromBitNum(BitNumber);

	return BitMapHeader->Buffer[array_index] & (1 << BitNumber) ? true : false;
}


DDKAPI
VOID RtlClearAllBits(PRTL_BITMAP BitMapHeader)
{
	DDKASSERT(NULL != BitMapHeader);

	const ULONG last_buffer_index = CalculateLastBufferIndex(BitMapHeader->SizeOfBitMap - 1);
	const ULONG last_bit_number = CalculateBitPositionInArrayElement((BitMapHeader->SizeOfBitMap - 1), last_buffer_index);

	// Zero all buffer elements up to the last element if
	// there is greater than one element
	if (last_buffer_index > 0)
		ClearArrayElements(BitMapHeader, 0, (last_buffer_index - 1));

	// Clear all the bits in the last buffer element.
	// This may not be 2^n boundary aligned.
	ClearBitsInRange(BitMapHeader->Buffer[last_buffer_index], 0, last_bit_number);
}


DDKAPI
VOID RtlSetAllBits(PRTL_BITMAP BitMapHeader)
{
	DDKASSERT(NULL != BitMapHeader);

	const ULONG last_buffer_index = CalculateLastBufferIndex(BitMapHeader->SizeOfBitMap - 1);
	const ULONG last_bit_number = CalculateBitPositionInArrayElement((BitMapHeader->SizeOfBitMap - 1), last_buffer_index);

	// Set the bits in all buffer elements up to the last element if
	// there is greater than one element
	if (last_buffer_index > 0)
		SetArrayElements(BitMapHeader, 0, (last_buffer_index - 1));

	// Set all the bits in the last buffer element.
	// This may not be 2^n boundary aligned.
	SetBitsInRange(BitMapHeader->Buffer[last_buffer_index], 0, last_bit_number);
}


DDKAPI
VOID RtlClearBits(PRTL_BITMAP BitMapHeader, ULONG StartingIndex, ULONG NumberToClear)
{
	DDKASSERT(NULL != BitMapHeader);
	DDKASSERT((StartingIndex + NumberToClear) <= BitMapHeader->SizeOfBitMap);

	if (0 != NumberToClear)
	{
		const ULONG start_buffer_index = CalculateArrayIndexFromBitNum(StartingIndex);
		const ULONG starting_bit_position = CalculateBitPositionInArrayElement(StartingIndex, start_buffer_index);

		const ULONG last_bit = (StartingIndex + NumberToClear) - 1;
		const ULONG last_buffer_index = CalculateLastBufferIndex(last_bit);
		const ULONG last_bit_position = CalculateBitPositionInArrayElement(last_bit, last_buffer_index);

		if (start_buffer_index == last_buffer_index)
		{
			// These may not be 2^n boundary aligned.
			ClearBitsInRange(BitMapHeader->Buffer[last_buffer_index], starting_bit_position, last_bit_position);
		}
		else
		{
			// Clear the bits in the first buffer element.
			// These may not be 2^n boundary aligned.
			ClearBitsInRange(BitMapHeader->Buffer[start_buffer_index], starting_bit_position, (BITS_PER_ARRAY - 1));

			// Clear all the buffer elements between the start buffer and end buffer.
			ClearArrayElements(BitMapHeader, (start_buffer_index + 1), (last_buffer_index - 1));

			// Clear the bits in the last buffer element.
			// These may not be 2^n boundary aligned.
			ClearBitsInRange(BitMapHeader->Buffer[last_buffer_index], 0, last_bit_position);
		}
	}
}


DDKAPI
VOID RtlSetBits(PRTL_BITMAP BitMapHeader, ULONG StartingIndex, ULONG NumberToSet)
{
	DDKASSERT(NULL != BitMapHeader);
	DDKASSERT((StartingIndex + NumberToSet) <= BitMapHeader->SizeOfBitMap);

	if (0 != NumberToSet)
	{
		const ULONG start_buffer_index = CalculateArrayIndexFromBitNum(StartingIndex);
		const ULONG starting_bit_position = CalculateBitPositionInArrayElement(StartingIndex, start_buffer_index);

		const ULONG last_bit = (StartingIndex + NumberToSet) - 1;
		const ULONG last_buffer_index = CalculateLastBufferIndex(last_bit);
		const ULONG last_bit_position = CalculateBitPositionInArrayElement(last_bit, last_buffer_index);

		if (start_buffer_index == last_buffer_index)
		{
			// These may not be 2^n boundary aligned.
			SetBitsInRange(BitMapHeader->Buffer[last_buffer_index], starting_bit_position, last_bit_position);
		}
		else
		{
			// Set the bits in the first buffer element.
			// These may not be 2^n boundary aligned.
			SetBitsInRange(BitMapHeader->Buffer[start_buffer_index], starting_bit_position, (BITS_PER_ARRAY - 1));

			// Set all the buffer elements between the start buffer and last buffer.
			SetArrayElements(BitMapHeader, (start_buffer_index + 1), (last_buffer_index - 1));

			// Set the bits in the last buffer element.
			// These may not be 2^n boundary aligned.
			SetBitsInRange(BitMapHeader->Buffer[last_buffer_index], 0, last_bit_position);
		}
	}
}


DDKAPI
ULONG RtlFindNextForwardRunClear(PRTL_BITMAP BitMapHeader, ULONG FromIndex, PULONG StartingRunIndex)
{
	DDKASSERT(NULL != BitMapHeader);
	DDKASSERT(FromIndex < BitMapHeader->SizeOfBitMap);
	DDKASSERT(NULL != StartingRunIndex);

	ULONG run_count = 0;

	const ULONG start_buffer_index = CalculateArrayIndexFromBitNum(FromIndex);
	const ULONG start_bit_position = CalculateBitPositionInArrayElement(FromIndex, start_buffer_index);

	const ULONG last_buffer_index = CalculateLastBufferIndex(BitMapHeader->SizeOfBitMap - 1);
	const ULONG last_bit_position = CalculateBitPositionInArrayElement((BitMapHeader->SizeOfBitMap - 1), last_buffer_index);

	ULONG run_start_bit = 0;
	bool run_started = FALSE;
	bool run_ended = FALSE;

	if (start_buffer_index == last_buffer_index)
	{
		CountClearRun(BitMapHeader->Buffer[start_buffer_index], start_buffer_index,
			start_bit_position, last_bit_position, run_started, run_ended, run_start_bit, run_count);
        *StartingRunIndex = run_start_bit;
		return run_count;
	}

	// Search the first buffer element
	CountClearRun(BitMapHeader->Buffer[start_buffer_index], start_buffer_index,
		start_bit_position, (BITS_PER_ARRAY - 1), run_started, run_ended, run_start_bit, run_count);

	if (run_ended)
	{
		*StartingRunIndex = run_start_bit;
		return run_count;
	}

	// Search buffer elements between the start and last buffers if the run
	// has not already ended.
	for (ULONG buffer_index = (start_buffer_index + 1); buffer_index < last_buffer_index; buffer_index++)
	{
		CountClearRun(BitMapHeader->Buffer[buffer_index], buffer_index, 0,
			(BITS_PER_ARRAY - 1), run_started, run_ended, run_start_bit, run_count);

		if (run_ended)
		{
			*StartingRunIndex = run_start_bit;
			return run_count;
		}
	}

	// Search the last buffer element
	CountClearRun(BitMapHeader->Buffer[last_buffer_index], last_buffer_index,
		0, last_bit_position, run_started, run_ended, run_start_bit, run_count);
	
	*StartingRunIndex = run_start_bit;
	return run_count;
}

DDKAPI
ULONG RtlFindFirstRunClear(PRTL_BITMAP BitMapHeader, PULONG StartingRunIndex) {
	return RtlFindNextForwardRunClear(BitMapHeader, 0, StartingRunIndex);
}

DDKAPI
BOOLEAN RtlAreBitsClear(PRTL_BITMAP BitMapHeader, ULONG StartingIndex, ULONG Length)
{
	DDKASSERT(NULL != BitMapHeader);

	if ((0 == Length) || (BitMapHeader->SizeOfBitMap < (StartingIndex + Length)))
	{
		return FALSE;
	}
	else
	{
		const ULONG start_buffer_index = CalculateArrayIndexFromBitNum(StartingIndex);
		const ULONG start_bit_number = CalculateBitPositionInArrayElement(StartingIndex, start_buffer_index);

		const ULONG last_buffer_index = CalculateArrayIndexFromBitNum((StartingIndex + Length - 1));
		const ULONG last_bit_number = CalculateBitPositionInArrayElement((StartingIndex + Length - 1), last_buffer_index);

		ULONG bits_checked = 0;

		if (start_buffer_index == last_buffer_index)
		{
			for (ULONG bit_num = start_bit_number; bit_num <= last_bit_number; bit_num++)
			{
				if (Length > bits_checked)
				{
					ULONG bit_set = BitMapHeader->Buffer[last_buffer_index] & (1 << bit_num);
					if (bit_set)
					{
						return FALSE;
					}

					bits_checked++;
				}
			}
			return TRUE;
		}

		// Search the start buffer element.
		for (ULONG bit_num = start_bit_number; bit_num < BITS_PER_ARRAY; bit_num++)
		{
			if (Length > bits_checked)
			{
				ULONG bit_set = BitMapHeader->Buffer[start_buffer_index] & (1 << bit_num);
				if (bit_set)
				{
					return FALSE;
				}

				bits_checked++;
			}
			else
			{
				return TRUE;
			}
		}

		// Search the buffer elements between start and last buffers.
		for (ULONG buffer_num = (start_buffer_index + 1); buffer_num < last_buffer_index; buffer_num++)
		{
			if (Length > bits_checked)
			{
				if (0 != BitMapHeader->Buffer[buffer_num])
				{
					return FALSE;
				}
				bits_checked += BITS_PER_ARRAY;
			}
			else
			{
				return TRUE;
			}
		}

		// Search the last buffer element.
		for (ULONG bit_num = 0; bit_num <= last_bit_number; bit_num++)
		{
			if (Length > bits_checked)
			{
				ULONG bit_set = BitMapHeader->Buffer[last_buffer_index] & (1 << bit_num);
				if (bit_set)
				{
					return FALSE;
				}

				bits_checked++;
			}
		}
	}

	return TRUE;
}


DDKAPI
BOOLEAN RtlAreBitsSet(PRTL_BITMAP BitMapHeader, ULONG StartingIndex, ULONG Length)
{
	DDKASSERT(NULL != BitMapHeader);

	if ((0 == Length) || (BitMapHeader->SizeOfBitMap < (StartingIndex + Length)))
	{
		return FALSE;
	}
	else
	{
		const ULONG start_buffer_index = CalculateArrayIndexFromBitNum(StartingIndex);
		const ULONG start_bit_number = CalculateBitPositionInArrayElement(StartingIndex, start_buffer_index);

		const ULONG last_buffer_index = CalculateArrayIndexFromBitNum((StartingIndex + Length - 1));
		const ULONG last_bit_number = CalculateBitPositionInArrayElement((StartingIndex + Length - 1), last_buffer_index);

		ULONG bits_checked = 0;

		if (start_buffer_index == last_buffer_index)
		{
			for (ULONG bit_num = start_bit_number; bit_num <= last_bit_number; bit_num++)
			{
				if (Length > bits_checked)
				{
					ULONG bit_set = BitMapHeader->Buffer[last_buffer_index] & (1 << bit_num);
					if (!bit_set)
					{
						return FALSE;
					}

					bits_checked++;
				}
			}
			return TRUE;
		}

		// Search the start buffer element.
		for (ULONG bit_num = start_bit_number; bit_num < BITS_PER_ARRAY; bit_num++)
		{
			if (Length > bits_checked)
			{
				ULONG bit_set = BitMapHeader->Buffer[start_buffer_index] & (1 << bit_num);
				if (!bit_set)
				{
					return FALSE;
				}

				bits_checked++;
			}
			else
			{
				return TRUE;
			}
		}

		// Search the buffer elements between start and last buffers.
		for (ULONG buffer_num = (start_buffer_index + 1); buffer_num < last_buffer_index; buffer_num++)
		{
			if (Length > bits_checked)
			{
				if (ALL_BITS_SET != BitMapHeader->Buffer[buffer_num])
				{
					return FALSE;
				}
				bits_checked += BITS_PER_ARRAY;
			}
			else
			{
				return TRUE;
			}
		}

		// Search the last buffer element.
		for (ULONG bit_num = 0; bit_num <= last_bit_number; bit_num++)
		{
			if (Length > bits_checked)
			{
				ULONG bit_set = BitMapHeader->Buffer[last_buffer_index] & (1 << bit_num);
				if (!bit_set)
				{
					return FALSE;
				}

				bits_checked++;
			}
		}
	}

	return TRUE;
}


DDKAPI
ULONG RtlFindClearBits(PRTL_BITMAP BitMapHeader, ULONG NumberToFind, ULONG HintIndex)
{
	DDKASSERT(NULL != BitMapHeader);

	if (NumberToFind < BitMapHeader->SizeOfBitMap)
	{
		ULONG start_buffer_index = CalculateArrayIndexFromBitNum(HintIndex);
		ULONG start_bit_position = CalculateBitPositionInArrayElement(HintIndex, start_buffer_index);

		const ULONG last_buffer_index = CalculateLastBufferIndex(BitMapHeader->SizeOfBitMap - 1);
		const ULONG last_bit_position = CalculateBitPositionInArrayElement((BitMapHeader->SizeOfBitMap - 1), last_buffer_index);

		ULONG num_bits_to_check = BitMapHeader->SizeOfBitMap - HintIndex;

		bool run_started = FALSE;
		int loop_count = 2;

		ULONG start_bit = ALL_BITS_SET;
		ULONG run_count = 0;

		bool num_clear_bits_found = FALSE;

		// If the number of clear bits to find is greater than the number of bits between
		// HintIndex and the end of the bit map OR HintIndex is greater than the size of the bit map
		// start looking from the start of the bit map.
		if ((NumberToFind > num_bits_to_check) || (BitMapHeader->SizeOfBitMap <= HintIndex))
		{
			start_buffer_index = 0;
			start_bit_position = 0;
			loop_count--;
		}

		while (0 != loop_count)
		{
			if (start_buffer_index == last_buffer_index)
			{
				num_clear_bits_found = NumClearBitsFound(BitMapHeader->Buffer[start_buffer_index],
					start_buffer_index, NumberToFind, start_bit_position, last_bit_position, run_started, start_bit, run_count);
				if (num_clear_bits_found)
				{
					return start_bit;
				}
			}
			else
			{
				// Search through the start buffer element.
				num_clear_bits_found = NumClearBitsFound(BitMapHeader->Buffer[start_buffer_index],
					start_buffer_index, NumberToFind, start_bit_position, (BITS_PER_ARRAY - 1), run_started, start_bit, run_count);
				if (num_clear_bits_found)
				{
					return start_bit;
				}

				// Check all the buffer elements between start and last.
				if (!num_clear_bits_found)
				{
					for (ULONG buffer_index = (start_buffer_index + 1); buffer_index < last_buffer_index; buffer_index++)
					{
						num_clear_bits_found = NumClearBitsFound(BitMapHeader->Buffer[buffer_index],
							buffer_index, NumberToFind, 0, (BITS_PER_ARRAY - 1), run_started, start_bit, run_count);
						if (num_clear_bits_found)
						{
							return start_bit;
						}
					}
				}

				// Search through the last buffer
				if (!num_clear_bits_found)
				{
					num_clear_bits_found = NumClearBitsFound(BitMapHeader->Buffer[last_buffer_index],
						last_buffer_index, NumberToFind, 0, last_bit_position, run_started, start_bit, run_count);
					if (num_clear_bits_found)
					{
						return start_bit;
					}
				}
			}

			// Reset variables before going back around the loop.
			if (!num_clear_bits_found)
			{
				start_buffer_index = 0;
				start_bit_position = 0;
				run_started = FALSE;
				run_count = 0;
				loop_count--;
			}
			else
			{
				loop_count = 0;
			}
		}
	}

	return ALL_BITS_SET;
}


DDKAPI
ULONG RtlFindSetBits(PRTL_BITMAP BitMapHeader, ULONG NumberToFind, ULONG HintIndex)
{
	DDKASSERT(NULL != BitMapHeader);

	if (NumberToFind < BitMapHeader->SizeOfBitMap)
	{
		ULONG start_buffer_index = CalculateArrayIndexFromBitNum(HintIndex);
		ULONG start_bit_position = CalculateBitPositionInArrayElement(HintIndex, start_buffer_index);

		const ULONG last_buffer_index = CalculateLastBufferIndex(BitMapHeader->SizeOfBitMap - 1);
		const ULONG last_bit_position = CalculateBitPositionInArrayElement((BitMapHeader->SizeOfBitMap - 1), last_buffer_index);

		ULONG num_bits_to_check = BitMapHeader->SizeOfBitMap - HintIndex;

		bool run_started = FALSE;
		int loop_count = 2;

		ULONG start_bit = ALL_BITS_SET;
		ULONG run_count = 0;

		bool num_set_bits_found = FALSE;

		// If the number of set bits to find is greater than the number of bits between
		// HintIndex and the end of the bit map OR HintIndex is greater than the size of the bit map,
		// start looking from the start of the bit map.
		if ((NumberToFind > num_bits_to_check) || (BitMapHeader->SizeOfBitMap <= HintIndex))
		{
			start_buffer_index = 0;
			start_bit_position = 0;
			loop_count--;
		}

		while (0 != loop_count)
		{
			if (start_buffer_index == last_buffer_index)
			{
				num_set_bits_found = NumSetBitsFound(BitMapHeader->Buffer[start_buffer_index], 
					start_buffer_index, NumberToFind, start_bit_position, last_bit_position, run_started, start_bit, run_count);
				if (num_set_bits_found)
				{
					return start_bit;
				}
			}
			else
			{
				// Search through the start buffer element.
				num_set_bits_found = NumSetBitsFound(BitMapHeader->Buffer[start_buffer_index], start_buffer_index, NumberToFind, start_bit_position, (BITS_PER_ARRAY - 1), run_started, start_bit, run_count);
				if (num_set_bits_found)
				{
					return start_bit;
				}

				// Check all the buffer elements between start and last.
				if (!num_set_bits_found)
				{
					for (ULONG buffer_index = (start_buffer_index + 1); buffer_index < last_buffer_index; buffer_index++)
					{
						num_set_bits_found = NumSetBitsFound(BitMapHeader->Buffer[buffer_index], buffer_index, NumberToFind, 0, (BITS_PER_ARRAY - 1), run_started, start_bit, run_count);
						if (num_set_bits_found)
						{
							return start_bit;
							break;
						}
					}
				}

				// Search through the last buffer
				if (!num_set_bits_found)
				{
					num_set_bits_found = NumSetBitsFound(BitMapHeader->Buffer[last_buffer_index], last_buffer_index, NumberToFind, 0, last_bit_position, run_started, start_bit, run_count);
					if (num_set_bits_found)
					{
						return start_bit;
					}
				}
			}

			// Reset variables before going back around the loop.
			if (!num_set_bits_found)
			{
				start_buffer_index = 0;
				start_bit_position = 0;
				run_started = FALSE;
				run_count = 0;
				loop_count--;
			}
			else
			{
				loop_count = 0;
			}
		}
	}

	return ALL_BITS_SET;
}


DDKAPI
ULONG RtlNumberOfSetBits(PRTL_BITMAP BitMapHeader)
{
	DDKASSERT(NULL != BitMapHeader);

	ULONG number_of_set_bits = 0;
	const ULONG last_buffer_index = CalculateLastBufferIndex(BitMapHeader->SizeOfBitMap - 1);
	const ULONG last_bit_position = CalculateBitPositionInArrayElement((BitMapHeader->SizeOfBitMap - 1), last_buffer_index);

	// Check all buffer elements up to the last element
	for (ULONG buffer_index = 0; buffer_index < last_buffer_index; buffer_index++)
	{
		if (ALL_BITS_SET == BitMapHeader->Buffer[buffer_index])
		{
			number_of_set_bits += BITS_PER_ARRAY;
		}
		else
		{
			for (ULONG bit_num = 0; bit_num < BITS_PER_ARRAY; bit_num++)
			{
				ULONG bit_set = BitMapHeader->Buffer[buffer_index] & (1 << bit_num);
				if (bit_set)
				{
					number_of_set_bits++;
				}
			}
		}
	}

	// Check last buffer element
	for (ULONG bit_num = 0; bit_num <= last_bit_position; bit_num++)
	{
		ULONG bit_set = BitMapHeader->Buffer[last_buffer_index] & (1 << bit_num);
		if (bit_set)
		{
			number_of_set_bits++;
		}
	}

	return number_of_set_bits;
}


DDKAPI
ULONG RtlFindClearBitsAndSet(PRTL_BITMAP BitMapHeader, ULONG NumberToFind, ULONG HintIndex)
{
	ULONG startBitIndex = RtlFindClearBits(BitMapHeader, NumberToFind, HintIndex);

	if (startBitIndex != ALL_BITS_SET)
		RtlSetBits(BitMapHeader, startBitIndex, NumberToFind);

	return startBitIndex;
}


DDKAPI
ULONG RtlFindSetBitsAndClear(PRTL_BITMAP BitMapHeader, ULONG NumberToFind, ULONG HintIndex)
{
	ULONG startBitIndex = RtlFindSetBits(BitMapHeader, NumberToFind, HintIndex);

	if (startBitIndex != ALL_BITS_SET)
		RtlClearBits(BitMapHeader, startBitIndex, NumberToFind);

	return startBitIndex;
}

/*
 *	Start of commonly used functions 
 */

// Calculate the array index from the given bit number.
ULONG CalculateArrayIndexFromBitNum(ULONG bitNumber)
{
	ULONG array_index = bitNumber / BITS_PER_ARRAY;
	return(array_index);
}

// Calculates the bit position within the given array element from the overall bit number
ULONG CalculateBitPositionInArrayElement(ULONG bitNumber, ULONG arrayIndex)
{
	ULONG bit_position = bitNumber - (arrayIndex * BITS_PER_ARRAY);
	return(bit_position);
}

// Calculates the last buffer index from the number of bits in the buffer
ULONG CalculateLastBufferIndex(ULONG lastBitNum)
{
	ULONG last_buffer_index = lastBitNum / BITS_PER_ARRAY;
	return(last_buffer_index);
}

// Clear array elements
VOID ClearArrayElements(PRTL_BITMAP bufferElement, ULONG startingIndex, ULONG endIndex)
{
	for (ULONG buffer_index = startingIndex; buffer_index <= endIndex; buffer_index++)
	{
		bufferElement->Buffer[buffer_index] = 0;
	}
}

// Clears all the bits in range to a 0.
VOID ClearBitsInRange(ULONG& bufferElement, ULONG startingIndex, ULONG endIndex)
{
	for (ULONG bit_num = startingIndex; bit_num <= endIndex; bit_num++)
	{
		bufferElement &= ~(1 << bit_num);
	}
}

// Set array elements
VOID SetArrayElements(PRTL_BITMAP bufferElement, ULONG startingIndex, ULONG endIndex)
{
	for (ULONG buffer_index = startingIndex; buffer_index <= endIndex; buffer_index++)
	{
		bufferElement->Buffer[buffer_index] = ALL_BITS_SET;
	}
}

// Sets all the bits in range to a 1.
VOID SetBitsInRange(ULONG& bufferElement, ULONG startingIndex, ULONG endIndex)
{
	for (ULONG bit_num = startingIndex; bit_num <= endIndex; bit_num++)
	{
		bufferElement |= (1 << bit_num);
	}
}

// Calculate the overall bit number in the bit map from the buffer index and bit position.
ULONG CalculateBitNumberInBitMap(ULONG bufferIndex, ULONG bitPosition)
{
	ULONG bit_number = (bufferIndex * BITS_PER_ARRAY) + bitPosition;
	return bit_number;
}

// Count the number of clear bits in a given buffer element.
// This function indicates when a run has started and finished and returns start of the run together
// with the number of clear bits in the run.
VOID CountClearRun(ULONG& bufferElement, ULONG bufferIndex, ULONG startBitPosition,
	ULONG lastBitPosition, bool& runStarted, bool& runEnded, ULONG& startBit, ULONG& runCount)
{
	// If the buffer contains all zeros and a run has not started, the start
	// of the run is the start bit position
	if ((0 == bufferElement) && (!runStarted))
	{
		startBit = CalculateBitNumberInBitMap(bufferIndex, startBitPosition);
		runCount = (lastBitPosition - startBitPosition) + 1;
		runStarted = TRUE;
		runEnded = FALSE;
	}
	// If the buffer contains all zeros and the run has started, add 32 to the run count.
	else if ((0 == bufferElement) && (runStarted))
	{
		runCount += BITS_PER_ARRAY;
	}
	// If the buffer does not contain all zeros, search for the start and end of the run.
	else
	{
		for (ULONG bit_position = startBitPosition; bit_position <= lastBitPosition; bit_position++)
		{
			ULONG bit_set = bufferElement & (1 << bit_position);
			if (!bit_set)
			{
				if (!runStarted)
				{
					startBit = CalculateBitNumberInBitMap(bufferIndex, bit_position);
					runStarted = TRUE;
				}
				runCount++;
			}
			else if (bit_set && runStarted)
			{
				runEnded = TRUE;
				break;
			}
		}
	}
}

// Count the number of contiguous clear bits in a given buffer element.
bool NumClearBitsFound(ULONG& bufferElement, ULONG bufferIndex, ULONG numToFind,
	ULONG startBitPosition, ULONG endBitPosition, bool& runStarted, ULONG& startBit, ULONG& runCount)
{
	// If the buffer contains all zeros and a run has not yet started
	if ((0 == bufferElement) && (!runStarted))
	{
		startBit = CalculateBitNumberInBitMap(bufferIndex, startBitPosition);
		runStarted = TRUE;
		runCount += (endBitPosition - startBitPosition) + 1;

		if (runCount >= numToFind)
		{
			return TRUE;
		}
	}
	else if ((0 == bufferElement) && (runStarted))
	{
		runCount += BITS_PER_ARRAY;

		if (runCount >= numToFind)
		{
			return TRUE;
		}
	}
	else
	{
		for (ULONG bit_num = startBitPosition; bit_num <= endBitPosition; bit_num++)
		{
			ULONG bit_set = bufferElement & (1 << bit_num);
			if (!bit_set)
			{
				if (!runStarted)
				{
					startBit = CalculateBitNumberInBitMap(bufferIndex, bit_num);
					runStarted = TRUE;
				}
				runCount++;

				if (runCount >= numToFind)
				{
					return TRUE;
				}
			}
			else
			{
				runCount = 0;
				runStarted = FALSE;
			}
		}
	}

	return FALSE;
}

// Count the number of contiguous set bits in a given buffer element.
bool NumSetBitsFound(ULONG& bufferElement, ULONG bufferIndex, ULONG numToFind,
	ULONG startBitPosition, ULONG endBitPosition, bool& runStarted, ULONG& startBit, ULONG& runCount)
{
	// If the buffer contains all ones and a run has not yet started
	if ((ALL_BITS_SET == bufferElement) && (!runStarted))
	{
		startBit = CalculateBitNumberInBitMap(bufferIndex, startBitPosition);
		runStarted = TRUE;
		runCount += (endBitPosition - startBitPosition) + 1;

		if (runCount >= numToFind)
		{
			return TRUE;
		}
	}
	else if ((ALL_BITS_SET == bufferElement) && (runStarted))
	{
		runCount += BITS_PER_ARRAY;

		if (runCount >= numToFind)
		{
			return TRUE;
		}
	}
	else
	{
		for (ULONG bit_position = startBitPosition; bit_position <= endBitPosition; bit_position++)
		{
			ULONG bit_set = bufferElement & (1 << bit_position);
			if (bit_set)
			{
				if (!runStarted)
				{
					startBit = CalculateBitNumberInBitMap(bufferIndex, bit_position);
					runStarted = TRUE;
				}
				runCount++;

				if (runCount >= numToFind)
				{
					return TRUE;
				}
			}
			else
			{
				runCount = 0;
				runStarted = FALSE;
			}
		}
	}

	return FALSE;
}
