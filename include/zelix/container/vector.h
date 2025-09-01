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
// Created by rodrigo on 7/29/25.
//

#pragma once
#include <cstdlib>
#include <type_traits>
#include "zelix/except/exception.h"
#include "zelix/except/out_of_range.h"
#include "zelix/except/uninitialized_memory.h"
#include "zelix/memory/array_resource.h"
#include "zelix/memory/system_resource.h"

namespace zelix::stl
{
    namespace pmr
    {
        /**
         * @brief A vector implementation that lazily initializes its elements.
         *
         * Only allows appending elements to the end of the vector.
         * Does not allow randomly setting elements at known indices.
         *
         * @tparam T Type of elements stored.
         * @tparam GrowthFactor Factor by which the capacity grows
         * @tparam Allocator Memory allocator to use for storage.
         */
        template <
            typename T,
            double GrowthFactor = 1.8,
            size_t InitialCapacity = 25,
            typename Allocator = memory::system_array_resource<T>,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::array_resource<T>, Allocator>
            >
        >
        class vector
        {
            bool initialized_ = false; ///< Indicates if the internal storage has been initialized.
            T* data;        ///< Pointer to the internal storage array.
            size_t size_ = 0;         ///< Number of elements currently stored.
            size_t capacity_;    ///< Current capacity of the internal storage.

            /**
             * @brief Initializes the internal storage.
             * Allocates memory for the initial capacity.
             */
            void init()
            {
                initialized_ = true;
                capacity_ = InitialCapacity;
                data = Allocator::allocate(capacity_);
            }

            /**
             * @brief Resizes the internal storage when capacity is exceeded.
             * Allocates new memory, moves existing elements, and releases old memory.
             * @param new_size The new capacity for the internal storage.
             */
            void resize(const size_t new_size)
            {
                if (!data)
                {
                    data = Allocator::allocate(new_size);
                    capacity_ = new_size;
                    return;
                }

                data = Allocator::reallocate(data, size_, new_size);
                capacity_ = new_size;
            }

            /**
             * @brief Destroys all elements and releases memory.
             * Calls the destructor for each element and deallocates the internal storage.
             */
            void destroy()
            {
                if (initialized_)
                {
                    // Trivial-copyable optimization
                    if constexpr (!std::is_trivially_copyable_v<T>)
                    {
                        for (size_t i = 0; i < size_; ++i)
                        {
                            data[i].~T(); // Call destructor for each element
                        }
                    }

                    aggressive_destroy(); // Deallocate memory
                }
            }
        public:
            /**
             * @brief Default constructor.
             *
             * Asserts that GrowthFactor is greater than 1.0.
             */
            vector()
                : initialized_(false), data(nullptr), size_(0), capacity_(10)
            {
                static_assert(GrowthFactor > 1.0, "Growth factor must be greater than 1.0");
            }

            vector(vector&& other) noexcept
                : initialized_(other.initialized_)
                , data(other.data)
                , size_(other.size_)
                , capacity_(other.capacity_)
            {
                other.data = nullptr;
                other.initialized_ = false;
                other.size_ = 0;
                other.capacity_ = 0;
            }

            vector(vector& other) noexcept
                : initialized_(other.initialized_)
                , data(other.data)
                , size_(other.size_)
                , capacity_(other.capacity_)
            {
                other.data = nullptr;
                other.initialized_ = false;
                other.size_ = 0;
                other.capacity_ = 0;
            }

            vector& operator=(const vector& other)
            {
                if (this != &other)
                {
                    destroy();
                    initialized_ = other.initialized_;
                    size_ = other.size_;
                    capacity_ = other.capacity_;

                    if (other.data)
                    {
                        data = Allocator::allocate(capacity_);
                        if constexpr (std::is_trivially_copyable_v<T>)
                        {
                            memcpy(data, other.data, sizeof(T) * size_);
                        }
                        else
                        {
                            for (size_t i = 0; i < size_; ++i)
                            {
                                new (&data[i]) T(other.data[i]);
                            }
                        }
                    }
                    else
                    {
                        data = nullptr;
                    }
                }

                return *this;
            }

            /**
             * @brief Appends a copy of the given element to the end of the vector.
             *
             * @param value Element to append.
             */
            template <class U = T>
            void push_back(U &&value)
            {
                emplace_back(stl::forward<decltype(value)>(value));
            }

            /**
             * @brief Appends a copy of the given element to the end of the vector.
             *
             * @param value Element to append.
            */
            template <class U = T>
            void push_back(const U &&value)
            {
                emplace_back(stl::forward<decltype(value)>(value));
            }

