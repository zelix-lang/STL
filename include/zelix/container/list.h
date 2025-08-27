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
// Created by rodrigo on 8/23/25.
//

#pragma once
#include "delist.h"
#include "forward.h"
#include "zelix/except/out_of_range.h"
#include "zelix/memory/resource.h"
#include "zelix/memory/system_resource.h"

namespace zelix::stl
{
    namespace pmr
    {
        /**
         * @brief Internal node element for list.
         * @tparam T Type of the stored value.
         */
        template <typename T>
        class __list_el
        {
        public:
            T data;
            __list_el *next = nullptr;

            __list_el() = default;

            template <typename ... Args>
            explicit __list_el(Args&&... args)
                : data(stl::forward<Args>(args)...)
            {}
        };

        /* * @brief A singly linked list implementation with custom memory resource support.
         * @tparam T Type of the stored value.
         * @tparam Allocator Memory resource to use for allocations. Defaults to system_resource.
         */
        template <
            typename T,
            typename Allocator = memory::system_resource<__list_el<T>>,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::resource<__list_el<T>>, Allocator>
            >
        >
        class list
        {
            size_t len = 0;
            __list_el<T> *head = nullptr;
            __list_el<T> *tail = nullptr;

        public:
            class iterator
            {
                __list_el<T> *current = nullptr;
                explicit iterator(__list_el<T> *start) : current(start) {}

            public:
                T &operator*()
                {
                    if (!current) throw except::out_of_range("Iterator out of range");
                    return current->data;
                }

                iterator &operator++()
                {
                    if (current) current = current->next;
                    return *this;
                }

                bool operator!=(const iterator &other) const
                {
                    return current != other.current;
                }

                [[nodiscard]] bool has_next() const
                {
                    return current != nullptr;
                }
            };
            /**
             * @brief Insert an element at the front of the list.
             * @param value Value to insert.
             */
            void push_front(T value)
            {
                emplace_front(value);
            }

            /**
             * @brief Insert an element at the back of the list.
             * @param value Value to insert.
             */
            void push_back(T value)
            {
                emplace_back(value);
            }

            /**
         * @brief Construct and insert an element at the front of the list.
         * @tparam Args Argument types for the element constructor.
         * @param args Arguments to forward to the element constructor.
         */
            template <typename ... Args>
            void emplace_front(Args&&... args)
            {
                len++;
                auto new_node = Allocator::allocate(stl::forward<Args>(args)...);
                new_node->next = head;
                head = new_node;
                if (!tail) tail = new_node; // First element
            }

            /**
             * @brief Construct and insert an element at the back of the list.
             * @tparam Args Argument types for the element constructor.
             * @param args Arguments to forward to the element constructor.
             */
            template <typename ... Args>
            void emplace_back(Args&&... args)
            {
                len++;
                auto new_node = Allocator::allocate(stl::forward<Args>(args)...);
                new_node->next = nullptr;
                new_node->prev = tail;
                if (tail) tail->next = new_node;
                tail = new_node;
                if (!head) head = new_node; // First element
            }

            /**
             * @brief Remove the element at the front of the list.
             *        Does nothing if the list is empty.
             */
            void pop_front()
            {
                len--;
                if (!head) return; // Empty delist
                auto old_head = head;
                head = head->next;
                if (head) head->prev = nullptr;
                else tail = nullptr; // Delist is now empty
                Allocator::deallocate(old_head);
            }

            /**
             * @brief Remove the element at the back of the list.
             *        Does nothing if the list is empty.
             */
            void pop_back()
            {
                len--;
                if (!tail) return; // Empty delist
                auto old_tail = tail;
                tail = tail->prev;
                if (tail) tail->next = nullptr;
                else head = nullptr; // Delist is now empty
                Allocator::deallocate(old_tail);
            }

            /**
             * @brief Remove all elements from the list.
             */
            void clear()
            {
                while (head)
                {
                    pop_front();
                }
            }

