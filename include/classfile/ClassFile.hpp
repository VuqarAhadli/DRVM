#pragma once

#include <cstring>
#include <Types.hpp>
#include <string>
#include <vector>
#include <memory>
#include "Common.hpp"
#include "BinaryReader.hpp"
#include "ConstantPool.hpp"
#include "Field.hpp"
#include "Method.hpp"
#include "Attribute.hpp"

/**
 * 
 * From JVM specification §4
    ClassFile {
    u4             magic;                                | COVERED
    u2             minor_version;                        | COVERED
    u2             major_version;                        | COVERED
    u2             constant_pool_count;                  | COVERED
    cp_info        constant_pool[constant_pool_count-1]; | COVERED
    u2             access_flags;                         | COVERED
    u2             this_class;                           | COVERED
    u2             super_class;                          | COVERED
    u2             interfaces_count;                     | COVERED
    u2             interfaces[interfaces_count];         | COVERED
    u2             fields_count;                         | COVERED
    field_info     fields[fields_count];                 | COVERED
    u2             methods_count;                        | COVERED
    method_info    methods[methods_count];               | COVERED
    u2             attributes_count;                     | COVERED
    attribute_info attributes[attributes_count];         | COVERED
    }
 * 
 */


class ClassFile
{
public:
    explicit ClassFile(const std::string& filename);

    void dump();
    void dumpConstantPoolTags();
    void dumpClassMetadata();
    void dumpFields();
    void dumpMethods();
    void dumpAttributes();

    template<typename T>
    T* getConstant(U2 index)
    {
        return static_cast<T*>(constantPool.at(index).get());
    }

private:
    void saveClassMetadata();
    void saveConstantPoolTags();
    void saveFields();
    void saveMethods();
    void saveAttributes();
    void dumpAttribute(const AttributeInfo* attribute, int indent = 0);
    std::unique_ptr<AttributeInfo> readAttribute();

    std::string filename;
    U4 magic;
    U2 minorVersion;
    U2 majorVersion;
    U2 constantPoolCount;
    std::vector<std::unique_ptr<CPInfo>> constantPool;

    U2 accessFlags;
    U2 thisClass;
    U2 superClass;
    U2 interfacesCount;
    std::vector<U2> interfaces;

    U2 fieldsCount;
    std::vector<FieldInfo> fields;
    U2 methodsCount;
    std::vector<MethodInfo> methods;
    U2 attributesCount;
    std::vector<std::unique_ptr<AttributeInfo>> attributes; 

    BinaryReader reader;
};