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
// Created by rodri on 8/13/25.
//

#pragma once
#include "forward.h"
#include "zelix/except/out_of_range.h"
#include "zelix/memory/system_resource.h"

namespace zelix::stl
{
    namespace pmr
    {
        /**
         * \brief A fixed-size ring buffer (circular vector) container.
         *
         * \tparam T   Type of elements stored.
         * \tparam Max Maximum number of elements the container can hold.
         *
         * Elements are stored in a fixed-size array. When the buffer is full,
         * new elements overwrite the oldest ones.
         */
        template <
            typename T,
            size_t Max,
            bool UseHeap,
            typename Allocator = memory::system_array_resource<T>,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::array_resource<T>, Allocator>
            >
        >
        class ring_buffer
        {
            using data_type = std::conditional_t<
                UseHeap,
                T*,          // heap allocation
                T[Max]       // stack allocation
            >;

            data_type data;
            size_t head = 0;  ///< The index of the next element to be added

        public:
            explicit ring_buffer()
            {
                if constexpr (UseHeap)
                {
                    data = Allocator::allocate(Max);
                }
            }

            /**
             * \brief Adds an element to the end of the ring buffer.
             *
             * If the buffer is full, the oldest element is overwritten.
             * \param value The element to add.
             */
            template <class U = T>
            void push_back(U &&value)
            {
                if (head >= Max)
                {
                    head = 0; // Reset the head if it exceeds the maximum size
                }

                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    data[head++] = value; // Add the element if there's space
                }
                else
                {
                    new (&data[head++]) T(stl::forward<decltype(value)>(value)); // Placement new for non-trivially copyable types
                }
            }

            template <typename... Args>
            void emplace_back(Args&&... args)
            {
                if (head >= Max)
                {
                    head = 0; // Reset the head if it exceeds the maximum size
                }

                new (&this->data[head]) T(stl::forward<Args>(args)...);
                ++head;
            }

            /**
             * \brief Accesses the element at the given index.
             *
             * \param index Index of the element to access.
             * \return Reference to the element at the specified index.
             * \throws except::exception if index is out of range.
             */
            T &operator[](size_t index)
            {
                if (index >= head)
                    throw except::out_of_range("Index out of range");

                if constexpr (UseHeap)
                {
                    return data[index];
                } else {
                    return data[index];
                }
            }

            /**
             * \brief Accesses the element at the given index (const version).
             *
             * \param index Index of the element to access.
             * \return Const reference to the element at the specified index.
             * \throws except::exception if index is out of range.
             */
            T &operator[](size_t index) const
            {
                if (index >= head)
                    throw except::out_of_range("Index out of range");

                if constexpr (UseHeap)
                {
                    return data[index];
                } else {
                    return data[index];
                }
            }

            /**
             * \brief Returns a pointer to the beginning of the data array.
             *
             * \return Pointer to the first element.
             */
            T *begin()
            {
                return data; // Return a pointer to the beginning of the data array
            }

            /**
             * \brief Returns a pointer to the end of the data array.
             *
             * \return Pointer to one past the last element.
             */
            T *end()
            {
                return data + head; // Return a pointer to the end of the data array
            }

            /**
             * \brief Checks if the ring buffer is full.
             *
             * \return True if the buffer is full, false otherwise.
             */
            [[nodiscard]] bool full() const
            {
                return head >= Max; // Check if the ring vector is full
            }

            /**
             * \brief Checks if the ring buffer is empty.
             *
             * \return True if empty, false otherwise.
             */
            [[nodiscard]] bool empty() const
            {
                return head == 0; // Check if the ring vector is empty
            }

            /**
             * \brief Returns the number of elements in the ring buffer.
             *
             * \return The current size.
             */
            [[nodiscard]] size_t size() const
            {
                return head;
            }

            /**
             * \brief Clears the ring buffer.
             *
             * Resets the buffer to an empty state by setting the head index to zero.
             */
            void flush()
            {
                head = 0; // Reset the head to clear the buffer
            }

            /**
             * \brief Returns the current head position in the ring buffer.
             *
             * \return The index where the next element will be inserted.
             */
            [[nodiscard]] size_t pos() const
            {
                return head; // Return the current position (head index)
            }

            /**
             * \brief Advances the head index by one.
             *
             * Moves the head forward, potentially exceeding the buffer size.
             * Caller is responsible for bounds checking if needed.
             */
            void advance()
            {
                if (head >= Max)
                {
                    head = 0; // Reset the head if it exceeds the maximum size
                    return;
                }

                head++;
            }

            /**
             * \brief Returns a pointer to the internal data array.
             *
             * \return Pointer to the internal data array.
             */
            T *ptr()
            {
                return data; // Return a pointer to the internal data array
            }

            /**
             * \brief Writes a block of elements into the ring buffer.
             *
             * \param buf   Pointer to the source buffer containing elements to write.
             * \param count Number of elements to write from the source buffer.
             */
            void unsafe_copy(const T *buf, const size_t count)
            {
                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    memcpy(data + head, buf, count * sizeof(T));
                }
                else
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        new (&data[head + i]) T(buf[i]);
                    }
                }
            }

            /**
             * \brief Writes a block of elements into the ring buffer.
             *
             * \param buf   Pointer to the source buffer containing elements to write.
             * \param count Number of elements to write from the source buffer.
             */
            template <bool BoundsChecking = true>
            void write(const T *buf, const size_t count)
            {
                if constexpr (BoundsChecking)
                {
                    if (head + count >= Max)
                    {
                        const auto what_fits = Max - head;
                        unsafe_copy(buf, what_fits);
                        head = 0;
                        unsafe_copy(buf + count, count - what_fits);
                        head += (count - what_fits);
                        return;
                    }
                }

                unsafe_copy(buf, count);
                head += count;
            }

            ~ring_buffer()
            {
                if constexpr (UseHeap)
                {
                    Allocator::deallocate(data);
                }
            }
        };
    }

    template <typename T, size_t Max, bool UseHeap>
    using ring_buffer = pmr::ring_buffer<T, Max, UseHeap>;
}