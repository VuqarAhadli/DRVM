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
    saveConstantPoolTags();

    saveClassMetadata();

}

std::unique_ptr<AttributeInfo> ClassFile::readAttribute()
{
    U2 nameIndex = reader.readU2();
    U4 length = reader.readU4();

    std::string name = getConstant<ConstantUtf8>(nameIndex)->value;

    if(name == "Code")
    {
        U2 maxStack = reader.readU2();
        U2 maxLocals = reader.readU2();
        U4 codeLength = reader.readU4();
        std::vector<U1> code = reader.readBytes(codeLength);

        U2 exceptionTableLength = reader.readU2();
        std::vector<ExceptionTableEntry> exceptionTable(exceptionTableLength);

        for(auto& entry : exceptionTable)
        {
            entry.startPc = reader.readU2();
            entry.endPc = reader.readU2();
            entry.handlerPc = reader.readU2(); 
            entry.catchType = reader.readU2();
        }

        U2 nestedAttributeCount = reader.readU2();
        std::vector<std::unique_ptr<AttributeInfo>> nestedAttributes(nestedAttributeCount);
        for (auto& attribute : nestedAttributes)
        {
            attribute = readAttribute();
        }

        return std::make_unique<CodeAttribute>(
            nameIndex, length, maxStack, maxLocals,
            std::move(code), std::move(exceptionTable), std::move(nestedAttributes)
        );
    }

    if (name == "ConstantValue")
    {
        U2 constantValueIndex = reader.readU2();
         return std::make_unique<ConstantValueAttribute>(
             nameIndex, length, constantValueIndex
         );
    }

    if (name == "Exceptions")
    {
        U2 numberOfExceptions = reader.readU2();
        std::vector<U2> exceptionIndexTable(numberOfExceptions);
    }
}




void ClassFile::saveClassMetadata()
{
    accessFlags = reader.readU2();
    thisClass = reader.readU2();
    superClass = reader.readU2();
    interfacesCount = reader.readU2();

    interfaces.resize(interfacesCount);
    for (U2 i = 0; i < interfacesCount; ++i)
    {
        interfaces[i] = reader.readU2();
    }
}

void ClassFile::dumpClassMetadata()
{
    std::cout << "\nAccess Flags:    0x" << std::hex << accessFlags << std::dec;
    if (accessFlags & ACC_PUBLIC) std::cout << " PUBLIC";
    if (accessFlags & ACC_FINAL) std::cout << " FINAL";
    if (accessFlags & ACC_SUPER) std::cout << " SUPER";
    if (accessFlags & ACC_INTERFACE) std::cout << " INTERFACE";
    if (accessFlags & ACC_ABSTRACT) std::cout << " ABSTRACT";
    if (accessFlags & ACC_MODULE) std::cout << " MODULE";
    if (accessFlags & ACC_SYNTHETIC) std::cout << " SYNTHETIC";
    if (accessFlags & ACC_ANNOTATION) std::cout << " ANNOTATION";
    if (accessFlags & ACC_ENUM) std::cout << " ENUM";

    std::cout << "\nThis Class:      #" << thisClass; // index in the constant pool
    if (auto* cls = getConstant<ConstantClass>(thisClass))
    {
        auto* name = getConstant<ConstantUtf8>(cls->nameIndex);
        std::cout << " (" << name->value << ")";
    }

    std::cout << "\nSuper Class:     "; // index in the constant pool
    if (superClass == 0)
    {
        std::cout << "(none -> this is java/lang/Object)";
    }
    else
    {
        std::cout << "#" << superClass;
        auto* cls = getConstant<ConstantClass>(superClass);
        if (cls)
        {
            auto* name = getConstant<ConstantUtf8>(cls->nameIndex);
            std::cout << " (" << name->value << ")";
        }
    }
    

    std::cout << "\nInterfaces:      " << interfacesCount << "\n";
    if (interfacesCount == 0)
    {
        std::cout << "  (none)\n";
    }
    else
    {
        for (U2 i = 0; i < interfacesCount; ++i)
        {
            auto* cls = getConstant<ConstantClass>(interfaces[i]);
            auto* name = getConstant<ConstantUtf8>(cls->nameIndex);
            std::cout << "  #" << interfaces[i] << " (" << name->value << ")\n";
        }
    }
}

