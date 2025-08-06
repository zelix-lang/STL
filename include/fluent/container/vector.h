/*
        ==== The Fluent Programming Language ====
---------------------------------------------------------
  - This file is part of the Fluent Programming Language
    codebase. Fluent is a fast, statically-typed and
    memory-safe programming language that aims to
    match native speeds while staying highly performant.
---------------------------------------------------------
  - Fluent is categorized as free software; you can
    redistribute it and/or modify it under the terms of
    the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.
---------------------------------------------------------
  - Fluent is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even
    the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public
    License for more details.
---------------------------------------------------------
  - You should have received a copy of the GNU General
    Public License along with Fluent. If not, see
    <https://www.gnu.org/licenses/>.
*/

//
// Created by rodrigo on 7/29/25.
//

#pragma once
#include "optional.h"
#include "move.h"
#include <cstring>
#include <type_traits>
#include <cstdlib>
#include "fluent/except/exception.h"

namespace fluent::container
{
    template <typename T, bool Flag, size_t N>
    struct __fluent_stl_lazy_buffer;

    // Static buffer version
    template <typename T, size_t N>
    struct __fluent_stl_lazy_buffer<T, true, N>
    {
        T buffer[N]; ///< Static buffer for small sizes
        bool stack = true; ///< Indicates if the buffer is using stack memory
    };

    // Dynamic/null buffer version
    template <typename T, size_t N>
    struct __fluent_stl_lazy_buffer<T, false, N>
    {
        // No buffer defined
    };

    /**
     * @brief A vector implementation that lazily initializes its elements.
     *
     * Only allows appending elements to the end of the vector.
     * Does not allow randomly setting elements at known indices.
     *
     * @tparam T Type of elements stored.
     * @tparam GrowthFactor Factor by which the capacity grows.
     * @tparam Hybrid If true, uses a static buffer for small sizes.
     * @tparam StackSize Size of the static buffer if Hybrid is true.
     */
    template <
        typename T,
        double GrowthFactor = 1.618,
        bool Hybrid = false,
        size_t StackSize = 30
    >
    class vector : __fluent_stl_lazy_buffer<T, Hybrid, StackSize>
    {
        bool initialized; ///< Indicates if the internal storage has been initialized.
        T* data;        ///< Pointer to the internal storage array.
        size_t size_;         ///< Number of elements currently stored.
        size_t capacity_;    ///< Current capacity of the internal storage.

        /**
         * @brief Initializes the internal storage.
         * Allocates memory for the initial capacity.
         */
        void init()
        {
            initialized = true;

            // Small-buffer optimization
            if constexpr (Hybrid)
            {
                // Do not initialize heap memory at all
                return;
            }

            // Trivial-copyable optimization
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                // Use malloc directly
                data = static_cast<T*>(malloc(sizeof(T) * capacity_));
            }
            else
            {
                // Use operator new to allocate raw memory
                data = static_cast<T*>(operator new(sizeof(T) * capacity_));
            }
        }

        /**
         * @brief Resizes the internal storage when capacity is exceeded.
         * Allocates new memory, moves existing elements, and releases old memory.
         * @param new_size The new capacity for the internal storage.
         */
        void resize(const size_t new_size)
        {

            // Trivial-copyable optimization
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                if constexpr (Hybrid)
                {
                    // Check if we are on stack memory
                    if (this->stack)
                    {
                        T* new_data = static_cast<T*>(malloc(sizeof(T) * new_size));
                        if (!new_data)
                        {
                            throw except::exception("Memory allocation failed");
                        }

                        memcpy(new_data, data, sizeof(T) * size_);
                        this->stack = false; // Switch to heap memory
                        data = new_data;
                        capacity_ = new_size;
                        return;
                    }
                }

                T* new_data = static_cast<T*>(realloc(data, sizeof(T) * new_size));
                if (!new_data)
                {
                    throw except::exception("Memory allocation failed");
                }

                free(data);
                data = new_data;
                capacity_ = new_size;
                return;
            }

            T* new_data = static_cast<T*>(operator new(sizeof(T) * new_size));

            // Move existing elements to new storage
            for (size_t i = 0; i < size_; ++i)
            {
                new (&new_data[i]) T(container::move(data[i]));
                data[i].~T(); // Call destructor for old element
            }

