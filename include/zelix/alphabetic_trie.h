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
// Created by rodrigo on 9/24/25.
//

#pragma once
#include <cstring>
#include <string>
#include <type_traits>

#include "memory/array_resource.h"
#include "memory/monotonic.h"
#include "memory/system_resource.h"
#include "owned_string.h"

namespace zelix::stl
{
    namespace pmr
    {
        template<
            bool UseHeap = false,
            typename Allocator = memory::system_array_resource<char>,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::array_resource<char>, Allocator>
            >
        >
        class alphabetic_trie_node
        {
            std::conditional_t<
                UseHeap,
                alphabetic_trie_node *,
                alphabetic_trie_node *[26]
            > children;
            bool is_end_of_word = false;

        public:
            alphabetic_trie_node()
            {
                is_end_of_word = false;
                if constexpr (UseHeap)
                {
                    children = Allocator::allocate(sizeof(alphabetic_trie_node *) * 26); // Allocate on heap
                    memset(children, 0, sizeof(alphabetic_trie_node *) * 26);
                }
                else
                {
                    memset(children, 0, sizeof(children));
                }
            }
        };

        template <
            bool OnlyLowercase = false,
            bool UseHeap = false,
            typename InnerAllocator = memory::system_array_resource<char>,
            typename ChildrenAllocator = memory::monotonic_resource<alphabetic_trie_node<UseHeap, InnerAllocator>>,
            typename DestructorQueueAllocator = memory::system_array_resource<alphabetic_trie_node<UseHeap, InnerAllocator>>,
            double DestructorQueueGrowthFactor = 1.8,
            size_t DestructorQueueInitialCapacity = 25,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::array_resource<char>, InnerAllocator>
            >,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::monotonic_resource<alphabetic_trie_node<UseHeap, InnerAllocator>>, ChildrenAllocator>
            >,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::array_resource<alphabetic_trie_node<UseHeap, InnerAllocator>>, DestructorQueueAllocator>
            >
        >
        class alphabetic_trie
        {
            using child = alphabetic_trie_node<UseHeap, InnerAllocator>;

            // Start at the root
            child *root = ChildrenAllocator::allocate();

            [[nodiscard]] static int get_idx(const char ch)
            {
                if constexpr (OnlyLowercase)
                {
                    // Only lowercase letters
                    return ch - 'a';
                }
                else
                {
                    return tolower(ch) - 'a';
                }
            }

        public:
            void insert(const char *word, const size_t n)
            {
                child* current = root;

                // Iterate over each letter
                for (size_t i = 0; i < n; i++)
                {
                    const auto idx = get_idx(word[i]);

                    if (current->children[idx] == nullptr)
                    {
                        current->children[idx] = ChildrenAllocator::allocate();
                    }


                    current = current->children[idx];
                }

                current->is_end_of_word = true;
            }

            void insert(const string<> &str)
            {
                insert(str.ptr(), str.size());
            }

            void insert(const std::string &str)
            {
                insert(str.data(), str.size());
            }

            bool search(const char *word, const size_t n)
            {
                child* current = root;

                // Iterate over each letter
                for (size_t i = 0; i < n; i++) {
                    const auto idx = get_idx(word[i]);

                    if (current->children[idx] == nullptr)
                    {
                        return false; // Character path not found
                    }

                    current = current->children[idx];
                }

                return current->is_end_of_word; // Check if the end node marks a complete word
            }

            void search(const string<> &str)
            {
                search(str.ptr(), str.size());
            }

            void search(const std::string &str)
            {
                search(str.data(), str.size());
            }

            bool starts_with(const char *word, const size_t n)
            {
                child* current = root;
                for (size_t i = 0; i < n; i++) {
                    const int index = get_idx(word[i]);

                    if (current->children[index] == nullptr)
                    {
                        return false; // Prefix path not found
                    }

                    current = current->children[index];
                }

                return true; // Prefix path exists
            }

            ~alphabetic_trie()
            {
                // Destroy all nodes using a queue to avoid deep recursion
                vector<child *, DestructorQueueGrowthFactor, DestructorQueueInitialCapacity, DestructorQueueAllocator> to_delete;
                to_delete.push_back(root);

                // Iterate over the queue
                while (!to_delete.empty())
                {
                    // Get the current node
                    auto node = to_delete.pop_back_move();

                    // Add children
                    for (size_t j = 0; j < 26; ++j)
                    {
                        // Add only children that exist
                        if (node->children[j] != nullptr)
                        {
                            to_delete.push_back(node->children[j]);
                        }
                    }

                    // Delete the node
                    DestructorQueueAllocator::deallocate(node);
                }
            }
        };
    }

    using alphabetic_trie = pmr::alphabetic_trie<>;
    using a_trie = alphabetic_trie;
    using lower_a_trie = pmr::alphabetic_trie<true>;
    using la_trie = pmr::alphabetic_trie<true>;
}