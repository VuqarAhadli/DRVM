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

#if defined(_WIN32)

    WIN32_FILE_ATTRIBUTE_DATA fileData{};

    fs::path filePath = fs::u8path(path);

    if (!GetFileAttributesExW(filePath.c_str(), GetFileExInfoStandard, &fileData))
    {
        return info;
    }

    info.size = (static_cast<U8>(fileData.nFileSizeHigh) << 32) | static_cast<U8>(fileData.nFileSizeLow);

    FILETIME localFileTime{};
    SYSTEMTIME systemTime{};

    if (!FileTimeToLocalFileTime(&fileData.ftLastWriteTime, &localFileTime))
    {
        return info;
    }

    if (!FileTimeToSystemTime(&localFileTime, &systemTime))
    {
        return info;
    }

    std::tm timeStruct{};

    timeStruct.tm_sec  = systemTime.wSecond;
    timeStruct.tm_min  = systemTime.wMinute;
    timeStruct.tm_hour = systemTime.wHour;
    timeStruct.tm_mday = systemTime.wDay;
    timeStruct.tm_mon  = systemTime.wMonth - 1;
    timeStruct.tm_year = systemTime.wYear - 1900;

#else

    struct stat fileStat{};

    if (stat(path.c_str(), &fileStat) != 0)
    {
        return info;
    }

    info.size = static_cast<U8>(fs::file_size(path));

    const std::time_t timeT = fileStat.st_mtime;
    const std::tm timeStruct = *std::localtime(&timeT);

#endif

    std::error_code errorCode;
    fs::path fullPath = fs::canonical(path, errorCode);

    if (errorCode)
    {
        fullPath = fs::absolute(path, errorCode);

        if (errorCode)
        {
            return FileInfo{};
        }
    }

    info.fullPath = fullPath.string();
    info.sha256 = sha256(fullPath);

    std::ostringstream oss;

#ifdef _WIN32
    oss << std::put_time(&timeStruct, "%T - %d %B %Y");
#else
    oss << std::put_time(&timeStruct, "%T - %d %B %Y");
#endif

    info.lastModified = oss.str();

    return info;
}
