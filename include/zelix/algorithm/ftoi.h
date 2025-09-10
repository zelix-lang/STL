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
#include <cstring>

#include "zelix/string_utils.h"

namespace zelix::stl::algorithm {
    inline size_t dtoi(char* buffer, double value, int decimals)
    {
        if (decimals < 0) decimals = 0;

        // Handle negative numbers
        const bool isNegative = value < 0;
        if (isNegative) value = -value;

        // Get integer part
        auto int_part = static_cast<long long>(value);

        // Get fractional part
        double frac_part = value - int_part;

        // Handle rounding
        double rounding = 0.5;
        for (int i = 0; i < decimals; ++i) rounding /= 10.0;
        frac_part += rounding;
        if (frac_part >= 1.0) {
            int_part += 1;
            frac_part -= 1.0;
        }

        // Convert integer part to string
        char int_buffer[32];
        int i = 0;
        if (int_part == 0) {
            int_buffer[i++] = '0';
        } else {
            long long tmp = int_part;
            while (tmp > 0) {
                int_buffer[i++] = '0' + (tmp % 10);
                tmp /= 10;
            }
            // Reverse the string
            for (int j = 0; j < i/2; ++j) {
                const char t = int_buffer[j];
                int_buffer[j] = int_buffer[i-1-j];
                int_buffer[i-1-j] = t;
            }
        }
        int_buffer[i] = '\0';

        // Start filling the main buffer
        int pos = 0;
        if (isNegative) buffer[pos++] = '-';
        strcpy(buffer + pos, int_buffer);
        pos += str::len(int_buffer);

        // Add decimal point and fractional digits
        if (decimals > 0) {
            buffer[pos++] = '.';
            for (int d = 0; d < decimals; ++d) {
                frac_part *= 10;
                const int digit = static_cast<int>(frac_part);
                buffer[pos++] = '0' + digit;
                frac_part -= digit;
            }
        }

        buffer[pos] = '\0';
        return pos; // Return the length of the resulting string
    }
}
