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
// Created by rodri on 8/15/25.
//

#pragma once
#include "external_string.h"
#include "owned_string.h"

namespace zelix::container
{
    struct string_equal {
        using is_transparent = void;

        bool operator()(string_utils const& lhs, string_utils const& rhs) const noexcept
        {
            return lhs == rhs;
        }

        bool operator()(external_string const& lhs, string_utils const& rhs) const noexcept
        {
            return lhs.size() == rhs.size() &&
                   memcmp(lhs.ptr(), rhs.ptr(), lhs.size()) == 0;
        }

        bool operator()(string_utils const& lhs, external_string const& rhs) const noexcept
        {
            return lhs.size() == rhs.size() &&
                   memcmp(lhs.ptr(), rhs.ptr(), lhs.size()) == 0;
        }

        bool operator()(external_string const& lhs, external_string const& rhs) const noexcept
        {
            return lhs == rhs;
        }
    };
}