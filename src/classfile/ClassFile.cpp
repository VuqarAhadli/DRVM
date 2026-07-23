#include "classfile/ClassFile.hpp"
#include "classfile/BinaryReader.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>


/**
 * Reads 1 byte from the file.
 * We will use this to read the constant pool tags.
 */
static U1 readU1(std::ifstream& file)
{
    U1 buffer[1];

    if (!file.read(reinterpret_cast<char*>(buffer), sizeof(U1)))
        throw std::runtime_error("Failed to read u1");

    return buffer[0];
}


/**
 * Reads 2 bytes from the file.
 * We will use this 3 times to read the minor version,
 * major version, and constant pool count.
 */
static U2 readU2(std::ifstream& file)
{
    U1 buffer[2];

    if (!file.read(reinterpret_cast<char*>(buffer), sizeof(U2)))
        throw std::runtime_error("Failed to read u2");

    return (U2(buffer[0]) << 8) |
            U2(buffer[1]);
}


/**
 * Reading the first 4 bytes of a class file,
 * which should be a magic number - 0xCAFEBABE.
 */
static U4 readU4(std::ifstream& file)
{
    U1 buffer[4];

    if (!file.read(reinterpret_cast<char*>(buffer), sizeof(U4)))
        throw std::runtime_error("Failed to read u4");

    return (U4(buffer[0]) << 24) | 
           (U4(buffer[1]) << 16) |
           (U4(buffer[2]) << 8)  |
            U4(buffer[3]);
}

inline ConstantTag readConstantTag(U1 value)
{
    return static_cast<ConstantTag>(value);
}

ClassFile::ClassFile(const std::string& filename) : filename(filename)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file)
        throw std::runtime_error("Could not open class file.");

    magic = readU4(file);

    if (magic != CLASS_MAGIC)
    {
        throw std::runtime_error("Invalid Java class file.");
    }

    minorVersion = readU2(file);
    majorVersion = readU2(file);
    constantPoolCount = readU2(file);

}

