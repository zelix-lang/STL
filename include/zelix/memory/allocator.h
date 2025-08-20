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
#include "resource.h"

namespace zelix::memory
{
    template <typename T>
    class allocator final : resource<T>
    {
    public:
        template <typename... Args>
        static T *allocate(const size_t size, Args&&... args)
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                return static_cast<T *>(malloc(sizeof(T)));
            }

            void* mem = operator new(sizeof(T));
            // Construct objects in the allocated memory
            return new (mem) T(container::forward<Args>(args)...);
        }

        static void deallocate(void *ptr) ///< Deallocate memory at given pointer
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                free(ptr);
                return;
            }

            operator delete(ptr);
        }

        static T *reallocate(void *ptr, const size_t new_siz) ///< Reallocate memory at given pointer to new size
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                return static_cast<T *>(realloc(ptr, new_siz));
            }

            // Allocate new memory
            T *new_ptr = static_cast<T *>(malloc(new_siz));
            if (new_ptr == nullptr)
            {
                return nullptr; // Allocation failed
            }

            // Copy the old object to the new memory
            new (new_ptr) T(*static_cast<T *>(ptr));

            // Deallocate the old memory
            deallocate(ptr);

            return new_ptr;
        }
    };
}