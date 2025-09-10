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
// Created by rodrigo on 7/30/25.
//

#pragma once
#include "zelix//vector.h"
#include "zelix//optional.h"

namespace zelix::stl
{
    namespace pmr
    {
        template <
            typename T,
            double GrowthFactor = 1.8,
            size_t InitialCapacity = 25,
            typename Allocator = memory::system_array_resource<T>,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::array_resource<T>, Allocator>
            >
        >
        class stream
        {
            vector<T, GrowthFactor, InitialCapacity, Allocator> data_; ///< Vector to hold the stream data
            size_t pos_ = 0; ///< Current position in the stream

        public:
            explicit stream(vector<T>& data) :
                data_(stl::move(data)) {}

            explicit stream(vector<T>&& data) :
                data_(stl::move(data)) {}

            /**
             * @brief Constructs an element in-place at the end of the vector.
             *
             * @tparam Args Argument types for T's constructor.
             * @param args Arguments to forward to T's constructor.
             */
            template <typename... Args>
            void emplace_back(Args&&... args)
            {
                data_.emplace_back(stl::forward<Args>(args)...);
            }

            void push(const T &value)
            {
                data_.push_back(value); ///< Add a new value to the stream
            }

            optional<T> peek(const size_t n)
            {
                if (pos_ + n >= data_.size())
                {
                    return optional<T>::none(); ///< Return nullopt if no more elements
                }

                return optional<T>::some(data_[pos_ + n]); ///< Return the next element without advancing
            }

            optional<T> peek()
            {
               return peek(0); ///< Peek at the current element without advancing
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

    template <typename T, double GrowthFactor = 1.8, size_t InitialCapacity = 25>
    using stream = pmr::stream<T, GrowthFactor, InitialCapacity>;
}