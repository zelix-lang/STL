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
// Created by rodrigo on 8/21/25.
//

#pragma once

#include <iostream>


#include "display.h"
#include "owned_string.h"
#if !defined(_WIN32) && defined(__has_include) && __has_include(<unistd.h>)
#   include <unistd.h>
#   define ZELIX_STL_FULL_IO_SUPPORT
#else
#   if defined(__has_include) && __has_include(<windows.h>)
#       define ZELIX_STL_FULL_IO_SUPPORT
#       include <windows.h>
#   endif
#endif

#ifdef ZELIX_STL_FULL_IO_SUPPORT
#   include "ring_buffer.h"
#   include "zelix/algorithm/ftoi.h"
#   include "zelix/algorithm/itoa.h"
#   include <mutex>
#else
#   include <iostream> // Fallback to standard library for Windows
#endif

namespace zelix::stl
{
#   if defined(ZELIX_STL_FULL_IO_SUPPORT) && defined(_WIN32)
    inline auto stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    inline auto stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
#   endif

#ifndef STDOUT_FILENO
    inline constexpr auto STDOUT_FILENO = 1;
#endif

#ifndef STDERR_FILENO
    inline constexpr auto STDERR_FILENO = 2;
#endif

    namespace pmr
    {
        template <
            int FileDescriptor,
            size_t Capacity,
            bool UseHeap = Capacity >= 256,
            typename Allocator = memory::system_array_resource<char>,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::array_resource<char>, Allocator>
            >
        >
        class ostream
        {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
            ring_buffer<char, Capacity, UseHeap, Allocator> buffer; ///< Ring buffer to hold the output data
#           endif
            void do_write(const char *data, const size_t size)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                // Write data to the file descriptor
                if (size > Capacity)
                {
                    size_t remaining = size;
                    auto ptr = const_cast<char *>(data);

                    // Write in chunks
                    while (remaining > Capacity)
                    {
                        // Write only what fits
                        size_t space = Capacity - buffer.pos();
                        buffer.template write<false>(ptr, space);
                        ptr += space;
                        remaining -= space;

                        // Check if the buffer is full
                        if (buffer.full())
                        {
                            flush();
                        }
                    }

                    return;
                }

                buffer.template write<false>(data, size);
                if (buffer.full())
                {
                    flush();
                }
#           else
                std::cout << data; ///< Use standard output
#           endif
            }

            template <typename T>
            void do_write_integral(T val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>)
                {
                    // Convert floating point numbers to string
                    char f_buffer[64] = {};
                    do_write(f_buffer, algorithm::dtoi(f_buffer, static_cast<double>(val), 2));
                }
                else if constexpr (std::is_integral_v<T>) {
                    char i_buffer[32] = {};
                    const size_t len = algorithm::itoa(val, i_buffer);
                    do_write(i_buffer, len);
                }
                else
                {
                    static_assert(
                        false,
                        "Unsupported type for ostream::do_write_integral"
                    );
                }
#           else
                std::cout << val;
#           endif
            }

        public:
            ostream()
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                : buffer()
#           endif
            {
                if constexpr (FileDescriptor != STDERR_FILENO && FileDescriptor != STDOUT_FILENO)
                {
                    static_assert(
                        false,
                        "Unsupported type for ostream, use STDOUT_FILENO or STDERR_FILENO"
                    );
                }
            }

