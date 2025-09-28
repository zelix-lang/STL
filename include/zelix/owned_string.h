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
#include <functional>
#include "external_string.h"
#include "string_utils.h"
#include "zelix/except/out_of_range.h"
#include "zelix/except/uninitialized_memory.h"
#include "zelix/memory/system_resource.h"

namespace zelix::stl
{
    namespace pmr
    {
        /**
         * @brief An owned string class.
         *
         * Provides basic push and reserve operations, and exposes a C-style string interface.
         */
        template <
            double GrowthFactor = 1.8,
            typename Allocator = memory::system_array_resource<char>,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::array_resource<char>, Allocator>
            >
        >
        class string
        {
            char* buffer = nullptr; ///< Pointer to heap-allocated string
            size_t len = 0; ///< Length of the string
            size_t max_capacity = 0; ///< Maximum capacity of the buffer, -1 for null terminator
            size_t capacity = 0; ///< Capacity of the string

            /**
             * @brief Reallocates the heap buffer to a larger size.
             *
             * Copies existing heap data to the new buffer and updates capacity.
             */
            void reallocate()
            {
                capacity = max_capacity + 1;
                buffer = Allocator::reallocate(buffer, len, capacity);
            }

        public:
            /**
             * @brief Default constructor. Initializes an empty string using stack memory.
             */
            string()
            {
                // Do not initialize memory right away
            }

            /**
             * @brief Constructs a string with a specified capacity.
             * @param capacity The initial capacity to reserve.
             *
             * Uses stack memory if capacity is small, otherwise allocates on the heap.
             */
            explicit string(const size_t capacity)
            {
                reserve(capacity);
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
                push(str, s_len);
            }

            /**
             * @brief Constructs a string from a null-terminated C-style string.
             * @param s Pointer to the null-terminated character array to copy from.
             *
             * Uses stack memory if the string is small enough, otherwise allocates on the heap.
             */
            string(const char *s)
            {
                const auto l = str::len(s);
                reserve(l);
                push(s, l);
            }

            string(string &&other) noexcept
                : buffer(other.buffer), len(other.len), max_capacity(other.max_capacity), capacity(other.capacity)
            {
                other.buffer = nullptr; // Transfer ownership, set other's heap to nullptr
                other.len = 0;
                other.max_capacity = 0;
                other.capacity = 0;
            }

            string(const string& other)
            {
                if (other.len == 0 || other.buffer == nullptr) return;
                reserve(other.len);
                push(other.ptr(), other.len);
            }

            string& operator=(const string& other)
            {
                if (this != &other)
                {
                    len = 0; // Reset length before pushing new data
                    if (other.len == 0 || other.buffer == nullptr)
                    {
                        Allocator::deallocate(buffer);
                        return *this;
                    }

                    reserve(other.len);
                    push(other.ptr(), other.len);
                }

                return *this;
            }

            /**
             * @brief Returns a pointer to a null-terminated C-style string.
             * @return Pointer to the string data.
             *
             * Ensures the string is null-terminated before returning.
             */
            [[nodiscard]] char *c_str()
            {
                if (buffer)
                {
                    reserve(1); // Ensure space for null terminator
                    buffer[len] = '\0'; // Ensure null termination for heap memory
                    return buffer;
                }

                throw except::uninitialized_memory("String not initialized");
            }

            [[nodiscard]] char *ptr()
            const {
                if (!buffer)
                {
                    throw except::uninitialized_memory("String not initialized");
                }

                return buffer; // Return heap memory pointer
            }

            /**
             * @brief Sets the internal string length to a new value.
             *
             * \warning This function does not perform bounds checking or memory management.
             * Use only if you are certain the new length is valid and the buffer is large enough.
             * @param new_len The new length to set for the string.
             */
            void calibrate(const size_t new_len)
            {
                // Warning: Only use if you know what you're doing
                len = new_len;
            }

            /**
             * @brief Ensures the string has enough capacity for additional data.
             * @param required The additional capacity to reserve.
             *
             * Switches to heap memory or grows the heap buffer if needed.
             */
            void reserve_growth(const size_t required)
            {
                if (!buffer)
                {
                    buffer = Allocator::allocate(required + 1); // +1 for null terminator
                    capacity = required + 1;
                    max_capacity = required;
                    return;
                }

                if (max_capacity > required + len)
                {
                    return; // Already have enough capacity
                }

                auto new_capacity = static_cast<size_t>(capacity * GrowthFactor);
                while (new_capacity < len + required)
                {
                    new_capacity = static_cast<size_t>(new_capacity * GrowthFactor); // Increase capacity by growth factor
                }

                max_capacity = new_capacity;
                reallocate();
            }