            /**
             * @brief Constructs an element in-place at the end of the vector.
             *
             * @tparam Args Argument types for T's constructor.
             * @param args Arguments to forward to T's constructor.
             */
            template <typename... Args>
            void emplace_back(Args&&... args)
            {
#           if defined(__GNUC__) || defined(__clang__)
                if (__builtin_expect(initialized_ == 0, 0))
#           else
                if (!initialized_)
#           endif
                {
                    init();
                }

                if (size_ >= capacity_)
                {
                    resize(static_cast<size_t>(capacity_ * GrowthFactor));
                }

                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    auto val = T(stl::forward<Args>(args)...);
                    data[size_] = val;
                }
                else
                {
                    new (&this->data[size_]) T(stl::forward<Args>(args)...);
                }
                ++size_;
            }

            /**
             * @brief Removes the last element from the vector.
             * Calls the destructor for the last element and decreases the size.
             */
            void pop_back()
            {
#           if defined(__GNUC__) || defined(__clang__)
                if (__builtin_expect(size_ > 0, 1))
#           else
                if (size_ > 0)
#           endif
                {
                    --size_;
                    data[size_].~T(); // Call destructor explicitly

                    // Free memory if we reached 0 elements
                    if (size_ == 0 && initialized_)
                    {
                        destroy();
                        initialized_ = false;
                        data = nullptr;
                        capacity_ = 0;
                    }
                }
            }

            /**
             * @brief Removes the last element from the vector and returns it by move.
             *
             * Decreases the size and moves the last element out of the vector.
             * Does not call the destructor for the removed element.
             *
             * @return T The last element, moved from the vector.
             * @note Use with caution: the destructor is not called for the removed element.
             */
            T pop_back_move() {
                --size_;
                return std::move(data[size_]);
            }

            /**
             * @brief Removes all elements from the vector.
             *
             * Calls the destructor for each element and resets the size to zero.
             */
            void clear()
            {
                if constexpr (!std::is_trivially_copyable_v<T>)
                {
                    for (size_t i = 0; i < size_; ++i)
                    {
                        data[i].~T();
                    }
                }

                size_ = 0;
            }

            /**
             * @brief Returns a pointer to the beginning of the data.
             *
             * @return T* Pointer to the first element.
             */
            T* begin()
            {
                return data;
            }

            /**
             * @brief Returns a pointer to the end of the data.
             *
             * @return T* Pointer past the last element.
             */
            T* end()
            {
                return data + size_;
            }

            /**
             * @brief Returns a pointer to the beginning of the data.
             *
             * @return T* Pointer to the first element.
             */
            [[nodiscard]] const T* begin() const
            {
                return data;
            }

            /**
             * @brief Returns a pointer to the end of the data.
             *
             * @return T* Pointer past the last element.
             */
            [[nodiscard]] const T* end() const
            {
                return data + size_;
            }

            /**
             * @brief Move assignment operator.
             *
             * Transfers ownership of resources from another vector.
             *
             * @param other The vector to move from.
             * @return vector& Reference to this vector.
             */
            vector& operator=(vector&& other) noexcept
            {
                if (this != &other)
                {
                    // Clean up current resources
                    destroy();

                    // Move resources from other
                    this->initialized_ = other.initialized_;
                    this->data = other.data;
                    this->size_ = other.size_;
                    this->capacity_ = other.capacity_;

                    // Reset the other vector
                    other.initialized_ = false;
                    other.data = nullptr;
                    other.size_ = 0;
                    other.capacity_ = 0;
                }

                return *this;
            }

            /**
             * @brief Accesses the element at the given index.
             *
             * @param index Index of the element.
             * @return T& Reference to the element.
             * @throws except::exception If accessed before initialization.
             * @throws except::exception If index is out of bounds.
             */
            T &operator[](size_t index)
            {
#           if defined(__GNUC__) || defined(__clang__)
                if (__builtin_expect(initialized_ == 0, 0))
#           else
                if (!initialized_)
#           endif
                    throw except::uninitialized_memory("Early access to vector");

                if (index >= size_)
                    throw except::out_of_range("Index out of range");

                return data[index];
            }

