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

#pragma once

#include <tuple>
#include <variant>
#include "../memory/array_allocator.h"
#include "../memory/resource.h"
#include "zelix/container/forward.h"
#include "zelix/memory/system_resource.h"

namespace zelix::stl
{
    namespace pmr
    {
        template <
            typename T,
            typename Allocator = std::conditional_t<
                std::is_array_v<T>,
                memory::system_array_resource<T>,
                memory::system_resource<T>
            >,
            typename = std::enable_if_t<
                std::is_base_of_v<
                    std::conditional_t<
                        std::is_array_v<T>,
                        memory::array_resource<std::remove_extent_t<T>>,
                        memory::resource<T>
                    >,
                    Allocator
                >
            >
        >
        class unique_ptr
        {
            T *ptr = nullptr; ///< Pointer to the managed object

        public:
            template <typename ...Args>
            unique_ptr(Args &&...args)
            {
                if constexpr (std::is_array_v<T>)
                {
                    ptr = Allocator::allocate(std::extent_v<T>);

                    // Move all elements
                    static_assert(sizeof...(args) == 1,
                           "Array case expects exactly one arg");

                    auto&& arr = std::get<0>(std::forward_as_tuple(args...));
                    constexpr size_t N = std::extent_v<T>;

                    for (std::size_t i = 0; i < N; ++i)
                    {
                        static_assert(
                            std::is_convertible_v<
                                std::remove_reference_t<decltype(arr[i])>,
                                std::remove_extent_t<T>
                            >,
                            "Array element type mismatch"
                        );

                        const auto &el = arr[i];
                        ptr[i] = el;
                    }
                }
                else
                {
                    ptr = Allocator::allocate(stl::forward<Args>(args)...);
                }
            }

            unique_ptr(const unique_ptr&) = delete;
            unique_ptr& operator=(const unique_ptr&) = delete;

            unique_ptr(unique_ptr&& other) noexcept : ptr(other.ptr)
            {
                other.ptr = nullptr; // give up ownership
            }

            unique_ptr& operator=(unique_ptr&& other) noexcept
            {
                if (this != &other)
                {
                    Allocator::deallocate(ptr); // clean up old pointer
                    ptr = other.ptr; // steal ownership
                    other.ptr = nullptr;
                }

                return *this;
            }

            // Overload operators to use like a raw pointer
            T& operator*() const
            {
                return *ptr;
            }

            T* operator->() const
            {
                return ptr;
            }

            T* get() const
            {
                return ptr;
            }

            // Release ownership (give up pointer without deleting)
            T* release()
            {
                T* tmp = ptr;
                ptr = nullptr;
                return tmp;
            }

            // Reset (delete old and replace with new)
            void reset(T* p = nullptr)
            {
                Allocator::deallocate(ptr);
                ptr = p;
            }
        };
    }

    template <typename T>
    using unique_ptr = pmr::unique_ptr<T>; ///< Non-concurrent shared pointer
}