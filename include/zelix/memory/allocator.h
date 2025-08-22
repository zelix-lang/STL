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
// Created by rodrigo on 8/22/25.
//

#pragma once

#include "zelix/container/vector.h"

namespace zelix::stl::memory
{
    namespace pmr
    {
        template <
            typename T,
            size_t Capacity,
            bool CallDestructors,
            typename Allocator = resource<T>,
            typename = std::enable_if_t<
                std::is_base_of_v<resource<T>, Allocator>
            >
        >
        class page
        {
            unsigned char *buffer = nullptr;
            size_t offset = 0;

        public:
            page()
            {
                buffer = static_cast<unsigned char *>(Allocator::raw(Capacity * sizeof(T)));
                if (!buffer) throw std::bad_alloc();
            }

            T *alloc(auto&&... args)
            {
                if (offset >= Capacity)
                {
                    throw except::failed_alloc("Out of memory in lazy page allocator");
                }

                // Allocate the next object in the buffer
                T* ptr = reinterpret_cast<T*>(buffer + offset * sizeof(T));
                ++offset;
                new (ptr) T(stl::forward<decltype(args)>(args)...); // Construct the object in place
                return ptr;
            }

            [[nodiscard]] bool full() const
            {
                return offset >= Capacity;
            }

            ~page()
            {
                if constexpr (CallDestructors)
                {
                    // Call the destructor of all allocated objects
                    for (size_t i = 0; i < offset; ++i)
                    {
                        T *ptr = reinterpret_cast<T*>(buffer + i * sizeof(T));
                        ptr->~T(); // Call the destructor
                    }
                }

                // Free the buffer memory
                Allocator::deallocate_raw(buffer);
            }
        };

        template <
            typename T,
            size_t Capacity = 256,
            bool CallDestructors = false,
            typename Allocator = resource<T>,
            typename = std::enable_if_t<
                std::is_base_of_v<resource<T>, Allocator>
            >
        >
        class lazy_allocator
        {
            vector<page<T, Capacity, CallDestructors>> pages;
            vector<T *> free_list;

        public:
            T *alloc(auto&&... args)
            {
                // Check the free list first
                if (!free_list.empty())
                {
                    T *ptr = free_list[free_list.size() - 1];
                    free_list.pop_back();
                    new (ptr) T(stl::forward<decltype(args)>(args)...); // Placement new to construct the object
                    return ptr;
                }

                // See if we have any pages available
                if (pages.empty())
                {
                    pages.emplace_back();
                }

                auto &back = pages.back();
                if (back.full())
                {
                    // Allocate a new page
                    pages.emplace_back();
                    return pages.back().alloc(stl::forward<decltype(args)>(args)...);
                }

                return back.alloc(stl::forward<decltype(args)>(args)...);
            }

            void dealloc(T *ptr)
            {
                ptr->~T(); // Call the destructor
                free_list.push_back(ptr);
            }

            ~lazy_allocator()
            {
                pages.clear(); // Clear the vector of pages
            }
        };
    }

    template <typename T, size_t Capacity = 256, bool CallDestructors = false>
    using lazy_allocator = pmr::lazy_allocator<T, Capacity, CallDestructors>; ///< Default lazy allocator type using the default page size and destructor behavior
}