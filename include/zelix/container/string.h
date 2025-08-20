/*
        === The Zelix Programming Language ====
---------------------------------------------------------
  - This file is part of the Zelix Programming Language
    codebase. Zelix is a fast, statically-typed and
    memory-safe programming language that aims to
    match native speeds while staying highly performant.
---------------------------------------------------------
  - Zelix is categorized as free software; you can
    redistribute it and/or modify it under the terms of
    the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.
---------------------------------------------------------
  - Zelix is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even
    the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public
    License for more details.
---------------------------------------------------------
  - You should have received a copy of the GNU General
    Public License along with Zelix. If not, see
    <https://www.gnu.org/licenses/>.
*/
//
// Created by rodrigo on 8/20/25.
//

#pragma once
#include <cstdint>
#if defined(__AVX__) || defined(__AVX2__) || (defined(__has_include) && __has_include(<immintrin.h>))
#   define ZELIX_STL_AVX_DEF
#   include <immintrin.h>
#elif defined(__SSE4_1__) || (defined(__has_include) && __has_include(<smmintrin.h>))
#   define ZELIX_STL_SSE4_1_DEF
#   include <smmintrin.h>
#elif defined(__SSSE3__) || (defined(__has_include) && __has_include(<tmmintrin.h>))
#   define ZELIX_STL_SSSE3_DEF
#   include <tmmintrin.h>
#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || (defined(__has_include) && __has_include(<emmintrin.h>))
#   define ZELIX_STL_SSE2_DEF
#   include <emmintrin.h>
#elif defined(__SSE__) || (defined(__has_include) && __has_include(<xmmintrin.h>))
#   define ZELIX_STL_SSE_DEF
#   include <xmmintrin.h>
#endif

namespace zelix::container::str
{
    /**
     * Checks if the given string `str` starts with the specified `prefix`.
     *
     * \param str     The string to check.
     * \param prefix  The prefix to look for at the start of `str`.
     * \return        TRUE if `str` starts with `prefix`, FALSE otherwise.
     *
     * Returns FALSE if either `str` or `prefix` is NULL.
     */
    static inline bool has_prefix(
        const char *str,
        const char *prefix
    )
    {
        // Check if the string or prefix is NULL
        if (!str || !prefix)
        {
            return false; // Return false if either is NULL
        }

        // Iterate over the characters of the prefix
        for (size_t i = 0; prefix[i] != '\0'; i++)
        {
            // If the current character in the string does not match the prefix
            if (str[i] != prefix[i])
            {
                return false; // Return false
            }
        }

        // If we reached here, the prefix matches the start of the string
        return true; // Return true
    }

    template <bool FallbackLibc=false>
    static inline size_t len(const char *str)
    {
#       if defined(__SSSE3__)
        const char *s = str;
        __m128i zero = _mm_setzero_si128();

        while (1) {
            // Load 16 bytes from memory
            __m128i chunk = _mm_loadu_si128((const __m128i *)s);

            // Compare each byte with 0
            __m128i cmp = _mm_cmpeq_epi8(chunk, zero);

            // Create a bitmask of comparison results
            int mask = _mm_movemask_epi8(cmp);

            if (mask != 0) {
                // Found a null terminator!
                int offset = __builtin_ctz(mask);  // position of first set bit
                return (s - str) + offset;
            }

            s += 16;
        }
#       elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
        const char *start = str;
        const __m128i zero = _mm_setzero_si128();

        // Align pointer until it's 16-byte aligned
        while ((reinterpret_cast<uintptr_t>(str) & 15) != 0) {
            if (*str == '\0') return str - start;
            str++;
        }

        for (;;)
        {
            const __m128i chunk = _mm_load_si128((__m128i *)str); // load 16 bytes
            const __m128i cmp = _mm_cmpeq_epi8(chunk, zero);   // compare with zero

            if (
                const int mask = _mm_movemask_epi8(cmp);
                mask != 0
            ) {
                // found a zero somewhere in the chunk
                return str - start + __builtin_ctz(mask);
            }

            str += 16; // next chunk
        }
#       else
        if constexpr (FallbackLibc)
        {
            return strlen(str);
        }
        else
        {
            const char *s = str;
            const char *start = s;

            while (*s) {
                s++;
            }

            return s - start;
        }
#       endif
    }
}