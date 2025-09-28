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
        /**
         * \brief A memory-safe, high-performance set implementation using a red-black tree.
         *
         * \tparam Key Type of the key.
         * \tparam DestructorQueueGrowthFactor Growth factor for the destructor queue.
         * \tparam DestructorQueueInitialCapacity Initial capacity for the destructor queue.
         * \tparam ChildrenAllocator Allocator for tree nodes.
         * \tparam DestructorQueueAllocator Allocator for the destructor queue.
         */
        template<
            typename Key,
            double DestructorQueueGrowthFactor = 1.8,
            int DestructorQueueInitialCapacity = 25,
            typename ChildrenAllocator = memory::monotonic_resource<__rb_node<Key>>,
            typename DestructorQueueAllocator = memory::system_array_resource<__rb_node<Key> *>
            // No need for SFINAE checks, rb_tree already does that
        >
        class set
        {
            using tree = rb_tree<
                Key,
                float,
                false,
                DestructorQueueGrowthFactor,
                DestructorQueueInitialCapacity,
                ChildrenAllocator,
                DestructorQueueAllocator
            >;

            tree tree_;

        public:
            /**
             * \brief Inserts a key-value pair into the set.
             * \param key The key to insert.
             */
            void insert(const Key &key)
            {
                tree_.insert(key);
            }

            /**
             * \brief Removes a key (and its value) from the set.
             * \param key The key to remove.
             * \return True if the key was found and removed, false otherwise.
             */
            bool erase(const Key &key)
            {
                return tree_.erase(key);
            }

            /**
             * \brief Checks if the set contains a given key.
             * \param key The key to check.
             * \return True if the key exists, false otherwise.
             */
            bool contains(const Key &key) const
            {
                return tree_.contains(key);
            }

            /**
             * \brief Returns an iterator to the beginning of the set.
             * \return Iterator to the first element.
             */
            tree::iterator begin()
            {
                return tree_.begin();
            }

            /**
             * \brief Returns an iterator to the end of the set.
             * \return Iterator to one past the last element.
             */
            tree::iterator end()
            {
                return tree_.end();
            }

            /**
             * \brief Returns the number of elements in the set.
             * \return The size of the set.
             */
            size_t size()
            {
                return tree_.size();
            }
        };
    } // namespace pmr

    template<typename Key>
    using set = pmr::set<Key>;
} // namespace zelix::stl
