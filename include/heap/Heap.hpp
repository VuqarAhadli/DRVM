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
    bool marked = false; // for GC
    HeapObject* allocatedNext = nullptr;

    explicit HeapObject(HeapType type)
        : type(type)
    {
    }

    virtual ~HeapObject() = default;
    virtual void trace(std::vector<HeapObject*>& out) const = 0;
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
    void trace(std::vector<HeapObject*>&) const override {}
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

    void trace(std::vector<HeapObject*>& out) const override
    {
        if (elementType == ValueType::Reference)
        {
            for (auto* ref : referenceData)
            {
                if (ref)
                {
                    out.push_back(ref);
                } 
            }
        }
    }
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
    ClassFile* javaClass;

    std::unordered_map<std::string, FieldSlot> fields;

    explicit ObjectHeapObject(ClassFile* javaClass)
        : HeapObject(HeapType::Object),
          javaClass(javaClass)
    {
    }

    void trace(std::vector<HeapObject*>& out) const override
    {
        for (auto& [name, slot] : fields)
        {
            if (slot.type == ValueType::Reference && slot.reference != nullptr)
            {
                out.push_back(slot.reference);
            }
        }
    }
};