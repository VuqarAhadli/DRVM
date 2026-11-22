

#pragma once 

#include <Common.hpp>
#include <Types.hpp>
#include <vm/VM.hpp>

class VM;

class JavaSystem
{
public:
    static void gc(VM& vm);
};