            void reserve(const size_t required)
            {
                if (!buffer)
                {
                    buffer = Allocator::allocate(required + 1); // +1 for null terminator
                    capacity = required + 1;
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
                reserve_growth(1);
                buffer[len++] = c; // Add character to heap memory
            }

            /**
             * @brief Appends a C-style string to the string.
             * @param c The null-terminated string to append.
             * @param c_len The length of the string to append. Defaults to strlen(c).
             */
            void push(const char *c, const size_t c_len)
            {
                reserve_growth(c_len);
                memcpy(buffer + len, c, c_len);
                len += c_len; // Update the length of the string
            }

            void push(const char *c)
            {
                push(c, str::len(c));
            }

            /**
             * @brief Concatenation operator for string + string.
             */
            string operator+(const string &other) const
            {
                string result;
                result.reserve(len + other.len);
                if (buffer != nullptr)
                {
                    result.push(c_str(), len);
                }

                if (other.buffer != nullptr)
                {
                    result.push(other.c_str(), other.len);
                }

                return result;
            }

            /**
             * @brief Provides non-const access to the character at the specified index.
             * @param index The position of the character to access.
             * @return Reference to the character at the given index.
             */
            char& operator[](const size_t index)
            {
                if (!buffer)
                {
                    throw except::uninitialized_memory("String not initialized");
                }

                if (index >= len)
                {
                    throw except::out_of_range("Index out of range");
                }

                return buffer[index];
            }

            /**
             * @brief Provides const access to the character at the specified index.
             * @param index The position of the character to access.
             * @return Const reference to the character at the given index.
             */
            const char& operator[](const size_t index) const
            {
                if (!buffer)
                {
                    throw except::uninitialized_memory("String not initialized");
                }

                if (index >= len)
                {
                    throw except::out_of_range("Index out of range");
                }

                return buffer[index];
            }

            /**
             * @brief Concatenation operator for string + const char*.
             */
            string operator+(const char* other)
            const
            {
                const size_t other_len = str::len(other);
                string result;
                result.reserve(len + other_len);
                if (buffer != nullptr)
                {
                    result.push(buffer, len);
                }

                result.push(other, other_len);
                return result;
            }

            bool operator==(const string& other) const
            {
                if (buffer == nullptr && other.buffer == nullptr)
                {
                    return true; // Empty strings are equal
                }

                if (buffer == nullptr || other.buffer == nullptr)
                {
                    return false; // One is empty, the other is not
                }

                if (len != other.len) return false; // Lengths differ, not equal
                return memcmp(buffer, other.buffer, len) == 0;
            }

            bool operator==(const external_string& other) const
            {
                if (buffer == nullptr)
                {
                    return false; // The string is empty
                }

                if (len != other.size()) return false; // Lengths differ, not equal
                return memcmp(buffer, other.ptr(), len) == 0;
            }

            bool operator==(const char *other) const
            {
                const auto other_len = str::len(other);
                if (buffer == nullptr && other_len == 0)
                {
                    return true; // Both are empty
                }

                if (len != other_len) return false; // Lengths differ, not equal
                return memcmp(buffer, other, len) == 0;
            }

            /**
             * @brief Returns the current length of the string.
             * @return The number of characters in the string.
             */
            [[nodiscard]] size_t size()
            const
            {
                return len;
            }

            /**
             * @brief Returns a pointer to the beginning of the string buffer.
             * @return Pointer to the first character of the string.
             * @throws except::uninitialized_memory if the buffer is not initialized.
             */
            [[nodiscard]] const char *begin() const
            {
                if (!buffer)
                {
                    throw except::uninitialized_memory("String not initialized");
                }

                return buffer;
            }

            /**
             * @brief Returns a pointer to one past the last character of the string buffer.
             * @return Pointer to the end of the string (buffer + len).
             * @throws except::uninitialized_memory if the buffer is not initialized.
             */
            [[nodiscard]] const char *end() const
            {
                if (!buffer)
                {
                    throw except::uninitialized_memory("String not initialized");
                }

                return buffer + len;
            }

            /**
             * @brief Returns a pointer to the beginning of the string buffer.
             * @return Pointer to the first character of the string.
             * @throws except::uninitialized_memory if the buffer is not initialized.
             */
            [[nodiscard]] const char *begin()
            {
                if (!buffer)
                {
                    throw except::uninitialized_memory("String not initialized");
                }

                return buffer;
            }

            /**
             * @brief Returns a pointer to one past the last character of the string buffer.
             * @return Pointer to the end of the string (buffer + len).
             * @throws except::uninitialized_memory if the buffer is not initialized.
             */
            [[nodiscard]] const char *end()
            {
                if (!buffer)
                {
                    throw except::uninitialized_memory("String not initialized");
                }

                return buffer + len;
            }

            /**
             * @brief Clears the string, setting its length to zero.
             *
             * Does not deallocate memory or modify the buffer contents.
             */
            void clear()
            {
                len = 0;
            }

            external_string external()
            {
                return { buffer, len };
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
            static string no_copy(char *buf, const size_t buf_len)
            {
                string result;

                if (buf_len == 0)
                {
                    return result; // Return empty string if buffer length is zero
                }

                result.buffer = buf;
                result.len = buf_len;
                result.capacity = buf_len; // +1 for null terminator
                result.max_capacity = buf_len - 1;
                return result;
            }

            /**
             * @brief Destructor. Releases heap memory if allocated.
             */
            ~string()
            {
                if (buffer)
                {
                    Allocator::deallocate(buffer);
                    buffer = nullptr;
                }
            }
        };
    }

    using string = pmr::string<>;

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
            const size_t len = str::len(c_str);
            return XXH64(c_str, len, len);
        }
    };
}

namespace std {
    template<>
    struct hash<zelix::stl::string> {
        size_t operator()(const zelix::stl::string& str) const noexcept {
            return zelix::stl::string_hash{}(str);
        }
    };
}