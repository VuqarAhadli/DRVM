#pragma once

#include <cstring>
#include <Types.hpp>
#include <string>
#include <vector>
#include <memory>
#include "Common.hpp"

class ClassFile
{
public:
    explicit ClassFile(const std::string& filename);

    void dump() const;
    void dumpConstantPoolTags(std::ifstream& file) const;

private:
    std::string filename;
    U4 magic;
    U2 minorVersion;
    U2 majorVersion;
    U2 constantPoolCount;

    std::vector<std::unique_ptr<ConstantTag>> constantPool;
};