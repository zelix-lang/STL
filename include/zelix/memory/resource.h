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
#include <type_traits>
#include "zelix/container/forward.h"
#include "zelix/container/move.h"
#include "zelix/except/failed_alloc.h"

namespace zelix::stl::memory
{
    template <typename T>
    class resource
    {
    public:
        static void *raw(const size_t bytes)
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                auto *mem = malloc(bytes);
                if (!mem)
                {
                    throw except::failed_alloc("Memory allocation failed");
                }

                return static_cast<T *>(mem);
            }
            else
            {
                // Allocate memory and construct the object
                auto *mem = operator new(bytes);
                if (!mem)
                {
                    throw except::failed_alloc("Memory allocation failed");
                }

                return mem;
            }
        }

        static T *arr(const size_t len = 10) ///< Allocate memory of given size
        {
            return static_cast<T *>(raw(sizeof(T) * len));
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
                    throw except::failed_alloc("Memory allocation failed");
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
            void *mem = raw(sizeof(T));
            new (mem) T(stl::forward<Args>(args)...);
            return static_cast<T *>(mem);
        }

        static void deallocate_raw(void *ptr)
        {
            if constexpr (std::is_trivially_destructible_v<T>)
            {
                free(ptr);
            }
            else
            {
                operator delete(ptr);
            }
        }

        template <bool CallDestructor = true>
        static void deallocate(T *ptr) ///< Deallocate memory at given pointer
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                // Call the destructor for non-trivially destructible types
                if constexpr (CallDestructor)
                {
                    ptr->~T();
                }
            }

            deallocate_raw(ptr); // Free the pointer directly
        }
    };
}