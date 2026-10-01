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
    if (argc < 2)
    {
        std::cout << "Usage: drvm <class file> [--dump] [--run] [--trace]\n";
        return 1;
    }

    bool dump = false;
    bool run = false;
    bool trace = false;
    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];


        if (arg == "--dump")
        {
            dump = true;
        }
        else if (arg == "--run")
        {
            run = true;
        }
        else if (arg == "--trace")
        {
            trace = true;
        }
        else
        {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }


    try
    {
        ClassFile cls(argv[1]);

        if (dump)
        {
            cls.dump();
            return 0;
        }


        std::filesystem::path classPath = std::filesystem::path(argv[1]).parent_path();

        std::filesystem::path runtimePath = classPath.parent_path() / "runtime" / "classes";

        ClassLoader loader(
            classPath.string(),
            runtimePath.string()
        );

        VM vm(loader);
        vm.setTrace(trace);

        std::cout << "Loading " << cls.getClassName() << '\n';


        /* first check main void main(String[] args) */
        const MethodInfo* mainMethod = cls.findMethod("main", "([Ljava/lang/String;)V");

        if (mainMethod)
        {
            vm.invoke(cls, *mainMethod);
            return 0;
        }

        /*
         * Initialise the class. | clinit returns void |
         */
        const MethodInfo* clinit = cls.findMethod("<clinit>", "()V");

        if (clinit)
        {
            std::cout << "Running... "
                      << cls.getClassName()
                      << ".<clinit>()V\n";

            vm.invoke(cls, *clinit);

            std::cout << "<clinit> completed\n";
        }

        /*
         * Construct the MIDlet object.
         */
        const MethodInfo* init = cls.findMethod("<init>", "()V");

        if (!init)
        {
            throw std::runtime_error(
                cls.getClassName() + " has no <init>()V"
            );
        }

        ObjectHeapObject& instance = vm.allocateObject(cls);

        std::cout << "Running "
                  << cls.getClassName()
                  << ".<init>()V\n";

        vm.invokeInstance(
            cls, *init, instance, {}
        );

        std::cout << "<init> completed\n";

        /*
         * Enter the MIDlet lifecycle.
         */
        const MethodInfo* startApp = cls.findMethod("startApp", "()V");

        if (!startApp)
        {
            throw std::runtime_error(
                cls.getClassName() + " has no startApp()V"
            );
        }

        std::cout << "Running "
                  << cls.getClassName()
                  << ".startApp()V\n";

        vm.invokeInstance(cls, *startApp, instance, {});

        std::cout << "startApp completed\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nVM error: "
                  << e.what()
                  << '\n';

        return 1;
    }

    return 0;
}