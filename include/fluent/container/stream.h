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
#include "fluent/container/vector.h"

namespace fluent::container
{
    template <typename T>
    class stream
    {
        vector<T> data_; ///< Vector to hold the stream data
        size_t pos_ = 0; ///< Current position in the stream

    public:
        explicit stream(vector<T> data) : data_(data)
            {}

        void push(const T &value)
        {
            data_.push_back(value); ///< Add a new value to the stream
        }

        optional<T> peek()
        {
            if (pos_ >= data_.size())
            {
                return optional<T>::none(); ///< Return nullopt if no more elements
            }

            return optional<T>::some(data_[pos_]); ///< Return the next element without advancing
        }

        optional<T> next()
        {
            if (pos_ >= data_.size())
            {
                return optional<T>::none(); ///< Return nullopt if no more elements
            }

            return optional<T>::some(data_[pos_++]); ///< Return the next element and advance the position
        }
    };
}