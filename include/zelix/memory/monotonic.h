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
// Created by rodrigo on 8/26/25.
//

#pragma once
#include "allocator.h"
#include "resource.h"

namespace zelix::stl::memory
{
    template <typename T>
    class monotonic_system_resource : public resource<T>
    {
        static pmr::lazy_allocator<T> allocator;

    public:
        template <typename... Args>
        static T *allocate(Args&&... args) ///< Allocate memory of given size
        {
            return allocator.alloc(stl::forward<Args>(args)...);
        }

        static void deallocate(T *ptr) ///< Deallocate memory at given pointer
        {
            return allocator.dealloc(ptr);
        }
    };
}