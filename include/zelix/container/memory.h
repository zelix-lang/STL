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
// Created by rodrigo on 9/10/25.
//

#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace zelix::stl
{
    /**
     * \brief Computes the byte offset of a member within a parent struct/class.
     *
     * \tparam Parent The parent struct/class type.
     * \tparam MemberType The type of the member.
     * \param member Pointer-to-member indicating the member within Parent.
     * \return The offset, in bytes, of the member within Parent.
     */
    template <typename Parent, typename MemberType>
    [[nodiscard]] inline constexpr size_t offset_of(MemberType Parent::* member)
    {
        return reinterpret_cast<size_t>(&(reinterpret_cast<Parent *>(0)->*member));
    }

    /**
     * \brief Retrieves a pointer to the parent struct/class from a pointer to one of its members.
     *
     * \tparam Parent The parent struct/class type.
     * \tparam MemberType The type of the member.
     * \param member_ptr Pointer to the member.
     * \param member Pointer-to-member indicating the member within Parent.
     * \return Pointer to the parent struct/class containing the member.
     */
    template <typename Parent, typename MemberType>
    [[nodiscard]] inline constexpr Parent* container_of(MemberType* member_ptr, MemberType Parent::* member)
    {
        auto offset = offset_of(member);
        return reinterpret_cast<Parent*>(reinterpret_cast<char*>(member_ptr) - offset);
    }

    /**
     * \brief Checks if a pointer is aligned to the specified alignment.
     *
     * \tparam Alignment The alignment boundary (must be a power of two).
     * \tparam T The pointer type.
     * \param ptr Pointer to check.
     * \return True if the pointer is aligned to Alignment, false otherwise.
     */
    template <size_t Alignment, typename T>
    constexpr bool is_aligned(T* ptr)
    {
        return reinterpret_cast<uintptr_t>(ptr) % Alignment == 0;
    }

    /**
     * \brief Rounds up a size to the nearest multiple of Alignment.
     *
     * \tparam Alignment The alignment boundary (must be a power of two).
     * \param size The size to align.
     * \return The aligned size.
     */
    template <size_t Alignment>
    constexpr size_t align_up(const size_t size)
    {
        return (size + Alignment - 1) & ~(Alignment - 1);
    }

    /**
     * \brief Returns a pointer offset by a given number of bytes.
     *
     * \tparam T The pointer type.
     * \param ptr The base pointer.
     * \param offset The byte offset.
     * \return Pointer of type T* offset by the specified number of bytes.
     */
    template <typename T>
    constexpr T* byte_offset(T* ptr, const size_t offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(ptr) + offset);
    }

    /**
     * \brief Tests if a specific bit is set in a value.
     *
     * \tparam T The value type (integral).
     * \param value The value to test.
     * \param bit The bit index to test.
     * \return True if the bit is set, false otherwise.
     */
    template <typename T>
    constexpr bool test_bit(T value, size_t bit)
    {
        return (value & (T(1) << bit)) != 0;
    }

    /**
     * \brief Sets a specific bit in a value.
     *
     * \tparam T The value type (integral).
     * \param value Reference to the value to modify.
     * \param bit The bit index to set.
     */
    template <typename T>
    constexpr void set_bit(T& value, size_t bit)
    {
        value |= (T(1) << bit);
    }

    /**
     * \brief Clears a specific bit in a value.
     *
     * \tparam T The value type (integral).
     * \param value Reference to the value to modify.
     * \param bit The bit index to clear.
     */
    template <typename T>
    constexpr void clear_bit(T& value, const size_t bit)
    {
        value &= ~(T(1) << bit);
    }

    /**
     * \brief Swaps the byte order of a 16-bit unsigned integer.
     *
     * \param x The 16-bit value to swap.
     * \return The value with its bytes swapped.
     */
    constexpr uint16_t swap_bytes(const uint16_t x)
    {
        return (x >> 8) | (x << 8);
    }

    /**
     * \brief Swaps the byte order of a 32-bit unsigned integer.
     *
     * \param x The 32-bit value to swap.
     * \return The value with its bytes swapped.
     */
    constexpr uint32_t swap_bytes(const uint32_t x)
    {
        return ((x >> 24) & 0x000000FF) |
               ((x >> 8)  & 0x0000FF00) |
               ((x << 8)  & 0x00FF0000) |
               ((x << 24) & 0xFF000000);
    }

    /**
     * \brief Sets a block of memory to zero.
     *
     * \tparam T The type of the elements.
     * \param ptr Pointer to the memory block.
     * \param count Number of elements to zero.
     */
    template <typename T>
    void mem_zero(T* ptr, const size_t count)
    {
        memset(ptr, 0, sizeof(T) * count);
    }

    /**
     * \brief Fills a block of memory with a specified value.
     *
     * \tparam T The type of the elements.
     * \param ptr Pointer to the memory block.
     * \param count Number of elements to fill.
     * \param value The value to fill with.
     */
    template <typename T>
    void mem_fill(T* ptr, const size_t count, T value)
    {
        for (size_t i = 0; i < count; ++i) ptr[i] = value;
    }
}