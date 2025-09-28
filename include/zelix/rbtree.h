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
#include <type_traits>

#include "memory/monotonic.h"

namespace zelix::stl
{
    namespace pmr
    {
        template<typename T>
        struct __rb_node
        {
            T key;
            bool red;
            __rb_node *left, *right, *parent;
            __rb_node(
                const T &k, const bool red,
                __rb_node *l, __rb_node *r, __rb_node *p
            )
                : key(k), red(red), left(l), right(r), parent(p)
            {}
        };

        /**
         * @brief Red-Black Tree implementation with custom memory resources.
         *
         * @tparam T Key type.
         * @tparam DestructorQueueGrowthFactor Growth factor for the destructor queue.
         * @tparam DestructorQueueInitialCapacity Initial capacity for the destructor queue.
         * @tparam ChildrenAllocator Allocator for tree nodes.
         * @tparam DestructorQueueAllocator Allocator for the destructor queue.
         */
        template<
            typename T,
            double DestructorQueueGrowthFactor = 1.8,
            int DestructorQueueInitialCapacity = 25,
            typename ChildrenAllocator = memory::monotonic_resource<__rb_node<T>>,
            typename DestructorQueueAllocator = memory::system_array_resource<__rb_node<T>*>,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::resource<__rb_node<T>>, ChildrenAllocator>
            >
            // No SFINAE check for DestructorQueueAllocator necessary
            // since vector<> already does that
        >
        class rb_tree
        {
        public:
            // Constructor: initializes the tree with a sentinel NIL node
            rb_tree() : root_(nullptr)
            {
                // create the sentinel NIL node
                nil_ = ChildrenAllocator::allocate(T(), false, nullptr, nullptr, nullptr);
                nil_->left = nil_->right = nil_->parent = nil_;
                root_ = nil_;
            }

            // Destructor: clears the tree and deallocates the NIL node
            ~rb_tree()
            {
                clear(root_);
                ChildrenAllocator::deallocate(nil_);
            }

            // Insert a key (duplicates are ignored)
            void insert(const T &key)
            {
                __rb_node<T> *z = ChildrenAllocator::allocate(key, true, nil_, nil_, nullptr);
                __rb_node<T> *y = nil_;
                __rb_node<T> *x = root_;
                while (x != nil_)
                {
                    y = x;
                    if (z->key < x->key)
                        x = x->left;
                    else if (x->key < z->key)
                        x = x->right;
                    else
                    { // equal -> ignore duplicate
                        ChildrenAllocator::deallocate(z);
                        return;
                    }
                }
                z->parent = y;
                if (y == nil_)
                    root_ = z;
                else if (z->key < y->key)
                    y->left = z;
                else
                    y->right = z;

                insert_fixup(z);
            }

            // Remove a key (if present)
            bool erase(const T &key)
            {
                __rb_node<T> *z = find_node(root_, key);
                if (z == nil_)
                    return false;
                __rb_node<T> *y = z;
                __rb_node<T> *x = nullptr;
                bool y_original_color = y->red;
                if (z->left == nil_)
                {
                    x = z->right;
                    transplant(z, z->right);
                }
                else if (z->right == nil_)
                {
                    x = z->left;
                    transplant(z, z->left);
                }
                else
                {
                    y = minimum(z->right);
                    y_original_color = y->red;
                    x = y->right;
                    if (y->parent == z)
                        x->parent = y;
                    else
                    {
                        transplant(y, y->right);
                        y->right = z->right;
                        y->right->parent = y;
                    }
                    transplant(z, y);
                    y->left = z->left;
                    y->left->parent = y;
                    y->red = z->red;
                }

                ChildrenAllocator::deallocate(z);
                if (!y_original_color)
                    delete_fixup(x);
                return true;
            }

            // Check if a key exists in the tree
            bool contains(const T &key) const { return find_node(root_, key) != nil_; }

        private:
            __rb_node<T> *root_; // Root of the tree
            __rb_node<T> *nil_;  // Sentinel NIL node

            // Left-rotate the subtree rooted at x
            void left_rotate(__rb_node<T> *x)
            {
                __rb_node<T> *y = x->right;
                x->right = y->left;
                if (y->left != nil_)
                    y->left->parent = x;

                y->parent = x->parent;
                if (x->parent == nil_)
                    root_ = y;
                else if (x == x->parent->left)
                    x->parent->left = y;
                else
                    x->parent->right = y;

                y->left = x;
                x->parent = y;
            }

            // Right-rotate the subtree rooted at x
            void right_rotate(__rb_node<T> *x)
            {
                __rb_node<T> *y = x->left;
                x->left = y->right;

                if (y->right != nil_)
                    y->right->parent = x;

                y->parent = x->parent;

                if (x->parent == nil_)
                    root_ = y;
                else if (x == x->parent->right)
                    x->parent->right = y;
                else
                    x->parent->left = y;

                y->right = x;
                x->parent = y;
            }

