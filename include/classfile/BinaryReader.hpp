#pragma once

#include <fstream>
#include <string>
#include "Common.hpp"
#include "Types.hpp"
#include <vector>

class BinaryReader
{
public:
    explicit BinaryReader(const std::string& filename);

    U1 readU1();
    U2 readU2();
    U4 readU4();
    std::vector<U1> readBytes(std::size_t count);
    void skip(std::size_t bytes);

private:
    std::ifstream stream;
};