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
// Created by rodrigo on 8/21/25.
//

#pragma once
#include "display.h"
#include "owned_string.h"
#include "zelix/algorithm/itoa.h"

namespace zelix::stl
{
    template <typename T>
    static inline string serialize(T &&val)
    {
        if constexpr (std::is_same_v<T, char>)
        {
            string res;
            res.push(val);
            return res;
        }
        if constexpr (std::is_same_v<T, const char *>)
        {
            return string::no_copy(val, str::len(val));
        }
        else if constexpr (std::is_same_v<T, string>)
        {
            // Already a string, just return it
            return stl::forward<T>(val);
        }
        else if constexpr (std::is_base_of_v<display, T>)
        {
            return val.serialize();
        }
        else if constexpr (
            std::is_same_v<T, long> ||
            std::is_same_v<T, int> ||
            std::is_same_v<T, unsigned int> ||
            std::is_same_v<T, unsigned long> ||
            std::is_same_v<T, short> ||
            std::is_same_v<T, unsigned short> ||
            std::is_same_v<T, long long> ||
            std::is_same_v<T, unsigned long long> ||
            std::is_same_v<T, size_t>
#           if defined(__ssize_t_defined)
            || std::is_same_v<T, ssize_t>
#           endif
        )
        {
            return algorithm::itoa(val);
        }
        else
        {
            static_assert(false,
                "Invalid type for str::serialize(). "
                "Expected: trivially copyable types or instances "
                "of container::display."
            );

            // Unreachable, but silences compiler warnings
            return {};
        }
    }
}