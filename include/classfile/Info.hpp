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