            void flush()
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                if (buffer.pos() == 0)
                {
                    return;
                }

#               ifdef _WIN32
                if constexpr (FileDescriptor == STDOUT_FILENO)
                {
                    WriteConsoleA(stdout_handle, buffer.ptr(), buffer.pos(), nullptr, nullptr);
                }
                else
                {
                    WriteConsoleA(stderr_handle, buffer.ptr(), buffer.pos(), nullptr, nullptr);
                }
#               else
                write(FileDescriptor, buffer.ptr(), buffer.pos());
#               endif
                buffer.flush(); // Clear the buffer after writing
#           else
                std::cout.flush(); // Use standard output flush for Windows
#           endif
            }

            template <class T = string<>>
            ostream &operator<<(T &&handle)
            {
                do_write(handle.ptr(), handle.size());
                return *this;
            }

            ostream &operator<<(const bool val)
            {
                if (val)
                {
                    do_write("true", 4);
                }
                else
                {
                    do_write("false", 5);
                }

                return *this;
            }

            template <
                class T = display,
                typename = std::enable_if_t<
                   std::is_base_of_v<T, display>
                >
            >
            ostream &operator<<(T &&d)
            {
                auto val = d.serialize();
                do_write(val.c_str(), val.size());
                return *this;
            }

            ostream &operator<<(const short val)
            {
                do_write_integral(val);
                return *this;
            }

            ostream &operator<<(const int val)
            {
                do_write_integral(val);
                return *this;
            }

            ostream &operator<<(const long val)
            {
                do_write_integral(val);
                return *this;
            }

            ostream &operator<<(const long long val)
            {
                do_write_integral(val);
                return *this;
            }

            ostream &operator<<(const unsigned short val)
            {
                do_write_integral(val);
                return *this;
            }

            ostream &operator<<(const unsigned int val)
            {
                do_write_integral(val);
                return *this;
            }

            ostream &operator<<(const unsigned long val)
            {
                do_write_integral(val);
                return *this;
            }

            ostream &operator<<(const unsigned long long val)
            {
                do_write_integral(val);
                return *this;
            }

            ostream &operator<<(const float val)
            {
                do_write_integral(val);
                return *this;
            }

            ostream &operator<<(const double val)
            {
                do_write_integral(val);
                return *this;
            }

            ostream &operator<<(const char *s)
            {
    #       ifdef ZELIX_STL_FULL_IO_SUPPORT
                // Append the string on the fly
                for (size_t i = 0; s[i] != '\0'; ++i)
                {
                    if (buffer.full())
                    {
                        flush(); ///< Flush the buffer if it's full
                    }

                    buffer.emplace_back(s[i]); ///< Add the character to the buffer
                }

    #       else
                std::cout << s; ///< Use standard output for Windows
    #       endif
                return *this;
            }

            ostream &operator<<(const char s)
            {
#       ifdef ZELIX_STL_FULL_IO_SUPPORT
                if (buffer.full())
                {
                    flush(); ///< Flush the buffer if it's full
                }

                buffer.emplace_back(s); ///< Add the character to the buffer
#       else
                std::cout << s; ///< Use standard output for Windows
#       endif
                return *this;
            }

            ~ostream()
            {
                flush();
            }
        };

        template <
            int FileDescriptor,
            size_t Capacity,
            bool UseHeap = Capacity >= 256,
            typename Allocator = memory::system_array_resource<char>,
            typename = std::enable_if_t<
                std::is_base_of_v<memory::array_resource<char>, Allocator>
            >
        >
        class concurrent_ostream : ostream<FileDescriptor, Capacity, UseHeap, Allocator>
        {
#       ifdef ZELIX_STL_FULL_IO_SUPPORT
            // Mutex for thread-safe operations
            std::mutex mutex_;
#       endif

        public:
            concurrent_ostream()
                : ostream<FileDescriptor, Capacity, UseHeap, Allocator>()
            {}

            template <class T = string<>>
            concurrent_ostream &operator<<(T &&handle)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<T>(handle));
                return *this;
            }

            concurrent_ostream &operator<<(const bool val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            template <
                class T = display,
                typename = std::enable_if_t<
                   std::is_base_of_v<T, display>
                >
            >
            concurrent_ostream &operator<<(T &&d)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(d)>(d));
                return *this;
            }

            concurrent_ostream &operator<<(const short val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            concurrent_ostream &operator<<(const int val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            concurrent_ostream &operator<<(const long val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            concurrent_ostream &operator<<(const long long val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            concurrent_ostream &operator<<(const unsigned short val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            concurrent_ostream &operator<<(const unsigned int val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            concurrent_ostream &operator<<(const unsigned long val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            concurrent_ostream &operator<<(const unsigned long long val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            concurrent_ostream &operator<<(const float val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            concurrent_ostream &operator<<(const double val)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(val)>(val));
                return *this;
            }

            concurrent_ostream &operator<<(const char *s)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(s)>(s));
                return *this;
            }

            concurrent_ostream &operator<<(const char s)
            {
#           ifdef ZELIX_STL_FULL_IO_SUPPORT
                std::unique_lock lock(mutex_);
#           endif
                ostream<FileDescriptor, Capacity, UseHeap, Allocator>::
                    operator<<(stl::forward<decltype(s)>(s));
                return *this;
            }
        };
    }

    template <
        int FileDescriptor,
        size_t Capacity,
        bool UseHeap = Capacity >= 256
    >
    using ostream = pmr::ostream<FileDescriptor, Capacity, UseHeap>;

    template <
        int FileDescriptor,
        size_t Capacity,
        bool UseHeap = Capacity >= 256
    >
    using concurrent_ostream = pmr::ostream<FileDescriptor, Capacity, UseHeap>;

#   ifndef _WIN32
    inline constexpr auto endl = "\n"; ///< Newline character for output streams
#   else
    inline constexpr auto endl = "\r\n"; ///< Newline character for output streams on Windows
#   endif
    inline ostream<STDOUT_FILENO, 1024> out; ///< Standard output stream
    inline ostream<STDERR_FILENO, 1024> err; ///< Standard error stream
    inline auto &cout = out; ///< Alias for standard output stream
    inline auto &cerr = err; ///< Alias for standard error stream

#   ifdef ZELIX_STL_USE_CONCURRENT_IO
    inline concurrent_ostream<STDOUT_FILENO, 1024> cstdout; ///< Standard output stream
    inline concurrent_ostream<STDERR_FILENO, 1024> cstderr; ///< Standard error stream
    inline auto &ccout = cstdout; ///< Alias for standard output stream
    inline auto &ccerr = cstderr; ///< Alias for standard error stream
#   endif
}