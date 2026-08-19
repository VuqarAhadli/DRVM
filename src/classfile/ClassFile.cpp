#include "classfile/ClassFile.hpp"
#include "classfile/BinaryReader.hpp"
#include "classfile/Attribute.hpp"
#include "classfile/Info.hpp"

#include "Opcode.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

std::string to_string(VerificationTypeTag tag)
{
    switch (tag)
    {
        case VerificationTypeTag::Top:
            return std::string(ANSI_FG_BRIGHT_BLACK) + "Top" + ANSI_RESET;

        case VerificationTypeTag::Integer:
            return std::string(INT_COLOUR) + "Integer" + ANSI_RESET;

        case VerificationTypeTag::Float:
            return std::string(FLOAT_COLOUR) + "Float" + ANSI_RESET;

        case VerificationTypeTag::Double:
            return std::string(DOUBLE_COLOUR) + "Double" + ANSI_RESET;

        case VerificationTypeTag::Long:
            return std::string(LONG_COLOUR) + "Long" + ANSI_RESET;

        case VerificationTypeTag::Null:
            return std::string(ANSI_FG_BRIGHT_BLACK) + "Null" + ANSI_RESET;

        case VerificationTypeTag::UninitializedThis:
            return std::string(ANSI_FG_YELLOW) + "UninitializedThis" + ANSI_RESET;

        case VerificationTypeTag::Object:
            return std::string(CLASS_COLOUR) + "Object" + ANSI_RESET;

        case VerificationTypeTag::Uninitialized:
            return std::string(ANSI_FG_YELLOW) + "Uninitialized" + ANSI_RESET;

        default:
            return std::string(ANSI_FG_RED) + "Unknown/Invalid("
                + std::to_string(static_cast<int>(tag)) + ")" + ANSI_RESET;
    }
}

static U2 computeArgsSize(const std::string& descriptor, bool isStatic)
{
    U2 size = isStatic ? 0 : 1;   

    std::size_t i = 1;   /* skip initial ( */
    while (i < descriptor.size() && descriptor[i] != ')')
    {
        char c = descriptor[i];

        if (c == '[')
        {
            while (i < descriptor.size() && descriptor[i] == '[')\
            {
                ++i;
            }
            
            if (i < descriptor.size() && descriptor[i] == 'L')
            {
                while (i < descriptor.size() && descriptor[i] != ';')
                {
                    ++i;
                }
            }
            ++i; /* Skip ; */
            size += 1;
        }
        else if (c == 'L')
        {
            /* Reference type */
            while (i < descriptor.size() && descriptor[i] != ';')
                ++i;
            ++i; /* Skip ; */
            size += 1;
        }
        else if (c == 'J' || c == 'D')
        {
            /* long / double 2 slots */
            size += 2;
            ++i; /* Skip ; */
        }
        else
        {
            /* B, C, F, I, S, Z    1 slot */
            size += 1;
            ++i; /* Skip ; */
        }
    }

    return size;
}

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
    saveFields();
    saveMethods();
    saveAttributes();

}


