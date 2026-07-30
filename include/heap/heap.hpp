#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Common.hpp"
#include "Types.hpp"
#include "classfile/ClassFile.hpp"

enum class HeapType : U1
{
    String,
    Array,
    Object
};

class HeapObject
{
public:
    HeapType type;

    explicit HeapObject(HeapType type)
        : type(type)
    {
    }

    virtual ~HeapObject() = default;
};

class StringHeapObject : public HeapObject
{
public:
    /* Java strings use UTF-16 encoding */
    UTF16 value; 

    explicit StringHeapObject(UTF16 value)
        : HeapObject(HeapType::String),
          value(std::move(value))
    {}
};

enum class ValueType : U1
{
    Boolean,
    Byte,
    Char,
    Short,
    Int,
    Long,
    Float,
    Double,
    Reference // HeapObject*
};

class ArrayHeapObject : public HeapObject
{
public:
    ValueType elementType;
    U4 length;

    // For primitives, storing raw bytes 
    // and reinterpreting based on elementType later.
    std::vector<U1> primitiveData;

    // For Reference, storing pointers.
    std::vector<HeapObject*> referenceData;

    ArrayHeapObject(ValueType elementType, U4 length)
        : HeapObject(HeapType::Array),
          elementType(elementType),
          length(length)
    {}
};


struct FieldSlot
{
    ValueType type;
    std::vector<U1> primitiveData; 
    HeapObject* reference = nullptr; 
};



class ObjectHeapObject : public HeapObject
{
public:
    U2 classIndex; 

    std::unordered_map<std::string, FieldSlot> fields;

    explicit ObjectHeapObject(U2 classIndex)
        : HeapObject(HeapType::Object),
          classIndex(classIndex)
    {
    }
};