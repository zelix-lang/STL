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
// Created by rodrigo on 9/28/25.
//

#pragma once

#include "rb_tree.h"

namespace zelix::stl
{
    namespace pmr
    {
        template <
            typename Key,
            typename Value,
            double DestructorQueueGrowthFactor = 1.8,
            int DestructorQueueInitialCapacity = 25,
            typename ChildrenAllocator = memory::monotonic_resource<__rb_node<__map_entry<Key, Value>>>,
            typename DestructorQueueAllocator = memory::system_array_resource<__rb_node<__map_entry<Key, Value>>*>
            // No need for SFINAE checks, rb_tree already does that
        >
        class map
        {
            rb_tree<Key, Value, true, DestructorQueueGrowthFactor, DestructorQueueInitialCapacity, ChildrenAllocator, DestructorQueueAllocator> tree_;

        public:
            void insert(const Key& key, const Value& value)
            {
                tree_.insert(key, value);
            }

            bool erase(const Key& key)
            {
                return tree_.erase(key);
            }

            bool contains(const Key& key) const
            {
                return tree_.contains(key);
            }

            Value &get(const Key& key)
            {
                return tree_.find_node(tree_.root_, key);
            }

            // Operator[] for convenience
            Value& operator[](const Key& key)
            {
                return get(key);
            }
        };
    }
}