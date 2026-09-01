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

#include <filesystem>
#if not defined(_WIN32)
#include <sys/stat.h>
#endif
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "Common.hpp"
#include "Types.hpp"

namespace fs = std::filesystem;

struct FileInfo
{
    std::string fullPath;
#if not defined(_WIN32)
    U8 size;
    std::string lastModified;
#endif
};

FileInfo getFileInfo(const std::string& path);