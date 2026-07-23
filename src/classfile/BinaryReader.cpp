#include <classfile/BinaryReader.hpp>

BinaryReader::BinaryReader(const std::string& filename) : stream(filename, std::ios::binary)
{
    if (!stream)
        throw std::runtime_error("Could not open file: " + filename);   
}


/**
 * Reads 1 byte from the file.
 * We will use this to read the constant pool tags.
 */
U1 BinaryReader::readU1()
{
    U1 buffer[1];
    if (!stream.read(reinterpret_cast<char*>(buffer), sizeof(U1)))
    {
        throw std::runtime_error("Unexpected end of file.");
    }  
    return buffer[0];
}


/**
 * Reads 2 bytes from the file.
 * We will use this 3 times to read the minor version,
 * major version, and constant pool count.
 */
U2 BinaryReader::readU2()
{
    U1 buffer[2];
    if (!stream.read(reinterpret_cast<char*>(buffer), sizeof(U2)))
    {
        throw std::runtime_error("Unexpected end of file.");
    }
    return (U2(buffer[0]) << 8) | U2(buffer[1]);
}


/**
 * Readins the first 4 bytes of a class file,
 * which should be a magic number - 0xCAFEBABE.
 */
U4 BinaryReader::readU4()
{
    U1 buffer[4];
    if (!stream.read(reinterpret_cast<char*>(buffer), sizeof(U4)))
    {
        throw std::runtime_error("Unexpected end of file.");
    }
    return (U4(buffer[0]) << 24) | (U4(buffer[1]) << 16) | (U4(buffer[2]) << 8) | U4(buffer[3]);
}

std::vector<U1> BinaryReader::readBytes(std::size_t count)
{
    std::vector<U1> buffer(count);
    if (count > 0 && !stream.read(reinterpret_cast<char*>(buffer.data()), count))
        throw std::runtime_error("Unexpected end of file.");
    return buffer;
}

void BinaryReader::skip(std::size_t bytes)
{
    stream.seekg(bytes, std::ios::cur);
}
