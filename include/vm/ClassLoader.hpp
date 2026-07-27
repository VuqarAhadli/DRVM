#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "classfile/ClassFile.hpp"

/**
 * Loads .class files from disk and caches them by fully-qualified
 * class name, so the VM never loads the same class twice.
 */
class ClassLoader
{
public:
    explicit ClassLoader(const std::string& classPath);


    ClassFile* loadClass(const std::string& className);

private:
    std::string resolvePath(const std::string& className) const;

    std::string classPath;
    std::unordered_map<std::string, std::unique_ptr<ClassFile>> loadedClasses;
};