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

#include "zelix/container/list.h"
#include "zelix/container/vector.h"

namespace zelix::stl::memory
{
    namespace pmr
    {
        template <
            typename T,
            size_t Capacity,
            bool CallDestructors,
            typename Allocator = system_array_resource<T>,
            typename = std::enable_if_t<
                std::is_base_of_v<array_resource<T>, Allocator>
            >
        >
        class page
        {
            unsigned char *buffer = nullptr;
            size_t offset = 0;

        public:
            page()
            {
                buffer = reinterpret_cast<unsigned char *>(Allocator::allocate(Capacity));
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
                if constexpr (!std::is_trivially_destructible_v<T> && CallDestructors)
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
            double FreeListGrowthFactor = 1.8,
            size_t FreeListInitialCapacity = 25,
            bool CallDestructors = true,
            typename Allocator = system_array_resource<T>,
            typename InnerAllocator = system_resource<stl::pmr::__list_el<T>>,
            typename FreeListAllocator = system_array_resource<T *>,
            typename = std::enable_if_t<
                std::is_base_of_v<array_resource<T>, Allocator>
            >,
            typename = std::enable_if_t<
                std::is_base_of_v<resource<stl::pmr::__list_el<T>>, InnerAllocator>
            >,
            typename = std::enable_if_t<
                std::is_base_of_v<array_resource<T *>, FreeListAllocator>
            >
        >
        class lazy_allocator
        {
            stl::pmr::list<page<T, Capacity, CallDestructors, Allocator>, InnerAllocator> pages;
            stl::pmr::vector<T *, FreeListGrowthFactor, FreeListInitialCapacity, FreeListAllocator> free_list;

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

    template <
        typename T,
        size_t Capacity = 256,
        double FreeListGrowthFactor = 1.8,
        size_t FreeListInitialCapacity = 25,
        bool CallDestructors = true
    >
    using lazy_allocator = pmr::lazy_allocator<T, Capacity, FreeListGrowthFactor, FreeListInitialCapacity, CallDestructors>; ///< Default lazy allocator type using the default page size and destructor behavior
}