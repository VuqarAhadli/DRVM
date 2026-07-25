#pragma once

#include <cstring>
#include <Types.hpp>
#include <string>
#include <vector>
#include <memory>
#include "Common.hpp"
#include "BinaryReader.hpp"
#include "ConstantPool.hpp"

class ClassFile
{
public:
    explicit ClassFile(const std::string& filename);

    void dump();
    void saveConstantPoolTags();
    void dumpConstantPoolTags();

    template<typename T>
    T* getConstant(U2 index)
    {
        return static_cast<T*>(constantPool.at(index).get());
    }

private:
    std::string filename;
    U4 magic;
    U2 minorVersion;
    U2 majorVersion;
    U2 constantPoolCount;

    BinaryReader reader;
    std::vector<std::unique_ptr<CPInfo>> constantPool;
};