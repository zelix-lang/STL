/*
        === The Zelix Programming Language ====
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
// Created by rodrigo on 8/20/25.
//

#pragma once
#include <cstddef>

namespace zelix::container::str
{
    /**
 * Checks if the given string `str` starts with the specified `prefix`.
 *
 * \param str     The string to check.
 * \param prefix  The prefix to look for at the start of `str`.
 * \return        TRUE if `str` starts with `prefix`, FALSE otherwise.
 *
 * Returns FALSE if either `str` or `prefix` is NULL.
 */
    static inline bool has_prefix(
        const char *str,
        const char *prefix
    )
    {
        // Check if the string or prefix is NULL
        if (!str || !prefix)
        {
            return false; // Return false if either is NULL
        }

        // Iterate over the characters of the prefix
        for (size_t i = 0; prefix[i] != '\0'; i++)
        {
            // If the current character in the string does not match the prefix
            if (str[i] != prefix[i])
            {
                return false; // Return false
            }
        }

        // If we reached here, the prefix matches the start of the string
        return true; // Return true
    }
}