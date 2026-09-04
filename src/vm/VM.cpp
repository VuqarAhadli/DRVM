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
                U8 opcodeStart = frame.programCounter - 1;

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
                U8 opcodeStart = frame.programCounter - 1;

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
                U8 opcodeStart = frame.programCounter - 1;

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
                U8 opcodeStart = frame.programCounter - 1;

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
                U8 opcodeStart = frame.programCounter - 1;

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
                U8 opcodeStart = frame.programCounter - 1;

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

                U8 opcodeStart = frame.programCounter - 1;

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

                U8 opcodeStart = frame.programCounter - 1;

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

                U8 opcodeStart = frame.programCounter - 1;

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

                U8 opcodeStart = frame.programCounter - 1;

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

                U8 opcodeStart = frame.programCounter - 1;

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

                U8 opcodeStart = frame.programCounter - 1;

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

                U8 opcodeStart = frame.programCounter - 1;

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

                U8 opcodeStart = frame.programCounter - 1;

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
                U8 opcodeStart = frame.programCounter - 1;

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
                U8 opcodeStart = frame.programCounter - 1;

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
                U8 opcodeStart = frame.programCounter - 1;

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
                U8 opcodeStart = frame.programCounter - 1;

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

                auto& fields = staticFields[&classFile];
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

                staticFields[&classFile][name] = frame.pop();
                
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