            /**
             * @brief Accesses the element at the given index (const overload).
             *
             * @param index Index of the element.
             * @return T& Reference to the element.
             * @throws except::exception If accessed before initialization.
             * @throws except::exception If index is out of bounds.
             */
            T &operator[](size_t index) const
            {
#           if defined(__GNUC__) || defined(__clang__)
                if (__builtin_expect(initialized_ == 0, 0))
#           else
                if (!initialized_)
#           endif
                    throw except::uninitialized_memory("Early access to vector");

                if (index >= size_)
                    throw except::out_of_range("Index out of range");

                return data[index];
            }

            /**
             * @brief Returns the number of elements in the vector.
             *
             * @return size_t Number of elements.
             */
            [[nodiscard]] size_t size() const
            {
                return size_;
            }

            /**
             * @brief Returns the current capacity of the vector.
             *
             * @return size_t Capacity.
             */
            [[nodiscard]] size_t capacity() const
            {
                return capacity_;
            }

            /**
             * @brief Returns the number of elements in the vector.
             *
             * @return size_t Number of elements.
             */
            [[nodiscard]] size_t size()
            {
                return size_;
            }

            /**
             * @brief Returns the current capacity of the vector.
             *
             * @return size_t Capacity.
             */
            [[nodiscard]] size_t capacity()
            {
                return capacity_;
            }

            /**
             * @brief Returns a pointer to the internal data.
             *
             * @return T* Pointer to data.
             */
            T *ptr()
            {
                return data;
            }

            /**
             * @brief Shrinks the capacity to fit the current size.
             * Reduces memory usage if possible.
             */
            void shrink_to_fit()
            {
                if (size_ < capacity_)
                {
                    resize(size_);
                }
            }

            /**
             * @brief Reserves capacity for at least new_capacity elements.
             * Increases the internal storage if needed.
             * @param new_capacity The minimum capacity to reserve.
             */
            void reserve(const size_t new_capacity)
            {
                if (new_capacity > capacity_)
                {
                    resize(new_capacity);
                    initialized_ = true;
                }
            }

            /**
             * @brief Returns a reference to the element at the specified index.
             *
             * @param index The index of the element to access.
             * @return T& Reference to the element at the given index.
             * @throws except::exception If the index is out of range.
             */
            T &ref_at(const size_t index)
            {
#           if defined(__GNUC__) || defined(__clang__)
                if (__builtin_expect(initialized_ == 0, 0))
#           else
                if (!initialized_)
#           endif
                    throw except::uninitialized_memory("Early access to vector");

                if (index >= size_)
                    throw except::out_of_range("Index out of range");

                return data[index];
            }

            /**
             * @brief Returns a reference to the last element in the vector.
             * @return T& Reference to the last element.
             * @throws except::exception If the vector is empty.
             */
            T& back() {
                if (size_ == 0) throw except::out_of_range("back() on empty vector");
                return data[size_ - 1];
            }

            /**
             * @brief Checks if the vector is empty (const version).
             *
             * @return true if the vector contains no elements, false otherwise.
             */
            [[nodiscard]] bool empty() const
            {
                return size_ == 0; // Check if the vector is empty
            }

            /**
             * @brief Checks if the vector is empty.
             *
             * @return true if the vector contains no elements, false otherwise.
             */
            [[nodiscard]] bool empty()
            {
                return size_ == 0; // Check if the vector is empty
            }

            /**
             * @brief Checks if the vector has been initialized.
             *
             * @return true if initialized, false otherwise.
             */
            [[nodiscard]] bool initialized() const
            {
                return initialized_;
            }

            /**
             * @brief Sets the internal size of the vector to the specified value.
             *
             * This function directly sets the size\_ variable without modifying the underlying data.
             * Use with caution: it does not construct or destroy elements.
             *
             * @param n The new size to set.
             */
            void calibrate(const size_t n)
            {
                size_ = n;
            }

            /**
             * @brief Aggressively destroys the vector without calling destructors.
             *
             * This function deallocates the internal storage without calling the destructors
             * of the contained elements. Use with extreme caution, as it may lead to resource leaks
             * if the elements manage resources that require proper cleanup.
             */
            void aggressive_destroy()
            {
                if (!initialized_ || !data)
                {
                    return;
                }

                Allocator::deallocate(data); // Deallocate memory
                data = nullptr;
            }

            /**
             * @brief Destructor. Destroys all elements and releases memory.
             */
            ~vector()
            {
                destroy();
            }
        };
    }

    template <typename T, double GrowthFactor = 1.8, size_t InitialCapacity = 25>
    using vector = pmr::vector<T, GrowthFactor, InitialCapacity, memory::system_array_resource<T>>; ///< Default vector type using the default allocator
}