#include "classfile/ClassFile.hpp"
#include "classfile/BinaryReader.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>


inline ConstantTag readConstantTag(U1 value)
{
    return static_cast<ConstantTag>(value);
}

ClassFile::ClassFile(const std::string& filename)
    : filename(filename), reader(filename)
{
    magic = reader.readU4();

    if (magic != CLASS_MAGIC)
    {
        throw std::runtime_error("Invalid Java class file.");
    }

    minorVersion = reader.readU2();
    majorVersion = reader.readU2();
    constantPoolCount = reader.readU2();

    constantPool.resize(constantPoolCount);


}

void ClassFile::dumpConstantPoolTags()
{
    for (U2 i = 1; i < constantPoolCount; ++i)
    {
      ConstantTag tag = static_cast<ConstantTag>(reader.readU1());

        std::cout << "#" << i
              << " Tag: "
              << static_cast<int>(tag)
              << '\n';
        switch (tag)
        {
            case ConstantTag::Utf8:
                {
                    U2 length = reader.readU2();
                    std::vector<U1> bytes = reader.readBytes(length);
                    std::string utf8String(bytes.begin(), bytes.end());
                    std::cout << "Utf8: " << utf8String << '\n';
                    constantPool[i] = std::make_unique<ConstantUtf8>(
                        utf8String
                    );
                }
                break;
            case ConstantTag::Integer:
                {
                    U4 value = reader.readU4();
                    std::cout << "Integer: " << value << '\n';
                }
                break;
            case ConstantTag::Float:
                {
                    U4 value = reader.readU4();
                    float floatValue;
                    std::memcpy(&floatValue, &value, sizeof(float));
                    std::cout << "Float: " << floatValue << '\n';
                }
                break;
            case ConstantTag::Long:
                {
                    U4 highBytes = reader.readU4();
                    U4 lowBytes = reader.readU4();
                    U8 longValue = (static_cast<U8>(highBytes) << 32) | lowBytes;
                    std::cout << "Long: " << longValue << '\n';
                }
                ++i;  /* Long/Double take two constant pool entries */
                break;
            case ConstantTag::Double:
                {
                    U4 highBytes = reader.readU4();
                    U4 lowBytes = reader.readU4();
                    U8 doubleBits = (static_cast<U8>(highBytes) << 32) | lowBytes;
                    double doubleValue;
                    std::memcpy(&doubleValue, &doubleBits, sizeof(double));
                    std::cout << "Double: " << doubleValue << '\n';
                }
                ++i;  /* Long/Double take two constant pool entries */
                break;
            case ConstantTag::Class:
                {
                    U2 nameIndex = reader.readU2();
                    std::cout << "Class: name_index=" << nameIndex << '\n';
                }
                break;
            case ConstantTag::String:
                {
                    U2 stringIndex = reader.readU2();
                    std::cout << "String: string_index=" << stringIndex << '\n';
                }
                break;
            case ConstantTag::Fieldref:
                {
                    U2 classIndex = reader.readU2();
                    U2 nameAndTypeIndex = reader.readU2();
                    std::cout << "Fieldref: class_index=" << classIndex
                            << " name_and_type_index=" << nameAndTypeIndex << '\n';
                }
                break;
            case ConstantTag::Methodref:
                {
                    U2 classIndex = reader.readU2();
                    U2 nameAndTypeIndex = reader.readU2();
                    std::cout << "Methodref: class_index=" << classIndex
                              << " name_and_type_index=" << nameAndTypeIndex << '\n';
                }
                break;
            case ConstantTag::InterfaceMethodref:
                {
                    U2 classIndex = reader.readU2();
                    U2 nameAndTypeIndex = reader.readU2();
                    std::cout << "InterfaceMethodref: class_index=" << classIndex
                              << " name_and_type_index=" << nameAndTypeIndex << '\n';
                }
                break;
            case ConstantTag::NameAndType:
                {
                    U2 nameIndex = reader.readU2();
                    U2 descriptorIndex = reader.readU2();
                    std::cout << "NameAndType: name_index=" << nameIndex
                              << " descriptor_index=" << descriptorIndex << '\n';
                }
                break;
            case ConstantTag::MethodHandle:
                {
                    U1 referenceKind = reader.readU1();
                    U2 referenceIndex = reader.readU2();
                    std::cout << "MethodHandle: reference_kind=" << static_cast<int>(referenceKind)
                              << " reference_index=" << referenceIndex << '\n';
                }
                break;
            case ConstantTag::MethodType:
                {
                    U2 descriptorIndex = reader.readU2();
                    std::cout << "MethodType: descriptor_index=" << descriptorIndex << '\n';
                }
                break;
            case ConstantTag::Dynamic:
                {
                    U2 bootstrapMethodAttrIndex = reader.readU2();
                    U2 nameAndTypeIndex = reader.readU2();
                    std::cout << "Dynamic: bootstrap_method_attr_index=" << bootstrapMethodAttrIndex
                              << " name_and_type_index=" << nameAndTypeIndex << '\n';
                }
                break;
            case ConstantTag::InvokeDynamic:
                {
                    U2 bootstrapMethodAttrIndex = reader.readU2();
                    U2 nameAndTypeIndex = reader.readU2();
                    std::cout << "InvokeDynamic: bootstrap_method_attr_index=" << bootstrapMethodAttrIndex
                              << " name_and_type_index=" << nameAndTypeIndex << '\n';
                }
                break;
            case ConstantTag::Module:
                {
                    U2 nameIndex = reader.readU2();
                    std::cout << "Module: name_index=" << nameIndex << '\n';
                }
                break;
            case ConstantTag::Package:
                {
                    U2 nameIndex = reader.readU2();
                    std::cout << "Package: name_index=" << nameIndex << '\n';
                }
                break;
            default:
                throw std::runtime_error("Unknown constant pool tag: " + std::to_string(static_cast<int>(tag)));
        }
    }
}

void ClassFile::dump()
{
    std::cout << "Magic:               \x1b[32m0x"
              << std::hex
              << std::uppercase
              << magic
              << "\x1b[0m"
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

    dumpConstantPoolTags();
}