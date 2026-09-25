/*
 * DRVM (drvm)
 * Copyright (C) 2026 Vugar Ahadli
 * Contact: vuqarahadli17@gmail.com | vuqar@div.edu.az
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "vm/VM.hpp"


static void parseArrayDescriptor(const std::string& desc, int& dimensions, ValueType& leafType)
{
    dimensions = 0;
    std::size_t i = 0;
    while (i < desc.size() && desc[i] == '[')
    {
        ++dimensions;
        ++i;
    }

    switch (desc[i])
    {
        case 'Z': 
            leafType = ValueType::Boolean; 
            break;
        case 'B': 
            leafType = ValueType::Byte; 
            break;
        case 'C': 
            leafType = ValueType::Char; 
            break;
        case 'S': 
            leafType = ValueType::Short; 
            break;
        case 'I': 
            leafType = ValueType::Int; 
            break;
        case 'J': 
            leafType = ValueType::Long; 
            break;
        case 'F': 
            leafType = ValueType::Float; 
            break;
        case 'D': 
            leafType = ValueType::Double; 
            break;
        case 'L': 
            leafType = ValueType::Reference; 
            break;
        default:
            throw std::runtime_error("multianewarray: unrecognized descriptor \"" + desc + "\"");
    }
}

static const UTF16& asStringData(const Value& v)
{
    auto* ref = std::get_if<HeapObject*>(&v);
    if (!ref || !*ref || (*ref)->type != HeapType::String)
    {
        throw std::runtime_error("expected a String reference");
    }
    return static_cast<StringHeapObject*>(*ref)->value;
}

VM::VM(ClassLoader& loader)
    : loader(loader)
{
}

HeapObject* VM::createMultiArray(const std::vector<S4>& dimSizes, std::size_t dimIndex, ValueType leafType, std::size_t totalDimensions)
{
    if (dimIndex >= dimSizes.size())
    {
        return nullptr;
    }

    S4 count = dimSizes[dimIndex];
    if (count < 0)
    {
        throw std::runtime_error("NegativeArraySizeException: multianewarray dimension is negative");
    }

    if (heap.size() >= gcThreshold)
    {
        collectGarbage();
    }

    bool isLeafDimension = (dimIndex + 1 == totalDimensions);

    if (isLeafDimension && leafType != ValueType::Reference)
    {
        std::size_t elementSize;
        switch (leafType)
        {
            case ValueType::Boolean: 
                elementSize = sizeof(U1); 
                break;
            case ValueType::Byte: 
                elementSize = sizeof(S1); 
                break;
            case ValueType::Char: 
                elementSize = sizeof(U2); 
                break;
            case ValueType::Short: 
                elementSize = sizeof(S2); 
                break;
            case ValueType::Int:
                elementSize = sizeof(S4); 
                break;
            case ValueType::Long:
                elementSize = sizeof(S8); 
                break;
            case ValueType::Float:
                elementSize = sizeof(F4); 
                break;
            case ValueType::Double: 
                elementSize = sizeof(F8); 
                break;
            default:
                elementSize = 0; 
                break;
        }

        auto arrayObj = std::make_unique<ArrayHeapObject>(leafType, static_cast<U4>(count));
        arrayObj->primitiveData.assign(static_cast<std::size_t>(count) * elementSize, 0);
        heap.push_back(std::move(arrayObj));
    }
    else
    {
        auto arrayObj = std::make_unique<ArrayHeapObject>(ValueType::Reference, static_cast<U4>(count));
        arrayObj->referenceData.assign(static_cast<std::size_t>(count), nullptr);
        HeapObject* thisLevel = arrayObj.get();
        heap.push_back(std::move(arrayObj));

        if (dimIndex + 1 < dimSizes.size())
        {
            auto* thisArray = static_cast<ArrayHeapObject*>(thisLevel);
            for (S4 k = 0; k < count; ++k)
            {
                thisArray->referenceData[static_cast<std::size_t>(k)] =
                    createMultiArray(dimSizes, dimIndex + 1, leafType, totalDimensions);
            }
        }
        return thisLevel;
    }

    HeapObject* result = heap.back().get();
    if (heap.size() >= gcThreshold)
    {
        gcThreshold = heap.size() * 2;
    }
    return result;
}

HeapObject* VM::allocateString(const std::string& utf8)
{
   if (heap.size() >= gcThreshold)
    {
        collectGarbage();
    }
        

    heap.push_back(std::make_unique<StringHeapObject>(utf8ToUtf16(utf8)));

    if (heap.size() >= gcThreshold)
    {
        gcThreshold = heap.size() * 2; 
    }
        
    return heap.back().get();
}

HeapObject* VM::allocateRuntimeConstant(ConstantTag tag, std::string value)
{
    if (heap.size() >= gcThreshold)
    {
        collectGarbage();
    }

    heap.push_back(std::make_unique<ConstantHeapObject>(tag, std::move(value)));
    HeapObject* constant = heap.back().get();

    if (heap.size() >= gcThreshold)
    {
        gcThreshold = heap.size() * 2;
    }

    return constant;
}

Value VM::invoke(ClassFile& classFile, const MethodInfo& method)
{
    const CodeAttribute* code = classFile.getCode(method);
    if (!code)
    {
        throw std::runtime_error("Method has no Code attribute -> (native/abstract)");
    }
    return execute(classFile, *code);
}



bool VM::isSubclassOf(const std::string& className, const std::string& targetClassName)
{
    if (className == targetClassName)
    {
        return true;
    }

    if (className == "java/lang/String")
    {
        if (targetClassName == "java/lang/Object" || targetClassName == "java/lang/String" || targetClassName == "java/io/Serializable" || targetClassName == "java/lang/Comparable")
        {
            return true;
        }
    }

    std::vector<std::string> pending{className};
    std::unordered_set<std::string> visited;

    while (!pending.empty())
    {
        std::string currentName = pending.back();
        pending.pop_back();

        if (!visited.insert(currentName).second)
        {
            continue;
        }

        if (currentName == "java/lang/Object" && targetClassName == "java/lang/Object")
        {
            return true;
        }

        ClassFile* current = nullptr;
        try
        {
            current = loader.loadClass(currentName);
        }
        catch (...)
        {
            current = nullptr;
        }

        if (!current)
        {
            continue;
        }

        U2 superIndex = current->getSuperClass();
        if (superIndex != 0)
        {
            ConstantClass* superRef = current->getConstant<ConstantClass>(superIndex);
            std::string superName = current->getConstant<ConstantUtf8>(superRef->nameIndex)->value;

            if (superName == targetClassName)
            {
                return true;
            }

            pending.push_back(superName);
        }

        for (const std::string& interfaceName : current->getInterfaceNames())
        {
            if (interfaceName == targetClassName)
            {
                return true;
            }

            pending.push_back(interfaceName);
        }
    }

    return false;
}


ClassFile* VM::resolveFieldOwner(ClassFile* startClass, const std::string& fieldName)
{
    if (!startClass)
    {
        return nullptr;
    }

    std::vector<ClassFile*> pending{startClass};
    std::unordered_set<std::string> visited;

    while (!pending.empty())
    {
        ClassFile* current = pending.back();
        pending.pop_back();

        if (!current)
        {
            continue;
        }

        if (!visited.insert(current->getClassName()).second)
        {
            continue;
        }

        for (const auto& field : current->getFields())
        {
            if (current->getConstant<ConstantUtf8>(field.nameIndex)->value == fieldName)
            {
                return current;
            }
        }

        U2 superIndex = current->getSuperClass();
        if (superIndex != 0)
        {
            ConstantClass* superRef = current->getConstant<ConstantClass>(superIndex);
            std::string superName = current->getConstant<ConstantUtf8>(superRef->nameIndex)->value;

            try
            {
                pending.push_back(loader.loadClass(superName));
            }
            catch (...)
            {
            }
        }

        for (const std::string& interfaceName : current->getInterfaceNames())
        {
            try
            {
                pending.push_back(loader.loadClass(interfaceName));
            }
            catch (...)
            {
            }
        }
    }

    return nullptr;
}

ClassFile* VM::resolveMethodOwner(ClassFile* startClass, const std::string& name,
                                   const std::string& descriptor, const MethodInfo** outMethod)
{
    if (!startClass)
    {
        return nullptr;
    }

    if (outMethod)
    {
        *outMethod = nullptr;
    }

    std::vector<ClassFile*> pending{startClass};
    std::unordered_set<std::string> visited;

    while (!pending.empty())
    {
        ClassFile* current = pending.back();
        pending.pop_back();

        if (!current)
        {
            continue;
        }

        if (!visited.insert(current->getClassName()).second)
        {
            continue;
        }

        if (const MethodInfo* m = current->findMethod(name, descriptor))
        {
            if (outMethod)
            {
                *outMethod = m;
            }
            return current;
        }

        U2 superIndex = current->getSuperClass();
        if (superIndex != 0)
        {
            ConstantClass* superRef = current->getConstant<ConstantClass>(superIndex);
            std::string superName = current->getConstant<ConstantUtf8>(superRef->nameIndex)->value;

            try
            {
                pending.push_back(loader.loadClass(superName));
            }
            catch (...)
            {
            }
        }

        for (const std::string& interfaceName : current->getInterfaceNames())
        {
            try
            {
                pending.push_back(loader.loadClass(interfaceName));
            }
            catch (...)
            {
            }
        }
    }

    return nullptr;
}


bool VM::tryInvokeNative(const std::string& className, const std::string& methodName, const std::string& descriptor, const MethodCall& call, Value& outResult)
{
    auto key = className + "." + methodName + ":" + descriptor;
    auto it = nativeMethods.find(key);
    if (it == nativeMethods.end())
    {
        return false;
    }
    outResult = it->second(*this, call);
    return true;
}

void VM::registerNativeMethods()
{

    nativeMethods["java/lang/System.gc:()V"] = [](VM& vm, const MethodCall&) -> Value
    {
        vm.collectGarbage();
        return Value();
    };

    /**
     * TODO:
     *  1. replace graphic api with SDL
     * 
     */

    

  
}


/**
 * Here are the Java-standard method names in the exact JVM form you can match in your VM, with the native keys spelled as `ClassName.methodName:descriptor`.

- `java/lang/Math.abs:(I)I`
- `java/lang/Math.abs:(J)J`
- `java/lang/StringBuffer.append:(Ljava/lang/String;)Ljava/lang/StringBuffer;`
- `java/lang/System.arraycopy:(Ljava/lang/Object;ILjava/lang/Object;II)V`
- `java/lang/Integer.byteValue:()B`
- `java/lang/String.charAt:(I)C`
- `java/io/InputStream.close:()V`
- `java/lang/String.compareTo:(Ljava/lang/String;)I`
- `java/lang/System.currentTimeMillis:()J`
- `java/lang/StringBuffer.delete:(II)Ljava/lang/StringBuffer;`
- `java/lang/String.endsWith:(Ljava/lang/String;)Z`
- `java/lang/Object.equals:(Ljava/lang/Object;)Z`
- `java/lang/Object.getClass:()Ljava/lang/Class;`
- `java/lang/Class.getResourceAsStream:(Ljava/lang/String;)Ljava/io/InputStream;`
- `java/lang/String.indexOf:(I)I`
- `java/lang/Integer.intValue:()I`
- `java/lang/String.length:()I`
- `java/util/Random.nextInt:(I)I`
- `java/lang/Object.notify:()V`
- `java/lang/Throwable.printStackTrace:()V`
- `java/io/PrintStream.println:(Ljava/lang/String;)V`
- `java/util/Hashtable.put:(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;`
- `java/io/InputStream.read:()I`
- `java/lang/StringBuffer.setCharAt:(IC)V`
- `java/io/InputStream.skip:(J)J`
- `java/lang/Thread.sleep:(J)V`
- `java/lang/Thread.start:()V`
- `java/lang/String.substring:(I)Ljava/lang/String;`
- `java/lang/String.toLowerCase:()Ljava/lang/String;`
- `java/lang/Object.toString:()Ljava/lang/String;`
- `java/lang/String.valueOf:(I)Ljava/lang/String;`
- `java/lang/Object.wait:()V`
- `java/lang/Thread.yield:()V`
*/

