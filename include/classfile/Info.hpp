#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "Common.hpp"
#include "Types.hpp"

namespace fs = std::filesystem;

struct FileInfo
{
    U8 size;
    std::string lastModified; 
};

FileInfo getFileInfo(const std::string& path);