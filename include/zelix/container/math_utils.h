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

namespace zelix::stl
{
    template <
        typename T,
        typename U,
        std::enable_if_t<std::is_integral_v<T>>,
        std::enable_if_t<std::is_integral_v<U>>
    >
    T max(T a, U b)
    {
        return a > b ? a : b;
    }

    template <
        typename T,
        typename U,
        std::enable_if_t<std::is_integral_v<T>>,
        std::enable_if_t<std::is_integral_v<U>>
    >
    T min(T a, U b)
    {
        return a > b ? a : b;
    }
}