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
#include "zelix/except/failed_alloc.h"
#include <atomic>
#include <cstdlib>
#include <type_traits>

#include "zelix/container/forward.h"

namespace zelix::stl::memory
{
    template <typename T, bool Concurrent = false>
    class shared_ptr
    {
        int *ref_count = nullptr; ///< Pointer to reference count
        T *ptr = nullptr; ///< Pointer to the managed object
        std::atomic<int> *atomic_ref_count = nullptr; ///< Atomic reference count for thread safety

        void destroy()
        {
            if constexpr (std::is_trivially_destructible_v<T>)
            {
                free(ptr);

                if constexpr (Concurrent)
                {
                    free(atomic_ref_count); // Free atomic reference count if used
                }
                else
                {
                    free(ref_count); // Free reference count if not using atomic
                }
            }
            else
            {
                delete ptr; // Delete the managed object

                if constexpr (Concurrent)
                {
                    delete atomic_ref_count; // Delete atomic reference count if used
                }
                else
                {
                    delete ref_count; // Delete reference count if not using atomic
                }
            }

            ptr = nullptr;
            ref_count = nullptr;
            atomic_ref_count = nullptr;
        }

    public:
        template <typename ...Args>
        shared_ptr(Args &&...args)
        {
            if constexpr (std::is_trivially_constructible_v<T>)
            {
                void *raw = static_cast<T *>(malloc(sizeof(T)));
                if (!raw)
                {
                    throw except::failed_alloc("Memory allocation failed");
                }

                new (raw) T(stl::forward<Args>(args)...); // Construct the object in allocated memory
                ptr = static_cast<T *>(raw);
            }
            else
            {
                void *raw = static_cast<T *>(operator new(sizeof(T)));
                if (!raw)
                {
                    throw except::failed_alloc("Memory allocation failed");
                }

                new (raw) T(stl::forward<Args>(args)...); // Construct the object in allocated memory
                ptr = static_cast<T *>(raw);
            }

            if constexpr (Concurrent)
            {
                atomic_ref_count = new std::atomic<int>(1);
            }
            else
            {
                ref_count = static_cast<int *>(malloc(sizeof(int)));
                if (!ref_count)
                {
                    free(ptr);
                    throw except::failed_alloc("Memory allocation failed");
                }

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
                    ++(*atomic_ref_count); // Increment atomic reference count
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

        ~shared_ptr()
        {
            *--ref_count;
            if (*ref_count == 0)
            {
                destroy(); // Destroy the managed object and free memory
            }
        }
    };
}