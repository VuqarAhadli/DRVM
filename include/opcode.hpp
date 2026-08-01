#pragma once

#include "Common.hpp"
#include "Types.hpp"
#include <string>




/**
 * Minimal opcode set from JVM specs (§6.5).
 */
enum class Opcode : U1
{
    Nop            = 0x00,
    AConstNull     = 0x01,
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

U1 operandSize(Opcode opcode);
bool isImplemented(Opcode opcode);
std::string toString(Opcode opcode);

