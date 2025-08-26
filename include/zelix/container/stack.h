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
// Created by rodrigo on 8/26/25.
//

#pragma once
#include "zelix/memory/allocator.h"

namespace zelix::stl
{
    namespace pmr
    {
        template <
            typename T,
            template <typename, typename... Args> class Container = delist,
            typename... ContainerArgs
        >
        class stack
        {
            Container<T, ContainerArgs...> list; ///< Underlying delist to hold the stack elements
        public:
            void push(const T& val)
            {
                list.push_back(val);
            }

            void pop()
            {
                list.pop_back();
            }

            T& top()
            {
                return list.back();
            }

            [[nodiscard]] bool empty() const
            {
                return list.empty();
            }

            [[nodiscard]] size_t size() const
            {
                return list.size();
            }
        };
    }

    template <typename T>
    using stack = pmr::stack<T>;
}