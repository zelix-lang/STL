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
#else
#   include <cstring> // Fallback to standard library
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
#   ifdef ZELIX_STL_AVX_DEF
        const char *s = str;
        const char *start = str;

        // Load in 32-byte chunks (AVX2 supports this)
        while (true) {
            const __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(s));

            // Compare against zero
            const __m256i cmp = _mm256_cmpeq_epi8(chunk, _mm256_setzero_si256());

            // Move mask into an int

            if (
                const unsigned int mask = _mm256_movemask_epi8(cmp);
                mask != 0
            ) {
                // Found a null terminator
                const int idx = __builtin_ctz(mask); // index of first zero
                return s - start + idx;
            }

            s += 32;
        }
#   elif defined(ZELIX_STL_SSE4_1_DEF)
        const char *start = str;
        __m128i zeros = _mm_setzero_si128();

        // Align to 16 bytes
        while (((uintptr_t)str & 0x0F) != 0) {
            if (*str == '\0') return (size_t)(str - start);
            str++;
        }

        for (;;) {
            __m128i chunk = _mm_load_si128((const __m128i *)str);
            __m128i cmp   = _mm_cmpeq_epi8(chunk, zeros);
            int mask      = _mm_movemask_epi8(cmp);

            if (mask != 0) {
                // Found a zero somewhere in the 16 bytes
                int offset = __builtin_ctz(mask); // index of first set bit
                return (size_t)(str - start + offset);
            }

            str += 16;
        }
#   elif defined(ZELIX_STL_SSSE3_DEF)
        const char *p = s;
        __m128i zero = _mm_setzero_si128();

        // scan 16 bytes at a time
        while (1) {
            __m128i chunk = _mm_loadu_si128((const __m128i*)p);
            __m128i cmp   = _mm_cmpeq_epi8(chunk, zero);
            int mask      = _mm_movemask_epi8(cmp);

            if (mask != 0) {
                // found a zero byte
                return (size_t)(p - s) + __builtin_ctz(mask);
            }
            p += 16;
        }
#   elif defined(ZELIX_STL_SSE2_DEF)
        const char *s = str;
        __m128i zero = _mm_setzero_si128();

        // Align to 16 bytes
        while (((uintptr_t)s & 0x0F) != 0) {
            if (*s == '\0')
                return (size_t)(s - str);
            s++;
        }

        for (;;) {
            __m128i chunk = _mm_load_si128((const __m128i *)s);
            __m128i cmp = _mm_cmpeq_epi8(chunk, zero);
            int mask = _mm_movemask_epi8(cmp);

            if (mask != 0) {
                // found a null byte
                int offset = __builtin_ctz(mask);
                return (size_t)((s - str) + offset);
            }
            s += 16;
        }
#   elif defined(ZELIX_STL_SSE_DEF)
        const char *start = str;
        // align pointer to 16-byte boundary
        while (((uintptr_t)str & 15) != 0) {
            if (*str == '\0')
                return str - start;
            str++;
        }

        // process 16 bytes at a time
        const __m128i zeros = _mm_setzero_si128();
        for (;;) {
            __m128i chunk = _mm_load_si128((const __m128i*)str);
            __m128i cmp   = _mm_cmpeq_epi8(chunk, zeros);
            int mask      = _mm_movemask_epi8(cmp);

            if (mask != 0) { // found null terminator
                int offset = __builtin_ctz(mask);
                return (str - start) + offset;
            }
            str += 16;
        }
#   else
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

    struct string_hash
    {
        using is_transparent = void;

        size_t operator()(const string &str) const
        {
            // Use xxHash
            return XXH3_64bits(str.ptr(), str.size());
        }

        size_t operator()(const external_string &str) const
        {
            // Use xxHash
            return XXH3_64bits(str.ptr(), str.size());
        }

        size_t operator()(const char* c_str) const
        {
            const size_t len = str::len(c_str);
            return XXH64(c_str, len, len);
        }
    };
}