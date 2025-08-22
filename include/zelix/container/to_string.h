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
        else
        {
            static_assert(false,
                "Invalid type for str::serialize(). "
                "Expected: trivially copyable types or instances "
                "of container::display."
            );
        }
    }
}