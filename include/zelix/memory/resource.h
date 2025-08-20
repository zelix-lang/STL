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
#include <type_traits>
#include <cstdlib>
#include "zelix/container/forward.h"

namespace zelix::memory
{
    template <typename T>
    class resource
    {
    public:
        static T *allocate(const size_t len = 10) ///< Allocate memory of given size
        {
            // Allocate directly for trivially copyable types
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                return malloc(len * sizeof(T));
            }

            // Allocate memory and construct the object
            return operator new(len * sizeof(T));
        }

        template <typename... Args>
        static T *allocate(Args&&... args) ///< Allocate memory of given size
        {
            // Allocate directly for trivially copyable types
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                return malloc(sizeof(T));
            }

            // Allocate memory and construct the object
            void *mem = operator new(sizeof(T));
            return new (mem) T(container::forward<Args>(args)...);
        }

        static void deallocate(T *ptr) ///< Deallocate memory at given pointer
        {
            if constexpr (std::is_trivially_destructible_v<T>)
            {
                // Free the pointer directly
                free(ptr);
                return;
            }

            // Call the destructor for non-trivially destructible types
            ptr->~T();
            operator delete(ptr);
        }
    };
}