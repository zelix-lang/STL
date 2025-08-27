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

    [[nodiscard]] inline magic_type magic_number(const size_t max)
    {
#   if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
        return (static_cast<magic_type>(1) << 64) / max;
#   else
        return (magic_type(1) << 32) / max;
#   endif
    }
}