void ClassFile::saveConstantPoolTags()
{
    for (U2 i = 1; i < constantPoolCount; ++i)
    {
      ConstantTag tag = readConstantTag(reader.readU1());

        switch (tag)
        {
            case ConstantTag::Utf8:
                {
                    U2 length = reader.readU2();
                    std::vector<U1> bytes = reader.readBytes(length);
                    std::string utf8String(bytes.begin(), bytes.end());

                    constantPool[i] = std::make_unique<ConstantUtf8>(
                        utf8String
                    );
                }
                break;
            case ConstantTag::Integer:
                {
                    U4 value = reader.readU4();
                    constantPool[i] = std::make_unique<ConstantInteger>(
                        value
                    );
                }
                break;
            case ConstantTag::Float:
                {
                    U4 value = reader.readU4();
                    float floatValue;
                    std::memcpy(&floatValue, &value, sizeof(float));

                    constantPool[i] = std::make_unique<ConstantFloat>(
                        floatValue
                    );
                }
                break;
            case ConstantTag::Long:
                {
                    U4 highBytes = reader.readU4();
                    U4 lowBytes = reader.readU4();
                    U8 longValue = (static_cast<U8>(highBytes) << 32) | lowBytes;
                    constantPool[i] = std::make_unique<ConstantLong>(
                        longValue
                    );
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
                    constantPool[i] = std::make_unique<ConstantDouble>(
                        doubleValue
                    );
                }
                ++i;  /* Long/Double take two constant pool entries */
                break;
            case ConstantTag::Class:
                {
                    U2 nameIndex = reader.readU2();
                    constantPool[i] = std::make_unique<ConstantClass>(
                        nameIndex
                    );
                }
                break;
            case ConstantTag::String:
                {
                    U2 stringIndex = reader.readU2();
                    constantPool[i] = std::make_unique<ConstantString>(
                        stringIndex
                    );
                }
                break;
            case ConstantTag::Fieldref:
                {
                    U2 classIndex = reader.readU2();
                    U2 nameAndTypeIndex = reader.readU2();

                    
                    constantPool[i] = std::make_unique<ConstantFieldref>(
                        classIndex, nameAndTypeIndex
                    );
                }
                break;
            case ConstantTag::Methodref:
                {
                    U2 classIndex = reader.readU2();
                    U2 nameAndTypeIndex = reader.readU2();

                    
                    constantPool[i] = std::make_unique<ConstantMethodref>(
                        classIndex, nameAndTypeIndex
                    );
                }
                break;
            case ConstantTag::InterfaceMethodref:
                {
                    U2 classIndex = reader.readU2();
                    U2 nameAndTypeIndex = reader.readU2();

                    
                    constantPool[i] = std::make_unique<ConstantInterfaceMethodref>(
                        classIndex, nameAndTypeIndex
                    );
                }
                break;
            case ConstantTag::NameAndType:
                {
                    U2 nameIndex = reader.readU2();
                    U2 descriptorIndex = reader.readU2();

                    
                    constantPool[i] = std::make_unique<ConstantNameAndType>(
                        nameIndex, descriptorIndex
                    );
                }
                break;
            case ConstantTag::MethodHandle:
                {
                    U1 referenceKind = reader.readU1();
                    U2 referenceIndex = reader.readU2();

                    
                    constantPool[i] = std::make_unique<ConstantMethodHandle>(
                        referenceKind, referenceIndex
                    );
                }
                break;
            case ConstantTag::MethodType:
                {
                    U2 descriptorIndex = reader.readU2();

                    
                    constantPool[i] = std::make_unique<ConstantMethodType>(
                        descriptorIndex
                    );
                }
                break;
            case ConstantTag::Dynamic:
                {
                    U2 bootstrapMethodAttrIndex = reader.readU2();
                    U2 nameAndTypeIndex = reader.readU2();
                    
                    
                    constantPool[i] = std::make_unique<ConstantDynamic>(
                        bootstrapMethodAttrIndex, nameAndTypeIndex
                    );
                }
                break;
            case ConstantTag::InvokeDynamic:
                {
                    U2 bootstrapMethodAttrIndex = reader.readU2();
                    U2 nameAndTypeIndex = reader.readU2();

                    
                    constantPool[i] = std::make_unique<ConstantInvokeDynamic>(
                        bootstrapMethodAttrIndex, nameAndTypeIndex
                    );
                }
                break;
            case ConstantTag::Module:
                {
                    U2 nameIndex = reader.readU2();

                    constantPool[i] = std::make_unique<ConstantModule>(
                        nameIndex
                    );
                }
                break;
            case ConstantTag::Package:
                {
                    U2 nameIndex = reader.readU2();

                    constantPool[i] = std::make_unique<ConstantPackage>(
                        nameIndex
                    );
                }
                break;
            default:
                throw std::runtime_error("Unknown constant pool tag: " + std::to_string(static_cast<int>(tag)));
        }
    }
}



