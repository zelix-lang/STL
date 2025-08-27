/*
        ==== The Zelix Programming Language ====
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
// Created by rodrigo on 8/24/25.
//

#pragma once
#include <type_traits>

#   if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
    using magic_type = __uint128_t;
#   else
using magic_type = uint32_t;
#   endif

namespace zelix::stl
{
    template <
        typename T,
        typename U,
        typename Return = T,
        typename = std::enable_if_t<std::is_integral_v<T>>,
        typename = std::enable_if_t<std::is_integral_v<U>>,
        typename = std::enable_if_t<std::is_integral_v<Return>>
    >
    [[nodiscard]] Return max(T a, U b)
    {
        if (a < b)
        {
            if constexpr (std::is_same_v<Return, T>)
            {
                return b;
            }
            else
            {
                return static_cast<Return>(b);
            }
        }

        if constexpr (std::is_same_v<Return, T>)
        {
            return a;
        }
        else
        {
            return static_cast<Return>(a);
        }
    }

    template <
        typename T,
        typename U,
        typename Return = T,
        typename = std::enable_if_t<std::is_integral_v<T>>,
        typename = std::enable_if_t<std::is_integral_v<U>>,
        typename = std::enable_if_t<std::is_integral_v<Return>>
    >
    [[nodiscard]] Return min(T a, U b)
    {
        if (a > b)
        {
            if constexpr (std::is_same_v<Return, T>)
            {
                return b;
            }
            else
            {
                return static_cast<Return>(b);
            }
        }

        if constexpr (std::is_same_v<Return, T>)
        {
            return a;
        }
        else
        {
            return static_cast<Return>(a);
        }
    }

    /**
     * \brief Computes the magic number used for fast modulo reduction.
     *
     * This function calculates a precomputed "magic" value for a given maximum divisor,
     * which can be used to replace division with faster multiplication and bit-shifting.
     * The implementation uses 128-bit arithmetic on 64-bit platforms and 32/64-bit on others.
     *
     * \param max The divisor for which to compute the magic number.
     * \return The computed magic number for use in fast modulo operations.
     */
    [[nodiscard]] inline magic_type magic_number(const size_t max)
    {
#   if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
        return (static_cast<magic_type>(1) << 64) / max;
#   else
        return (magic_type(1) << 32) / max;
#   endif
    }

    /**
     * \brief Computes the modulo of two numbers using a precomputed magic number.
     *
     * This function performs a fast modulo operation using a precomputed magic number,
     * replacing division with multiplication and bit-shifting for efficiency.
     * The implementation uses 128-bit arithmetic on 64-bit platforms and 32/64-bit on others.
     *
     * \param a The dividend.
     * \param b The divisor.
     * \param magic The precomputed magic number for the divisor.
     * \return The result of a % b.
     */
    inline size_t mod(const size_t a, const size_t b, const magic_type magic)
    {
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
        const __uint128_t prod = static_cast<__uint128_t>(a) * magic;
        const size_t quotient = prod >> 64;
        return a - quotient * b;
#else
        const uint64_t prod = static_cast<uint64_t>(a) * magic;
        const size_t quotient = prod >> 32;
        return a - quotient * b;
#endif
    }
}