/*
 * DRVM (drvm)
 * Copyright (C) 2026 Vugar Ahadli
 * Contact: vuqarahadli17@gmail.com | vuqar@div.edu.az
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>
#include <string>

using U1 = std::uint8_t;
using U2 = std::uint16_t; // JVM Char
using U4 = std::uint32_t;
using U8 = std::uint64_t;

using S1 = std::int8_t;   // Byte
using S2 = std::int16_t;  // Short
using S4 = std::int32_t;  // Int
using S8 = std::int64_t;  // Long 

using F4 = float;         // Float
using F8 = double;        // Double

using UTF16 = std::u16string;
using UTF32 = std::u32string;

using UTF16Char = char16_t;
using UTF32Char = char32_t;

