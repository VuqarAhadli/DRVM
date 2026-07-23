#include <classfile/binaryreader.hpp>

BinaryReader::BinaryReader(const std::string& filename) : stream(filename, std::ios::binary)
{
}

U1 BinaryReader::readU1()
{
    U1 buffer[1];
    stream.read(reinterpret_cast<char*>(buffer), sizeof(U1));
    return buffer[0];
}

U2 BinaryReader::readU2()
{
    U1 buffer[2];
    stream.read(reinterpret_cast<char*>(buffer), sizeof(U2));
    return (U2(buffer[0]) << 8) | U2(buffer[1]);
}

U4 BinaryReader::readU4()
{
    U1 buffer[4];
    stream.read(reinterpret_cast<char*>(buffer), sizeof(U4));
    return (U4(buffer[0]) << 24) | (U4(buffer[1]) << 16) | (U4(buffer[2]) << 8) | U4(buffer[3]);
}

void BinaryReader::skip(std::size_t bytes)
{
    stream.seekg(bytes, std::ios::cur);
}
