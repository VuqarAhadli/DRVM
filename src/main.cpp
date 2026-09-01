/*
 * DRVM (drvm)
 * Copyright (C) 2026 Vugar Ahadli
 * Contact: vuqarahadli17@gmail.com | vuqar@div.edu.az
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
    if (argc == 3 )
    {
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
    }
    return 0;
}