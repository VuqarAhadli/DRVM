#include <iostream>
#include <filesystem>

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

    /* testing execution */
    const MethodInfo* clinit = cls.findMethod("<clinit>", "()V");
    if (clinit)
    {
        std::cout << "Attempting to run clinit for " << cls.getClassName() << "\n";

        std::filesystem::path classPath = std::filesystem::path(argv[1]).parent_path();
        ClassLoader loader(classPath.string());
        VM vm(loader);
        try
        {
            vm.invoke(cls, *clinit);
            std::cout << "<clinit> completed\n";
        }
        catch(const std::exception& e)
        {
            std::cerr << "VM error: " << e.what() << '\n';
        }
    
    }
    
    else
    {
        std::cout << "\nNo <clinit> found, skipping VM test.\n";
    }

    return 0;
}