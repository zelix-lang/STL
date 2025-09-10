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
}