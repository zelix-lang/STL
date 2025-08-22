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
#include "zelix/container/owned_string.h"

namespace zelix::stl::algorithm
{
    static inline string itoa(long value)
    {
        // Edge case: when the value is 0
        if (value == 0)
        {
            return "0";
        }

        string result;
        long chars = 0;
        int is_negative = false;

        // Negate the number if needed
        if (value < 0)
        {
            value = -value;
            is_negative = true;
        }

        // Calculate the number of characters we are going to need
        long value_clone = value;
        while (value_clone > 0)
        {
            // Increment the count
            chars++;
            // Decrement the number
            value_clone = value_clone / 10;
        }

        // Add 1 if the number is negative
        if (is_negative)
        {
            chars++;
        }

        // Position the index accordingly
        long index = chars - 1;

        // Put character by character in the buffer
        while (value > 0)
        {
            // Get the digit
            const long digit = value % 10;

            // Put the character
            result[index] = (char)('0' + digit);
            index--;

            // Decrement the number
            value = value / 10;
        }

        // Add a negative sign
        if (is_negative)
        {
            result[0] = '-';
        }

        return result;
    }
}