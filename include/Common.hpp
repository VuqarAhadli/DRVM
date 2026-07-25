#pragma once

#include <Types.hpp>

constexpr U4 CLASS_MAGIC = 0xCAFEBABEu;




/**
 * The constant pool tags as defined in the JVM specification.
 */
enum class ConstantTag : U1
{
    Utf8                = 1,
    Integer             = 3,
    Float               = 4,
    Long                = 5,
    Double              = 6,
    Class               = 7,
    String              = 8,
    Fieldref            = 9, 
    Methodref           = 10,
    InterfaceMethodref  = 11,
    NameAndType         = 12,
    MethodHandle        = 15,
    MethodType          = 16,
    Dynamic             = 17,
    InvokeDynamic       = 18,
    Module              = 19,
    Package             = 20
};

/**
 * Class access and property modifiers (§4.1 ClassFile).
 * Also reused (with different applicable subsets) for fields
 * and methods (§4.5, §4.6).
 */
enum AccessFlag : U2
{
    ACC_PUBLIC       = 0x0001,
    ACC_PRIVATE      = 0x0002, // fields/methods only
    ACC_PROTECTED    = 0x0004, // fields/methods only
    ACC_STATIC       = 0x0008, // fields/methods only
    ACC_FINAL        = 0x0010,
    ACC_SUPER        = 0x0020, // classes only (treat superclass methods specially)
    ACC_SYNCHRONIZED = 0x0020, // methods only (same bit as ACC_SUPER)
    ACC_VOLATILE     = 0x0040, // fields only
    ACC_BRIDGE       = 0x0040, // methods only
    ACC_TRANSIENT    = 0x0080, // fields only
    ACC_VARARGS      = 0x0080, // methods only
    ACC_NATIVE       = 0x0100, // methods only
    ACC_INTERFACE    = 0x0200,
    ACC_ABSTRACT     = 0x0400,
    ACC_STRICT       = 0x0800, // methods only
    ACC_SYNTHETIC    = 0x1000,
    ACC_ANNOTATION   = 0x2000,
    ACC_ENUM         = 0x4000,
    ACC_MODULE       = 0x8000  // classes only
};