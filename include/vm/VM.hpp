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

#pragma once

#include <vector>
#include <variant>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include "Types.hpp"
#include "classfile/ClassFile.hpp"
#include "vm/ClassLoader.hpp"
#include "heap/Heap.hpp"
#include "Opcode.hpp"

/**
 *        Union
 *     |  |  |  |  
 *     v  v  v  v  
 */
using Value = std::variant<S4,
                           S8,
                           F4,
                           F8,
                           HeapObject*>;  

                           
/**
 * One method invocation's execution context: operand stack + local variable array, 
 * sized per the method's Code attribute.
 */
struct Frame
{
    std::vector<Value> locals;
    std::vector<Value> operandStack;
    U4 programCounter = 0; 

    explicit Frame(U2 maxLocals, U2 maxStack)
        : locals(maxLocals), operandStack()
    {
        operandStack.reserve(maxStack);
    }

    void push(Value v) 
    {
        operandStack.push_back(v);
    }

    /**
     *  MacOS libc++ requires explicit type conversion unlike Linux's libstdc++.
     *  Despite, older version compiles successfully in both platforms, the language
     *  server shows it as an error.
     */
    template<typename V>
    void push(V v)
    {
        operandStack.push_back(Value(v));
    }

    template<typename V>
    void setLocal(U4 index, V v)
    {
        locals[index] = Value(v);
    }

    Value pop()
    {
        if (operandStack.empty())
        {
            throw std::runtime_error("Operand stack underflow");
        }
        Value v = operandStack.back();
        operandStack.pop_back();
        return v;
    }
};



/*
    For NewArray 0xBC (188) 
    |  |  |  |  
    v  v  v  v  
*/
/* From JVM specs §6.5 */
//  Array Type 	atype
//  T_BOOLEAN 	4
//  T_CHAR 	    5
//  T_FLOAT     6
//  T_DOUBLE 	7
//  T_BYTE 	    8
//  T_SHORT     9
//  T_INT 	    10
//  T_LONG 	    11

enum class ArrayType : U1
{
    T_BOOLEAN = 4,
    T_CHAR    = 5,
    T_FLOAT   = 6,
    T_DOUBLE  = 7,
    T_BYTE    = 8,
    T_SHORT   = 9,
    T_INT     = 10,
    T_LONG    = 11
};

class VM
{
public:
    explicit VM(ClassLoader& loader);

    
    Value invoke(ClassFile& classFile, const MethodInfo& method);
    HeapObject* allocateString(const std::string& utf8);
    
    void collectGarbage();

private:
    friend struct FrameGuard;
    U4 gcThreshold = 1024;
    
    Value execute(ClassFile& classFile, const CodeAttribute& code);
    std::vector<HeapObject*> gatherRoots();
    std::unordered_map<ClassFile*, std::unordered_map<std::string, Value>> staticFields;
    ClassLoader& loader;
    std::vector<std::unique_ptr<HeapObject>> heap;
    std::vector<Frame*> callStack;
    
};

struct FrameGuard
{
    VM& vm;
    FrameGuard(VM& vm, Frame& frame) : vm(vm) 
    { 
        vm.callStack.push_back(&frame); 
    }
    ~FrameGuard()
    {
        vm.callStack.pop_back();
    }
};