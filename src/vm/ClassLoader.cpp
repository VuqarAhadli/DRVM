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

#include "vm/ClassLoader.hpp"

ClassLoader::ClassLoader(const std::string& classPath, const std::string& apiClassPath)
    : classPath(classPath), apiClassPath(apiClassPath)
{
}

std::string ClassLoader::resolvePath(const std::string& basePath, const std::string& className) const
{
    return basePath + "/" + className + ".class";
}

ClassFile* ClassLoader::loadClass(const std::string& className)
{
    auto iter = loadedClasses.find(className);
    
    if (iter != loadedClasses.end())
    {
        return iter->second.get();
    }

    std::string path = resolvePath(classPath, className);

    if (!std::filesystem::exists(path))
    {
        path = resolvePath(apiClassPath, className);
    }
    if (!std::filesystem::exists(path))
    {
        return nullptr;
    }


    std::cout << "Loading " << className << " from " << path << '\n';
   
    auto classFile = std::make_unique<ClassFile>(path);

    ClassFile* rawFile = classFile.get();
    loadedClasses[className] = std::move(classFile);



    return rawFile;
}