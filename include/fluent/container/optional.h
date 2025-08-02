/*
        ==== The Fluent Programming Language ====
---------------------------------------------------------
  - This file is part of the Fluent Programming Language
    codebase. Fluent is a fast, statically-typed and
    memory-safe programming language that aims to
    match native speeds while staying highly performant.
---------------------------------------------------------
  - Fluent is categorized as free software; you can
    redistribute it and/or modify it under the terms of
    the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.
---------------------------------------------------------
  - Fluent is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even
    the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public
    License for more details.
---------------------------------------------------------
  - You should have received a copy of the GNU General
    Public License along with Fluent. If not, see
    <https://www.gnu.org/licenses/>.
*/

//
// Created by rodrigo on 7/30/25.
//

#pragma once

#include "forward.h"
#include "fluent/except/exception.h"

namespace fluent::container
{
    template <typename T>
    class optional
    {
        alignas(T) unsigned char data_[sizeof(T)]; ///< Storage for the value
        bool has_value = false; ///< Flag to indicate if the optional has a value

    public:
        static constexpr optional none() noexcept
        {
            return optional(); // Return an empty optional
        }

        static constexpr optional some(T val) noexcept
        {
            optional opt; // Create an optional instance
            new (opt.data_) T(val); // Construct the value in place
            opt.has_value = true; // Set the has_value flag
            return opt; // Return the optional with the value
        }

        template <typename... Args>
        static constexpr optional emplace(Args&&... args)
        {
            optional opt; // Create an optional instance
            new (opt.data_) T(forward<Args>(args)...); // Direct construction
            opt.has_value = true;
            return opt; // Return the optional with the value
        }

        [[nodiscard]] bool is_some() const noexcept
        {
            return has_value; // Check if the optional has a value
        }

        [[nodiscard]] bool is_none() const noexcept
        {
            return !has_value; // Check if the optional is empty
        }

        [[nodiscard]] T &get() noexcept(false)
        {
            if (!has_value)
            {
                throw except::exception("Optional has no value"); // Throw an error if no value is present
            }
            return *reinterpret_cast<T *>(data_); // Return the stored value
        }

        [[nodiscard]] const T &get() const noexcept(false)
        {
            if (!has_value)
            {
                throw except::exception("Optional has no value"); // Throw an error if no value is present
            }

            return *reinterpret_cast<const T *>(data_); // Return the stored value
        }
    };
}