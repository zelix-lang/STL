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
#include "zelix/container/move.h"

namespace zelix::stl::memory
{
    template <typename T>
    class resource
    {
    public:
        static T *arr(const size_t len = 10) ///< Allocate memory of given size
        {
            // Allocate directly for trivially copyable types
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                return static_cast<T *>(malloc(len * sizeof(T)));
            }

            // Allocate memory and construct the object
            return static_cast<T *>(operator new(len * sizeof(T)));
        }

        static T *reallocate(T *ptr, const size_t old_len, const size_t new_len)
        {
            // Trivial-copyable optimization
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                T* new_data = static_cast<T*>(realloc(ptr, sizeof(T) * new_len));
                if (!new_data)
                {
                    deallocate(ptr);
                    throw except::exception("Memory allocation failed");
                }

                return new_data;
            }

            T* new_data = arr(new_len);

            // Move existing elements to new storage
            for (size_t i = 0; i < old_len; ++i)
            {
                new (&new_data[i]) T(stl::move(ptr[i]));
                ptr[i].~T(); // Call destructor for old element
            }

            deallocate(ptr); // Deallocate old memory
            return new_data;
        }

        template <typename... Args>
        static T *allocate(Args&&... args) ///< Allocate memory of given size
        {
            // Allocate directly for trivially copyable types
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                return static_cast<T *>(malloc(sizeof(T)));
            }

            // Allocate memory and construct the object
            void *mem = operator new(sizeof(T));
            return new (mem) T(stl::forward<Args>(args)...);
        }

        template <bool CallDestructor = true>
        static void deallocate(T *ptr) ///< Deallocate memory at given pointer
        {
            if constexpr (std::is_trivially_destructible_v<T>)
            {
                // Free the pointer directly
                free(ptr);
                return;
            }

            // Call the destructor for non-trivially destructible types
            if constexpr (CallDestructor)
            {
                ptr->~T();
            }
            operator delete(ptr);
        }
    };
}