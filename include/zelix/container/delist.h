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
         * @brief Internal node element for delist.
         * @tparam T Type of the stored value.
         */
        template <typename T>
        class __delist_el
        {
        public:
            T data;
            __delist_el *next = nullptr;
            __delist_el *prev = nullptr;

            __delist_el() = default;

            template <typename ... Args>
            explicit __delist_el(Args&&... args)
                : data(stl::forward<Args>(args)...)
            {}
        };

        /**
         * @brief Double-ended linked list with custom allocator support.
         * @tparam T Type of the stored value.
         * @tparam Allocator Allocator type for node management.
         */
        template <
            typename T,
            typename Allocator = memory::system_resource<__delist_el<T>>,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::resource<__delist_el<T>>, Allocator>
            >
        >
        class delist
        {
            size_t len = 0;
            __delist_el<T> *head = nullptr;
            __delist_el<T> *tail = nullptr;

        public:
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
                new_node->prev = nullptr;
                if (head) head->prev = new_node;
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
             * @brief Destructor. Clears the list and deallocates all nodes.
             */
            ~delist()
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
    using delist = pmr::delist<T>;
}