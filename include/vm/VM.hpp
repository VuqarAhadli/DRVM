#pragma once

#include <vector>
#include <variant>
#include <string>
#include "Types.hpp"
#include "classfile/ClassFile.hpp"
#include "vm/ClassLoader.hpp"

/**
 * Union
 */
using Value = std::variant<S4, S8, F4, F8, void*>;  // int32 or reference; expand with F4/S8/F8 as needed

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
        Value v = operandStack.back();
        operandStack.pop_back();
        return v;
    }
};

/**
 * Minimal opcode set from JVM specs (§6.5).
 */
enum class Opcode : U1
{
    Nop            = 0x00,
    IConstM1       = 0x02,
    IConst0        = 0x03,
    IConst1        = 0x04,
    IConst2        = 0x05,
    IConst3        = 0x06,
    IConst4        = 0x07,
    IConst5        = 0x08,
    Ldc            = 0x12, 
    LdcW           = 0x13,  
    Ldc2W          = 0x14,  
    BiPush         = 0x10,
    SiPush         = 0x11,
    ILoad          = 0x15,
    ILoad0         = 0x1A,
    ILoad1         = 0x1B,
    ILoad2         = 0x1C,
    ILoad3         = 0x1D,
    IStore         = 0x36,
    IStore0        = 0x3B,
    IStore1        = 0x3C,
    IStore2        = 0x3D,
    IStore3        = 0x3E,
    IAdd           = 0x60,
    ISub           = 0x64,
    NewArray       = 0xBC,  
    ANewArray      = 0xBD,  
    ArrayLength    = 0xBE,
    Goto           = 0xA7,
    IReturn        = 0xAC,
    Return         = 0xB1,
    GetStatic      = 0xB2,
    PutStatic      = 0xB3,
    GetField       = 0xB4,
    PutField       = 0xB5,
    InvokeVirtual  = 0xB6,
    InvokeSpecial  = 0xB7,
    InvokeStatic   = 0xB8,
    New            = 0xBB,  
};

//  Array Type 	atype
//  T_BOOLEAN 	4
//  T_CHAR 	    5
//  T_FLOAT     6
//  T_DOUBLE 	7
//  T_BYTE 	    8
//  T_SHORT     9
//  T_INT 	    10
//  T_LONG 	    11


class VM
{
public:
    explicit VM(ClassLoader& loader);

    
    Value invoke(ClassFile& classFile, const MethodInfo& method);

private:
    Value execute(ClassFile& classFile, const CodeAttribute& code);
    std::unordered_map<ClassFile*, std::unordered_map<std::string, Value>> staticFields;
    ClassLoader& loader;
};