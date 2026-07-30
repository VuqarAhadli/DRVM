#pragma once

#include <vector>
#include <variant>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include "Types.hpp"
#include "classfile/ClassFile.hpp"
#include "vm/ClassLoader.hpp"
#include "heap/heap.hpp"
#include "opcode.hpp"

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

    Value pop()
    {
        if (operandStack.empty())
        {
            throw std::runtime_error("Operand stack underflow");
        }
        Value v = std::move(operandStack.back());
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
    

private:
    Value execute(ClassFile& classFile, const CodeAttribute& code);
    std::unordered_map<ClassFile*, std::unordered_map<std::string, Value>> staticFields;
    ClassLoader& loader;
    std::vector<std::unique_ptr<HeapObject>> heap;
    
};

