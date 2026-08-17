#include "classfile/Info.hpp"




FileInfo getFileInfo(const std::string& path)
{
    FileInfo info{};
    info.size = static_cast<U8>(fs::file_size(path));

    auto lastTime = fs::last_write_time(path);

    auto sysClock = std::chrono::clock_cast<std::chrono::system_clock>(lastTime);

    std::time_t timeT = std::chrono::system_clock::to_time_t(sysClock);
    std::tm timeStruct = *std::localtime(&timeT);

    std::ostringstream oss;
    oss << std::put_time(&timeStruct, "%d %b %Y"); // "31 Jul 2006"
    info.lastModified = oss.str();

    return info;
}