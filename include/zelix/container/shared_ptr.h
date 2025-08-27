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
#include <atomic>
#include <tuple>
#include <type_traits>

#include "zelix/container/forward.h"
#include "zelix/memory/array_resource.h"
#include "zelix/memory/monotonic.h"
#include "zelix/memory/resource.h"
#include "zelix/memory/system_resource.h"

namespace zelix::stl
{
    namespace pmr
    {
        template <
            typename T,
            bool Concurrent,
            typename Allocator = std::conditional_t<
                std::is_array_v<T>,
                memory::system_array_resource<T>,
                memory::monotonic_system_resource<T>
            >,
            typename ARefCountAllocator = memory::system_resource<std::atomic<int>>,
            typename RefCountAllocator = memory::system_resource<int>,
            typename = std::enable_if_t<
                std::is_base_of_v<
                    std::conditional_t<
                        std::is_array_v<T>,
                        memory::array_resource<std::remove_extent_t<T>>,
                        memory::resource<T>
                    >,
                    Allocator
                >
            >,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::resource<std::atomic<int>>, ARefCountAllocator>
            >,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::resource<int>, RefCountAllocator>
            >
        >
        class shared_ptr
        {
            int *ref_count = nullptr; ///< Pointer to reference count
            T *ptr = nullptr; ///< Pointer to the managed object
            std::atomic<int> *atomic_ref_count = nullptr; ///< Atomic reference count for thread safety

            void destroy()
            {
                Allocator::deallocate(ptr); // Deallocate the managed object memory

                if constexpr (Concurrent)
                {
                    ARefCountAllocator::deallocate(atomic_ref_count); // Free atomic reference count if used
                    atomic_ref_count = nullptr;
                }
                else
                {
                    RefCountAllocator::deallocate(ref_count); // Free reference count if not using atomic
                    ref_count = nullptr;
                }

                ptr = nullptr;
            }

        public:
            template <typename ...Args>
            shared_ptr(Args &&...args)
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

                if constexpr (Concurrent)
                {
                    atomic_ref_count = ARefCountAllocator::allocate(1); // Allocate memory for atomic reference count
                }
                else
                {
                    ref_count = RefCountAllocator::allocate(); // Allocate memory for reference count
                    *ref_count = 1; // Initialize reference count
                }
            }

            shared_ptr(shared_ptr &&other) noexcept
                : ref_count(other.ref_count), ptr(other.ptr), atomic_ref_count(other.atomic_ref_count)
            {
                other.ref_count = nullptr; // Transfer ownership
                other.ptr = nullptr;
                other.atomic_ref_count = nullptr;
            }

            shared_ptr(shared_ptr &other) noexcept
            {
                if constexpr (Concurrent)
                {
                    atomic_ref_count = other.atomic_ref_count;
                    if (atomic_ref_count)
                    {
                        atomic_ref_count->fetch_add(1); // Increment atomic reference count
                    }
                }
                else
                {
                    ref_count = other.ref_count;
                    if (ref_count)
                    {
                        ++(*ref_count); // Increment reference count
                    }
                }

                ptr = other.ptr; // Share the managed object
            }

            shared_ptr &operator=(shared_ptr &&other) noexcept
            {
                if (this != &other)
                {
                    // Decrement current reference count
                    if constexpr (Concurrent)
                    {
                        if (atomic_ref_count && --(*atomic_ref_count) == 0)
                        {
                            destroy();
                        }
                    }
                    else
                    {
                        if (ref_count && --(*ref_count) == 0)
                        {
                            destroy();
                        }
                    }

                    // Transfer ownership from other
                    ref_count = other.ref_count;
                    ptr = other.ptr;
                    atomic_ref_count = other.atomic_ref_count;

                    other.ref_count = nullptr; // Reset other to a null state
                    other.ptr = nullptr;
                    other.atomic_ref_count = nullptr;
                }

                return *this;
            }

            T *operator->() const
            {
                return ptr; // Access the managed object
            }

            bool operator==(const shared_ptr &other) const
            {
                return *ptr == *other.ptr; // Compare the managed objects
            }

            T *operator *() const
            {
                return ptr; // Dereference to get the managed object
            }

            ~shared_ptr()
            {
                if constexpr (Concurrent)
                {
                    if (atomic_ref_count && --(*atomic_ref_count) == 0)
                    {
                        destroy();
                    }
                }
                else
                {
                    if (ref_count && --(*ref_count) == 0)
                    {
                        destroy();
                    }
                }
            }
        };
    }

    template <typename T>
    using shared_ptr = pmr::shared_ptr<T, false>; ///< Non-concurrent shared pointer

    template <typename T>
    using concurrent_ptr = pmr::shared_ptr<T, true>; ///< Non-concurrent shared pointer
}