            /**
             * @brief Access the first element.
             * @return Reference to the first element.
             * @throws except::out_of_range if the list is empty.
             */
            T &front()
            {
                if (!head) throw except::out_of_range("Delist is empty");
                return head->data;
            }

            /**
             * @brief Access the last element.
             * @return Reference to the last element.
             * @throws except::out_of_range if the list is empty.
             */
            T &back()
            {
                if (!tail) throw except::out_of_range("Delist is empty");
                return tail->data;
            }

            /**
             * @brief Returns a pointer to the first element in the list.
             * @return Pointer to the head node, or nullptr if the list is empty.
             */
            __list_el<T> *begin()
            {
                return head;
            }

            /**
             * @brief Returns a pointer to the last element in the list.
             * @return Pointer to the tail node, or nullptr if the list is empty.
             */
            __list_el<T> *end()
            {
                return nullptr;
            }

            /**
             * @brief Returns a pointer to the first element in the list.
             * @return Pointer to the head node, or nullptr if the list is empty.
             */
            __list_el<T> *begin() const
            {
                return head;
            }

            /**
             * @brief Returns a pointer to the last element in the list.
             * @return Pointer to the tail node, or nullptr if the list is empty.
             */
            __list_el<T> *end() const
            {
                return nullptr;
            }

            /**
             * @brief Access element by index.
             * @param index Position of the element.
             * @return Reference to the element at the given index.
             * @throws except::out_of_range if index is out of bounds.
             */
            T &operator[](const size_t index)
            {
                if (index >= len) throw except::out_of_range("Index out of range");

                auto current = head;
                size_t i = 0;
                while (current)
                {
                    if (i == index) return current->data;
                    current = current->next;
                    i++;
                }

                throw except::out_of_range("Index out of range");
            }

            /**
             * @brief Get the number of elements in the list.
             * @return Number of elements.
             */
            [[nodiscard]] size_t size() const
            {
                return len;
            }

            /**
             * @brief Check if the list is empty.
             * @return True if empty, false otherwise.
             */
            [[nodiscard]] bool empty() const
            {
                return len == 0;
            }

            /**
             * @brief Get the number of elements in the list.
             * @return Number of elements.
             */
            [[nodiscard]] size_t size()
            {
                return len;
            }

            /**
             * @brief Check if the list is empty.
             * @return True if empty, false otherwise.
             */
            [[nodiscard]] bool empty()
            {
                return len == 0;
            }

            /**
             * @brief Removes the element at the specified index from the list.
             *
             * This function removes the element at position \p n in the list. If the index is out of range,
             * an except::out_of_range exception is thrown. The function updates the head and tail pointers
             * as necessary and deallocates the removed node.
             *
             * @param n The index of the element to remove.
             * @throws except::out_of_range if the index is out of bounds.
            */
            void erase(const size_t n)
            {
                if (n >= len) throw except::out_of_range("Index out of range");
                len--;
                __list_el<T> prev = nullptr;
                auto current = head;

                // Iterate over the list to find the nth node
                size_t i = 0;
                while (current)
                {
                    if (i == n)
                    {
                        // Edge case: removing head
                        if (prev != nullptr)
                        {
                            prev->next = current->next;
                            break;
                        }

                        head = current->next;
                        break;
                    }

                    prev = current;
                    current = current->next;
                    i++;
                }

                // Update the tail accordingly
                if (current == tail)
                {
                    tail = prev;
                }

                // Deallocate the removed node
                Allocator::deallocate(current);
            }

            /**
             * @brief Returns an iterator to the beginning of the list.
             * @return Iterator pointing to the head node.
             */
            iterator it()
            {
                return iterator(head);
            }

            /**
             * @brief Destructor. Clears the list and deallocates all nodes.
             */
            ~list()
            {
                clear();
            }
        };
    }

    /**
     * @brief Alias for pmr::delist using the default allocator.
     * @tparam T Type of the stored value.
     */
    template <typename T>
    using list = pmr::list<T>;
}