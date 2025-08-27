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
#include <cstdlib>


#include "array_resource.h"
#include "resource.h"
#include "zelix/container/forward.h"
#include "zelix/container/math_utils.h"
#include "zelix/container/move.h"

namespace zelix::stl::memory
{
    template<typename T>
    class system_resource : public resource<T>
    {
    public:
        template <typename... Args>
        static T *allocate(Args&&... args) ///< Allocate memory of given size
        {
            void *mem = nullptr;
            if constexpr (std::is_trivially_constructible_v<T>)
            {
                mem = static_cast<T *>(malloc(sizeof(T)));
            }
            else
            {
                mem = operator new(sizeof(T));
            }

            // Ensure the allocation succeeded
            if (mem == nullptr)
            {
                throw except::failed_alloc("Memory allocation failed.");
            }

            // Use placement new to construct the object in the allocated memory
            return new (mem) T(stl::forward<Args>(args)...);
        }

        static void deallocate(T *ptr) ///< Deallocate memory at given pointer
        {
            if constexpr (std::is_trivially_destructible_v<T>)
            {
                free(ptr);
            }
            else
            {
                ptr->~T(); // Call the destructor
                operator delete(ptr);
            }
        }
    };

    template<typename T>
    class system_array_resource : public array_resource<T>
    {
    public:
        static T *allocate(size_t n) ///< Allocate memory for the given elements
        {
            if constexpr (std::is_trivially_constructible_v<T>)
            {
                return static_cast<T *>(malloc(sizeof(T) * n));
            }
            else
            {
                return static_cast<T *>(operator new(sizeof(T) * n));
            }
        }

        static T *reallocate(T *ptr, const size_t old_len, const size_t new_len) ///< Allocate memory for the given elements
        {
            if constexpr (std::is_trivially_destructible_v<T>)
            {
                return static_cast<T *>(realloc(ptr, sizeof(T) * new_len));
            }
            else
            {
                T *new_ptr = static_cast<T *>(operator new(sizeof(T) * new_len));
                const size_t min_len = min(old_len, new_len);

                // Move the existing elements to the new memory
                for (size_t i = 0; i < min_len; ++i)
                {
                    new (&new_ptr[i]) T(stl::move(ptr[i]));
                    ptr[i].~T(); // Call destructor for the moved-from element
                }

                operator delete(ptr); // Free the old memory
                return new_ptr;
            }
        }

        static void deallocate(T *ptr) ///< Deallocate memory at given pointer
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
    };
}