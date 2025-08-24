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
#define ANSI_BOLD_BRIGHT_BLUE "\033[1;94m"
#define ANSI_BOLD_BRIGHT_GREEN "\033[1;92m"
#define ANSI_BOLD_BRIGHT_RED "\033[1;91m"
#define ANSI_BOLD_BRIGHT_YELLOW "\033[1;93m"
#define ANSI_BOLD_BRIGHT_PURPLE "\033[1;95m"
#define ANSI_UNDERLINE "\033[4m"
#define ANSI_DIM "\e[2m"
#define ANSI_DIM_END "\e[22m"

namespace zelix::stl
{
    constexpr auto ansi_reset = ANSI_RESET;
    constexpr auto ansi_black = ANSI_BLACK;
    constexpr auto ansi_red = ANSI_RED;
    constexpr auto ansi_green = ANSI_GREEN;
    constexpr auto ansi_yellow = ANSI_YELLOW;
    constexpr auto ansi_blue = ANSI_BLUE;
    constexpr auto ansi_purple = ANSI_PURPLE;
    constexpr auto ansi_cyan = ANSI_CYAN;
    constexpr auto ansi_white = ANSI_WHITE;
    constexpr auto ansi_bright_black = ANSI_BRIGHT_BLACK;
    constexpr auto ansi_bright_red = ANSI_BRIGHT_RED;
    constexpr auto ansi_bright_green = ANSI_BRIGHT_GREEN;
    constexpr auto ansi_bright_yellow = ANSI_BRIGHT_YELLOW;
    constexpr auto ansi_bright_blue = ANSI_BRIGHT_BLUE;
    constexpr auto ansi_bright_purple = ANSI_BRIGHT_PURPLE;
    constexpr auto ansi_bright_cyan = ANSI_BRIGHT_CYAN;
    constexpr auto ansi_bright_white = ANSI_BRIGHT_WHITE;
    constexpr auto ansi_bold = ANSI_BOLD;
    constexpr auto ansi_bold_bright_blue = ANSI_BOLD_BRIGHT_BLUE;
    constexpr auto ansi_bold_bright_green = ANSI_BOLD_BRIGHT_GREEN;
    constexpr auto ansi_bold_bright_red = ANSI_BOLD_BRIGHT_RED;
    constexpr auto ansi_bold_bright_yellow = ANSI_BOLD_BRIGHT_YELLOW;
    constexpr auto ansi_bold_bright_purple = ANSI_BOLD_BRIGHT_PURPLE;
    constexpr auto ansi_underline = ANSI_UNDERLINE;
    constexpr auto ansi_dim = ANSI_DIM;
    constexpr auto ansi_dim_end = ANSI_DIM_END;
}