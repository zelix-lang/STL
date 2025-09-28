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

#include "except/element_not_found.h"
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

            __rb_node(const T &k, const bool red, __rb_node *l, __rb_node *r, __rb_node *p) :
                key(k), red(red), left(l), right(r), parent(p)
            {
            }
        };

        template<typename Key, typename Value>
        struct __rb_pair
        {
            Key key;
            Value value;
            bool red;
            __rb_pair *left, *right, *parent;

            __rb_pair(const Key &k, const Value &v, const bool red, __rb_pair *l, __rb_pair *r, __rb_pair *p) :
                key(k), value(v), red(red), left(l), right(r), parent(p)
            {
            }
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
            typename Value = float,
            bool IsPair = false,
            double DestructorQueueGrowthFactor = 1.8,
            int DestructorQueueInitialCapacity = 25,
            typename ChildrenAllocator = memory::monotonic_resource<
                std::conditional_t<
                    IsPair,
                    __rb_pair<T, Value>,
                    __rb_node<T>
                >
            >,
            typename DestructorQueueAllocator = memory::system_array_resource<
                std::conditional_t<
                    IsPair,
                    __rb_pair<T, Value> *,
                    __rb_node<T> *
                >
            >,
            typename = std::enable_if_t<
                std::is_base_of_v<
                memory::resource<
                    std::conditional_t<
                        IsPair,
                        __rb_pair<T, Value>,
                        __rb_node<T>
                    >
                >,
                ChildrenAllocator>
            >
            // No SFINAE check for DestructorQueueAllocator necessary
            // since vector<> already does that
        >
        class rb_tree
        {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = std::conditional_t<IsPair, __rb_pair<T, Value>, __rb_node<T>>;
            using pointer = value_type *;
            using reference = value_type &;
            using iterator_category = std::bidirectional_iterator_tag;

            class iterator
            {
            public:
                iterator(pointer node, pointer nil) : node_(node), nil_(nil) {}

                reference operator*() const
                {
                    return *node_;
                }

                pointer operator->() const
                {
                    return node_;
                }

                iterator &operator++()
                { // inorder successor
                    if (node_->right != nil_)
                    {
                        node_ = node_->right;
                        while (node_->left != nil_)
                            node_ = node_->left;
                    }
                    else
                    {
                        auto y = node_->parent;
                        while (y != nil_ && node_ == y->right)
                        {
                            node_ = y;
                            y = y->parent;
                        }
                        node_ = y;
                    }
                    return *this;
                }

                iterator operator++(int)
                {
                    iterator tmp = *this;
                    ++(*this);
                    return tmp;
                }

                iterator &operator--()
                { // inorder predecessor
                    if (node_->left != nil_)
                    {
                        node_ = node_->left;
                        while (node_->right != nil_)
                            node_ = node_->right;
                    }
                    else
                    {
                        auto y = node_->parent;
                        while (y != nil_ && node_ == y->left)
                        {
                            node_ = y;
                            y = y->parent;
                        }
                        node_ = y;
                    }
                    return *this;
                }

                iterator operator--(int)
                {
                    iterator tmp = *this;
                    --(*this);
                    return tmp;
                }

                bool operator==(const iterator &other) const { return node_ == other.node_; }
                bool operator!=(const iterator &other) const { return node_ != other.node_; }

                pointer get_node() const { return node_; }

            private:
                pointer node_;
                pointer nil_;
            };

            // Constructor: initializes the tree with a sentinel NIL node
            rb_tree() : root_(nullptr)
            {
                // create the sentinel NIL node
                if constexpr (IsPair)
                {
                    nil_ = ChildrenAllocator::allocate(T(), Value(), false, nullptr, nullptr, nullptr);
                }
                else
                {
                    nil_ = ChildrenAllocator::allocate(T(), false, nullptr, nullptr, nullptr);
                }

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
            template<bool B = IsPair, typename = std::enable_if_t<!B>>
            void insert(const T &key)
            {
                unified_insert(ChildrenAllocator::allocate(key, true, nil_, nil_, nullptr));
            }

            template<bool B = IsPair, typename = std::enable_if_t<B>>
            void insert(const T &key, const Value &value)
            {
                unified_insert(ChildrenAllocator::allocate(key, value, true, nil_, nil_, nullptr));
            }

            // Remove a key (if present)
            bool erase(const T &key)
            {
                if (len_ == 0)
                    return false; // Tree is empty
                len_--;

                pointer z = search(key);
                if (z == nil_)
                    return false;
                pointer y = z;
                pointer x = nullptr;
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
            bool contains(const T &key) const
            {
                return search(key) != nil_;
            }

            [[nodiscard]] size_t size() const
            {
                return len_;
            }

            [[nodiscard]] bool empty() const
            {
                return len_ == 0;
            }

            // Find a node with the given key in subtree x
            pointer search(const T &key) const
            {
                pointer x = root_;
                while (x != nil_)
                {
                    if (key < x->key)
                        x = x->left;
                    else if (x->key < key)
                        x = x->right;
                    else
                        return x;
                }

                throw except::element_not_found();
            }

            // Clear all nodes in the subtree rooted at x
            void clear(pointer x)
            {
                if (x == nil_)
                    return;

                // Use a stack to iteratively delete nodes
                vector<pointer, DestructorQueueGrowthFactor, DestructorQueueInitialCapacity, DestructorQueueAllocator>
                        stack;
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

            iterator begin()
            {
                pointer x = root_;
                if (x == nil_)
                    return end();
                while (x->left != nil_)
                    x = x->left;
                return iterator(x, nil_);
            }

            iterator end()
            {
                return iterator(nil_, nil_);
            }

        private:
            pointer root_; // Root of the tree
            pointer nil_; // Sentinel NIL node
            size_t len_; // Number of nodes in the tree

            // Insertion
            void unified_insert(pointer c)
            {
                len_++;
                pointer y = nil_;
                pointer x = root_;
                while (x != nil_)
                {
                    y = x;
                    if (c->key < x->key)
                        x = x->left;
                    else if (x->key < c->key)
                        x = x->right;
                    else
                    { // equal -> ignore duplicate
                        ChildrenAllocator::deallocate(c);
                        return;
                    }
                }

                c->parent = y;
                if (y == nil_)
                    root_ = c;

                else if (c->key < y->key)
                    y->left = c;

                else
                    y->right = c;

                insert_fixup(c);
            }

            // Left-rotate the subtree rooted at x
            void left_rotate(pointer x)
            {
                pointer y = x->right;
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
            void right_rotate(pointer x)
            {
                pointer y = x->left;
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
            void insert_fixup(pointer z)
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
                        if (pointer y = z->parent->parent->left; y->red)
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
            void transplant(pointer u, pointer v)
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
            void delete_fixup(pointer x)
            {
                while (x != root_ && x->red == false)
                {
                    if (x == x->parent->left)
                    {
                        pointer w = x->parent->right;
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
                        pointer w = x->parent->left;
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
            pointer minimum(pointer x) const
            {
                while (x->left != nil_)
                    x = x->left;
                return x;
            }
        };
    } // namespace pmr

    template<typename T>
    using rb_tree = pmr::rb_tree<T>;

    template<typename Key, typename Value>
    using rb_pair = pmr::rb_tree<Key, Value, true>;
} // namespace zelix::stl
