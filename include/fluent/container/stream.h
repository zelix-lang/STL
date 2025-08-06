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

namespace zelix::container
{
    template <typename T>
    class stream
    {
        vector<T> data_; ///< Vector to hold the stream data
        size_t pos_ = 0; ///< Current position in the stream

    public:
        explicit stream(vector<T>& data) :
            data_(container::move(data)) {}

        explicit stream(vector<T>&& data) :
            data_(container::move(data)) {}

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

		optional<T> curr()
        {
            if (pos_ == 0 || pos_ > data_.size())
            {
                return optional<T>::none(); ///< Return nullopt if no current element
            }

            return optional<T>::some(data_[pos_ - 1]); ///< Return the current element
        }

        optional<T> next()
        {
            if (pos_ >= data_.size())
            {
                return optional<T>::none(); ///< Return nullopt if no more elements
            }

            return optional<T>::some(data_[pos_++]); ///< Return the next element and advance the position
        }

		vector<T> &ptr()
        {
            return data_; ///< Return a reference to the underlying vector
        }

        void reset()
        {
            pos_ = 0; ///< Reset the position to the start of the stream
        }

        [[nodiscard]] size_t pos() const
        {
            return pos_; ///< Return the current position in the stream
        }

        void set_pos(const size_t pos)
        {
            if (pos > data_.size())
            {
                throw except::exception("Position out of bounds"); ///< Throw an error if the position is invalid
            }

            pos_ = pos; ///< Set the current position in the stream
        }

        [[nodiscard]] bool empty() const
        {
            return data_.empty(); ///< Check if the stream is empty
        }

        [[nodiscard]] size_t size() const
        {
            return data_.size(); ///< Return the size of the stream
        }
    };
}