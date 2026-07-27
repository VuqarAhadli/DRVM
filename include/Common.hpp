#pragma once

#include <Types.hpp>

/* SPDX-License-Identifier: Unlicense */
/* Public-domain ANSI/ECMA-48 Select Graphic Rendition string literals. */

#define ANSI_ESC "\x1b["
#define ANSI_SGR(code) ANSI_ESC code "m"

#define ANSI_RESET ANSI_SGR("0")
#define ANSI_BOLD ANSI_SGR("1")
#define ANSI_DIM ANSI_SGR("2")
#define ANSI_ITALIC ANSI_SGR("3")
#define ANSI_UNDERLINE ANSI_SGR("4")
#define ANSI_INVERSE ANSI_SGR("7")
#define ANSI_STRIKETHROUGH ANSI_SGR("9")

#define ANSI_FG_BLACK ANSI_SGR("30")
#define ANSI_FG_RED ANSI_SGR("31")
#define ANSI_FG_GREEN ANSI_SGR("32")
#define ANSI_FG_YELLOW ANSI_SGR("33")
#define ANSI_FG_BLUE ANSI_SGR("34")
#define ANSI_FG_MAGENTA ANSI_SGR("35")
#define ANSI_FG_CYAN ANSI_SGR("36")
#define ANSI_FG_WHITE ANSI_SGR("37")

#define ANSI_FG_BRIGHT_BLACK ANSI_SGR("90")
#define ANSI_FG_BRIGHT_RED ANSI_SGR("91")
#define ANSI_FG_BRIGHT_GREEN ANSI_SGR("92")
#define ANSI_FG_BRIGHT_YELLOW ANSI_SGR("93")
#define ANSI_FG_BRIGHT_BLUE ANSI_SGR("94")
#define ANSI_FG_BRIGHT_MAGENTA ANSI_SGR("95")
#define ANSI_FG_BRIGHT_CYAN ANSI_SGR("96")
#define ANSI_FG_BRIGHT_WHITE ANSI_SGR("97")

#define ANSI_BG_BLACK ANSI_SGR("40")
#define ANSI_BG_RED ANSI_SGR("41")
#define ANSI_BG_GREEN ANSI_SGR("42")
#define ANSI_BG_YELLOW ANSI_SGR("43")
#define ANSI_BG_BLUE ANSI_SGR("44")
#define ANSI_BG_MAGENTA ANSI_SGR("45")
#define ANSI_BG_CYAN ANSI_SGR("46")
#define ANSI_BG_WHITE ANSI_SGR("47")

#define ANSI_BG_BRIGHT_BLACK ANSI_SGR("100")
#define ANSI_BG_BRIGHT_RED ANSI_SGR("101")
#define ANSI_BG_BRIGHT_GREEN ANSI_SGR("102")
#define ANSI_BG_BRIGHT_YELLOW ANSI_SGR("103")
#define ANSI_BG_BRIGHT_BLUE ANSI_SGR("104")
#define ANSI_BG_BRIGHT_MAGENTA ANSI_SGR("105")
#define ANSI_BG_BRIGHT_CYAN ANSI_SGR("106")
#define ANSI_BG_BRIGHT_WHITE ANSI_SGR("107")

#define ANSI_STRINGIFY_INNER(value) #value
#define ANSI_STRINGIFY(value) ANSI_STRINGIFY_INNER(value)

/* Arguments must be literal or preprocessor numeric values, not runtime ints. */
#define ANSI_FG_256(index) ANSI_ESC "38;5;" ANSI_STRINGIFY(index) "m"
#define ANSI_BG_256(index) ANSI_ESC "48;5;" ANSI_STRINGIFY(index) "m"
#define ANSI_FG_RGB(r, g, b) \
  ANSI_ESC "38;2;" ANSI_STRINGIFY(r) ";" ANSI_STRINGIFY(g) ";" ANSI_STRINGIFY(b) "m"
#define ANSI_BG_RGB(r, g, b) \
  ANSI_ESC "48;2;" ANSI_STRINGIFY(r) ";" ANSI_STRINGIFY(g) ";" ANSI_STRINGIFY(b) "m"


/**
 * Colouring all Constant pool entry types for better readability
 */
#define UTF8_COLOUR ANSI_FG_RGB(97, 175, 239)   
#define INT_COLOUR ANSI_FG_RGB(116, 173, 76)  
#define FLOAT_COLOUR ANSI_FG_RGB(12, 204, 140)  
#define LONG_COLOUR ANSI_FG_RGB(224, 108, 117) 
#define DOUBLE_COLOUR ANSI_FG_RGB(198, 120, 221)  
#define CLASS_COLOUR ANSI_FG_RGB(224, 187, 117) 
#define STRING_COLOUR ANSI_FG_RGB(152, 195, 121)  
#define FIELDREF_COLOUR ANSI_FG_RGB(86, 182, 194)   
#define METHODREF_COLOUR ANSI_FG_RGB(97, 175, 239)   
#define IFACE_MR_COLOUR ANSI_FG_RGB(86, 182, 194)   
#define NAT_COLOUR ANSI_FG_RGB(155, 232, 202)  
#define MHANDLE_COLOUR ANSI_FG_RGB(224, 108, 117)  
#define MTYPE_COLOUR ANSI_FG_RGB(131, 199, 172)  
#define DYNAMIC_COLOUR ANSI_FG_RGB(198, 120, 221)  
#define INVOKEDYN_COLOUR ANSI_FG_RGB(198, 120, 221)  
#define MODULE_COLOUR ANSI_FG_RGB(186, 127, 161)   
#define PACKAGE_COLOUR ANSI_FG_RGB(140, 121, 173)    


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