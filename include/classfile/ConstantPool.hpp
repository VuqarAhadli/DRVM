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

#include "Common.hpp"
#include "Types.hpp"
#include <string>
#include <utility>

struct CPInfo
{
	ConstantTag tag;

	explicit CPInfo(ConstantTag t)
		: tag(t)
	{
	}

	virtual ~CPInfo() = default;
};

class ConstantUtf8 : public CPInfo
{
public:
	std::string value;

	ConstantUtf8(std::string value)
		: CPInfo(ConstantTag::Utf8),
		  value(std::move(value))
	{}
};

class ConstantClass : public CPInfo
{
public:
	U2 nameIndex;

	ConstantClass(U2 index)
		: CPInfo(ConstantTag::Class),
		  nameIndex(index)
	{}
};

class ConstantString : public CPInfo
{
public:
	U2 stringIndex;

	ConstantString(U2 index)
		: CPInfo(ConstantTag::String),
		  stringIndex(index)
	{}
};

class ConstantFieldref : public CPInfo
{
public:
	U2 classIndex;
	U2 nameAndTypeIndex;

	ConstantFieldref(U2 c, U2 nt)
		: CPInfo(ConstantTag::Fieldref),
		  classIndex(c),
		  nameAndTypeIndex(nt)
	{}
};

class ConstantInteger : public CPInfo
{
public:
	S4 value;

	ConstantInteger(S4 v)
		: CPInfo(ConstantTag::Integer),
		  value(v)
	{}
};

class ConstantFloat : public CPInfo
{
public:
	F4 value;

	ConstantFloat(F4 v)
		: CPInfo(ConstantTag::Float),
		  value(v)
	{}
};

class ConstantLong : public CPInfo
{
public:
	S8 value;

	ConstantLong(S8 v)
		: CPInfo(ConstantTag::Long),
		  value(v)
	{}
};

class ConstantDouble : public CPInfo
{
public:
	F8 value;

	ConstantDouble(F8 v)
		: CPInfo(ConstantTag::Double),
		  value(v)
	{}
};

class ConstantMethodref : public CPInfo
{
public:
	U2 classIndex;
	U2 nameAndTypeIndex;

	ConstantMethodref(U2 classIndex, U2 nameAndTypeIndex)
		: CPInfo(ConstantTag::Methodref),
		  classIndex(classIndex),
		  nameAndTypeIndex(nameAndTypeIndex)
	{
	}
};

class ConstantInterfaceMethodref : public CPInfo
{
public:
	U2 classIndex;
	U2 nameAndTypeIndex;

	ConstantInterfaceMethodref(U2 classIndex, U2 nameAndTypeIndex)
		: CPInfo(ConstantTag::InterfaceMethodref),
		  classIndex(classIndex),
		  nameAndTypeIndex(nameAndTypeIndex)
	{
	}
};

class ConstantNameAndType : public CPInfo
{
public:
    U2 nameIndex;
    U2 descriptorIndex;

    ConstantNameAndType(U2 nameIndex, U2 descriptorIndex)
        : CPInfo(ConstantTag::NameAndType),
          nameIndex(nameIndex),
          descriptorIndex(descriptorIndex)
    {
    }
};

class ConstantMethodHandle : public CPInfo
{
public:
	U1 referenceKind;
	U2 referenceIndex;

	ConstantMethodHandle(U1 referenceKind, U2 referenceIndex)
		: CPInfo(ConstantTag::MethodHandle),
		  referenceKind(referenceKind),
		  referenceIndex(referenceIndex)
	{}
};

class ConstantMethodType : public CPInfo
{
public:
	U2 descriptorIndex;

	ConstantMethodType(U2 descriptorIndex)
		: CPInfo(ConstantTag::MethodType),
		  descriptorIndex(descriptorIndex)
	{}
};

class ConstantDynamic : public CPInfo
{
public:
	U2 bootstrapMethodAttrIndex;
	U2 nameAndTypeIndex;

	ConstantDynamic(U2 bootstrapMethodAttrIndex, U2 nameAndTypeIndex)
		: CPInfo(ConstantTag::Dynamic),
		  bootstrapMethodAttrIndex(bootstrapMethodAttrIndex),
		  nameAndTypeIndex(nameAndTypeIndex)
	{}
};

class ConstantInvokeDynamic : public CPInfo
{
public:
	U2 bootstrapMethodAttrIndex;
	U2 nameAndTypeIndex;

	ConstantInvokeDynamic(U2 bootstrapMethodAttrIndex, U2 nameAndTypeIndex)
		: CPInfo(ConstantTag::InvokeDynamic),
		  bootstrapMethodAttrIndex(bootstrapMethodAttrIndex),
		  nameAndTypeIndex(nameAndTypeIndex)
	{}
};

class ConstantModule : public CPInfo
{
public:
	U2 nameIndex;

	ConstantModule(U2 nameIndex)
		: CPInfo(ConstantTag::Module),
		  nameIndex(nameIndex)
	{}
};

class ConstantPackage : public CPInfo
{
public:
	U2 nameIndex;

	ConstantPackage(U2 nameIndex)
		: CPInfo(ConstantTag::Package),
		  nameIndex(nameIndex)
	{}
};
