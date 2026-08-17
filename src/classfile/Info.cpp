#include "classfile/Info.hpp"


FileInfo getFileInfo(const std::string& path)
{
    FileInfo info{};
    struct stat fileStat{};

    if(stat(path.c_str(), &fileStat) != 0)
    {
        return info;
    }

    info.size = static_cast<U8>(fs::file_size(path));


    const std::time_t timeT = fileStat.st_mtime;
    const std::tm timeStruct = *std::localtime(&timeT);

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