            // Restore red-black properties after insertion
            void insert_fixup(__rb_node<T> *z)
            {
                while (z->parent->red)
                {
                    if (z->parent == z->parent->parent->left)
                    {
                        if (auto y = z->parent->parent->right; y->red)
                        {
                            // Case 1: uncle is red
                            z->parent->red = false;
                            y->red = false;
                            z->parent->parent->red = true;
                            z = z->parent->parent;
                        }
                        else
                        {
                            if (z == z->parent->right)
                            {
                                // Case 2: z is right child
                                z = z->parent;
                                left_rotate(z);
                            }
                            // Case 3: z is left child
                            z->parent->red = false;
                            z->parent->parent->red = true;
                            right_rotate(z->parent->parent);
                        }
                    }
                    else
                    {
                        if (__rb_node<T> *y = z->parent->parent->left; y->red)
                        {
                            // Case 1: uncle is red
                            z->parent->red = false;
                            y->red = false;
                            z->parent->parent->red = true;
                            z = z->parent->parent;
                        }
                        else
                        {
                            if (z == z->parent->left)
                            {
                                // Case 2: z is left child
                                z = z->parent;
                                right_rotate(z);
                            }
                            // Case 3: z is right child
                            z->parent->red = false;
                            z->parent->parent->red = true;
                            left_rotate(z->parent->parent);
                        }
                    }
                }
                root_->red = false;
            }

            // Replace subtree u with subtree v
            void transplant(__rb_node<T> *u, __rb_node<T> *v)
            {
                if (u->parent == nil_)
                    root_ = v;
                else if (u == u->parent->left)
                    u->parent->left = v;
                else
                    u->parent->right = v;
                v->parent = u->parent;
            }

            // Restore red-black properties after deletion
            void delete_fixup(__rb_node<T> *x)
            {
                while (x != root_ && x->red == false)
                {
                    if (x == x->parent->left)
                    {
                        __rb_node<T> *w = x->parent->right;
                        if (w->red)
                        {
                            // Case 1: sibling is red
                            w->red = false;
                            x->parent->red = true;
                            left_rotate(x->parent);
                            w = x->parent->right;
                        }
                        if (!w->left->red && !w->right->red)
                        {
                            // Case 2: sibling's children are black
                            w->red = true;
                            x = x->parent;
                        }
                        else
                        {
                            if (!w->right->red)
                            {
                                // Case 3: sibling's right child is black
                                w->left->red = false;
                                w->red = true;
                                right_rotate(w);
                                w = x->parent->right;
                            }
                            // Case 4: sibling's right child is red
                            w->red = x->parent->red;
                            x->parent->red = false;
                            w->right->red = false;
                            left_rotate(x->parent);
                            x = root_;
                        }
                    }
                    else
                    {
                        __rb_node<T> *w = x->parent->left;
                        if (w->red)
                        {
                            // Case 1: sibling is red
                            w->red = false;
                            x->parent->red = true;
                            right_rotate(x->parent);
                            w = x->parent->left;
                        }

                        if (!w->right->red && !w->left->red)
                        {
                            // Case 2: sibling's children are black
                            w->red = true;
                            x = x->parent;
                        }
                        else
                        {
                            if (!w->left->red)
                            {
                                // Case 3: sibling's left child is black
                                w->right->red = false;
                                w->red = true;
                                left_rotate(w);
                                w = x->parent->left;
                            }
                            // Case 4: sibling's left child is red
                            w->red = x->parent->red;
                            x->parent->red = false;
                            w->left->red = false;
                            right_rotate(x->parent);
                            x = root_;
                        }
                    }
                }
                x->red = false;
            }

            // Find the node with the minimum key in subtree x
            __rb_node<T> *minimum(__rb_node<T> *x) const
            {
                while (x->left != nil_)
                    x = x->left;
                return x;
            }

            // Find a node with the given key in subtree x
            __rb_node<T> *find_node(__rb_node<T> *x, const T &key) const
            {
                while (x != nil_)
                {
                    if (key < x->key)
                        x = x->left;
                    else if (x->key < key)
                        x = x->right;
                    else
                        return x;
                }
                return nil_;
            }

            // Clear all nodes in the subtree rooted at x
            void clear(__rb_node<T> *x)
            {
                if (x == nil_)
                    return;

                // Use a stack to iteratively delete nodes
                vector<__rb_node<T> *, DestructorQueueGrowthFactor, DestructorQueueInitialCapacity, DestructorQueueAllocator> stack;
                stack.push_back(x);

                // Delete iteratively
                while (!stack.empty())
                {
                    auto node = stack.pop_back_move();

                    if (node->left != nil_)
                        stack.push_back(node->left);
                    if (node->right != nil_)
                        stack.push_back(node->right);

                    ChildrenAllocator::deallocate(node);
                }
            }
        };
    } // namespace pmr

    template <typename T>
    using rb_tree = pmr::rb_tree<T>;
} // namespace zelix::stl
