/*
	BSD 2-Clause License
	
	Copyright (c) 2020, Marcel Pi Nacy
	All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	
	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.
	
	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once
#include <iterator>
#include <bitset>
#include <cstdint>

namespace detail::american_flag_sort
{
	template <size_t RadixSize, typename I, typename F>
	constexpr void american_flag_sort_core(I begin, I end, size_t digit_index, F& extract_digit)
	{
		// Note: the offsets and next arrays can be marked as static thread_local to minimize stack space.

		std::bitset<RadixSize> presence;
		size_t counts[RadixSize] = {};
		size_t offsets[RadixSize];
		size_t next[RadixSize];

		while (true)
		{
			for (I i = begin; i < end; ++i)
			{
				const auto digit = extract_digit(*i, digit_index);
				presence.set(digit);
				++counts[digit];
			}
			if (presence.count() > 1)
				break;
			--digit_index;
			presence.reset();
			std::fill(std::begin(counts), std::end(counts), 0);
		}

		size_t offset = 0;
		for (uint_fast16_t i = 0; i < RadixSize; ++i)
		{
			offsets[i] = offset;
			offset += counts[i];
		}

		offset = 0;
		std::copy(std::begin(offsets), std::end(offsets), std::begin(next));

		for (size_t i = 0; i < RadixSize - 1;)
		{
			const size_t n = i + 1;
			if (offset >= offsets[n])
			{
				i = n;
				continue;
			}

			const auto digit = extract_digit(*(begin + offset), digit_index);
			if (digit == i)
			{
				++offset;
				continue;
			}

			size_t& target = next[digit];
			std::iter_swap(begin + offset, begin + target);
			++target;
		}

		if (digit_index == 0)
			return;
		--digit_index;

		for (size_t i = 0; i < RadixSize; ++i)
		{
			const size_t k = counts[i];
			if (k == 0)
				continue;
			const auto b = begin;
			begin += k;
			american_flag_sort_core<RadixSize>(b, begin, digit_index, extract_digit);
		}
	}
}

template <size_t MaxDigits, size_t RadixSize = 256, typename RandomAccessIterator, typename ExtractDigit>
constexpr void american_flag_sort(RandomAccessIterator begin, RandomAccessIterator end, ExtractDigit&& extract_digit)
{
	static_assert(
		std::is_same<typename std::iterator_traits<RandomAccessIterator>::iterator_category, std::random_access_iterator_tag>::value,
		"American Flag Sort: Only random access iterators are suported.");
	detail::american_flag_sort::american_flag_sort_core<RadixSize>(begin, end, MaxDigits - 1, extract_digit);
}