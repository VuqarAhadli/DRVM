#include "vm/VM.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
VM::VM(ClassLoader& loader)
    : loader(loader)
{
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
    const std::vector<U1>& bytecode = code.code;

    while (frame.programCounter < bytecode.size())
    {
        auto opcode = static_cast<Opcode>(bytecode[frame.programCounter++]);


        switch (opcode)
        {
            case Opcode::Nop:
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
                    // heap not yet implemented
                    throw std::runtime_error("ldc: String constants not yet supported");
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
                // If it names a different class (e.g. inherited static fields) — not handled yet.
                auto& fields = staticFields[&classFile];
                auto iter = fields.find(fieldName);
                if (iter == fields.end())
                {
                    // Field never explicitly initialized yet — JVM default is 0 for numerics.
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