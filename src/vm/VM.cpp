#include "vm/VM.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <variant>



VM::VM(ClassLoader& loader)
    : loader(loader)
{
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

Value VM::invoke(ClassFile& classFile, const MethodInfo& method)
{
    const CodeAttribute* code = classFile.getCode(method);
    if (!code)
    {
        throw std::runtime_error("Method has no Code attribute -> (native/abstract)");
    }
    return execute(classFile, *code);
}

Value VM::execute(ClassFile& classFile, const CodeAttribute& code)
{
    Frame frame(code.maxLocals, code.maxStack);
    FrameGuard guard(*this, frame);
    const std::vector<U1>& bytecode = code.code;

    while (frame.programCounter < bytecode.size())
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
                CPInfo* entry = classFile.getConstant<CPInfo>(index);
                if (entry->tag == ConstantTag::Integer)
                {
                    frame.push(classFile.getConstant<ConstantInteger>(index)->value);
                }
                else if (entry->tag == ConstantTag::Float)
                {
                    frame.push(classFile.getConstant<ConstantFloat>(index)->value);
                }
                else if (entry->tag == ConstantTag::String)
                {
                    auto* stringConst = classFile.getConstant<ConstantString>(index);
                    auto* utf8Const = classFile.getConstant<ConstantUtf8>(stringConst->stringIndex);
                    HeapObject* stringObj = allocateString(utf8Const->value);
                    frame.push(stringObj);
                }
                else
                {
                    throw std::runtime_error("ldc: unexpected constant pool tag");
                }
                break;
            }

            case Opcode::LdcW:
            {
                U2 index = static_cast<U2>((bytecode[frame.programCounter] << 8) | bytecode[frame.programCounter + 1]);
                frame.programCounter += 2;

                CPInfo* entry = classFile.getConstant<CPInfo>(index);
                if (entry->tag == ConstantTag::Integer)
                {
                    frame.push(classFile.getConstant<ConstantInteger>(index)->value);
                }
                else if (entry->tag == ConstantTag::Float)
                {
                    frame.push(classFile.getConstant<ConstantFloat>(index)->value);
                }
                else
                {
                    throw std::runtime_error("ldc_w: unexpected/unsupported constant pool tag");
                }
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
                else
                {
                    throw std::runtime_error("ldc_2w: unexpected/unsupported constant pool tag");
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

            case Opcode::IAdd:
            {
                S4 b = std::get<S4>(frame.pop());
                S4 a = std::get<S4>(frame.pop());
                frame.push(S4(a + b));
                break;
            }

            case Opcode::ISub:
            {
                S4 b = std::get<S4>(frame.pop());
                S4 a = std::get<S4>(frame.pop());
                frame.push(S4(a - b));
                break;
            }

            case Opcode::IReturn:
                return frame.pop();

            case Opcode::Return:
                return Value();

            

            case Opcode::GetStatic:
            {
                U2 index = static_cast<U2>((bytecode[frame.programCounter] << 8) | bytecode[frame.programCounter + 1]);
                frame.programCounter += 2;

                auto* fieldref = classFile.getConstant<ConstantFieldref>(index);
                auto* nameAndType = classFile.getConstant<ConstantNameAndType>(fieldref->nameAndTypeIndex);
                std::string fieldName = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex)->value;

                // Assumes the field belongs to the current class (classFile). yet...
                auto& fields = staticFields[&classFile];
                auto iter = fields.find(fieldName);
                if (iter == fields.end())
                {
                    // JVM default is 0 for numerics.

                    fields[fieldName] = Value(std::in_place_type<S4>, 0);
                    iter = fields.find(fieldName);
                }
                frame.push(iter->second);
                break;
            }

            case Opcode::PutStatic:
            {
                U2 index = static_cast<U2>((bytecode[frame.programCounter] << 8) | bytecode[frame.programCounter + 1]);
                frame.programCounter += 2;

                auto* fieldref = classFile.getConstant<ConstantFieldref>(index);
                auto* nameAndType = classFile.getConstant<ConstantNameAndType>(fieldref->nameAndTypeIndex);
                std::string fieldName = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex)->value;

                staticFields[&classFile][fieldName] = frame.pop();
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