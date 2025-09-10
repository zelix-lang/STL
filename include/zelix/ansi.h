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
// Created by rodrigo on 8/22/25.
//

#pragma once

#define ANSI_RESET "\033[0m"
#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_PURPLE "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"
#define ANSI_BRIGHT_BLACK "\033[90m"
#define ANSI_BRIGHT_RED "\033[91m"
#define ANSI_BRIGHT_GREEN "\033[92m"
#define ANSI_BRIGHT_YELLOW "\033[93m"
#define ANSI_BRIGHT_BLUE "\033[94m"
#define ANSI_BRIGHT_PURPLE "\033[95m"
#define ANSI_BRIGHT_CYAN "\033[96m"
#define ANSI_BRIGHT_WHITE "\033[97m"
#define ANSI_BOLD "\033[1m"
#define ANSI_BOLD_BLUE "\033[1;34m"
#define ANSI_BOLD_GREEN "\033[1;32m"
#define ANSI_BOLD_RED "\033[1;31m"
#define ANSI_BOLD_YELLOW "\033[1;33m"
#define ANSI_BOLD_PURPLE "\033[1;35m"
#define ANSI_BOLD_BRIGHT_BLUE "\033[1;94m"
#define ANSI_BOLD_BRIGHT_GREEN "\033[1;92m"
#define ANSI_BOLD_BRIGHT_RED "\033[1;91m"
#define ANSI_BOLD_BRIGHT_YELLOW "\033[1;93m"
#define ANSI_BOLD_BRIGHT_PURPLE "\033[1;95m"
#define ANSI_UNDERLINE "\033[4m"
#define ANSI_DIM "\e[2m"
#define ANSI_DIM_END "\e[22m"

namespace zelix::stl::ansi
{
    namespace bold
    {
        inline constexpr const char bold[] = ANSI_BOLD;
        inline constexpr const char blue[] = ANSI_BOLD_BLUE;
        inline constexpr const char green[] = ANSI_BOLD_GREEN;
        inline constexpr const char red[] = ANSI_BOLD_RED;
        inline constexpr const char yellow[] = ANSI_BOLD_YELLOW;
        inline constexpr const char purple[] = ANSI_BOLD_PURPLE;

        namespace bright
        {
            inline constexpr const char blue[] = ANSI_BOLD_BRIGHT_BLUE;
            inline constexpr const char green[] = ANSI_BOLD_BRIGHT_GREEN;
            inline constexpr const char red[] = ANSI_BOLD_BRIGHT_RED;
            inline constexpr const char yellow[] = ANSI_BOLD_BRIGHT_YELLOW;
            inline constexpr const char purple[] = ANSI_BOLD_BRIGHT_PURPLE;
        }
    }

    namespace bright
    {
        inline constexpr const char black[] = ANSI_BRIGHT_BLACK;
        inline constexpr const char red[] = ANSI_BRIGHT_RED;
        inline constexpr const char green[] = ANSI_BRIGHT_GREEN;
        inline constexpr const char yellow[] = ANSI_BRIGHT_YELLOW;
        inline constexpr const char blue[] = ANSI_BRIGHT_BLUE;
        inline constexpr const char purple[] = ANSI_BRIGHT_PURPLE;
        inline constexpr const char cyan[] = ANSI_BRIGHT_CYAN;
        inline constexpr const char white[] = ANSI_BRIGHT_WHITE;
    }

    inline constexpr const char reset[] = ANSI_RESET;
    inline constexpr const char black[] = ANSI_BLACK;
    inline constexpr const char red[] = ANSI_RED;
    inline constexpr const char green[] = ANSI_GREEN;
    inline constexpr const char yellow[] = ANSI_YELLOW;
    inline constexpr const char blue[] = ANSI_BLUE;
    inline constexpr const char purple[] = ANSI_PURPLE;
    inline constexpr const char cyan[] = ANSI_CYAN;
    inline constexpr const char white[] = ANSI_WHITE;
    inline constexpr const char underline[] = ANSI_UNDERLINE;
    inline constexpr const char dim[] = ANSI_DIM;
    inline constexpr const char dim_end[] = ANSI_DIM_END;
}