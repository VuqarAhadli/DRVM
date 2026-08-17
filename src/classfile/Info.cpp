#include "classfile/Info.hpp"




FileInfo getFileInfo(const std::string& path)
{
    FileInfo info{};
    info.size = static_cast<U8>(fs::file_size(path));

    auto lastTime = fs::last_write_time(path);

    auto sysClock = std::chrono::clock_cast<std::chrono::system_clock>(lastTime);

    std::time_t timeT = std::chrono::system_clock::to_time_t(sysClock);
    std::tm timeStruct = *std::localtime(&timeT);

    std::error_code errorCode;
    fs::path fullPath = fs::canonical(path,errorCode);

    if(errorCode)
    {
        fullPath = fs::absolute(path, errorCode);
    }

    info.fullPath = fullPath.string();


    std::ostringstream oss;
    oss << std::put_time(&timeStruct, "%T - %d %B %Y"); /* Format: "16:26:38 - 11 May 2006" */
    info.lastModified = oss.str();

    return info;
}