            operator delete(data);
            data = new_data;
            capacity_ = new_size;
        }

        /**
         * @brief Destroys all elements and releases memory.
         * Calls the destructor for each element and deallocates the internal storage.
         */
        void destroy()
        {
            if (initialized)
            {
                // Trivial-copyable optimization
                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    if constexpr (Hybrid)
                    {
                        if (this->stack)
                        {
                            // No need to free stack memory
                            return;
                        }
                    }

                    // Use free for trivial types
                    free(data);
                }
                else
                {
                    for (size_t i = 0; i < size_; ++i)
                    {
                        data[i].~T(); // Call destructor for each element
                    }

                    if constexpr (Hybrid)
                    {
                        if (this->stack)
                        {
                            // No need to free stack memory
                            return;
                        }
                    }
                    operator delete(data);
                }
            }
        }
    public:
        /**
         * @brief Default constructor.
         *
         * Asserts that GrowthFactor is greater than 1.0.
         */
        vector()
            : initialized(false), data(nullptr), size_(0), capacity_(10)
        {
            static_assert(GrowthFactor > 1.0, "Growth factor must be greater than 1.0");
            if constexpr (Hybrid)
            {
                // Initialize static buffer if Hybrid is true
                data = this->buffer;
                // Set the capacity for stack memory
                capacity_ = StackSize;
            }
        }

        /**
         * @brief Appends a copy of the given element to the end of the vector.
         *
         * @param value Element to append.
         */
        void push_back(T &value)
        {
            if (!initialized)
            {
                init();
            }

            if (size_ >= capacity_)
            {
                resize(static_cast<size_t>(capacity_ * GrowthFactor));
            }

            new (&this->data[size_]) T(container::move(value));
            ++size_;
        }

        /**
         * @brief Appends a copy of the given element to the end of the vector.
         *
         * @param value Element to append.
         */
        void push_back(T &&value)
        {
            if (!initialized)
            {
                init();
            }

            if (size_ >= capacity_)
            {
                resize(static_cast<size_t>(capacity_ * GrowthFactor));
            }

            new (&this->data[size_]) T(container::move(value));
            ++size_;
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
            if (!initialized)
            {
                init();
            }

            if (size_ >= capacity_)
            {
                resize(static_cast<size_t>(capacity_ * GrowthFactor));
            }

            new (&this->data[size_]) T(container::forward<Args>(args)...);
            ++size_;
        }

        /**
         * @brief Removes the last element from the vector.
         * Calls the destructor for the last element and decreases the size.
         */
        void pop_back()
        {
            if (size_ > 0)
            {
                --size_;
                data[size_].~T(); // Call destructor explicitly
            }
        }

        /**
         * @brief Removes all elements from the vector.
         *
         * Calls the destructor for each element and resets the size to zero.
         */
        void clear()
        {
            for (size_t i = 0; i < size_; ++i)
            {
                data[i].~T();
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
         * @brief Deleted copy constructor.
         * Prevents copying of vector instances.
         */
        vector(const vector&) = delete;

        /**
         * @brief Deleted copy assignment operator.
         * Prevents copying of vector instances.
         */
        vector& operator=(const vector&) = delete;

        /**
         * @brief Move constructor.
         *
         * Transfers ownership of resources from another vector.
         *
         * @param other The vector to move from.
         */
        vector(vector&& other) noexcept
        {
            this->initialized = other.initialized;
            this->data = other.data;
            this->size_ = other.size_;
            this->capacity_ = other.capacity_;

            // Also move the stack buffer if Hybrid is true
            if constexpr (Hybrid)
            {
                this->stack = other.stack;
            }

            // Reset the other vector
            other.initialized = false;
            other.data = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
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
                this->initialized = other.initialized;
                this->data = other.data;
                this->size_ = other.size_;
                this->capacity_ = other.capacity_;

                // Reset the other vector
                other.initialized = false;
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
            if (!initialized)
                throw except::exception("Early access to vector");

            if (index >= size_)
                throw except::exception("Index out of range");

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
            if (!initialized)
                throw except::exception("Early access to vector");

            if (index >= size_)
                throw except::exception("Index out of range");

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
                initialized = true;
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
            if (!initialized)
                throw except::exception("Early access to vector");

            if (index >= size_)
                throw except::exception("Index out of range");

            return data[index];
        }

        /**
         * @brief Returns a reference to the last element in the vector.
         * @return T& Reference to the last element.
         * @throws except::exception If the vector is empty.
         */
        T& back() {
            if (size_ == 0) throw except::exception("back() on empty vector");
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
         * @brief Destructor. Destroys all elements and releases memory.
         */
        ~vector()
        {
            destroy();
        }
    };
}