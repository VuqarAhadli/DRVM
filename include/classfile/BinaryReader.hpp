#pragma once

#include <fstream>
#include <string>
#include <types.hpp>
#include "common.hpp"
#include "types.hpp"

class BinaryReader
{
public:
    explicit BinaryReader(const std::string& filename);

    U1 readU1();
    U2 readU2();
    U4 readU4();

    void skip(std::size_t bytes);

private:
    std::ifstream stream;
};