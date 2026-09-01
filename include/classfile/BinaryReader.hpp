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

#include <fstream>
#include <string>
#include "Common.hpp"
#include "Types.hpp"
#include <vector>

class BinaryReader
{
public:
    explicit BinaryReader(const std::string& filename);

    U1 readU1();
    U2 readU2();
    U4 readU4();
    std::vector<U1> readBytes(std::size_t count);
    void skip(std::size_t bytes);

private:
    std::ifstream stream;
};