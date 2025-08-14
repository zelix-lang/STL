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
#include <cstddef>

namespace zelix::container
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
    template <typename T, size_t Max>
    class ring_buffer
    {
        T data[Max];      ///< Fixed-size array to hold the elements
        size_t head = 0;  ///< The index of the next element to be added

    public:
        /**
         * \brief Adds an element to the end of the ring buffer.
         *
         * If the buffer is full, the oldest element is overwritten.
         * \param value The element to add.
         */
        void push_back(T &value)
        {
            if (head > Max)
            {
                head = 0; // Reset the head if it exceeds the maximum size
            }

            data[head++] = value; // Add the element if there's space
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
                throw except::exception("Index out of range");

            return data[index];
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
                throw except::exception("Index out of range");

            return data[index];
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
    };
}