#pragma once

#include "classfile/ClassFile.hpp"
#include "heap/Heap.hpp"

struct JavaException
{
    HeapObject* exceptionObject;
    std::string className; 
};