void ClassFile::dumpConstantPoolTags()
{
    for (U2 i = 1; i < constantPoolCount; ++i)
    {

        if (!constantPool[i])
        {
            continue;
        }

        switch (constantPool.at(i) -> tag)
        {
            case ConstantTag::Utf8:
                {
                    auto* utf8 = getConstant<ConstantUtf8>(i);
                    std::cout << "#" << i
                              << " Utf8: "
                              << utf8 -> value
                              << "\n";
                }
                break;
            case ConstantTag::Integer:
                {
                    auto* integer = getConstant<ConstantInteger>(i);
                    std::cout << "#" << i
                              << " Integer: "
                              << integer -> value
                              << "\n";

                }
                break;
            case ConstantTag::Float:
            {
                auto* f = getConstant<ConstantFloat>(i);
                std::cout << "#" << i << " Float: " << f->value << "\n";
                break;
            }
            case ConstantTag::Long:
            {
                auto* l = getConstant<ConstantLong>(i);
                std::cout << "#" << i << " Long: " << l->value << "\n";
                ++i;  /* Long/Double take two constant pool entries */
                break;
            }
            case ConstantTag::Double:
            {
                auto* d = getConstant<ConstantDouble>(i);
                std::cout << "#" << i << " Double: " << d->value << "\n";
                ++i;  /* Long/Double take two constant pool entries */
                break;
            }
            case ConstantTag::Class:
            {
                auto* c = getConstant<ConstantClass>(i);
                std::cout << "#" << i << " Class: name_index=" << c->nameIndex << "\n";
                break;
            }
            case ConstantTag::String:
            {
                auto* s = getConstant<ConstantString>(i);
                std::cout << "#" << i << " String: string_index=" << s->stringIndex << "\n";
                break;
            }
            case ConstantTag::Fieldref:
            {
                auto* f = getConstant<ConstantFieldref>(i);
                std::cout << "#" << i << " Fieldref: class_index=" << f->classIndex
                           << " name_and_type_index=" << f->nameAndTypeIndex << "\n";
                break;
            }
            case ConstantTag::Methodref:
            {
                auto* m = getConstant<ConstantMethodref>(i);
                std::cout << "#" << i << " Methodref: class_index=" << m->classIndex
                           << " name_and_type_index=" << m->nameAndTypeIndex << "\n";
                break;
            }
            case ConstantTag::InterfaceMethodref:
            {
                auto* m = getConstant<ConstantInterfaceMethodref>(i);
                std::cout << "#" << i << " InterfaceMethodref: class_index=" << m->classIndex
                           << " name_and_type_index=" << m->nameAndTypeIndex << "\n";
                break;
            }
            case ConstantTag::NameAndType:
            {
                auto* nt = getConstant<ConstantNameAndType>(i);
                std::cout << "#" << i << " NameAndType: name_index=" << nt->nameIndex
                           << " descriptor_index=" << nt->descriptorIndex << "\n";
                break;
            }
            case ConstantTag::MethodHandle:
            {
                auto* mh = getConstant<ConstantMethodHandle>(i);
                std::cout << "#" << i << " MethodHandle: reference_kind="
                           << static_cast<int>(mh->referenceKind)
                           << " reference_index=" << mh->referenceIndex << "\n";
                break;
            }
            case ConstantTag::MethodType:
            {
                auto* mt = getConstant<ConstantMethodType>(i);
                std::cout << "#" << i << " MethodType: descriptor_index="
                           << mt->descriptorIndex << "\n";
                break;
            }
            case ConstantTag::Dynamic:
            {
                auto* dyn = getConstant<ConstantDynamic>(i);
                std::cout << "#" << i << " Dynamic: bootstrap_method_attr_index="
                           << dyn->bootstrapMethodAttrIndex
                           << " name_and_type_index=" << dyn->nameAndTypeIndex << "\n";
                break;
            }
            case ConstantTag::InvokeDynamic:
            {
                auto* inv = getConstant<ConstantInvokeDynamic>(i);
                std::cout << "#" << i << " InvokeDynamic: bootstrap_method_attr_index="
                           << inv->bootstrapMethodAttrIndex
                           << " name_and_type_index=" << inv->nameAndTypeIndex << "\n";
                break;
            }
            case ConstantTag::Module:
            {
                auto* mod = getConstant<ConstantModule>(i);
                std::cout << "#" << i << " Module: name_index=" << mod->nameIndex << "\n";
                break;
            }
            case ConstantTag::Package:
            {
                auto* pkg = getConstant<ConstantPackage>(i);
                std::cout << "#" << i << " Package: name_index=" << pkg->nameIndex << "\n";
                break;
            }
            default:
                throw std::runtime_error(
                    "Unknown constant pool tag: " +
                    std::to_string(static_cast<int>(constantPool[i]->tag)));
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
    
    std::cout << std::dec << std::nouppercase; 

    dumpConstantPoolTags();
    dumpClassMetadata();
}