Value VM::execute(ClassFile& classFile, const CodeAttribute& code)
{
    Frame frame(code.maxLocals, code.maxStack);
    FrameGuard guard(*this, frame);
    const std::vector<U1>& bytecode = code.code;

    auto loadConstant = [this, &classFile](U2 index) -> Value
    {
        CPInfo* entry = classFile.getConstant<CPInfo>(index);

        switch (entry->tag)
        {
            case ConstantTag::Integer:
                return classFile.getConstant<ConstantInteger>(index)->value;
            case ConstantTag::Float:
                return classFile.getConstant<ConstantFloat>(index)->value;
            case ConstantTag::Long:
                return classFile.getConstant<ConstantLong>(index)->value;
            case ConstantTag::Double:
                return classFile.getConstant<ConstantDouble>(index)->value;
            case ConstantTag::String:
            {
                auto* stringConstant = classFile.getConstant<ConstantString>(index);
                auto* stringValue = classFile.getConstant<ConstantUtf8>(stringConstant->stringIndex);
                return allocateString(stringValue->value);
            }
            case ConstantTag::Class:
            {
                auto* classConstant = classFile.getConstant<ConstantClass>(index);
                auto* className = classFile.getConstant<ConstantUtf8>(classConstant->nameIndex);
                return allocateRuntimeConstant(ConstantTag::Class, className->value);
            }
            case ConstantTag::MethodType:
            {
                auto* methodType = classFile.getConstant<ConstantMethodType>(index);
                auto* descriptor = classFile.getConstant<ConstantUtf8>(methodType->descriptorIndex);
                return allocateRuntimeConstant(ConstantTag::MethodType, descriptor->value);
            }
            case ConstantTag::MethodHandle:
            {
                auto* methodHandle = classFile.getConstant<ConstantMethodHandle>(index);
                return allocateRuntimeConstant(
                    ConstantTag::MethodHandle,
                    "reference_kind=" + std::to_string(methodHandle->referenceKind) +
                    ",reference_index=" + std::to_string(methodHandle->referenceIndex));
            }
            case ConstantTag::Dynamic:
            {
                auto* dynamic = classFile.getConstant<ConstantDynamic>(index);
                auto* nameAndType = classFile.getConstant<ConstantNameAndType>(dynamic->nameAndTypeIndex);
                auto* name = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex);
                auto* descriptor = classFile.getConstant<ConstantUtf8>(nameAndType->descriptorIndex);
                return allocateRuntimeConstant(
                    ConstantTag::Dynamic,
                    "bootstrap=" + std::to_string(dynamic->bootstrapMethodAttrIndex) +
                    ",name=" + name->value + ",descriptor=" + descriptor->value);
            }
            default:
                throw std::runtime_error("constant pool entry is not loadable by ldc");
        }
    };

    while (frame.programCounter < bytecode.size())
    {
        U4 instructionStart = frame.programCounter;
        try
        {
            auto opcode = static_cast<Opcode>(bytecode[frame.programCounter++]);


            switch (opcode)
            {
                case Opcode::Nop:
                    break;

                case Opcode::AConstNull:
                    frame.push(nullptr);
                    break;

                case Opcode::IConstM1:
                    frame.push(S4(-1));
                    break;
                case Opcode::IConst0:
                    frame.push(S4(0));
                    break;
                case Opcode::IConst1:   
                    frame.push(S4(1));  
                    break;
                case Opcode::IConst2:
                    frame.push(S4(2));
                    break;
                case Opcode::IConst3:  
                    frame.push(S4(3));  
                    break;
                case Opcode::IConst4:
                    frame.push(S4(4));
                    break;
                case Opcode::IConst5: 
                    frame.push(S4(5));
                    break;
                
                case Opcode::LConst0:
                    frame.push(S8(0));
                    break;
                case Opcode::LConst1:
                    frame.push(S8(1));
                    break;                

                case Opcode::FConst0:
                    frame.push(F4(0.0f));
                    break;
                case Opcode::FConst1:
                    frame.push(F4(1.0f));
                    break;                
                case Opcode::FConst2:
                    frame.push(F4(2.0f));
                    break;

                case Opcode::DConst0:
                    frame.push(F8(0.0f));
                    break;
                case Opcode::DConst1:
                    frame.push(F8(1.0f));
                    break;

                case Opcode::BiPush:
                {
                    S1 value = static_cast<S1>(bytecode[frame.programCounter]);
                    frame.programCounter += 1;
                    frame.push(S4(value));
                    break;
                }

                case Opcode::SiPush:
                {
                    S2 value = static_cast<S2>((bytecode[frame.programCounter] << 8) | bytecode[frame.programCounter + 1]);
                    frame.programCounter += 2;
                    frame.push(S4(value));
                    break;
                }

                case Opcode::Ldc:
                {
                    U1 index = bytecode[frame.programCounter];
                    frame.programCounter++;
                    frame.push(loadConstant(index));
                    break;
                }

                case Opcode::LdcW:
                {
                    U2 index = static_cast<U2>((bytecode[frame.programCounter] << 8) | bytecode[frame.programCounter + 1]);
                    frame.programCounter += 2;

                    frame.push(loadConstant(index));
                    break;
                }

                case Opcode::Ldc2W:
                {
                    U2 index = static_cast<U2>((bytecode[frame.programCounter] << 8) | bytecode[frame.programCounter + 1]);
                    frame.programCounter += 2;

                    CPInfo* entry = classFile.getConstant<CPInfo>(index);
                    if (entry->tag == ConstantTag::Long)
                    {
                        frame.push(classFile.getConstant<ConstantLong>(index)->value);
                    }
                    else if (entry->tag == ConstantTag::Double)
                    {
                        frame.push(classFile.getConstant<ConstantDouble>(index)->value);
                    }
                    else if (entry->tag == ConstantTag::Dynamic)
                    {
                        frame.push(loadConstant(index));
                    }
                    else
                    {
                        throw std::runtime_error("ldc2_w: unexpected/unsupported constant pool tag");
                    }
                    break;
                }

                case Opcode::ILoad:
                case Opcode::LLoad:
                case Opcode::FLoad:
                case Opcode::DLoad:
                case Opcode::ALoad:
                {
                    U1 index = bytecode[frame.programCounter];
                    frame.programCounter++;
                    
                    if (index >= frame.locals.size())
                    {
                        throw std::runtime_error("*load: local variable index out of bounds");
                    }
                    frame.push(frame.locals[index]);
                    break;
                }

                case Opcode::ILoad0:
                case Opcode::LLoad0:
                case Opcode::FLoad0:
                case Opcode::DLoad0:
                case Opcode::ALoad0:
                {
                    frame.push(frame.locals[0]); 
                    break;
                }

                case Opcode::ILoad1:
                case Opcode::LLoad1:
                case Opcode::FLoad1:
                case Opcode::DLoad1:
                case Opcode::ALoad1:
                {
                    frame.push(frame.locals[1]);
                    break;
                }

                case Opcode::ILoad2:
                case Opcode::LLoad2:
                case Opcode::FLoad2:
                case Opcode::DLoad2:
                case Opcode::ALoad2:
                {
                    frame.push(frame.locals[2]);
                    break;
                }

                case Opcode::ILoad3:
                case Opcode::LLoad3:
                case Opcode::FLoad3:
                case Opcode::DLoad3:
                case Opcode::ALoad3:
                {
                    frame.push(frame.locals[3]);
                    break;
                }


                case Opcode::IALoad:
                {
                    S4 index = std::get<S4>(frame.pop());
                    Value arrayReferenceValue = frame.pop();

                    auto* ref = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if (!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: iaload on null array reference");
                    }
                    if ((*ref)->type != HeapType::Array)
                    {
                        throw std::runtime_error("iaload: reference is not an array");
                    }

                    auto* arrayObj = static_cast<ArrayHeapObject*>(*ref);
                    if (arrayObj->elementType != ValueType::Int)
                    {
                        throw std::runtime_error("iaload: array element type is not int");
                    }
                    if (index < 0 || static_cast<U4>(index) >= arrayObj->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    S4 value;
                    std::memcpy(&value, &arrayObj->primitiveData[static_cast<size_t>(index) * sizeof(S4)], sizeof(S4));
                    frame.push(value);
                    break;
                }
                case Opcode::LALoad:
                {
                    S4 index = std::get<S4>(frame.pop());
                    Value arrayReferenceValue = frame.pop();

                    auto* ref = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if (!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: laload on null array reference");
                    }
                    if ((*ref)->type != HeapType::Array)
                    {
                        throw std::runtime_error("laload: reference is not an array");
                    }

                    auto* arrayObj = static_cast<ArrayHeapObject*>(*ref);
                    if (arrayObj->elementType != ValueType::Long)
                    {
                        throw std::runtime_error("laload: array element type is not long");
                    }
                    if (index < 0 || static_cast<U4>(index) >= arrayObj->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    S8 value;
                    std::memcpy(&value, &arrayObj->primitiveData[static_cast<size_t>(index) * sizeof(S8)], sizeof(S8));
                    frame.push(value);
                    break;
                }
                case Opcode::FALoad:
                {
                    S4 index = std::get<S4>(frame.pop());
                    Value arrayReferenceValue = frame.pop();

                    auto* ref = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if (!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: faload on null array reference");
                    }
                    if ((*ref)->type != HeapType::Array)
                    {
                        throw std::runtime_error("faload: reference is not an array");
                    }

                    auto* arrayObj = static_cast<ArrayHeapObject*>(*ref);
                    if (arrayObj->elementType != ValueType::Float)
                    {
                        throw std::runtime_error("faload: array element type is not float");
                    }
                    if (index < 0 || static_cast<U4>(index) >= arrayObj->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    F4 value;
                    std::memcpy(&value, &arrayObj->primitiveData[static_cast<size_t>(index) * sizeof(F4)], sizeof(F4));
                    frame.push(value);
                    break;
                }
                case Opcode::DALoad:
                {
                    S4 index = std::get<S4>(frame.pop());
                    Value arrayReferenceValue = frame.pop();

                    auto* ref = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if (!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: daload on null array reference");
                    }
                    if ((*ref)->type != HeapType::Array)
                    {
                        throw std::runtime_error("daload: reference is not an array");
                    }

                    auto* arrayObj = static_cast<ArrayHeapObject*>(*ref);
                    if (arrayObj->elementType != ValueType::Double)
                    {
                        throw std::runtime_error("daload: array element type is not double");
                    }
                    if (index < 0 || static_cast<U4>(index) >= arrayObj->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    F8 value;
                    std::memcpy(&value, &arrayObj->primitiveData[static_cast<size_t>(index) * sizeof(F8)], sizeof(F8));
                    frame.push(value);
                    break;
                }
                case Opcode::AALoad:
                {
                    S4 index = std::get<S4>(frame.pop());
                    Value arrayReferenceValue = frame.pop();

                    auto* ref = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if (!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: aaload on null array reference");
                    }
                    if ((*ref)->type != HeapType::Array)
                    {
                        throw std::runtime_error("aaload: reference is not an array");
                    }

                    auto* arrayObj = static_cast<ArrayHeapObject*>(*ref);
                    if (arrayObj->elementType != ValueType::Reference)
                    {
                        throw std::runtime_error("aaload: array element is not of a reference type");
                    }
                    if (index < 0 || static_cast<U4>(index) >= arrayObj->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    HeapObject* value = arrayObj->referenceData[static_cast<size_t>(index)];
                    frame.push(value);
                    break;
                }
                case Opcode::BALoad:
                {
                    S4 index = std::get<S4>(frame.pop());
                    Value arrayReferenceValue = frame.pop();

                    auto* ref = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if (!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: baload on null array reference");
                    }
                    if ((*ref)->type != HeapType::Array)
                    {
                        throw std::runtime_error("baload: reference is not an array");
                    }

                    auto* arrayObj = static_cast<ArrayHeapObject*>(*ref);
                    if (arrayObj->elementType != ValueType::Byte && arrayObj->elementType != ValueType::Boolean)
                    {
                        throw std::runtime_error("baload: array element type is not byte/boolean");
                    }
                    if (index < 0 || static_cast<U4>(index) >= arrayObj->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    U1 value;
                    std::memcpy(&value, &arrayObj->primitiveData[static_cast<size_t>(index) * sizeof(U1)], sizeof(U1));
                    frame.push(value);
                    break;
                }
                case Opcode::CALoad:
                {
                    S4 index = std::get<S4>(frame.pop());
                    Value arrayReferenceValue = frame.pop();

                    auto* ref = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if (!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: caload on null array reference");
                    }
                    if ((*ref)->type != HeapType::Array)
                    {
                        throw std::runtime_error("caload: reference is not an array");
                    }

                    auto* arrayObj = static_cast<ArrayHeapObject*>(*ref);
                    if (arrayObj->elementType != ValueType::Char)
                    {
                        throw std::runtime_error("caload: array element type is not char");
                    }
                    if (index < 0 || static_cast<U4>(index) >= arrayObj->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    U2 value;
                    std::memcpy(&value, &arrayObj->primitiveData[static_cast<size_t>(index) * sizeof(U2)], sizeof(U2));
                    frame.push(static_cast<S4>(value));
                    break;
                }
                case Opcode::SALoad:
                {
                    S4 index = std::get<S4>(frame.pop());
                    Value arrayReferenceValue = frame.pop();

                    auto* ref = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if (!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: saload on null array reference");
                    }
                    if ((*ref)->type != HeapType::Array)
                    {
                        throw std::runtime_error("saload: reference is not an array");
                    }

                    auto* arrayObj = static_cast<ArrayHeapObject*>(*ref);
                    if (arrayObj->elementType != ValueType::Short)
                    {
                        throw std::runtime_error("saload: array element type is not short");
                    }
                    if (index < 0 || static_cast<U4>(index) >= arrayObj->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    S2 value;
                    std::memcpy(&value, &arrayObj->primitiveData[static_cast<size_t>(index) * sizeof(S2)], sizeof(S2));
                    frame.push(static_cast<S4>(value));
                    break;
                }
                

                case Opcode::IStore:
                case Opcode::LStore:
                case Opcode::FStore:
                case Opcode::DStore:
                {
                    U1 index = bytecode[frame.programCounter];
                    frame.programCounter++;

                    if (index >= frame.locals.size())
                    {
                        throw std::runtime_error("*store: local variable index out of bounds");
                    }

                    frame.locals[index] = frame.pop();
                    break;

                }
                case Opcode::AStore:
                {
                    U1 index = bytecode[frame.programCounter];
                    frame.programCounter++;
                    Value value = frame.pop();
                    HeapObject** objectRef = std::get_if<HeapObject*>(&value);
                    if(!objectRef)
                    {
                        throw std::runtime_error("astore: value on stack is not a reference type");
                    }
                    if (index >= frame.locals.size())
                    {
                        throw std::runtime_error("astore: local variable index out of bounds");
                    }
                    
                    
                    frame.setLocal(index, *objectRef);

                    break;
                }


                case Opcode::IStore0:
                case Opcode::LStore0:
                case Opcode::FStore0:
                case Opcode::DStore0:
                    frame.locals[0] = frame.pop();
                    break;
                case Opcode::IStore1:
                case Opcode::LStore1:
                case Opcode::FStore1:
                case Opcode::DStore1:
                    frame.locals[1] = frame.pop();
                    break;
                case Opcode::IStore2:
                case Opcode::LStore2:
                case Opcode::FStore2:
                case Opcode::DStore2:
                    frame.locals[2] = frame.pop(); 
                    break;
                case Opcode::IStore3:
                case Opcode::LStore3:
                case Opcode::FStore3:
                case Opcode::DStore3:
                    frame.locals[3] = frame.pop();
                    break;

                case Opcode::AStore0:
                {
                    Value value = frame.pop();
                    HeapObject** objectRef = std::get_if<HeapObject*>(&value);
                    if (!objectRef)
                    {
                        throw std::runtime_error("astore0: value on stack is not a reference type");
                    }
                    frame.setLocal(0, *objectRef);
                    break;
                }
                case Opcode::AStore1:
                {
                    Value value = frame.pop();
                    HeapObject** objectRef = std::get_if<HeapObject*>(&value);
                    if (!objectRef)
                    {
                        throw std::runtime_error("astore1: value on stack is not a reference type");
                    }
                    frame.setLocal(1, *objectRef);
                    break;
                }
                case Opcode::AStore2:
                {
                    Value value = frame.pop();
                    HeapObject** objectRef = std::get_if<HeapObject*>(&value);
                    if (!objectRef)
                    {
                        throw std::runtime_error("astore2: value on stack is not a reference type");
                    }
                    frame.setLocal(2, *objectRef);
                    break;
                }
                case Opcode::AStore3:
                {
                    Value value = frame.pop();
                    HeapObject** objectRef = std::get_if<HeapObject*>(&value);
                    if (!objectRef)
                    {
                        throw std::runtime_error("astore3: value on stack is not a reference type");
                    }
                    frame.setLocal(3, *objectRef);
                    break;
                }

                case Opcode::IAStore:
                {
                    S4 value = std::get<S4>(frame.pop());
                    S4 index = std::get<S4>(frame.pop());

                    Value arrayReferenceValue = frame.pop();

                    HeapObject** arrayRef = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if(!arrayRef || !(*arrayRef))
                    {
                        throw std::runtime_error("NullPointerException: iastore on null array refernce");
                    }
                    if((*arrayRef)->type != HeapType::Array)
                    {
                        throw std::runtime_error("iastore: reference is not an array");
                    }

                    ArrayHeapObject* arrayObject = static_cast<ArrayHeapObject*>(*arrayRef);
                    if(arrayObject->elementType != ValueType::Int)
                    {
                        throw std::runtime_error("iastore: array element type is not int");
                    }
                    if(index < 0 || static_cast<U4>(index) >= arrayObject->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    std::memcpy(&arrayObject->primitiveData[static_cast<size_t>(index) * sizeof(S4)], &value, sizeof(S4));
                    break;
                }
                case Opcode::LAStore:
                {
                    S8 value = std::get<S8>(frame.pop());
                    S4 index = std::get<S4>(frame.pop());

                    Value arrayReferenceValue = frame.pop();

                    HeapObject** arrayRef = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if(!arrayRef || !(*arrayRef))
                    {
                        throw std::runtime_error("NullPointerException: lastore on null array refernce");
                    }
                    if((*arrayRef)->type != HeapType::Array)
                    {
                        throw std::runtime_error("lastore: reference is not an array");
                    }

                    ArrayHeapObject* arrayObject = static_cast<ArrayHeapObject*>(*arrayRef);
                    if(arrayObject->elementType != ValueType::Long)
                    {
                        throw std::runtime_error("lastore: array element type is not long");
                    }
                    if(index < 0 || static_cast<U4>(index) >= arrayObject->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    std::memcpy(&arrayObject->primitiveData[static_cast<size_t>(index) * sizeof(S8)], &value, sizeof(S8));
                    break;
                }
                case Opcode::FAStore:
                {
                    F4 value = std::get<F4>(frame.pop());
                    S4 index = std::get<S4>(frame.pop());

                    Value arrayReferenceValue = frame.pop();

                    HeapObject** arrayRef = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if(!arrayRef || !(*arrayRef))
                    {
                        throw std::runtime_error("NullPointerException: fastore on null array refernce");
                    }
                    if((*arrayRef)->type != HeapType::Array)
                    {
                        throw std::runtime_error("fastore: reference is not an array");
                    }

                    ArrayHeapObject* arrayObject = static_cast<ArrayHeapObject*>(*arrayRef);
                    if(arrayObject->elementType != ValueType::Float)
                    {
                        throw std::runtime_error("fastore: array element type is not float");
                    }
                    if(index < 0 || static_cast<U4>(index) >= arrayObject->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    std::memcpy(&arrayObject->primitiveData[static_cast<size_t>(index) * sizeof(F4)], &value, sizeof(F4));
                    break;
                }
                case Opcode::DAStore:
                {
                    F8 value = std::get<F8>(frame.pop());
                    S4 index = std::get<S4>(frame.pop());

                    Value arrayReferenceValue = frame.pop();

                    HeapObject** arrayRef = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if(!arrayRef || !(*arrayRef))
                    {
                        throw std::runtime_error("NullPointerException: dastore on null array refernce");
                    }
                    if((*arrayRef)->type != HeapType::Array)
                    {
                        throw std::runtime_error("dastore: reference is not an array");
                    }

                    ArrayHeapObject* arrayObject = static_cast<ArrayHeapObject*>(*arrayRef);
                    if(arrayObject->elementType != ValueType::Double)
                    {
                        throw std::runtime_error("dastore: array element type is not float");
                    }
                    if(index < 0 || static_cast<U4>(index) >= arrayObject->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    std::memcpy(&arrayObject->primitiveData[static_cast<size_t>(index) * sizeof(F8)], &value, sizeof(F8));
                    break;
                }
                case Opcode::AAStore:
                {
                    Value value = frame.pop();
                    HeapObject** obj = std::get_if<HeapObject*>(&value);

                    if (!obj)
                    {
                        throw std::runtime_error("aastore: value on stack is not a reference type");
                    }

                    S4 index = std::get<S4>(frame.pop());

                    Value arrayReferenceValue = frame.pop();

                    HeapObject** arrayRef = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if(!arrayRef || !(*arrayRef))
                    {
                        throw std::runtime_error("NullPointerException: aastore on null array refernce");
                    }
                    if((*arrayRef)->type != HeapType::Array)
                    {
                        throw std::runtime_error("aastore: reference is not an array");
                    }

                    ArrayHeapObject* arrayObject = static_cast<ArrayHeapObject*>(*arrayRef);
                    if(arrayObject->elementType != ValueType::Reference)
                    {
                        throw std::runtime_error("aastore: array element type is not reference type");
                    }
                    if(index < 0 || static_cast<U4>(index) >= arrayObject->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    arrayObject->referenceData[static_cast<size_t>(index)] = *obj;

                    break;
                }
                case Opcode::BAStore:
                {
                    U1 value = static_cast<U1>(std::get<S4>(frame.pop()));
                    S4 index = std::get<S4>(frame.pop());

                    Value arrayReferenceValue = frame.pop();

                    HeapObject** arrayRef = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if(!arrayRef || !(*arrayRef))
                    {
                        throw std::runtime_error("NullPointerException: bastore on null array refernce");
                    }
                    if((*arrayRef)->type != HeapType::Array)
                    {
                        throw std::runtime_error("bastore: reference is not an array");
                    }

                    ArrayHeapObject* arrayObject = static_cast<ArrayHeapObject*>(*arrayRef);
                    if(arrayObject->elementType != ValueType::Byte && arrayObject->elementType != ValueType::Boolean)
                    {
                        throw std::runtime_error("bastore: array element type is not bool / byte");
                    }
                    if(index < 0 || static_cast<U4>(index) >= arrayObject->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    std::memcpy(&arrayObject->primitiveData[static_cast<size_t>(index) * sizeof(U1)], &value, sizeof(U1));
                    break;
                }
                case Opcode::CAStore:
                {
                    U2 value = static_cast<U2>(std::get<S4>(frame.pop()));
                    S4 index = std::get<S4>(frame.pop());

                    Value arrayReferenceValue = frame.pop();

                    HeapObject** arrayRef = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if(!arrayRef || !(*arrayRef))
                    {
                        throw std::runtime_error("NullPointerException: castore on null array refernce");
                    }
                    if((*arrayRef)->type != HeapType::Array)
                    {
                        throw std::runtime_error("castore: reference is not an array");
                    }

                    ArrayHeapObject* arrayObject = static_cast<ArrayHeapObject*>(*arrayRef);
                    if(arrayObject->elementType != ValueType::Char)
                    {
                        throw std::runtime_error("castore: array element type is not char");
                    }
                    if(index < 0 || static_cast<U4>(index) >= arrayObject->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    std::memcpy(&arrayObject->primitiveData[static_cast<size_t>(index) * sizeof(U2)], &value, sizeof(U2));
                    break;
                }
                case Opcode::SAStore:
                {
                    S4 value = std::get<S4>(frame.pop());
                    S4 index = std::get<S4>(frame.pop());

                    Value arrayReferenceValue = frame.pop();

                    HeapObject** arrayRef = std::get_if<HeapObject*>(&arrayReferenceValue);
                    if(!arrayRef || !(*arrayRef))
                    {
                        throw std::runtime_error("NullPointerException: sastore on null array refernce");
                    }
                    if((*arrayRef)->type != HeapType::Array)
                    {
                        throw std::runtime_error("sastore: reference is not an array");
                    }

                    ArrayHeapObject* arrayObject = static_cast<ArrayHeapObject*>(*arrayRef);
                    if(arrayObject->elementType != ValueType::Short)
                    {
                        throw std::runtime_error("sastore: array element type is not short");
                    }
                    if(index < 0 || static_cast<U4>(index) >= arrayObject->length)
                    {
                        throw std::runtime_error("ArrayIndexOutOfBoundsException");
                    }

                    S2 s2Value = static_cast<S2>(value);
                    std::memcpy(&arrayObject->primitiveData[static_cast<size_t>(index) * sizeof(S2)], &s2Value, sizeof(S2));
                    break;
                }

                case Opcode::Pop:
                {
                    frame.pop();
                    break;
                }
                case Opcode::Pop2:
                {
                    Value temp = frame.pop(); 

                    if (!std::get_if<S8>(&temp) && !std::get_if<F8>(&temp))
                    {
                        frame.pop();
                    }
                    
                    break;
                }

                case Opcode::Dup:
                {
                    auto top = frame.pop();

                    frame.push(top);
                    frame.push(top);

                    break;
                }
                case Opcode::DupX1:
                {
                    Value val1 = frame.pop(); 
                    Value val2 = frame.pop(); 

                    bool val1Is2Bytes = !std::get_if<F8>(&val1) && !std::get_if<S8>(&val1);
                    bool val2Is2Bytes = !std::get_if<F8>(&val2) && !std::get_if<S8>(&val2);

                    if (val1Is2Bytes || val2Is2Bytes)
                    {
                        throw std::runtime_error("dupx1: illegal value types (double / long)");
                    }

                    frame.push(val1);
                    frame.push(val2);
                    frame.push(val1);

                    break;
                }
                case Opcode::DupX2:
                {
                    Value val1 = frame.pop();
                    Value val2 = frame.pop();

                    bool val1Is2Bytes = std::get_if<F8>(&val1) || std::get_if<S8>(&val1);
                    bool val2Is2Bytes = std::get_if<F8>(&val2) || std::get_if<S8>(&val2);

                    if (val1Is2Bytes)
                    {
                        throw std::runtime_error("dup_x2: illegal value type of value1 (double / long)");
                    }

                    if (val2Is2Bytes)
                    {
                        frame.push(val1);
                        frame.push(val2);
                        frame.push(val1);
                    }
                    else
                    {
                        Value val3 = frame.pop();

                        frame.push(val1);
                        frame.push(val3);
                        frame.push(val2);
                        frame.push(val1);
                    }

                    break;
                }
                case Opcode::Dup2:
                {
                    Value val1 = frame.pop();

                    bool val1Is2Bytes = std::get_if<F8>(&val1) || std::get_if<S8>(&val1);

                    if (!val1Is2Bytes)
                    {
                        Value val2 = frame.pop();
                        bool val2Is2Bytes = std::get_if<F8>(&val2) || std::get_if<S8>(&val2);
                        if (val2Is2Bytes)
                        {
                            throw std::runtime_error("dup2: illegal value type of value2 (double / long)");
                        }
                        frame.push(val2);
                        frame.push(val1);
                        frame.push(val2);
                        frame.push(val1);
                        break;
                    }

                    frame.push(val1);
                    frame.push(val1);
                    

                    break;
                }
                case Opcode::Dup2X1:
                {
                    Value val1 = frame.pop();

                    bool val1Is2Bytes = std::get_if<F8>(&val1) || std::get_if<S8>(&val1);

                    if (!val1Is2Bytes)
                    {
                        Value val2 = frame.pop();
                        bool val2Is2Bytes = std::get_if<F8>(&val2) || std::get_if<S8>(&val2);
                        if (val2Is2Bytes)
                        {
                            throw std::runtime_error("dup2_x1: illegal value type of value2 (double / long)");
                        }

                        Value val3 = frame.pop();
                        bool val3Is2Bytes = std::get_if<F8>(&val3) || std::get_if<S8>(&val3);
                        if (val3Is2Bytes)
                        {
                            throw std::runtime_error("dup2_x1: illegal value type of value3 (double / long)");
                        }

                        frame.push(val2);
                        frame.push(val1);
                        frame.push(val3);
                        frame.push(val2);
                        frame.push(val1);

                        break;
                    }

                    Value val2 = frame.pop();
                    bool val2Is2Bytes = std::get_if<F8>(&val2) || std::get_if<S8>(&val2);
                    if (val2Is2Bytes)
                    {
                        throw std::runtime_error("dup2_x1: illegal value type of value2 (double / long)");
                    }

                    frame.push(val1);
                    frame.push(val2);
                    frame.push(val1);


                    break;
                }
                case Opcode::Dup2X2:
                {
                    Value val1 = frame.pop();
                    bool val1Is2Bytes = std::get_if<F8>(&val1) || std::get_if<S8>(&val1);

                    if (!val1Is2Bytes)
                    {
                        Value val2 = frame.pop();
                        bool val2Is2Bytes = std::get_if<F8>(&val2) || std::get_if<S8>(&val2);
                        if (val2Is2Bytes)
                        {
                            throw std::runtime_error("dup2_x2: value2 must 1 byte value1 is 1 byte");
                        }

                        Value val3 = frame.pop();
                        bool val3Is2Bytes = std::get_if<F8>(&val3) || std::get_if<S8>(&val3);
                        if (val3Is2Bytes)
                        {
                            frame.push(val2);
                            frame.push(val1);
                            frame.push(val3);
                            frame.push(val2);
                            frame.push(val1);
                            break;
                        }

                        Value val4 = frame.pop();
                        bool val4Is2Bytes = std::get_if<F8>(&val4) || std::get_if<S8>(&val4);
                        if (val4Is2Bytes)
                        {
                            throw std::runtime_error("dup2_x2: value4 must be 1 byte");
                        }

                        frame.push(val2);
                        frame.push(val1);
                        frame.push(val4);
                        frame.push(val3);
                        frame.push(val2);
                        frame.push(val1);
                        break;
                    }

                    Value val2 = frame.pop();
                    bool val2Is2Bytes = std::get_if<F8>(&val2) || std::get_if<S8>(&val2);
                    if (val2Is2Bytes)
                    {
                        frame.push(val1);
                        frame.push(val2);
                        frame.push(val1);
                        break;
                    }

                    Value val3 = frame.pop();
                    bool val3Is2Bytes = std::get_if<F8>(&val3) || std::get_if<S8>(&val3);
                    if (val3Is2Bytes)
                    {
                        throw std::runtime_error("dup2_x2: value3 must be 1 byte");
                    }

                    frame.push(val1);
                    frame.push(val3);
                    frame.push(val2);
                    frame.push(val1);
                    break;
                }

                case Opcode::Swap:
                {
                    auto val1 = frame.pop();

                    bool val1Is2Bytes = std::get_if<F8>(&val1) || std::get_if<S8>(&val1);

                    if (val1Is2Bytes)
                    {
                        throw std::runtime_error("swap: illegal value type of value1 (double / long)");
                    }

                    auto val2 = frame.pop();

                    bool val2Is2Bytes = std::get_if<F8>(&val2) || std::get_if<S8>(&val2);

                    if (val2Is2Bytes)
                    {
                        throw std::runtime_error("swap: illegal value type of value2 (double / long)");
                    }

                    frame.push(val1);
                    frame.push(val2);

                    break;
                }
                


                case Opcode::IAdd:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());
                    frame.push(S4(a + b));
                    break;
                }
                case Opcode::LAdd:
                {
                    S8 b = std::get<S8>(frame.pop());
                    S8 a = std::get<S8>(frame.pop());
                    frame.push(S8(a + b));
                    break;
                }
                case Opcode::FAdd:
                {
                    F4 b = std::get<F4>(frame.pop());
                    F4 a = std::get<F4>(frame.pop());
                    frame.push(F4(a + b));
                    break;
                }
                case Opcode::DAdd:
                {
                    F8 b = std::get<F8>(frame.pop());
                    F8 a = std::get<F8>(frame.pop());
                    frame.push(F8(a + b));
                    break;
                }



                case Opcode::ISub:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());
                    frame.push(S4(a - b));
                    break;
                }
                case Opcode::LSub:
                {
                    S8 b = std::get<S8>(frame.pop());
                    S8 a = std::get<S8>(frame.pop());
                    frame.push(S8(a - b));
                    break;
                }
                case Opcode::FSub:
                {
                    F4 b = std::get<F4>(frame.pop());
                    F4 a = std::get<F4>(frame.pop());
                    frame.push(F4(a - b));
                    break;
                }
                case Opcode::DSub:
                {
                    F8 b = std::get<F8>(frame.pop());
                    F8 a = std::get<F8>(frame.pop());
                    frame.push(F8(a - b));
                    break;
                }

                case Opcode::IMul:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());
                    frame.push(S4(a * b));
                    break;
                }
                case Opcode::LMul:
                {
                    S8 b = std::get<S8>(frame.pop());
                    S8 a = std::get<S8>(frame.pop());
                    frame.push(S8(a * b));
                    break;
                }
                case Opcode::FMul:
                {
                    F4 b = std::get<F4>(frame.pop());
                    F4 a = std::get<F4>(frame.pop());
                    frame.push(F4(a * b));
                    break;
                }
                case Opcode::DMul:
                {
                    F8 b = std::get<F8>(frame.pop());
                    F8 a = std::get<F8>(frame.pop());
                    frame.push(F8(a * b));
                    break;
                }


                case Opcode::IDiv:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());

                    if (b == 0)
                    {
                        throw std::runtime_error("ArithmeticException: division by zero");
                    }

                    frame.push(S4(a / b));
                    break;
                }
                case Opcode::LDiv:
                {
                    S8 b = std::get<S8>(frame.pop());
                    S8 a = std::get<S8>(frame.pop());

                    if (b == 0)
                    {
                        throw std::runtime_error("ArithmeticException: division by zero");
                    }

                    frame.push(S8(a / b));
                    break;
                }
                case Opcode::FDiv:
                {
                    F4 b = std::get<F4>(frame.pop());
                    F4 a = std::get<F4>(frame.pop());

                    frame.push(F4(a / b));
                    break;
                }
                case Opcode::DDiv:
                {
                    F8 b = std::get<F8>(frame.pop());
                    F8 a = std::get<F8>(frame.pop());

                    frame.push(F8(a / b));
                    break;
                }



                case Opcode::IRem:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());

                    if (b == 0)
                    {
                        throw std::runtime_error("ArithmeticException: division by zero");
                    }

                    frame.push(S4(a % b));
                    break;
                }
                case Opcode::LRem:
                {
                    S8 b = std::get<S8>(frame.pop());
                    S8 a = std::get<S8>(frame.pop());

                    if (b == 0)
                    {
                        throw std::runtime_error("ArithmeticException: division by zero");
                    }

                    frame.push(S8(a % b));
                    break;
                }
                case Opcode::FRem:
                {
                    F4 b = std::get<F4>(frame.pop());
                    F4 a = std::get<F4>(frame.pop());

                    frame.push(F4(std::fmod(a, b)));
                    break;
                }
                case Opcode::DRem:
                {
                    F8 b = std::get<F8>(frame.pop());
                    F8 a = std::get<F8>(frame.pop());

                    frame.push(F8(std::fmod(a, b)));
                    break;
                }



                case Opcode::INeg:
                {
                    S4 a = std::get<S4>(frame.pop());


                    frame.push(S4(-a));
                    break;
                }
                case Opcode::LNeg:
                {
                    S8 a = std::get<S8>(frame.pop());

                    frame.push(S8(-a));
                    break;
                }
                case Opcode::FNeg:
                {
                    F4 a = std::get<F4>(frame.pop());

                    frame.push(F4(-a));
                    break;
                }
                case Opcode::DNeg:
                {
                    F8 a = std::get<F8>(frame.pop());

                    frame.push(F8(-a));
                    break;
                }


                case Opcode::IShl:
                {
                    S4 pos = std::get<S4>(frame.pop());
                    S4 x = std::get<S4>(frame.pop());

                    U4 val = static_cast<U4>(x);
                    U4 shift = static_cast<U4>(pos) & 0x1F; /* 5 low bits 0x...***** */


                    frame.push(static_cast<S4>(val << shift));
                    break;
                }
                case Opcode::LShl:
                {
                    S4 pos = std::get<S4>(frame.pop());
                    S8 x = std::get<S8>(frame.pop());

                    U8 val = static_cast<U8>(x);
                    U8 shift = static_cast<U8>(pos) & 0x3F; /* 6 low bits 0x...****** */

                    frame.push(static_cast<S8>(val << shift));
                    break;
                }


                case Opcode::IShr:
                {
                    S4 pos = std::get<S4>(frame.pop());
                    S4 x = std::get<S4>(frame.pop());

                    U4 shift = static_cast<U4>(pos) & 0x1F; /* 5 low bits 0x...***** */


                    frame.push(static_cast<S4>(x >> shift));
                    break;
                }
                case Opcode::LShr:
                {
                    S4 pos = std::get<S4>(frame.pop());
                    S8 x = std::get<S8>(frame.pop());

                    U8 shift = static_cast<U8>(pos) & 0x3F; /* 6 low bits 0x...****** */

                    frame.push(static_cast<S8>(x >> shift));
                    break;
                }


                case Opcode::IUShr:
                {
                    S4 pos = std::get<S4>(frame.pop());
                    S4 x = std::get<S4>(frame.pop());

                    U4 val = static_cast<U4>(x);
                    U4 shift = static_cast<U4>(pos) & 0x1F; /* 5 low bits 0x...***** */


                    frame.push(static_cast<S4>(val >> shift));
                    break;
                }
                case Opcode::LUShr:
                {
                    S4 pos = std::get<S4>(frame.pop());
                    S8 x = std::get<S8>(frame.pop());

                    U8 val = static_cast<U8>(x);
                    U8 shift = static_cast<U8>(pos) & 0x3F; /* 6 low bits 0x...****** */

                    frame.push(static_cast<S8>(val >> shift));
                    break;
                }




                case Opcode::IAnd:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());
                    frame.push(S4(a & b));
                    break;
                }
                case Opcode::LAnd:
                {
                    S8 b = std::get<S8>(frame.pop());
                    S8 a = std::get<S8>(frame.pop());
                    frame.push(S8(a & b));
                    break;
                }

                case Opcode::IOr:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());
                    frame.push(S4(a | b));
                    break;
                }
                case Opcode::LOr:
                {
                    S8 b = std::get<S8>(frame.pop());
                    S8 a = std::get<S8>(frame.pop());
                    frame.push(S8(a | b));
                    break;
                }

                case Opcode::IXor:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());
                    frame.push(S4(a ^ b));
                    break;
                }
                case Opcode::LXor:
                {
                    S8 b = std::get<S8>(frame.pop());
                    S8 a = std::get<S8>(frame.pop());
                    frame.push(S8(a ^ b));
                    break;
                }

                case Opcode::IInc:
                {
                    U1 idx = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S1 cons = static_cast<S1>(bytecode[frame.programCounter]);
                    frame.programCounter++;

                    if (idx >= frame.locals.size())
                    {
                        throw std::runtime_error("iinc: local variable index out of bounds");
                    }

                    S4 val = std::get<S4>(frame.locals[idx]);

                    frame.locals[idx] = S4(val  + cons);

                    break;
                }

                case Opcode::I2L:
                {
                    S4 intVal = std::get<S4>(frame.pop());

                    S8 longVal = static_cast<S8>(intVal);

                    frame.push(longVal);

                    break;
                }
                case Opcode::I2F:
                {
                    S4 intVal = std::get<S4>(frame.pop());

                    F4 floatVal = static_cast<F4>(intVal);

                    frame.push(floatVal);

                    break;
                }
                case Opcode::I2D:
                {
                    S4 intVal = std::get<S4>(frame.pop());

                    F8 doubleVal = static_cast<F8>(intVal);

                    frame.push(doubleVal);

                    break;
                }
                case Opcode::L2I:
                {
                    S8 longVal = std::get<S8>(frame.pop());

                    S4 intVal = static_cast<S4>(longVal);

                    frame.push(intVal);
                    break;
                }
                case Opcode::L2F:
                {
                    S8 longVal = std::get<S8>(frame.pop());

                    F4 floatVal = static_cast<F4>(longVal);
                    
                    frame.push(floatVal);
                    break;
                }
                case Opcode::L2D:
                {
                    S8 longVal = std::get<S8>(frame.pop());

                    F8 doubleVal = static_cast<F8>(longVal);

                    frame.push(doubleVal);

                    break;
                }
                case Opcode::F2I:
                {
                    F4 floatVal = std::get<F4>(frame.pop());

                    S4 intVal;

                    if (std::isnan(floatVal))
                    {
                        intVal = 0;
                    }
                    else if (floatVal >= static_cast<F4>(std::numeric_limits<S4>::max()))
                    {
                        intVal = std::numeric_limits<S4>::max();
                    }
                    else if (floatVal <= static_cast<F4>(std::numeric_limits<S4>::min()))
                    {
                        intVal = std::numeric_limits<S4>::min();
                    }
                    else
                    {
                        intVal = static_cast<S4>(floatVal);
                    }

                    
                    frame.push(intVal);
                    
                    break;
                }
                case Opcode::F2L:
                {
                    F4 floatVal = std::get<F4>(frame.pop());
                    S8 longVal;

                    if (std::isnan(floatVal))
                    {
                        longVal = 0;
                    }
                    else if (floatVal >= static_cast<F4>(std::numeric_limits<S8>::max()))
                    {
                        longVal = std::numeric_limits<S8>::max();
                    }
                    else if (floatVal <= static_cast<F4>(std::numeric_limits<S8>::min()))
                    {
                        longVal = std::numeric_limits<S8>::min();
                    }
                    else
                    {
                        longVal = static_cast<S8>(floatVal);
                    }


                    frame.push(longVal);
                    break;
                }
                case Opcode::F2D:
                {
                    F4 floatVal = std::get<F4>(frame.pop());

                    F8 doubleVal = static_cast<F8>(floatVal);

                    frame.push(doubleVal);

                    break;
                }
                case Opcode::D2I:
                {
                    F8 doubleVal = std::get<F8>(frame.pop());
                    S4 intVal;

                    if (std::isnan(doubleVal))
                    {
                        intVal = 0;
                    }
                    else if (doubleVal >= static_cast<F8>(std::numeric_limits<S4>::max()))
                    {
                        intVal = std::numeric_limits<S4>::max();
                    }
                    else if (doubleVal <= static_cast<F8>(std::numeric_limits<S4>::min()))
                    {
                        intVal = std::numeric_limits<S4>::min();
                    }
                    else
                    {
                        intVal = static_cast<S4>(doubleVal);
                    }


                    frame.push(intVal);
                    break;
                }
                case Opcode::D2L:
                {
                    F8 doubleVal = std::get<F8>(frame.pop());
                    S8 longVal;

                    if (std::isnan(doubleVal))
                    {
                        longVal = 0;
                    }
                    else if (doubleVal >= static_cast<F8>(std::numeric_limits<S8>::max()))
                    {
                        longVal = std::numeric_limits<S8>::max();
                    }
                    else if (doubleVal <= static_cast<F8>(std::numeric_limits<S8>::min()))
                    {
                        longVal = std::numeric_limits<S8>::min();
                    }
                    else
                    {
                        longVal = static_cast<S8>(doubleVal);
                    }

                    frame.push(longVal);
                    break;
                }
                case Opcode::D2F:
                {
                    F8 doubleVal = std::get<F8>(frame.pop());
                    F4 floatVal = static_cast<F4>(doubleVal);
                    frame.push(floatVal);
                    break;
                }
                case Opcode::I2B:
                {
                    S4 intVal = std::get<S4>(frame.pop());
                    S1 byteVal = static_cast<S1>(intVal);

                    frame.push(S4(byteVal));   
                    break;
                }
                case Opcode::I2C:
                {
                    S4 intVal = std::get<S4>(frame.pop());
                    U2 charVal = static_cast<U2>(intVal);

                    frame.push(S4(charVal));   
                    break;
                }
                case Opcode::I2S:
                {
                    S4 intVal = std::get<S4>(frame.pop());
                    S2 shortVal = static_cast<S2>(intVal);

                    frame.push(S4(shortVal));  

                    break;
                }

                case Opcode::LCmp:
                {
                    S8 b = std::get<S8>(frame.pop());
                    S8 a = std::get<S8>(frame.pop());

                    if (b == a)
                    {
                        frame.push(S4(0));
                        break;
                    }
                    else if (b < a)
                    {
                        frame.push(S4(1));
                        break;
                    }
                    else
                    {
                        frame.push(S4(-1));
                        break;
                    }

                    break;
                }
                case Opcode::FCmpL:
                {
                    F4 b = std::get<F4>(frame.pop());
                    F4 a = std::get<F4>(frame.pop());

                    if (std::isnan(a) || std::isnan(b))
                    {
                        frame.push(S4(-1));
                    }
                
                    else if (b == a)
                    {
                        frame.push(S4(0));
                        break;
                    }
                    else if (b < a)
                    {
                        frame.push(S4(1));
                        break;
                    }
                    else
                    {
                        frame.push(S4(-1));
                        break;
                    }

                    break;
                }
                case Opcode::FCmpG:
                {
                    F4 b = std::get<F4>(frame.pop());
                    F4 a = std::get<F4>(frame.pop());

                    if (std::isnan(a) || std::isnan(b))
                    {
                        frame.push(S4(1));
                    }
                    else if (b == a)
                    {
                        frame.push(S4(0));
                        break;
                    }
                    else if (b < a)
                    {
                        frame.push(S4(1));
                        break;
                    }
                    else
                    {
                        frame.push(S4(-1));
                        break;
                    }

                    break;
                }
                case Opcode::DCmpL:
                {
                    F8 b = std::get<F8>(frame.pop());
                    F8 a = std::get<F8>(frame.pop());

                    if (std::isnan(a) || std::isnan(b))
                    {
                        frame.push(S4(-1));
                    }
                
                    else if (b == a)
                    {
                        frame.push(S4(0));
                        break;
                    }
                    else if (b < a)
                    {
                        frame.push(S4(1));
                        break;
                    }
                    else
                    {
                        frame.push(S4(-1));
                        break;
                    }

                    break;
                }
                case Opcode::DCmpG:
                {
                    F8 b = std::get<F8>(frame.pop());
                    F8 a = std::get<F8>(frame.pop());

                    if (std::isnan(a) || std::isnan(b))
                    {
                        frame.push(S4(1));
                    }
                    else if (b == a)
                    {
                        frame.push(S4(0));
                        break;
                    }
                    else if (b < a)
                    {
                        frame.push(S4(1));
                        break;
                    }
                    else
                    {
                        frame.push(S4(-1));
                        break;
                    }

                    break;
                }
                case Opcode::IfEq:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);

                    S4 val = std::get<S4>(frame.pop());

                    if (val == 0)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }

                    break;
                }
                case Opcode::IfNe:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);

                    S4 val = std::get<S4>(frame.pop());

                    if (val != 0)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }

                    break;
                }
                case Opcode::IfLt:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);

                    S4 val = std::get<S4>(frame.pop());

                    if (val < 0)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }

                    break;
                }
                case Opcode::IfGe:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);

                    S4 val = std::get<S4>(frame.pop());

                    if (val >= 0)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }

                    break;
                }
                case Opcode::IfGt:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);

                    S4 val = std::get<S4>(frame.pop());

                    if (val > 0)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }

                    break;
                }
                case Opcode::IfLe:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);

                    S4 val = std::get<S4>(frame.pop());

                    if (val <= 0)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }

                    break;
                }
                case Opcode::IfICmpEq:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());

                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);


                    if (a == b)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }


                    break;
                }
                case Opcode::IfICmpNe:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());

                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);


                    if (a != b)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }


                    break;
                }
                case Opcode::IfICmpLt:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());

                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);


                    if (a < b)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }


                    break;
                }
                case Opcode::IfICmpGe:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());

                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);


                    if (a >= b)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }


                    break;
                }
                case Opcode::IfICmpGt:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());

                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);


                    if (a > b)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }


                    break;
                }
                case Opcode::IfICmpLe:
                {
                    S4 b = std::get<S4>(frame.pop());
                    S4 a = std::get<S4>(frame.pop());

                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);


                    if (a <= b)
                    {
                        frame.programCounter = opcodeStart + brachOffset;
                    }


                    break;
                }
                case Opcode::IfACmpEq:
                {
                    Value bRef = frame.pop();
                    Value aRef = frame.pop();

                    HeapObject** b = std::get_if<HeapObject*>(&bRef);
                    HeapObject** a = std::get_if<HeapObject*>(&aRef);

                    if (!a || !b)
                    {
                        throw std::runtime_error("if_acmpeq: operands are not reference types");
                    }

                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S2 branchOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);

                    if (*a == *b)
                    {
                        frame.programCounter = opcodeStart + branchOffset;
                    }
                    break;
                }
                case Opcode::IfACmpNe:
                {
                    Value bRef = frame.pop();
                    Value aRef = frame.pop();

                    HeapObject** b = std::get_if<HeapObject*>(&bRef);
                    HeapObject** a = std::get_if<HeapObject*>(&aRef);

                    if (!a || !b)
                    {
                        throw std::runtime_error("if_acmpne: operands are not reference types");
                    }

                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S2 branchOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);

                    if (*a != *b)
                    {
                        frame.programCounter = opcodeStart + branchOffset;
                    }
                    break;
                }

                case Opcode::Goto:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    S2 brachOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);

                    frame.programCounter = opcodeStart + brachOffset;

                    break;
                }


                case Opcode::Jsr:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    U1 branchByte1 = bytecode[frame.programCounter++];
                    U1 branchByte2 = bytecode[frame.programCounter++];
                    S2 branchOffset = static_cast<S2>(branchByte1 << 8 | branchByte2);

                    S4 returnAddress = static_cast<S4>(frame.programCounter);
                    frame.push(returnAddress);

                    frame.programCounter = opcodeStart + branchOffset;
                    break;
                }
                case Opcode::Ret:
                {
                    U1 index = bytecode[frame.programCounter];
                    frame.programCounter++;

                    if (index >= frame.locals.size())
                    {
                        throw std::runtime_error("ret: local variable index out of bounds");
                    }

                    S4 returnAddress = std::get<S4>(frame.locals[index]);
                    frame.programCounter = static_cast<U8>(returnAddress);
                    break;
                }


                case Opcode::TableSwitch:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    while (frame.programCounter % 4 != 0)
                    {
                        frame.programCounter++;
                    }

                    S4 defaultOffset = static_cast<S4>(
                        (static_cast<U4>(bytecode[frame.programCounter]) << 24) |
                        (static_cast<U4>(bytecode[frame.programCounter + 1]) << 16) |
                        (static_cast<U4>(bytecode[frame.programCounter + 2]) << 8) |
                        (static_cast<U4>(bytecode[frame.programCounter + 3]))
                    );
                    frame.programCounter += 4;


                    S4 low = static_cast<S4>(
                        (static_cast<U4>(bytecode[frame.programCounter]) << 24) |
                        (static_cast<U4>(bytecode[frame.programCounter + 1]) << 16) |
                        (static_cast<U4>(bytecode[frame.programCounter + 2]) << 8) |
                        (static_cast<U4>(bytecode[frame.programCounter + 3]))
                    );
                    frame.programCounter += 4;

                    S4 high = static_cast<S4>(
                        (static_cast<U4>(bytecode[frame.programCounter]) << 24) |
                        (static_cast<U4>(bytecode[frame.programCounter + 1]) << 16) |
                        (static_cast<U4>(bytecode[frame.programCounter + 2]) << 8) |
                        (static_cast<U4>(bytecode[frame.programCounter + 3]))
                    );
                    frame.programCounter += 4;

                    S4 index = std::get<S4>(frame.pop());

                    if (index < low || index > high)
                    {
                        frame.programCounter = opcodeStart + defaultOffset;
                    }
                    else
                    {
                        U8 entryPos = frame.programCounter + static_cast<U8>(index - low) * 4;
                        
                        S4 jumpOffset = static_cast<S4>(
                            (static_cast<U4>(bytecode[entryPos]) << 24) |
                            (static_cast<U4>(bytecode[entryPos + 1]) << 16) |
                            (static_cast<U4>(bytecode[entryPos + 2]) << 8) |
                            (static_cast<U4>(bytecode[entryPos + 3]))
                        );

                        frame.programCounter = opcodeStart + jumpOffset;
                    }
                    break;
                }
                case Opcode::LookupSwitch:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    while (frame.programCounter % 4 != 0)
                    {
                        frame.programCounter++;
                    }

                    S4 defaultOffset = static_cast<S4>(
                        (static_cast<U4>(bytecode[frame.programCounter]) << 24) |
                        (static_cast<U4>(bytecode[frame.programCounter + 1]) << 16) |
                        (static_cast<U4>(bytecode[frame.programCounter + 2]) << 8) |
                        (static_cast<U4>(bytecode[frame.programCounter + 3]))
                    );
                    frame.programCounter += 4;

                    S4 npairs = static_cast<S4>(
                        (static_cast<U4>(bytecode[frame.programCounter]) << 24) |
                        (static_cast<U4>(bytecode[frame.programCounter + 1]) << 16) |
                        (static_cast<U4>(bytecode[frame.programCounter + 2]) << 8) |
                        (static_cast<U4>(bytecode[frame.programCounter + 3]))
                    );
                    frame.programCounter += 4;

                    S4 key = std::get<S4>(frame.pop());

                    S4 target = defaultOffset;
                    bool found = false;

                    for (S4 i = 0; i < npairs && !found; ++i)
                    {
                        S4 match = static_cast<S4>(
                            (static_cast<U4>(bytecode[frame.programCounter]) << 24) |
                            (static_cast<U4>(bytecode[frame.programCounter + 1]) << 16) |
                            (static_cast<U4>(bytecode[frame.programCounter + 2]) << 8) |
                            (static_cast<U4>(bytecode[frame.programCounter + 3]))
                        );
                        frame.programCounter += 4;

                        S4 offset = static_cast<S4>(
                            (static_cast<U4>(bytecode[frame.programCounter]) << 24) |
                            (static_cast<U4>(bytecode[frame.programCounter + 1]) << 16) |
                            (static_cast<U4>(bytecode[frame.programCounter + 2]) << 8) |
                            (static_cast<U4>(bytecode[frame.programCounter + 3]))
                        );
                        frame.programCounter += 4;

                        if (match == key)
                        {
                            target = offset;
                            found = true;
                        }
                    }

                    frame.programCounter = opcodeStart + target;
                    break;
                }


                case Opcode::IReturn:
                {
                    return std::get<S4>(frame.pop());
                }
                case Opcode::LReturn:
                {
                    return std::get<S8>(frame.pop());
                }
                case Opcode::FReturn:
                {
                    return std::get<F4>(frame.pop());
                }
                case Opcode::DReturn:
                {
                    return std::get<F8>(frame.pop());
                }
                case Opcode::AReturn:
                {
                    return frame.pop();   
                }
                case Opcode::Return:
                {
                    return Value();
                }

                

                case Opcode::GetStatic:
                {
                    auto indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantFieldref* fieldref = classFile.getConstant<ConstantFieldref>(index);
                    ConstantNameAndType* nameAndType = classFile.getConstant<ConstantNameAndType>(fieldref->nameAndTypeIndex);
                    ConstantUtf8*  fieldNameUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex);
                    std::string name = fieldNameUTF8->value;

                    ClassFile* owner = resolveFieldOwner(&classFile, name);
                    if (!owner)
                    {
                        throw std::runtime_error("getstatic: field \"" + name + "\" not found in class hierarchy");
                    }

                    auto& fields = staticFields[owner];
                    auto iter = fields.find(name);

                    if (iter == fields.end())
                    {
                        fields[name] = Value(std::in_place_type<S4>, 0);
                        iter = fields.find(name);
                    }
                    
                    frame.push(iter->second);
                    break;
                }

                case Opcode::PutStatic:
                {
                    auto indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantFieldref* fieldref = classFile.getConstant<ConstantFieldref>(index);
                    ConstantNameAndType* nameAndType = classFile.getConstant<ConstantNameAndType>(fieldref->nameAndTypeIndex);
                    ConstantUtf8*  fieldNameUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex);
                    std::string name = fieldNameUTF8->value;
                    
                    ClassFile* owner = resolveFieldOwner(&classFile, name);
                    if (!owner)
                    {
                        throw std::runtime_error("putstatic: field \"" + name + "\" not found in class hierarchy");
                    }


                    staticFields[owner][name] = frame.pop();
                    
                    break;
                }
                case Opcode::GetField:
                {
                    auto indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantFieldref* fieldref = classFile.getConstant<ConstantFieldref>(index);
                    ConstantNameAndType* nameAndType = classFile.getConstant<ConstantNameAndType>(fieldref->nameAndTypeIndex);
                    ConstantUtf8*  fieldNameUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex);
                    std::string name = fieldNameUTF8->value;
                    

                    Value objRefVal = frame.pop();
                    HeapObject** objRef = std::get_if<HeapObject*>(&objRefVal);

                    if (!objRef || !*objRef)
                    {
                        throw std::runtime_error("NullPointerException: getfield on null reference");
                    }
                    if ((*objRef)->type != HeapType::Object)
                    {
                        throw std::runtime_error("getfield: reference is not an object instance");
                    }
                    
                    auto* instance = static_cast<ObjectHeapObject*>(*objRef);
                    auto iter = instance->fields.find(name);


                    if (iter == instance->fields.end())
                    {
                        throw std::runtime_error("getfield: field \"" + name + "\" not found on instance" );
                    }
                    
                    FieldSlot& slot = iter->second;

                    switch (slot.type)
                    {
                        case ValueType::Reference:
                        {
                            frame.push(slot.reference);
                            break;
                        }

                        case ValueType::Int:
                        {
                            S4 in;
                            std::memcpy(&in, slot.primitiveData.data(), sizeof(S4));
                            frame.push(in);
                            break;
                        }
                        case ValueType::Long:
                        {
                            S8 lo;
                            std::memcpy(&lo, slot.primitiveData.data(), sizeof(S8));
                            frame.push(lo);
                            break;
                        }
                        case ValueType::Float:
                        {
                            F4 fl;
                            std::memcpy(&fl, slot.primitiveData.data(), sizeof(F4));
                            frame.push(fl);
                            break;
                        }
                        case ValueType::Double:
                        {
                            F8 db;
                            std::memcpy(&db, slot.primitiveData.data(), sizeof(F8));
                            frame.push(db);
                            break;
                        }
                        case ValueType::Boolean:
                        {
                            U1 b;
                            std::memcpy(&b, slot.primitiveData.data(), sizeof(U1));
                            frame.push(S4(b));
                            break;
                        }
                        case ValueType::Byte:
                        {
                            S1 by;
                            std::memcpy(&by, slot.primitiveData.data(), sizeof(S1));
                            frame.push(S4(by));
                            break;
                        }
                        case ValueType::Char:
                        {
                            U2 ch;
                            std::memcpy(&ch, slot.primitiveData.data(), sizeof(U2));
                            frame.push(S4(ch));
                            break;
                        }
                        case ValueType::Short:
                        {
                            S2 sh;
                            std::memcpy(&sh, slot.primitiveData.data(), sizeof(S2));
                            frame.push(S4(sh));
                            break;
                        }
                        default:
                        {
                            throw std::runtime_error("getfield: unsupported or wrong field type");
                        }
                    }

                    break;
                }
                case Opcode::PutField:
                {
                    auto indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantFieldref* fieldref = classFile.getConstant<ConstantFieldref>(index);
                    ConstantNameAndType* nameAndType = classFile.getConstant<ConstantNameAndType>(fieldref->nameAndTypeIndex);
                    ConstantUtf8*  fieldNameUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex);
                    std::string name = fieldNameUTF8->value;
                    
                    Value val = frame.pop();
                    Value objRefVal = frame.pop();
                    HeapObject** objRef = std::get_if<HeapObject*>(&objRefVal);

                    if (!objRef || !*objRef)
                    {
                        throw std::runtime_error("NullPointerException: putfield on null reference");
                    }
                    if ((*objRef)->type != HeapType::Object)
                    {
                        throw std::runtime_error("putfield: reference is not an object instance");
                    }
                                
                    auto* instance = static_cast<ObjectHeapObject*>(*objRef);
                    FieldSlot slot;

                    auto* descriptorUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->descriptorIndex);
                    std::string descriptor = descriptorUTF8->value;

                    if (auto* ref = std::get_if<HeapObject*>(&val))
                    {
                        slot.type = ValueType::Reference;
                        slot.reference = *ref;
                    }
                    else if (auto* in = std::get_if<S4>(&val))
                    {
                        S4 raw = *in;
                        if (descriptor == "Z")
                        {
                            slot.type = ValueType::Boolean;
                            U1 narrowed = static_cast<U1>(raw) & 0x1;
                            slot.primitiveData.assign(1, narrowed);
                        }
                        else if (descriptor == "B")
                        {
                            slot.type = ValueType::Byte;
                            S1 narrowed = static_cast<S1>(raw);
                            slot.primitiveData.resize(sizeof(S1));
                            std::memcpy(slot.primitiveData.data(), &narrowed, sizeof(S1));
                        }
                        else if (descriptor == "C")
                        {
                            slot.type = ValueType::Char;
                            U2 narrowed = static_cast<U2>(raw);
                            slot.primitiveData.resize(sizeof(U2));
                            std::memcpy(slot.primitiveData.data(), &narrowed, sizeof(U2));
                        }
                        else if (descriptor == "S")
                        {
                            slot.type = ValueType::Short;
                            S2 narrowed = static_cast<S2>(raw);
                            slot.primitiveData.resize(sizeof(S2));
                            std::memcpy(slot.primitiveData.data(), &narrowed, sizeof(S2));
                        }
                        else
                        {
                            slot.type = ValueType::Int;
                            slot.primitiveData.resize(sizeof(S4));
                            std::memcpy(slot.primitiveData.data(), &raw, sizeof(S4));
                        }
                    }
                    else if (auto* l = std::get_if<S8>(&val))
                    {
                        slot.type = ValueType::Long;
                        slot.primitiveData.resize(sizeof(S8));
                        std::memcpy(slot.primitiveData.data(), l, sizeof(S8));
                    }
                    else if (auto* f = std::get_if<F4>(&val))
                    {
                        slot.type = ValueType::Float;
                        slot.primitiveData.resize(sizeof(F4));
                        std::memcpy(slot.primitiveData.data(), f, sizeof(F4));
                    }
                    else if (auto* d = std::get_if<F8>(&val))
                    {
                        slot.type = ValueType::Double;
                        slot.primitiveData.resize(sizeof(F8));
                        std::memcpy(slot.primitiveData.data(), d, sizeof(F8));
                    }
                    else
                    {
                        throw std::runtime_error("putfield: unsupported or wrong value type");
                    }

                    instance->fields[name] = slot;

                    break;
                }
                case Opcode::InvokeVirtual:
                {
                    auto indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantMethodref* methodref = classFile.getConstant<ConstantMethodref>(index);
                    ConstantNameAndType* nameAndType = classFile.getConstant<ConstantNameAndType>(methodref->nameAndTypeIndex);
                    ConstantUtf8* methodNameUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex);
                    ConstantUtf8* descriptorUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->descriptorIndex);
                    std::string methodName = methodNameUTF8->value;
                    std::string descriptor = descriptorUTF8->value;

                    std::vector<char> paramTypes = parseParameterTypes(descriptor);

                    std::vector<Value> args(paramTypes.size());

                    for (U4 i = paramTypes.size() ; i > 0;)
                    {
                        --i;
                        args[i] = frame.pop();
                    }

                    Value objRefVal = frame.pop();
                    HeapObject** objectRef = std::get_if<HeapObject*>(&objRefVal);
                    if (!objectRef || !*objectRef)
                    {
                        throw std::runtime_error("NullPointerException: invokevirtual on null reference");
                    }

                    ObjectHeapObject* instance = static_cast<ObjectHeapObject*>(*objectRef);
                    
                    const MethodInfo* targetMethod = nullptr;
                    ClassFile* targetClass = resolveMethodOwner(instance->javaClass, methodName, descriptor, &targetMethod);
                    if (!targetClass)
                    {
                        throw std::runtime_error("invokevirtual: method \"" + methodName + " " + descriptor + "\" not found");
                    }
                    if (!targetMethod)
                    {
                        throw std::runtime_error("invokevirtual: method \"" + methodName + " " + descriptor + "\" not found");
                    }

                    const CodeAttribute* targetCode = targetClass->getCode(*targetMethod);
                    if (!targetCode)
                    {
                        throw std::runtime_error("invokevirtual: method \"" + methodName + "\" has no Code attribute");
                    }

                    Frame invokedFrame(targetCode->maxLocals, targetCode->maxStack);
                    invokedFrame.setLocal(0, *objectRef);

                    U2 localSlot = 1;

                    for (U4 l = 0; l < paramTypes.size(); ++l)
                    {
                        invokedFrame.locals[localSlot] = args[l];
                        localSlot += (paramTypes[l] == 'J' || paramTypes[l] == 'D') ? 2 : 1;
                    }

                    FrameGuard invokedGuard(*this,invokedFrame);
                    Value result = execute(*targetClass, *targetCode);

                    char returnType = descriptor.back();

                    if (returnType != 'V')
                    {
                        frame.push(result);
                    }

                    break;
                }
                case Opcode::InvokeSpecial:
                {
                    auto indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    auto indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantMethodref* methodref = classFile.getConstant<ConstantMethodref>(index);
                    ConstantClass* classRef = classFile.getConstant<ConstantClass>(methodref->classIndex);
                    ConstantUtf8* classNameUTF8 = classFile.getConstant<ConstantUtf8>(classRef->nameIndex);
                    std::string targetClassName = classNameUTF8->value;

                    ConstantNameAndType* nameAndType = classFile.getConstant<ConstantNameAndType>(methodref->nameAndTypeIndex);
                    ConstantUtf8* methodNameUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex);
                    ConstantUtf8* descriptorUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->descriptorIndex);
                    std::string methodName = methodNameUTF8->value;
                    std::string descriptor = descriptorUTF8->value;

                    std::vector<char> paramTypes = parseParameterTypes(descriptor);

                    std::vector<Value> args(paramTypes.size());
                    for (U4 i = paramTypes.size(); i > 0;)
                    {
                        --i;
                        args[i] = frame.pop();
                    }

                    Value objRefVal = frame.pop();
                    HeapObject** objectRef = std::get_if<HeapObject*>(&objRefVal);
                    if (!objectRef || !*objectRef)
                    {
                        throw std::runtime_error("NullPointerException: invokespecial on null reference");
                    }

                    ClassFile* namedClass = loader.loadClass(targetClassName);
                    if (!namedClass)
                    {
                        throw std::runtime_error("invokespecial: failed to load class \"" + targetClassName + "\"");
                    }

                    const MethodInfo* targetMethod = nullptr;
                    ClassFile* targetClass = resolveMethodOwner(namedClass, methodName, descriptor, &targetMethod);
                    if (!targetClass)
                    {
                        throw std::runtime_error("invokespecial: method \"" + methodName + " " + descriptor + "\" not found");
                    }

                    const CodeAttribute* targetCode = targetClass->getCode(*targetMethod);
                    if (!targetCode)
                    {
                        throw std::runtime_error("invokespecial: method \"" + methodName + "\" has no Code attribute");
                    }

                    Frame invokedFrame(targetCode->maxLocals, targetCode->maxStack);
                    invokedFrame.setLocal(0, *objectRef);

                    U2 localSlot = 1;
                    for (U4 l = 0; l < paramTypes.size(); ++l)
                    {
                        invokedFrame.locals[localSlot] = args[l];
                        localSlot += (paramTypes[l] == 'J' || paramTypes[l] == 'D') ? 2 : 1;
                    }

                    FrameGuard invokedGuard(*this, invokedFrame);
                    Value result = execute(*targetClass, *targetCode);

                    char returnType = descriptor.back();
                    if (returnType != 'V')
                    {
                        frame.push(result);
                    }

                    break;
                }
                case Opcode::InvokeStatic:
                {
                    U1 indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    U1 indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantMethodref* methodref = classFile.getConstant<ConstantMethodref>(index);
                    ConstantClass* classRef = classFile.getConstant<ConstantClass>(methodref->classIndex);
                    ConstantUtf8* classNameUTF8 = classFile.getConstant<ConstantUtf8>(classRef->nameIndex);
                    std::string targetClassName = classNameUTF8->value;

                    ConstantNameAndType* nameAndType = classFile.getConstant<ConstantNameAndType>(methodref->nameAndTypeIndex);
                    ConstantUtf8* methodNameUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex);
                    ConstantUtf8* descriptorUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->descriptorIndex);
                    std::string methodName = methodNameUTF8->value;
                    std::string descriptor = descriptorUTF8->value;

                    std::vector<char> paramTypes = parseParameterTypes(descriptor);

                    std::vector<Value> args(paramTypes.size());
                    for (U4 i = paramTypes.size(); i > 0; )
                    {
                        --i;
                        args[i] = frame.pop();
                    }

                    ClassFile* namedClass = loader.loadClass(targetClassName);

                    const MethodInfo* targetMethod = nullptr;
                    ClassFile* targetClass = namedClass ? resolveMethodOwner(namedClass, methodName, descriptor, &targetMethod) : nullptr;

                    const CodeAttribute* targetCode = (targetClass && targetMethod) ? targetClass->getCode(*targetMethod) : nullptr;

                    if (targetCode)
                    {
                        Frame invokedFrame(targetCode->maxLocals, targetCode->maxStack);

                        U2 localSlot = 0;
                        for (U4 l = 0; l < paramTypes.size(); ++l)
                        {
                            invokedFrame.locals[localSlot] = args[l];
                            localSlot += (paramTypes[l] == 'J' || paramTypes[l] == 'D') ? 2 : 1;
                        }

                        FrameGuard invokedGuard(*this, invokedFrame);
                        Value result = execute(*targetClass, *targetCode);

                        if (descriptor.back() != 'V')
                        {
                            frame.push(result);
                        }
                        break;
                    }

                    MethodCall call
                    { 
                        .receiver = nullptr, 
                        .args = std::move(args) 
                    };
                    
                    Value nativeResult;
                    if (tryInvokeNative(targetClassName, methodName, descriptor, call, nativeResult))
                    {
                        if (descriptor.back() != 'V')
                        {
                            frame.push(nativeResult);
                        }
                        break;
                    }

                    throw std::runtime_error("invokestatic: method \"" + methodName + " " + descriptor + "\" not found (no bytecode, no native)");
                }
                case Opcode::InvokeInterface:
                {
                    auto indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    auto indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U1 count = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U1 reserved = bytecode[frame.programCounter];
                    frame.programCounter++;

                    (void) count;
                    (void) reserved;

                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantInterfaceMethodref* methodref = classFile.getConstant<ConstantInterfaceMethodref>(index);
                    ConstantNameAndType* nameAndType = classFile.getConstant<ConstantNameAndType>(methodref->nameAndTypeIndex);
                    ConstantUtf8* methodNameUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex);
                    ConstantUtf8* descriptorUTF8 = classFile.getConstant<ConstantUtf8>(nameAndType->descriptorIndex);
                    std::string methodName = methodNameUTF8->value;
                    std::string descriptor = descriptorUTF8->value;

                    std::vector<char> paramTypes = parseParameterTypes(descriptor);

                    std::vector<Value> args(paramTypes.size());
                    for (U4 i = paramTypes.size(); i > 0; )
                    {
                        --i;
                        args[i] = frame.pop();
                    }

                    Value objRefVal = frame.pop();
                    HeapObject** objectRef = std::get_if<HeapObject*>(&objRefVal);
                    if (!objectRef || !*objectRef)
                    {
                        throw std::runtime_error("NullPointerException: invokeinterface on null reference");
                    }
                    if ((*objectRef)->type != HeapType::Object)
                    {
                        throw std::runtime_error("invokeinterface: reference is not an object instance");
                    }

                    ObjectHeapObject* instance = static_cast<ObjectHeapObject*>(*objectRef);
                    const MethodInfo* targetMethod = nullptr;
                    ClassFile* targetClass = resolveMethodOwner(instance->javaClass, methodName, descriptor, &targetMethod);
                    if (!targetClass)
                    {
                        throw std::runtime_error("invokeinterface: method \"" + methodName + " " + descriptor + "\" not found");
                    }

                    const CodeAttribute* targetCode = targetClass->getCode(*targetMethod);
                    if (!targetCode)
                    {
                        throw std::runtime_error("invokeinterface: method \"" + methodName + "\" has no Code attribute");
                    }

                    Frame invokedFrame(targetCode->maxLocals, targetCode->maxStack);
                    invokedFrame.setLocal(0, *objectRef);

                    U2 localSlot = 1;
                    for (U4 l = 0; l < paramTypes.size(); ++l)
                    {
                        invokedFrame.locals[localSlot] = args[l];
                        localSlot += (paramTypes[l] == 'J' || paramTypes[l] == 'D') ? 2 : 1;
                    }

                    FrameGuard invokedGuard(*this, invokedFrame);
                    Value result = execute(*targetClass, *targetCode);

                    if (descriptor.back() != 'V')
                    {
                        frame.push(result);
                    }

                    break;
                }
                case Opcode::InvokeDynamic:
                {
                    // Invokedynamic opcode is way too complicated to implement
                    // at this stage of the project. I will keep it unimplemented as
                    // the source files are from the J2ME era, which is 5 years prior
                    // to invokedynamic's release.
                    break;
                }

                case Opcode::New:
                {
                    U1 indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U1 indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantClass* cls = classFile.getConstant<ConstantClass>(index);
                    ConstantUtf8* clsNameUTF8 = classFile.getConstant<ConstantUtf8>(cls->nameIndex);
                    std::string className = clsNameUTF8->value;
                    
                    ClassFile* targetClass = loader.loadClass(className);
                    if(!targetClass)
                    {
                        throw std::runtime_error("new: failed to load class \"" + className + "\"");
                    }

                    if (heap.size() >= gcThreshold)
                    {
                        collectGarbage();
                    }

                    heap.push_back(std::make_unique<ObjectHeapObject>(targetClass));
                    HeapObject* newClass = heap.back().get();

                    if(heap.size() >= gcThreshold)
                    {
                        gcThreshold = heap.size() * 2;
                    }

                    frame.push(newClass);

                    break;
                }
                case Opcode::NewArray:
                {
                    U1 arrayType = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S4 count = std::get<S4>(frame.pop());
                    if (count < 0)
                    {
                        throw std::runtime_error("NegativeArraySizeException: newarray count is negative");
                    }

                    ValueType elementType;
                    std::size_t elementSize;

                    switch (static_cast<ArrayType>(arrayType))
                    {
                        case ArrayType::TBoolean:
                            elementType = ValueType::Boolean;
                            elementSize = sizeof(U1);
                            break;
                        case ArrayType::TChar:
                            elementType = ValueType::Char;
                            elementSize = sizeof(U2);
                            break;
                        case ArrayType::TFloat:
                            elementType = ValueType::Float;
                            elementSize = sizeof(F4);
                            break;
                        case ArrayType::TDouble:
                            elementType = ValueType::Double;
                            elementSize = sizeof(F8);
                            break;
                        case ArrayType::TByte:
                            elementType = ValueType::Byte;
                            elementSize = sizeof(S1);
                            break;
                        case ArrayType::TShort:
                            elementType = ValueType::Short;
                            elementSize = sizeof(S2);
                            break;
                        case ArrayType::TInt:
                            elementType = ValueType::Int;
                            elementSize = sizeof(S4);
                            break;
                        case ArrayType::TLong:
                            elementType = ValueType::Long;
                            elementSize = sizeof(S8);
                            break;
                        default:
                            throw std::runtime_error("newarray: unrecognized atype " + std::to_string(static_cast<int>(arrayType)));
                    }

                    if (heap.size() >= gcThreshold)
                    {
                        collectGarbage();
                    }

                    auto arrayObj = std::make_unique<ArrayHeapObject>(elementType,static_cast<U4>(count));
                    arrayObj->primitiveData.assign(static_cast<std::size_t>(count) * elementSize, 0);           

                    heap.push_back(std::move(arrayObj));
                    HeapObject* newArray = heap.back().get();

                    if (heap.size() >= gcThreshold)
                    {
                        gcThreshold = heap.size() * 2;
                    }
                    frame.push(newArray);
                    break;
                }
                case Opcode::ANewArray:
                {
                    U1 indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U1 indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantClass* cls = classFile.getConstant<ConstantClass>(index);
                    ConstantUtf8* classNameUTF8 = classFile.getConstant<ConstantUtf8>(cls->nameIndex);
                    std::string className = classNameUTF8->value;

                    S4 count = std::get<S4>(frame.pop());
                    if (count < 0)
                    {
                        throw std::runtime_error("NegativeArraySizeException: anewarray count is negative");
                    }

                    if (heap.size() >= gcThreshold)
                    {
                        collectGarbage();
                    }

                    auto arrayObj = std::make_unique<ArrayHeapObject>(ValueType::Reference, static_cast<U4>(count));
                    arrayObj->referenceData.assign(static_cast<std::size_t>(count), nullptr);

                    heap.push_back(std::move(arrayObj));
                    HeapObject* newArray = heap.back().get();

                    if (heap.size() >= gcThreshold)
                    {
                        gcThreshold = heap.size() * 2;
                    }

                    frame.push(newArray);
                    
                    break;
                }

                case Opcode::ArrayLength:
                {
                    Value arrayRefVal = frame.pop();
                    HeapObject** ref = std::get_if<HeapObject*>(&arrayRefVal);

                    if(!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: arraylength on null reference");
                    }
                    if((*ref)->type != HeapType::Array)
                    {
                        throw std::runtime_error("arraylength: reference is not an array");
                    }

                    auto* arrayObject = static_cast<ArrayHeapObject*>(*ref);


                    frame.push(S4(arrayObject->length));

                    break;
                }

                case Opcode::AThrow:
                {
                    Value objRefVal = frame.pop();
                    HeapObject** ref = std::get_if<HeapObject*>(&objRefVal);

                    if(!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: athrow on null reference");
                    }
                    if((*ref)->type != HeapType::Object)
                    {
                        throw std::runtime_error("athrow: reference is not an object");
                    }

                    auto* exceptionObject = static_cast<ObjectHeapObject*>(*ref);
                    std::string exceptionClassName = exceptionObject->javaClass->getClassName();


                    throw JavaException 
                    {
                        *ref,
                        exceptionClassName
                    };
                }

                case Opcode::CheckCast:
                {
                    U1 indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    U1 indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantClass* cls = classFile.getConstant<ConstantClass>(index);
                    std::string targetClassName = classFile.getConstant<ConstantUtf8>(cls->nameIndex)->value;

                    Value objRefVal = frame.pop();
                    HeapObject** ref = std::get_if<HeapObject*>(&objRefVal);

                    if (!ref || !*ref)
                    {
                        frame.push(nullptr);   // checkcast on null always succeeds, and pushes null to the stack
                        break;
                    }
                    if ((*ref)->type == HeapType::Array)
                    {
                        if (targetClassName == "java/lang/Object" ||
                            targetClassName == "java/io/Serializable" ||
                            targetClassName == "java/lang/Cloneable")
                        {
                            frame.push(*ref);
                            break;
                        }

                        throw std::runtime_error("ClassCastException: cannot cast to \"" + targetClassName + "\"");
                    }

                    if ((*ref)->type == HeapType::String)
                    {
                        if (targetClassName == "java/lang/String" ||
                            targetClassName == "java/lang/Object" ||
                            targetClassName == "java/io/Serializable" ||
                            targetClassName == "java/lang/Comparable")
                        {
                            frame.push(*ref);
                            break;
                        }

                        throw std::runtime_error("ClassCastException: cannot cast to \"" + targetClassName + "\"");
                    }

                    if ((*ref)->type != HeapType::Object || !isSubclassOf(static_cast<ObjectHeapObject*>(*ref)->javaClass->getClassName(), targetClassName))
                    {
                        throw std::runtime_error("ClassCastException: cannot cast to \"" + targetClassName + "\"");
                    }

                    frame.push(*ref);
                    break;
                }

                case Opcode::InstanceOf:
                {
                    U1 indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    U1 indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantClass* cls = classFile.getConstant<ConstantClass>(index);
                    std::string targetClassName = classFile.getConstant<ConstantUtf8>(cls->nameIndex)->value;

                    Value objRefVal = frame.pop();
                    HeapObject** ref = std::get_if<HeapObject*>(&objRefVal);

                    if (!ref || !*ref)
                    {
                        frame.push(S4(0));   // instanceof on null is always false
                        break;
                    }
                    if ((*ref)->type != HeapType::Object)
                    {
                        if ((*ref)->type == HeapType::Array)
                        {
                            frame.push(S4(targetClassName == "java/lang/Object" ||
                                          targetClassName == "java/io/Serializable" ||
                                          targetClassName == "java/lang/Cloneable" ? 1 : 0));
                            break;
                        }

                        if ((*ref)->type == HeapType::String)
                        {
                            frame.push(S4(targetClassName == "java/lang/String" ||
                                          targetClassName == "java/lang/Object" ||
                                          targetClassName == "java/io/Serializable" ||
                                          targetClassName == "java/lang/Comparable" ? 1 : 0));
                            break;
                        }

                        frame.push(S4(0));   
                        break;
                    }

                    auto* instance = static_cast<ObjectHeapObject*>(*ref);
                    std::string actualClassName = instance->javaClass->getClassName();

                    frame.push(S4(isSubclassOf(actualClassName, targetClassName) ? 1 : 0));
                    break;
                }


                case Opcode::MonitorEnter:
                {
                    Value objRefVal = frame.pop();
                    HeapObject** ref = std::get_if<HeapObject*>(&objRefVal);
                    if (!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: monitorenter on null reference");
                    }
                    // single threaded interpreter. Basically, no-op
                    break;
                }
                case Opcode::MonitorExit:
                {
                    Value objRefVal = frame.pop();
                    HeapObject** ref = std::get_if<HeapObject*>(&objRefVal);
                    if (!ref || !*ref)
                    {
                        throw std::runtime_error("NullPointerException: monitorexit on null reference");
                    }
                    // single threaded interpreter. Basically, no-op
                    break;
                }

                case Opcode::Wide:
                {
                    Opcode wideOpcode = static_cast<Opcode>(bytecode[frame.programCounter]);
                    frame.programCounter++;

                    U1 indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    U1 indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U2 wideIndex = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    if (wideOpcode == Opcode::IInc)
                    {
                        U1 countByte1 = bytecode[frame.programCounter];
                        frame.programCounter++;
                        U1 countByte2 = bytecode[frame.programCounter];
                        frame.programCounter++;

                        S2 wideConst = static_cast<S2>((countByte1 << 8) | countByte2);

                        if (wideIndex >= frame.locals.size())
                        {
                            throw std::runtime_error("wide iinc: local variable index out of bounds");
                        }

                        S4 val = std::get<S4>(frame.locals[wideIndex]);
                        frame.locals[wideIndex] = S4(val + wideConst);
                        break;
                    }

                    if (wideIndex >= frame.locals.size())
                    {
                        throw std::runtime_error("wide: local variable index out of bounds");
                    }

                    switch (wideOpcode)
                    {
                        case Opcode::ILoad:
                        case Opcode::LLoad:
                        case Opcode::FLoad:
                        case Opcode::DLoad:
                        case Opcode::ALoad:
                            frame.push(frame.locals[wideIndex]);
                            break;

                        case Opcode::IStore:
                        case Opcode::LStore:
                        case Opcode::FStore:
                        case Opcode::DStore:
                            frame.locals[wideIndex] = frame.pop();
                            break;

                        case Opcode::AStore:
                        {
                            Value value = frame.pop();
                            HeapObject** objectRef = std::get_if<HeapObject*>(&value);
                            if (!objectRef)
                            {
                                throw std::runtime_error("wide astore: value on stack is not a reference type");
                            }
                            frame.setLocal(wideIndex, *objectRef);
                            break;
                        }

                        case Opcode::Ret:
                        {
                            S4 returnAddress = std::get<S4>(frame.locals[wideIndex]);
                            frame.programCounter = static_cast<U4>(returnAddress);
                            break;
                        }

                        default:
                            throw std::runtime_error("wide: unsupported modified opcode 0x" + std::to_string(static_cast<int>(wideOpcode)));
                    }

                    break;
                }


                case Opcode::MultiANewArray:
                {
                    U1 indexByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    U1 indexByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U1 dimensionsOperand = bytecode[frame.programCounter];
                    frame.programCounter++;

                    U2 index = static_cast<U2>((indexByte1 << 8) | indexByte2);

                    ConstantClass* cls = classFile.getConstant<ConstantClass>(index);
                    std::string arrayDescriptor = classFile.getConstant<ConstantUtf8>(cls->nameIndex)->value;

                    int totalDimensions;
                    ValueType leafType;
                    parseArrayDescriptor(arrayDescriptor, totalDimensions, leafType);

                    if (dimensionsOperand == 0 || dimensionsOperand > totalDimensions)
                    {
                        throw std::runtime_error("multianewarray: invalid dimensions operand");
                    }

                    std::vector<S4> dimSizes(dimensionsOperand);
                    for (U1 d = dimensionsOperand; d > 0; )
                    {
                        --d;
                        dimSizes[d] = std::get<S4>(frame.pop());
                    }

                    HeapObject* result = createMultiArray(dimSizes, 0, leafType, totalDimensions);
                    frame.push(result);
                    break;
                }

                case Opcode::IfNull:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    U1 branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    U1 branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S2 branchOffset = static_cast<S2>((branchByte1 << 8) | branchByte2);

                    Value value = frame.pop();

                    

                    if (std::holds_alternative<HeapObject*>(value) && std::get<HeapObject*>(value) == nullptr)
                    {
                        frame.programCounter = opcodeStart + branchOffset;
                    }

                    break;
                }

                case Opcode::IfNonNull:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    U1 branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    U1 branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    S2 branchOffset = static_cast<S2>((branchByte1 << 8) | branchByte2);

                    Value value = frame.pop();

                    

                    if (std::holds_alternative<HeapObject*>(value) && std::get<HeapObject*>(value) != nullptr)
                    {
                        frame.programCounter = opcodeStart + branchOffset;
                    }

                    break;
                }

                case Opcode::GotoW:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte3 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte4 = bytecode[frame.programCounter];
                    frame.programCounter++;


                    S4 branchOffset = static_cast<S4>((branchByte1 << 24) | (branchByte2 << 16) | (branchByte3 << 8) | (branchByte4));

                    frame.programCounter = opcodeStart + branchOffset;

                    break;
                }

                case Opcode::JsrW:
                {
                    U4 opcodeStart = frame.programCounter - 1;

                    auto branchByte1 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte2 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte3 = bytecode[frame.programCounter];
                    frame.programCounter++;

                    auto branchByte4 = bytecode[frame.programCounter];
                    frame.programCounter++;
                    S4 branchOffset = static_cast<S4>((branchByte1 << 24) | (branchByte2 << 16) | (branchByte3 << 8) | (branchByte4));

                    S4 returnAddress = static_cast<S4>(frame.programCounter);
                    frame.push(returnAddress);

                    frame.programCounter = opcodeStart + branchOffset;
                    break;
                }

                case Opcode::Breakpoint:
                case Opcode::Unused_0xCB:
                case Opcode::Unused_0xCC:
                case Opcode::Unused_0xCD:
                case Opcode::Unused_0xCE:
                case Opcode::Unused_0xCF:
                case Opcode::Unused_0xD0:
                case Opcode::Unused_0xD1:
                case Opcode::Unused_0xD2:
                case Opcode::Unused_0xD3:
                case Opcode::Unused_0xD4:
                case Opcode::Unused_0xD5:
                case Opcode::Unused_0xD6:
                case Opcode::Unused_0xD7:
                case Opcode::Unused_0xD8:
                case Opcode::Unused_0xD9:
                case Opcode::Unused_0xDA:
                case Opcode::Unused_0xDB:
                case Opcode::Unused_0xDC:
                case Opcode::Unused_0xDD:
                case Opcode::Unused_0xDE:
                case Opcode::Unused_0xDF:
                case Opcode::Unused_0xE0:
                case Opcode::Unused_0xE1:
                case Opcode::Unused_0xE2:
                case Opcode::Unused_0xE3:
                case Opcode::Unused_0xE4:
                case Opcode::Unused_0xE5:
                case Opcode::Unused_0xE6:
                case Opcode::Unused_0xE7:
                case Opcode::Unused_0xE8:
                case Opcode::Unused_0xE9:
                case Opcode::Unused_0xEA:
                case Opcode::Unused_0xEB:
                case Opcode::Unused_0xEC:
                case Opcode::Unused_0xED:
                case Opcode::Unused_0xEE:
                case Opcode::Unused_0xEF:
                case Opcode::Unused_0xF0:
                case Opcode::Unused_0xF1:
                case Opcode::Unused_0xF2:
                case Opcode::Unused_0xF3:
                case Opcode::Unused_0xF4:
                case Opcode::Unused_0xF5:
                case Opcode::Unused_0xF6:
                case Opcode::Unused_0xF7:
                case Opcode::Unused_0xF8:
                case Opcode::Unused_0xF9:
                case Opcode::Unused_0xFA:
                case Opcode::Unused_0xFB:
                case Opcode::Unused_0xFC:
                case Opcode::Unused_0xFD:
                case Opcode::ImpDep1:
                case Opcode::ImpDep2:
                {
                    /* No-op */
                    break;
                }

                default:
                {
                    std::ostringstream oss;
                    oss << "Unimplemented opcode: 0x" << std::hex << std::uppercase
                        << static_cast<int>(opcode)
                        << " at programCounter=" << std::dec << (frame.programCounter - 1);
                    throw std::runtime_error(oss.str());
                }
            }
            
        }
        catch (JavaException& ex)
        {
            bool handled = false;

            for (const auto& entry : code.exceptionTable)
            {
                if (instructionStart < entry.startPc || instructionStart >= entry.endPc)
                {
                    continue;
                }

                bool typeMatches = false;
                if (entry.catchType == 0)
                {
                    typeMatches = true;   
                }
                else
                {
                    ConstantClass* catchClassRef = classFile.getConstant<ConstantClass>(entry.catchType);
                    ConstantUtf8* catchClassNameUTF8 = classFile.getConstant<ConstantUtf8>(catchClassRef->nameIndex);
                    std::string catchClassName = catchClassNameUTF8->value;

                    typeMatches = isSubclassOf(ex.className, catchClassName);
                }

                if (typeMatches)
                {
                    frame.operandStack.clear();
                    frame.push(ex.exceptionObject);
                    frame.programCounter = entry.handlerPc;
                    handled = true;
                    break;
                }
            }

            if (!handled)
            {
                throw;
            }
        }
    }

    throw std::runtime_error("Fell off end of method without return");
}

