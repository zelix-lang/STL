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
// Created by rodrigo on 8/20/25.
//

#pragma once
#include <cstdlib>


#include "resource.h"

namespace zelix::memory
{
    class allocator final : resource
    {
    public:
        void *allocate(const size_t size) override
        {
            return malloc(size);
        }

        void deallocate(void *ptr) override
        {
            free(ptr);
        }

        void *reallocate(void *ptr, const size_t new_size) override
        {
            return realloc(ptr, new_size);
        }
    };

    inline allocator base_allocator; ///< Global allocator instance
}