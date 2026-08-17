#include <filesystem>
#include <sys/stat.h>
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
    U8 size;
    std::string lastModified; 
};

FileInfo getFileInfo(const std::string& path);