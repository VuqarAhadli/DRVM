#include "vm/ClassLoader.hpp"
#include <stdexcept>

ClassLoader::ClassLoader(const std::string& classPath)
    : classPath(classPath)
{
}

std::string ClassLoader::resolvePath(const std::string& className) const
{
    return classPath + "/" + className + ".class";
}

ClassFile* ClassLoader::loadClass(const std::string& className)
{
    auto iter = loadedClasses.find(className);
    
    if (iter != loadedClasses.end())
    {
        return iter->second.get();
    }

    std::string path = resolvePath(className);
    auto classFile = std::make_unique<ClassFile>(path);

    ClassFile* rawFile = classFile.get();
    loadedClasses[className] = std::move(classFile);
    return rawFile;
}