void ClassFile::dumpConstantPoolTags(std::ifstream& file)
const
{
    for (U2 i = 1; i < constantPoolCount; ++i)
    {
      ConstantTag tag = static_cast<ConstantTag>(readU1(file));

        std::cout << "#" << i
              << " Tag: "
              << static_cast<int>(tag)
              << '\n';
        switch (tag)
        {
            case ConstantTag::Utf8:
                {
                    U2 length = readU2(file);
                    std::vector<U1> bytes(length);
                    if (!file.read(reinterpret_cast<char*>(bytes.data()), length))
                        throw std::runtime_error("Failed to read Utf8 bytes");
                    std::string utf8String(bytes.begin(), bytes.end());
                    std::cout << "Utf8: " << utf8String << '\n';
                }
                break;
            case ConstantTag::Integer:
                {
                    U4 value = readU4(file);
                    std::cout << "Integer: " << value << '\n';
                }
                break;
            case ConstantTag::Float:
                {
                    U4 value = readU4(file);
                    float floatValue;
                    std::memcpy(&floatValue, &value, sizeof(float));
                    std::cout << "Float: " << floatValue << '\n';
                }
                break;
            case ConstantTag::Long:
                {
                    U4 highBytes = readU4(file);
                    U4 lowBytes = readU4(file);
                    U8 longValue = (static_cast<U8>(highBytes) << 32) | lowBytes;
                    std::cout << "Long: " << longValue << '\n';
                }
                ++i;  /* Long/Double take two constant pool entries */
                break;
            case ConstantTag::Double:
                {
                    U4 highBytes = readU4(file);
                    U4 lowBytes = readU4(file);
                    U8 doubleBits = (static_cast<U8>(highBytes) << 32) | lowBytes;
                    double doubleValue;
                    std::memcpy(&doubleValue, &doubleBits, sizeof(double));
                    std::cout << "Double: " << doubleValue << '\n';
                }
                ++i;  /* Long/Double take two constant pool entries */
                break;
            case ConstantTag::Class:
                {
                    U2 nameIndex = readU2(file);
                    std::cout << "Class: name_index=" << nameIndex << '\n';
                }
                break;
            case ConstantTag::String:
                {
                    U2 stringIndex = readU2(file);
                    std::cout << "String: string_index=" << stringIndex << '\n';
                }
                break;
            case ConstantTag::Fieldref:
                {
                    U2 classIndex = readU2(file);
                    U2 nameAndTypeIndex = readU2(file);
                    std::cout << "Fieldref: class_index=" << classIndex
                            << " name_and_type_index=" << nameAndTypeIndex << '\n';
                }
                break;
            case ConstantTag::Methodref:
                {
                    U2 classIndex = readU2(file);
                    U2 nameAndTypeIndex = readU2(file);
                    std::cout << "Methodref: class_index=" << classIndex
                              << " name_and_type_index=" << nameAndTypeIndex << '\n';
                }
                break;
            case ConstantTag::InterfaceMethodref:
                {
                    U2 classIndex = readU2(file);
                    U2 nameAndTypeIndex = readU2(file);
                    std::cout << "InterfaceMethodref: class_index=" << classIndex
                              << " name_and_type_index=" << nameAndTypeIndex << '\n';
                }
                break;
            case ConstantTag::NameAndType:
                {
                    U2 nameIndex = readU2(file);
                    U2 descriptorIndex = readU2(file);
                    std::cout << "NameAndType: name_index=" << nameIndex
                              << " descriptor_index=" << descriptorIndex << '\n';
                }
                break;
            case ConstantTag::MethodHandle:
                {
                    U1 referenceKind = readU1(file);
                    U2 referenceIndex = readU2(file);
                    std::cout << "MethodHandle: reference_kind=" << static_cast<int>(referenceKind)
                              << " reference_index=" << referenceIndex << '\n';
                }
                break;
            case ConstantTag::MethodType:
                {
                    U2 descriptorIndex = readU2(file);
                    std::cout << "MethodType: descriptor_index=" << descriptorIndex << '\n';
                }
                break;
            case ConstantTag::Dynamic:
                {
                    U2 bootstrapMethodAttrIndex = readU2(file);
                    U2 nameAndTypeIndex = readU2(file);
                    std::cout << "Dynamic: bootstrap_method_attr_index=" << bootstrapMethodAttrIndex
                              << " name_and_type_index=" << nameAndTypeIndex << '\n';
                }
                break;
            case ConstantTag::InvokeDynamic:
                {
                    U2 bootstrapMethodAttrIndex = readU2(file);
                    U2 nameAndTypeIndex = readU2(file);
                    std::cout << "InvokeDynamic: bootstrap_method_attr_index=" << bootstrapMethodAttrIndex
                              << " name_and_type_index=" << nameAndTypeIndex << '\n';
                }
                break;
            case ConstantTag::Module:
                {
                    U2 nameIndex = readU2(file);
                    std::cout << "Module: name_index=" << nameIndex << '\n';
                }
                break;
            case ConstantTag::Package:
                {
                    U2 nameIndex = readU2(file);
                    std::cout << "Package: name_index=" << nameIndex << '\n';
                }
                break;
            default:
                throw std::runtime_error("Unknown constant pool tag: " + std::to_string(static_cast<int>(tag)));
        }
    }
}

void ClassFile::dump() const
{
    std::cout << "Magic:               0x"
              << std::hex
              << std::uppercase
              << magic
              << '\n'
              << "Minor Version:       "
              << std::dec
              << minorVersion
              << '\n'
              << "Major Version:       "
              << majorVersion
              << '\n'
              << "Constant Pool Count: "
              << constantPoolCount - 1
              << '\n';        

    std::ifstream file(filename, std::ios::binary);

    if (!file)
        throw std::runtime_error("Could not reopen class file.");

    file.seekg(10, std::ios::beg);  
    /**
     * Skipping the first 10 bytes of data.
     * 4 bytes for magic, 2 bytes for minor version,
     * 2 bytes for major version, and 2 bytes for constant pool count.
     */

    dumpConstantPoolTags(file);
}