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

            case Opcode::ILoad0:
                frame.push(frame.locals[0]); 
                break;

            case Opcode::ILoad1:
                frame.push(frame.locals[1]);
                break;
            case Opcode::ILoad2:
                frame.push(frame.locals[2]);
                break;
            case Opcode::ILoad3:
                frame.push(frame.locals[3]);
                break;

            case Opcode::IStore0:
                frame.locals[0] = frame.pop();
                break;
            case Opcode::IStore1:
                frame.locals[1] = frame.pop();
                break;
            case Opcode::IStore2:
                frame.locals[2] = frame.pop(); 
                break;
            case Opcode::IStore3:
                frame.locals[3] = frame.pop();
                break;

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

            case Opcode::GetStatic:
            {
                U2 index = static_cast<U2>((bytecode[frame.programCounter] << 8) | bytecode[frame.programCounter + 1]);
                frame.programCounter += 2;

                auto* fieldref = classFile.getConstant<ConstantFieldref>(index);
                auto* nameAndType = classFile.getConstant<ConstantNameAndType>(fieldref->nameAndTypeIndex);
                std::string fieldName = classFile.getConstant<ConstantUtf8>(nameAndType->nameIndex)->value;

                // Assumes the field belongs to the current class (classFile).
                auto& fields = staticFields[&classFile];
                auto iter = fields.find(fieldName);
                if (iter == fields.end())
                {
                    // JVM default is 0 for numerics.
                    fields[fieldName] = S4(0);
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