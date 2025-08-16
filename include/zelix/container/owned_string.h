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
// Created by rodrigo on 7/29/25.
//

#pragma once
#include <cstring>
#include <xxh3.h>
#include "external_string.h"

namespace zelix::container
{
    /**
     * @brief A owned string class.
     *
     * Provides basic push and reserve operations, and exposes a C-style string interface.
     */
    class string
    {
        char* heap = nullptr; ///< Pointer to heap-allocated string for larger strings
        size_t len = 0; ///< Length of the string
        size_t max_capacity = 0; ///< Maximum capacity of the stack buffer, -1 for null terminator
        size_t capacity = 0; ///< Capacity of the string
        double growth_factor = 1.8; ///< Growth factor for heap allocation

        /**
         * @brief Initializes the heap-allocated buffer and copies stack data.
         *
         * Called when the string grows beyond the stack buffer.
         */
        void heap_init()
        {
            // Initialize the heap-allocated string if needed
            max_capacity = static_cast<size_t>(capacity * growth_factor - 1); // Adjust max capacity based on growth factor
            capacity = max_capacity + 1;
            heap = new char[capacity];
        }

        /**
         * @brief Reallocates the heap buffer to a larger size.
         *
         * Copies existing heap data to the new buffer and updates capacity.
         */
        void reallocate()
        {
            // Initialize the heap-allocated string if needed
            capacity = max_capacity + 1;
            const auto new_heap = new char[capacity];
            memcpy(new_heap, heap, len); // Copy existing data to new heap
            delete[] heap; // Free old heap memory
            heap = new_heap; // Update heap pointer
        }

    public:
        /**
         * @brief Default constructor. Initializes an empty string using stack memory.
         */
        explicit string()
        {
            capacity = 12;
            heap_init();
        }

        /**
         * @brief Constructs a string with a specified capacity.
         * @param capacity The initial capacity to reserve.
         *
         * Uses stack memory if capacity is small, otherwise allocates on the heap.
         */
        explicit string(const size_t capacity)
        {
            this->capacity = capacity;
            heap_init(); // Initialize heap buffer
        }

        /**
         * @brief Constructs a string from a C-style string with a specified capacity.
         * @param str Pointer to the character array to copy from.
         * @param s_len The number of characters to copy.
         *
         * Uses stack memory if the capacity is small enough, otherwise allocates on the heap.
         */
        explicit string(const char *str, const size_t s_len)
        {
            reserve(s_len);
            memcpy(heap, str, s_len);
            len = s_len; // Set the length of the string
        }

        /**
         * @brief Constructs a string from a null-terminated C-style string.
         * @param str Pointer to the null-terminated character array to copy from.
         *
         * Uses stack memory if the string is small enough, otherwise allocates on the heap.
         */
        explicit string(const char *str)
        {
            const auto s_len = strlen(str); // Get the length of the string

            this->capacity = s_len;
            heap_init(); // Initialize heap buffer
            memcpy(heap, str, s_len);
            len = s_len; // Set the length of the string
        }

        string(string && other) noexcept
            : heap(other.heap), len(other.len), max_capacity(other.max_capacity), capacity(other.capacity)
        {
            other.heap = nullptr; // Transfer ownership, set other's heap to nullptr
            other.len = 0;
            other.max_capacity = 0;
            other.capacity = 0;
        }

        string(const string& other)
        {
            len = other.len;
            max_capacity = other.max_capacity;
            capacity = other.capacity;
            heap = new char[capacity];
            memcpy(heap, other.heap, len);
        }

        string& operator=(const string& other)
        {
            if (this != &other) {
                delete[] heap;
                len = other.len;
                max_capacity = other.max_capacity;
                capacity = other.capacity;
                heap = new char[capacity];
                memcpy(heap, other.heap, len);
            }
            return *this;
        }

        /**
         * @brief Returns a pointer to a null-terminated C-style string.
         * @return Pointer to the string data.
         *
         * Ensures the string is null-terminated before returning.
         */
        [[nodiscard]] char *c_str() const
        {
            if (heap)
            {
                heap[len] = '\0'; // Ensure null termination for heap memory
                return heap;
            }

            return nullptr; // Return nullptr if no memory is allocated
        }

        [[nodiscard]] const char *ptr()
        const
        {
            return heap; // Return heap memory pointer
        }

