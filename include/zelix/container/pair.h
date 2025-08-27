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
// Created by rodrigo on 8/27/25.
//

#pragma once
#include "forward.h"

namespace zelix::stl
{
    /**
     * \brief A lightweight, memory-safe pair container for two values.
     *
     * Stores two objects of (possibly) different types, constructed in-place.
     * Provides accessors for both elements and ensures proper destruction.
     *
     * \tparam K Type of the first element.
     * \tparam V Type of the second element.
     */
    template <typename K, typename V>
    class pair
    {
        alignas(K) unsigned char first_[sizeof(K)];
        alignas(V) unsigned char second_[sizeof(V)];

    public:
        /**
         * \brief Copy constructor.
         *
         * Performs a member-wise copy of the pair.
         */
        constexpr pair(const pair&) = default;

        /**
         * \brief Move constructor.
         *
         * Moves the contents of another pair into this one.
         */
        constexpr pair(pair&&) = default;

        /**
         * \brief Constructs a pair from two values.
         *
         * The values are perfectly forwarded to the underlying types.
         *
         * \param key   The value for the first element.
         * \param value The value for the second element.
         */
        template <class U = K, class W = V>
        pair(U &&key, W &&value)
        {
            new (first_) K(stl::forward<U>(key));
            new (second_) V(stl::forward<W>(value));
        }

        /**
         * \brief Returns a reference to the first element.
         *
         * \return Reference to the first element.
         */
        K &first()
        {
            return *reinterpret_cast<K *>(first_);
        }

        /**
         * \brief Returns a const reference to the first element.
         *
         * \return Const reference to the first element.
         */
        const K &first() const
        {
            return *reinterpret_cast<const K *>(first_);
        }

        /**
         * \brief Returns a reference to the second element.
         *
         * \return Reference to the second element.
         */
        V &second()
        {
            return *reinterpret_cast<V *>(second_);
        }

        /**
         * \brief Returns a const reference to the second element.
         *
         * \return Const reference to the second element.
         */
        const V &second() const
        {
            return *reinterpret_cast<const V *>(second_);
        }

        /**
         * \brief Destructor.
         *
         * Destroys both elements in the pair.
         */
        ~pair()
        {
            reinterpret_cast<K*>(first_)->~K();
            reinterpret_cast<V*>(second_)->~V();
        }
    };
}