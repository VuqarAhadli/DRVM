#include <iostream>

#include "classfile/ClassFile.hpp"
#include "vm/ClassLoader.hpp"
#include "vm/VM.hpp"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: drvm <class file>\n";
        return 1;
    }

    ClassFile cls(argv[1]);

    cls.dump();
}