        /**
         * @brief Ensures the string has enough capacity for additional data.
         * @param required The additional capacity to reserve.
         *
         * Switches to heap memory or grows the heap buffer if needed.
         */
        void reserve_growth(const size_t required)
        {
            if (heap == nullptr)
            {
                capacity = required + 1;
                heap = new char[capacity];
                max_capacity = required;
                return;
            }

            if (len + required > max_capacity)
            {
                // Check if we need to grow the heap memory
                auto new_capacity = static_cast<size_t>(capacity * growth_factor);
                const size_t count_to = len + required;

                while (new_capacity < count_to)
                {
                    new_capacity *= growth_factor; // Increase capacity by growth factor
                }

                max_capacity = new_capacity;
                reallocate();
            }
        }

        void reserve(const size_t required)
        {
            if (heap == nullptr)
            {
                capacity = required + 1;
                heap = new char[capacity];
                max_capacity = required;
                return;
            }

            if (len + required > max_capacity)
            {
                max_capacity = len + required;
                reallocate();
            }
        }

        /**
         * @brief Appends a single character to the string.
         * @param c The character to append.
         */
        void push(const char c)
        {
            reserve_growth(1); // Reserve space for one character
            heap[len++] = c; // Add character to heap memory
        }

        /**
         * @brief Appends a C-style string to the string.
         * @param c The null-terminated string to append.
         * @param c_len The length of the string to append. Defaults to strlen(c).
         */
        void push(const char *c, const size_t c_len)
        {
            reserve_growth(c_len); // Reserve space for one character
            memcpy(heap + len, c, c_len);
            len += c_len; // Update the length of the string
        }

        void push(const char *c)
        {
            push(c, strlen(c)); // Push with length
        }

        /**
         * @brief Concatenation operator for string + string.
         */
        string operator+(const string & other) const
        {
            string result;
            result.reserve(len + other.len);
            if (heap != nullptr)
            {
                result.push(c_str(), len);
            }

            if (other.heap != nullptr)
            {
                result.push(other.c_str(), other.len);
            }

            return result;
        }

        /**
         * @brief Concatenation operator for string + const char*.
         */
        string operator+(const char* other)
        const {
            const size_t other_len = strlen(other);
            string result;
            result.reserve_growth(len + other_len);
            if (heap != nullptr)
            {
                result.push(c_str(), len);
            }

            result.push(other, other_len);
            return result;
        }

        bool operator==(const string& other) const
        {
            if (len != other.len) return false;
            return memcmp(ptr(), other.ptr(), len) == 0;
        }

        bool operator==(const char *other) const
        {
            if (len != strlen(other)) return false;
            return memcmp(ptr(), other, len) == 0;
        }

        /**
         * @brief Returns the current length of the string.
         * @return The number of characters in the string.
         */
        [[nodiscard]] size_t size()
        const {
            return len;
        }

        /**
         * @brief Creates a string object that uses an external buffer without copying.
         *
         * The returned string will directly reference the provided buffer.
         * The caller is responsible for ensuring the buffer remains valid for the lifetime of the string.
         * No memory is allocated or copied.
         *
         * @param buf Pointer to the external character buffer.
         * @param buf_len Length of the buffer.
         * @return string referencing the external buffer.
         */
        static string no_copy(const char *buf, const size_t buf_len)
        {
            string str;
            str.heap = const_cast<char*>(buf); // Use the provided buffer directly
            str.len = buf_len;
            str.capacity = buf_len + 1; // Set capacity to length + null terminator
            str.max_capacity = buf_len; // Set max capacity to length
            return str;
        }

        /**
         * @brief Destructor. Releases heap memory if allocated.
         */
        ~string()
        {
            if (heap)
            {
                delete[] heap; // Free heap memory if allocated
                heap = nullptr;
            }
        }
    };

    struct string_hash
    {
        using is_transparent = void;

        size_t operator()(const string &str) const
        {
            // Use xxHash
            return XXH3_64bits(str.ptr(), str.size());
        }

        size_t operator()(const external_string &str) const
        {
            // Use xxHash
            return XXH3_64bits(str.ptr(), str.size());
        }

        size_t operator()(const char* c_str) const
        {
            const size_t len = strlen(c_str);
            return XXH64(c_str, len, len);
        }
    };
}