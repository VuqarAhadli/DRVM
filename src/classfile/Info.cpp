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

#include "classfile/Info.hpp"


FileInfo getFileInfo(const std::string& path)
{
    FileInfo info{};

#if not defined(_WIN32)
    struct stat fileStat{};

    if(stat(path.c_str(), &fileStat) != 0)
    {
        return info;
    }

    info.size = static_cast<U8>(fs::file_size(path));

    const std::time_t timeT = fileStat.st_mtime;
    const std::tm timeStruct = *std::localtime(&timeT);

#endif

    std::error_code errorCode;
    fs::path fullPath = fs::canonical(path,errorCode);

    if(errorCode)
    {
        fullPath = fs::absolute(path, errorCode);
    }

    info.fullPath = fullPath.string();

#if not defined(_WIN32)
    std::ostringstream oss;
    oss << std::put_time(&timeStruct, "%T - %d %B %Y"); /* Format: "16:26:38 - 11 May 2006" */
    info.lastModified = oss.str();
#endif

    return info;
}


/**
 * *** TODO:
 * *** Use Windows API to access file size & last modified date
 */