void ClassFile::dumpAttribute(const AttributeInfo* attribute, int indent)
{
    std::string pad(indent, ' ');
    std::string name = getConstant<ConstantUtf8>(attribute->attributeNameIndex) -> value;

    if (auto* codeObj = dynamic_cast<const CodeAttribute*>(attribute))
    {
        std::cout << CODE_ATTRIBUTE_COLOUR << pad << "Code" << ANSI_RESET << ": max_stack=" << codeObj->maxStack
                   << " max_locals=" << codeObj->maxLocals
                   << " code_length=" << codeObj->codeLength << "\n\n";

        if (!codeObj->code.empty())
        {
            U4 index = 1;
            std::cout << pad + "Opcodes:\n";

            U8 pc = 0;
            const auto& code = codeObj->code;

            while (pc < code.size())
            {
                Opcode op = static_cast<Opcode>(code[pc]);

                bool implemented = isImplemented(op);
                U1 opSize = operandSize(op);
                
                std::cout << pad 
                          << "  "
                          << std::left
                          << std::setw(7)
                          << ("#" + std::to_string(index++))
                          << " pc="
                          << CODE_ATTRIBUTE_COLOUR
                          << std::left 
                          << std::setw(5) 
                          << pc
                          << ANSI_RESET
                          << ' '
                          << (implemented ? ANSI_BG_GREEN  : ANSI_BG_RED)
                          << "0x"
                          << std::hex
                          << std::uppercase
                          << std::setw(2)
                          << std::setfill('0')
                          << static_cast<int>(code[pc])
                          << ANSI_RESET
                          << std::nouppercase
                          << std::dec
                          << std::setfill(' ')
                          << (implemented ? ANSI_FG_GREEN  : ANSI_FG_RED)
                          << std::left
                          << std::setw(16)
                          << (' ' + toString(op))
                          << ANSI_RESET;

                if (opSize > 0 && pc + opSize < code.size())
                {
                    U4 operandVal = 0;
                    for (U1 i = 1; i <= opSize; ++i)
                    {
                        operandVal = (operandVal << 8) | code[pc + i];
                    }
                    if (usesBranchOperand(op) && opSize == 2)
                    {
                        S2 signedOffset = static_cast<S2>(static_cast<U2>(operandVal));
                        U8 target = pc + signedOffset;

                        std::cout << "target=" 
                                  << target
                                  << " (offset=" 
                                  << (signedOffset >= 0 ? "+" : "") 
                                  << signedOffset 
                                  << ")";
                    }
                    else if (usesConstantPoolOperand(op))
                    {
                        std::string resolved = describeConstant(static_cast<U2>(operandVal));
                        std::cout << "cp=#" << operandVal;
                        if (!resolved.empty())
                            std::cout << " (" << resolved << ")";
                    }
                    else
                    {   
                    std::cout << "operand="
                              << std::setw(5)
                              << operandVal
                              << " (0x" 
                              << std::hex 
                              << std::uppercase
                              << std::setfill('0')
                              << std::setw(4)
                              << std::right
                              << operandVal
                              << std::dec 
                              << std::nouppercase 
                              << std::setfill(' ')
                              << ")";
                    }
                }
                std::cout << '\n';
                pc += 1 + opSize;
            }
            std::cout << '\n';
        }
        
        if (!codeObj->exceptionTable.empty())
        {
            std::cout << pad << "  Exception Table:\n";
            for (auto& e : codeObj->exceptionTable)
            {
                std::cout << pad << "    start_pc=" << e.startPc
                           << " end_pc=" << e.endPc
                           << " handler_pc=" << e.handlerPc
                           << " catch_type=" << e.catchType << "\n";
            }
        }

        for (auto& nested : codeObj->attributes)
        {
            dumpAttribute(nested.get(), indent + 2);
        }
    }
    else if (auto* cv = dynamic_cast<const ConstantValueAttribute*>(attribute))
    {
        std::cout << CONSTVAL_ATTRIBUTE_COLOUR << pad << "ConstantValue" << ANSI_RESET << ": constant_value_index=" << cv->constantValueIndex << "\n";
    }
    else if (auto* ex = dynamic_cast<const ExceptionsAttribute*>(attribute))
    {
        std::cout << EXCEPTIONS_ATTRIBUTE_COLOUR << pad << "Exceptions" << ANSI_RESET << ":" << "\n";
        for (U2 index : ex->exceptionIndexTable)
        {
            std::cout << pad << "#" << index;

            if(auto* cls = getConstant<ConstantClass>(index))
            {
                auto* nameUTF8 = getConstant<ConstantUtf8>(cls->nameIndex);
                std::string name = nameUTF8->value;
                std::cout << " -> " 
                          << "(" 
                          << CLASS_COLOUR 
                          << name 
                          << ANSI_RESET 
                          << ")";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    else if (auto* sf = dynamic_cast<const SourceFileAttribute*>(attribute))
    {
        std::cout << SOURCE_FILE_ATTRIBUTE_COLOUR << pad << "SourceFile" << ANSI_RESET << ": source_file_index=" << sf->sourceFileIndex << "\n";
    }
    else if (auto* lnt = dynamic_cast<const LineNumberTableAttribute*>(attribute))
    {
        std::cout << LINE_NUMBER_ATTRIBUTE_COLOUR << pad << "LineNumberTable" << ANSI_RESET << ": line_number_table:" << "\n";
        U4 index = 1;
        for (auto& entry : lnt->lineNumberTable)
        {
            std::cout << pad
                      << std::left 
                      << std::setw(4) 
                      << ("  #" + std::to_string(index++) + ':') 
                      << "start_pc: "
                      << entry.startPc
                      << " line_number: "
                      << entry.lineNumber
                      << "\n";
        }
    }
    else if (auto* sm = dynamic_cast<const StackMapAttribute*>(attribute))
    {
        std::cout << STACKMAP_ATTRIBUTE_COLOUR << pad << "StackMap" << ANSI_RESET << ": number_of_entries=" << sm->entries.size() << "\n";
        U4 index = 1;
        for (auto& frame : sm->entries)
        {
            std::cout << pad << "  #" << index++ << ": offset=" << frame.offset << "\n";

            std::cout << pad << "    locals (" << frame.locals.size() << "): ";
            for (auto& v : frame.locals)
            {
                std::cout << to_string(v.tag);
                if (v.tag == VerificationTypeTag::Object)
                {
                    std::cout << "(cp#" << v.extra << ")";
                    auto* cls = getConstant<ConstantClass>(v.extra);
                    if(cls)
                    {
                        auto* name = getConstant<ConstantUtf8>(cls->nameIndex);
                        std::cout <<  " -> (" << ANSI_BOLD << CLASS_COLOUR << name->value  << ANSI_RESET << ") ";
                    }
                }
                else if (v.tag == VerificationTypeTag::Uninitialized)
                {
                    std::cout << "(offset=" << v.extra << ")";
                }
                std::cout << " ";
            }
            std::cout << "\n";

            std::cout << pad << "    stack (" << frame.stack.size() << "): ";
            for (auto& v : frame.stack)
            {
                std::cout << to_string(v.tag);
                if (v.tag == VerificationTypeTag::Object)
                {
                    std::cout << "(cp#" << v.extra << ")";
                    auto* cls = getConstant<ConstantClass>(v.extra);
                    if(cls)
                    {
                        auto* name = getConstant<ConstantUtf8>(cls->nameIndex);
                        std::cout <<  " -> (" << ANSI_BOLD << CLASS_COLOUR << name->value << ANSI_RESET << ") ";
                    }
                }
                else if (v.tag == VerificationTypeTag::Uninitialized)
                {
                    std::cout << "(offset=" << v.extra << ")";
                }
                std::cout << " ";
            }
            std::cout << "\n";
        }
    }
    else
    {
        std::cout << GENERIC_ATTRIBUTE_COLOUR << pad << name << ANSI_RESET << " (Generic/Unknown attr. " << attribute->attributeLength << " bytes)\n";
    }
}

void ClassFile::dumpFields()
{
    std::cout << "\nFields: " << fieldsCount << "\n";
    for (U2 i = 0; i < fieldsCount; ++i)
    {
        auto* name = getConstant<ConstantUtf8>(fields[i].nameIndex);
        auto* descriptor = getConstant<ConstantUtf8>(fields[i].descriptorIndex);

        std::cout << "  #" << i << " " << name->value
                   << " : " << descriptor->value
                   << " (access_flags=0x" << std::hex << fields[i].accessFlags << std::dec << ")\n";
        std::cout << "  Access flags:";
        if (fields[i].accessFlags & ACC_PUBLIC) std::cout << " PUBLIC";
        if (fields[i].accessFlags & ACC_FINAL) std::cout << " FINAL";
        if (fields[i].accessFlags & ACC_SUPER) std::cout << " SUPER";
        if (fields[i].accessFlags & ACC_INTERFACE) std::cout << " INTERFACE";
        if (fields[i].accessFlags & ACC_ABSTRACT) std::cout << " ABSTRACT";
        if (fields[i].accessFlags & ACC_MODULE) std::cout << " MODULE";
        if (fields[i].accessFlags & ACC_SYNTHETIC) std::cout << " SYNTHETIC";
        if (fields[i].accessFlags & ACC_ANNOTATION) std::cout << " ANNOTATION";
        if (fields[i].accessFlags & ACC_ENUM) std::cout << " ENUM";

        std::cout << "\n";
        for (auto& attribute : fields[i].attributes)
        {
            dumpAttribute(attribute.get(), 4);
        }
    }
}

void ClassFile::dumpMethods()
{
    std::cout << "\nMethods: " << methodsCount << "\n";
    for (U2 i = 0; i < methodsCount; ++i)
    {
        auto* name = getConstant<ConstantUtf8>(methods[i].nameIndex);
        auto* descriptor = getConstant<ConstantUtf8>(methods[i].descriptorIndex);

        bool isStatic = methods[i].accessFlags & ACC_STATIC;
        U2 argsSize = computeArgsSize(descriptor->value, isStatic);

        std::cout << "  #" << i << " " << name->value
                  << " : " << descriptor->value
                  << " (access_flags=0x" << std::hex << methods[i].accessFlags << std::dec << ")"
                  << " args_size=" << argsSize << "\n";
        std::cout << "  Access flags:";
        if (methods[i].accessFlags & ACC_PUBLIC) std::cout << " PUBLIC";
        if (methods[i].accessFlags & ACC_FINAL) std::cout << " FINAL";
        if (methods[i].accessFlags & ACC_SUPER) std::cout << " SUPER";
        if (methods[i].accessFlags & ACC_INTERFACE) std::cout << " INTERFACE";
        if (methods[i].accessFlags & ACC_ABSTRACT) std::cout << " ABSTRACT";
        if (methods[i].accessFlags & ACC_MODULE) std::cout << " MODULE";
        if (methods[i].accessFlags & ACC_SYNTHETIC) std::cout << " SYNTHETIC";
        if (methods[i].accessFlags & ACC_ANNOTATION) std::cout << " ANNOTATION";
        if (methods[i].accessFlags & ACC_ENUM) std::cout << " ENUM";
        if (methods[i].accessFlags & ACC_STATIC) std::cout << " STATIC";

        std::cout << "\n";
        for (auto& attribute : methods[i].attributes)
        {
            dumpAttribute(attribute.get(), 4);
        }
    }
}

void ClassFile::dumpAttributes()
{
    std::cout << "\nClass Attributes: " << attributesCount << "\n";
    for (auto& attribute : attributes)
    {
        dumpAttribute(attribute.get(), 2);
    }
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
        for (auto& index : exceptionIndexTable)
        {
            index = reader.readU2();
        }
        return std::make_unique<ExceptionsAttribute>(
             nameIndex, length, numberOfExceptions,
             std::move(exceptionIndexTable)
         );
    }

    if (name == "SourceFile")
    {
        U2 sourceFileIndex = reader.readU2();
        return std::make_unique<SourceFileAttribute>(
            nameIndex, length, sourceFileIndex
        );
    }

    if (name == "LineNumberTable")
    {
        U2 lineNumberTableLength = reader.readU2();
        std::vector<LineNumberTableEntry> lineNumberTable(lineNumberTableLength);

        for (auto& entry : lineNumberTable)
        {
            entry.startPc = reader.readU2();
            entry.lineNumber = reader.readU2();
        }

        return std::make_unique<LineNumberTableAttribute>(
            nameIndex, length, std::move(lineNumberTable)
        );
    }

    if (name == "StackMap")
    {
        U2 numberOfEntries = reader.readU2();
        std::vector<StackMapFrame> entries(numberOfEntries);

        for (auto& frame : entries)
        {
            frame.offset = reader.readU2();

            U2 numberOfLocals = reader.readU2();
            frame.locals.resize(numberOfLocals);

            for (auto& local : frame.locals)
            {
                local.tag = static_cast<VerificationTypeTag>(reader.readU1());
                local.extra = (local.tag == VerificationTypeTag::Object ||
                     local.tag == VerificationTypeTag::Uninitialized ?
                     reader.readU2() : 0);
            }   

            U2 numberOfStack = reader.readU2();
            frame.stack.resize(numberOfStack);

            for (auto& stack : frame.stack)
            {
                stack.tag = static_cast<VerificationTypeTag>(reader.readU1());
                stack.extra = (stack.tag == VerificationTypeTag::Object ||
                     stack.tag == VerificationTypeTag::Uninitialized ?
                     reader.readU2() : 0);
            }
        }

        return std::make_unique<StackMapAttribute>(
            nameIndex, length, std::move(entries)
        );
    }

    std::vector<U1> raw = reader.readBytes(length);
    return std::make_unique<GenericAttribute>(nameIndex, length, std::move(raw));
}


void ClassFile::saveFields()
{
    fieldsCount = reader.readU2();
    fields.resize(fieldsCount);

    for (U2 i = 0; i < fieldsCount; ++i)
    {
        fields[i].accessFlags = reader.readU2();
        fields[i].nameIndex = reader.readU2();
        fields[i].descriptorIndex = reader.readU2();
        fields[i].attributesCount = reader.readU2();

        fields[i].attributes.resize(fields[i].attributesCount);
        for (U2 j = 0; j < fields[i].attributesCount; ++j)
        {
            fields[i].attributes[j] = readAttribute();
        }
    }
}

void ClassFile::saveMethods()
{
    methodsCount = reader.readU2();
    methods.resize(methodsCount);

    for (U2 i = 0; i < methodsCount; ++i)
    {
        methods[i].accessFlags = reader.readU2();
        methods[i].nameIndex = reader.readU2();
        methods[i].descriptorIndex = reader.readU2();
        methods[i].attributesCount = reader.readU2();

        methods[i].attributes.resize(methods[i].attributesCount);
        for (U2 j = 0; j < methods[i].attributesCount; ++j)
        {
            methods[i].attributes[j] = readAttribute();
        }
    }
}

void ClassFile::saveAttributes()
{
    attributesCount = reader.readU2();
    attributes.resize(attributesCount);

    for (U2 i = 0; i < attributesCount; ++i)
    {
        attributes[i] = readAttribute();
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
                    std::cout << std::left
                              << std::setw(5)
                              << ("#" + std::to_string(i))
                              << " Utf8: "
                              << UTF8_COLOUR
                              << utf8 -> value
                              << ANSI_RESET
                              << "\n";
                }
                break;
            case ConstantTag::Integer:
                {
                    auto* integer = getConstant<ConstantInteger>(i);
                    std::cout << std::left
                              << std::setw(5)
                              << ("#" + std::to_string(i))
                              << " Integer: "
                              << INT_COLOUR
                              << integer -> value
                              << ANSI_RESET
                              << "\n";
                }
                break;
            case ConstantTag::Float:
                {
                    auto* f = getConstant<ConstantFloat>(i);
                    std::cout << std::left
                              << std::setw(5)
                              << ("#" + std::to_string(i))
                              << " Float: "
                              << FLOAT_COLOUR
                              << f->value
                              << ANSI_RESET
                              << "\n";
                    break;
                }
            case ConstantTag::Long:
            {
                auto* l = getConstant<ConstantLong>(i);
                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " Long: "
                          << LONG_COLOUR
                          << l->value
                          << ANSI_RESET
                          << "\n";
                ++i;  /* Long/Double take two constant pool entries */
                break;
            }
            case ConstantTag::Double:
            {
                auto* d = getConstant<ConstantDouble>(i);
                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " Double: "
                          << DOUBLE_COLOUR
                          << d->value
                          << ANSI_RESET
                          << "\n";
                ++i;  /* Long/Double take two constant pool entries */
                break;
            }
            case ConstantTag::Class:
            {
                auto* c = getConstant<ConstantClass>(i);
                auto* nameUTF8 = getConstant<ConstantUtf8>(c->nameIndex);
                std::string name = nameUTF8->value; 
                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " Class: name_index="
                          << CLASS_COLOUR
                          << c->nameIndex
                          << std::setw(3)
                          << ANSI_RESET
                          << " -> "
                          << std::setw(3)
                          << CLASS_COLOUR
                          << name
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::String:
            {
                auto* s = getConstant<ConstantString>(i);
                auto* stringUTF8 = getConstant<ConstantUtf8>(s->stringIndex);
                std::string stringValue = stringUTF8->value; 
                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " String: string_index="
                          << STRING_COLOUR
                          << s->stringIndex
                          << std::setw(3)
                          << ANSI_RESET
                          << " -> "
                          << std::setw(3)
                          << STRING_COLOUR
                          << stringValue
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::Fieldref:
            {
                auto* f = getConstant<ConstantFieldref>(i);
                auto* cls = getConstant<ConstantClass>(f->classIndex);
                auto* clsNameUTF8 = getConstant<ConstantUtf8>(cls->nameIndex);
                std::string className = clsNameUTF8->value;

                auto* nt = getConstant<ConstantNameAndType>(f->nameAndTypeIndex);
                auto* fieldNameUTF8 = getConstant<ConstantUtf8>(nt->nameIndex);
                auto* descriptorUTF8 = getConstant<ConstantUtf8>(nt->descriptorIndex);
                std::string fieldName = fieldNameUTF8->value;
                std::string descriptor = descriptorUTF8->value;

                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " Fieldref: class_index="
                          << FIELDREF_COLOUR
                          << f->classIndex
                          << ANSI_RESET
                          << " name_and_type_index="
                          << FIELDREF_COLOUR
                          << f->nameAndTypeIndex
                          << ANSI_RESET
                          << " -> "
                          << FIELDREF_COLOUR
                          << className
                          << "."
                          << fieldName
                          << ":"
                          << descriptor   
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::Methodref:
            {
                auto* m = getConstant<ConstantMethodref>(i);
                auto* cls = getConstant<ConstantClass>(m->classIndex);
                auto* clsNameUTF8 = getConstant<ConstantUtf8>(cls->nameIndex);
                std::string className = clsNameUTF8->value;

                auto* nt = getConstant<ConstantNameAndType>(m->nameAndTypeIndex);
                auto* methodNameUTF8 = getConstant<ConstantUtf8>(nt->nameIndex);
                auto* descriptorUTF8 = getConstant<ConstantUtf8>(nt->descriptorIndex);
                std::string methodName = methodNameUTF8->value;
                std::string descriptor = descriptorUTF8->value;


                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " Methodref: class_index="
                          << METHODREF_COLOUR
                          << m->classIndex
                          << ANSI_RESET
                          << " name_and_type_index="
                          << METHODREF_COLOUR
                          << m->nameAndTypeIndex
                          << ANSI_RESET
                          << " -> "
                          << METHODREF_COLOUR
                          << className
                          << "."
                          << methodName
                          << ":"
                          << descriptor
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::InterfaceMethodref:
            {
                auto* m = getConstant<ConstantInterfaceMethodref>(i);
                auto* cls = getConstant<ConstantClass>(m->classIndex);
                auto* clsNameUTF8 = getConstant<ConstantUtf8>(cls->nameIndex);
                std::string className = clsNameUTF8->value;

                auto* nt = getConstant<ConstantNameAndType>(m->nameAndTypeIndex);
                auto* methodNameUTF8 = getConstant<ConstantUtf8>(nt->nameIndex);
                auto* descriptorUTF8 = getConstant<ConstantUtf8>(nt->descriptorIndex);
                std::string methodName = methodNameUTF8->value;
                std::string descriptor = descriptorUTF8->value;

                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " InterfaceMethodref: class_index="
                          << IFACE_MR_COLOUR
                          << m->classIndex
                          << ANSI_RESET
                          << " name_and_type_index="
                          << IFACE_MR_COLOUR
                          << m->nameAndTypeIndex
                          << ANSI_RESET
                          << " -> "
                          << IFACE_MR_COLOUR
                          << className
                          << "."
                          << methodName
                          << ":"
                          << descriptor
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::NameAndType:
            {
                auto* nt = getConstant<ConstantNameAndType>(i);
                auto* nameUTF8 = getConstant<ConstantUtf8>(nt->nameIndex);
                auto* descriptorUTF8 = getConstant<ConstantUtf8>(nt->descriptorIndex);
                std::string name = nameUTF8->value;
                std::string descriptor = descriptorUTF8->value;

                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " NameAndType: name_index="
                          << NAT_COLOUR
                          << nt->nameIndex
                          << ANSI_RESET
                          << " descriptor_index="
                          << NAT_COLOUR
                          << nt->descriptorIndex
                          << ANSI_RESET
                          << " -> "
                          << NAT_COLOUR
                          << name
                          << ":"
                          << descriptor
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::MethodHandle:
            {
                auto* mh = getConstant<ConstantMethodHandle>(i);
                int kind = static_cast<int>(mh->referenceKind);

                std::string className, memberName, descriptor;

                if (kind >= 1 && kind <= 4)
                {
                    // Fieldref
                    auto* f = getConstant<ConstantFieldref>(mh->referenceIndex);
                    auto* cls = getConstant<ConstantClass>(f->classIndex);
                    className = getConstant<ConstantUtf8>(cls->nameIndex)->value;
                    auto* nt = getConstant<ConstantNameAndType>(f->nameAndTypeIndex);
                    memberName = getConstant<ConstantUtf8>(nt->nameIndex)->value;
                    descriptor = getConstant<ConstantUtf8>(nt->descriptorIndex)->value;
                }
                else if (kind >= 5 && kind <= 8)
                {
                    // Methodref
                    auto* m = getConstant<ConstantMethodref>(mh->referenceIndex);
                    auto* cls = getConstant<ConstantClass>(m->classIndex);
                    className = getConstant<ConstantUtf8>(cls->nameIndex)->value;
                    auto* nt = getConstant<ConstantNameAndType>(m->nameAndTypeIndex);
                    memberName = getConstant<ConstantUtf8>(nt->nameIndex)->value;
                    descriptor = getConstant<ConstantUtf8>(nt->descriptorIndex)->value;
                }
                else if (kind == 9)
                {
                    // InterfaceMethodref
                    auto* m = getConstant<ConstantInterfaceMethodref>(mh->referenceIndex);
                    auto* cls = getConstant<ConstantClass>(m->classIndex);
                    className = getConstant<ConstantUtf8>(cls->nameIndex)->value;
                    auto* nt = getConstant<ConstantNameAndType>(m->nameAndTypeIndex);
                    memberName = getConstant<ConstantUtf8>(nt->nameIndex)->value;
                    descriptor = getConstant<ConstantUtf8>(nt->descriptorIndex)->value;
                }

                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " MethodHandle: reference_kind="
                          << MHANDLE_COLOUR
                          << kind
                          << ANSI_RESET
                          << " reference_index="
                          << MHANDLE_COLOUR
                          << mh->referenceIndex
                          << ANSI_RESET
                          << " -> "
                          << MHANDLE_COLOUR
                          << className
                          << "."
                          << memberName
                          << ":"
                          << descriptor
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::MethodType:
            {
                auto* mt = getConstant<ConstantMethodType>(i);
                auto* descriptorUTF8 = getConstant<ConstantUtf8>(mt->descriptorIndex);
                std::string descriptor = descriptorUTF8->value;

                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " MethodType: descriptor_index="
                          << MTYPE_COLOUR
                          << mt->descriptorIndex
                          << ANSI_RESET
                          << " -> "
                          << MTYPE_COLOUR
                          << descriptor
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::Dynamic:
            {
                auto* dyn = getConstant<ConstantDynamic>(i);
                auto* nt = getConstant<ConstantNameAndType>(dyn->nameAndTypeIndex);
                auto* nameUTF8 = getConstant<ConstantUtf8>(nt->nameIndex);
                auto* descriptorUTF8 = getConstant<ConstantUtf8>(nt->descriptorIndex);
                std::string name = nameUTF8->value;
                std::string descriptor = descriptorUTF8->value;


                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " Dynamic: bootstrap_method_attr_index="
                          << DYNAMIC_COLOUR
                          << dyn->bootstrapMethodAttrIndex
                          << ANSI_RESET
                          << " name_and_type_index="
                          << DYNAMIC_COLOUR
                          << dyn->nameAndTypeIndex
                          << ANSI_RESET
                          << " -> "
                          << DYNAMIC_COLOUR
                          << name
                          << ":"
                          << descriptor
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::InvokeDynamic:
            {
                auto* inv = getConstant<ConstantInvokeDynamic>(i);
                auto* nt = getConstant<ConstantNameAndType>(inv->nameAndTypeIndex);
                auto* nameUTF8 = getConstant<ConstantUtf8>(nt->nameIndex);
                auto* descriptorUTF8 = getConstant<ConstantUtf8>(nt->descriptorIndex);
                std::string name = nameUTF8->value;
                std::string descriptor = descriptorUTF8->value;
                
                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " InvokeDynamic: bootstrap_method_attr_index="
                          << INVOKEDYN_COLOUR
                          << inv->bootstrapMethodAttrIndex
                          << ANSI_RESET
                          << " name_and_type_index="
                          << INVOKEDYN_COLOUR
                          << inv->nameAndTypeIndex
                          << ANSI_RESET
                          << " -> "
                          << INVOKEDYN_COLOUR
                          << name
                          << ":"
                          << descriptor
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::Module:
            {
                auto* mod = getConstant<ConstantModule>(i);
                auto* nameUTF8 = getConstant<ConstantUtf8>(mod->nameIndex);
                std::string name = nameUTF8->value;

                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " Module: name_index="
                          << MODULE_COLOUR
                          << mod->nameIndex
                          << ANSI_RESET
                          << " -> "
                          << MODULE_COLOUR
                          << name
                          << ANSI_RESET
                          << "\n";
                break;
            }
            case ConstantTag::Package:
            {
                auto* pkg = getConstant<ConstantPackage>(i);
                auto* nameUTF8 = getConstant<ConstantUtf8>(pkg->nameIndex);
                std::string name = nameUTF8->value;

                std::cout << std::left
                          << std::setw(5)
                          << ("#" + std::to_string(i))
                          << " Package: name_index="
                          << PACKAGE_COLOUR
                          << pkg->nameIndex
                          << ANSI_RESET
                          << " -> "
                          << PACKAGE_COLOUR
                          << name
                          << ANSI_RESET
                          << "\n";
                break;
            }
            default:
                throw std::runtime_error(
                    "Unknown constant pool tag: " +
                    std::to_string(static_cast<int>(constantPool[i]->tag)));
        }
    }
}

std::string ClassFile::describeConstant(U2 index)
{
    if (index == 0 || index >= constantPool.size() || !constantPool[index])
        return "";

    switch (constantPool[index]->tag)
    {
        case ConstantTag::Utf8:
            return getConstant<ConstantUtf8>(index)->value;

        case ConstantTag::Integer:
            return std::to_string(getConstant<ConstantInteger>(index)->value);

        case ConstantTag::Float:
            return std::to_string(getConstant<ConstantFloat>(index)->value);

        case ConstantTag::Long:
            return std::to_string(getConstant<ConstantLong>(index)->value);

        case ConstantTag::Double:
            return std::to_string(getConstant<ConstantDouble>(index)->value);

        case ConstantTag::String:
        {
            auto* s = getConstant<ConstantString>(index);
            return "\"" + describeConstant(s->stringIndex) + "\"";
        }

        case ConstantTag::Class:
        {
            auto* c = getConstant<ConstantClass>(index);
            return describeConstant(c->nameIndex);
        }

        case ConstantTag::Fieldref:
        case ConstantTag::Methodref:
        case ConstantTag::InterfaceMethodref:
        {
            U2 classIndex = 0;
            U2 ntIndex = 0;
            switch (constantPool[index]->tag)
            {
                case ConstantTag::Fieldref:
                {
                    auto* f = getConstant<ConstantFieldref>(index);
                    classIndex = f->classIndex;
                    ntIndex = f->nameAndTypeIndex;
                    break;
                }
                case ConstantTag::Methodref:
                {
                    auto* m = getConstant<ConstantMethodref>(index);
                    classIndex = m->classIndex;
                    ntIndex = m->nameAndTypeIndex;
                    break;
                }
                default: // InterfaceMethodref
                {
                    auto* im = getConstant<ConstantInterfaceMethodref>(index);
                    classIndex = im->classIndex;
                    ntIndex = im->nameAndTypeIndex;
                    break;
                }
            }

            auto* nt = getConstant<ConstantNameAndType>(ntIndex);
            return describeConstant(classIndex) + "." + describeConstant(nt->nameIndex) + ":" + describeConstant(nt->descriptorIndex);
        }

        case ConstantTag::NameAndType:
        {
            auto* nt = getConstant<ConstantNameAndType>(index);
            return describeConstant(nt->nameIndex) + ":" + describeConstant(nt->descriptorIndex);
        }

        default:
            return "";
    }
}

void ClassFile::dump()
{
    FileInfo infoStruct = getFileInfo(filename);

    std::cout << "File:                "
              << infoStruct.fullPath
              << "\n"
#if not defined(_WIN32)
              << "File size:           "
              << infoStruct.size 
              << " bytes"
              << "\n"
              << "Last modified:       "
              << infoStruct.lastModified
              << "\n"
#endif
              << "Magic:"
	          << ANSI_FG_GREEN
	          << "               0x"
              << std::hex
              << std::uppercase
              << magic
	          << ANSI_RESET
              << '\n'
              << "Minor version:       "
              << std::dec
              << minorVersion
              << '\n'
              << "Major version:       "
              << majorVersion
              << '\n'
              << "Constant pool count: "
              << CONSTANT_POOL_COLOUR
              << constantPoolCount - 1
              << ANSI_RESET
              << "\n"
              << "Interface count:     "
              << INTERFACE_COLOUR
              << interfacesCount
              << ANSI_RESET
              << "\n"
              << "Field count:         "
              << FIELD_COLOUR
              << fieldsCount
              << ANSI_RESET
              << "\n"
              << "Method count:        "
              << METHOD_COLOUR
              << methodsCount
              << ANSI_RESET
              << "\n"
              << "Attribute count:     "
              << ATTRIBUTE_COLOUR
              << attributesCount
              << ANSI_RESET
              << '\n';      
    
    std::cout << std::dec << std::nouppercase; 

    dumpConstantPoolTags();
    dumpClassMetadata();
    dumpFields();
    dumpMethods();
    dumpAttributes();
}

