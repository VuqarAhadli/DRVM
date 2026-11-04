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

#include <filesystem>

#include "classfile/ClassFile.hpp"
#include "vm/ClassLoader.hpp"
#include "vm/VM.hpp"
#include "maintain/Help.hpp"
#include "maintain/ProgramInfo.hpp"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printUsage();
        return 1;
    }

    bool dump = false;
    bool run = false;
    bool trace = false;
    bool debug = false;
    std::string classFilePath;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help")
        {
            printHelp();
            return 0;
        }
        else if (arg == "--version")
        {
            printVersion();
            return 0;
        }
        else if (arg == "--license")
        {
            printLicense();
            return 0;
        }
        else if (arg == "--contributors")
        {
            printContributors();
            return 0;
        }
        else if (arg == "--usage")
        {
            printUsage();
            return 0;
        }
        else if (arg == "--dump")
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
        else if (arg == "--debug")
        {
            debug = true;
        }
        else if (arg.rfind("--", 0) == 0)
        {
            std::cerr << "Unknown argument: " << arg << '\n';
            printUsage();
            return 1;
        }
        else if (classFilePath.empty())
        {
            classFilePath = arg;
        }
        else
        {
            std::cerr << "Unexpected argument: " << arg << '\n';
            printUsage();
            return 1;
        }
    }

    if (classFilePath.empty())
    {
        std::cerr << "No class file given.\n";
        printUsage();
        return 1;
    }

    if (!dump && !run)
    {
        std::cerr << "Nothing to do: pass --dump and/or --run.\n";
        return 1;
    }

    try
    {
        ClassFile cls(classFilePath);

        if (dump)
        {
            cls.dump();
        }

        if (run)
        {
            std::filesystem::path classPath = std::filesystem::absolute(classFilePath).parent_path();
            std::filesystem::path runtimePath = classPath.parent_path() / "runtime" / "classes";

            ClassLoader loader(classPath.string(), runtimePath.string());

            VM vm(loader, trace, debug);

            std::cout << "Loading " << cls.getClassName() << '\n';

            /* First check for: public static void main(String[] args) */
            const MethodInfo* mainMethod = cls.findMethod("main", "([Ljava/lang/String;)V");

            if (mainMethod)
            {
                vm.invoke(cls, *mainMethod);
                return 0;
            }

            /* Initialise the class. <clinit> returns void. */
            const MethodInfo* clinit = cls.findMethod("<clinit>", "()V");

            if (clinit)
            {
                std::cout << "Running " << cls.getClassName() << ".<clinit>()V\n";

                vm.invoke(cls, *clinit);

                std::cout << "<clinit> completed\n";
            }

            /* Construct the MIDlet object. */
            const MethodInfo* init = cls.findMethod("<init>", "()V");

            if (!init)
            {
                throw std::runtime_error(cls.getClassName() + " has no <init>()V");
            }

            ObjectHeapObject& instance = vm.allocateObject(cls);

            std::cout << "Running " << cls.getClassName() << ".<init>()V\n";

            vm.invokeInstance(cls, *init, instance, {});

            std::cout << "<init> completed\n";

            /* Enter the MIDlet lifecycle. */
            const MethodInfo* startApp = cls.findMethod("startApp", "()V");

            if (!startApp)
            {
                throw std::runtime_error(cls.getClassName() + " has no startApp()V");
            }

            std::cout << "Running " << cls.getClassName() << ".startApp()V\n";

            vm.invokeInstance(cls, *startApp, instance, {});

            std::cout << "startApp completed\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nVM error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}