std::vector<HeapObject*> VM::gatherRoots()
{
    std::vector<HeapObject*> roots;
    for (auto& [cls, fields] : staticFields)
    {
        for (auto& [name, val] : fields)
        {
            if (auto* ref = std::get_if<HeapObject*>(&val); ref && *ref)
            {
                roots.push_back(*ref);
            }
        }
    }

    for (Frame* frame : callStack)
        {
            for (auto& v : frame->locals)
            {
                if (auto* ref = std::get_if<HeapObject*>(&v); ref && *ref)
                {
                    roots.push_back(*ref);
                }
            }

            for (auto& v : frame->operandStack)
            {
                if (auto* ref = std::get_if<HeapObject*>(&v); ref && *ref)
                {
                    roots.push_back(*ref);
                }
            }
        }

    return roots;
}

void VM::collectGarbage()
{
    std::vector<HeapObject*> roots = gatherRoots();
    for (auto* obj : roots)
    {
        obj->marked = true;
    } 

    while (!roots.empty())
    {
        HeapObject* obj = roots.back();
        roots.pop_back();

        std::vector<HeapObject*> children;
        obj->trace(children);
        for (auto* child : children)
        {
            if (child && !child->marked)
            {
                child->marked = true;
                roots.push_back(child);
            }
        }
    }

    heap.erase(std::remove_if(
            heap.begin(), heap.end(), 
            [](const std::unique_ptr<HeapObject>& obj) 
            { 
                return !obj->marked;
            }),
        heap.end());

    for (auto& obj : heap)
    {
        obj->marked = false;
    }
}