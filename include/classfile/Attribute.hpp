#pragma once

#include <cstring>
#include <Types.hpp>
#include <string>
#include <vector>
#include <memory>
#include "Common.hpp"

/**
 * From JVM specification §4.7
 * attribute_info {
    u2 attribute_name_index;
    u4 attribute_length;
    u1 info[attribute_length];
    }
 */

/**
 * A list of attributes from JVM specification §4.7.1 Table 4.7-B
 *  Attribute 	                        class file 	Java SE Section
    ConstantValue                           45.3 	1.0.2 	§4.7.2
    Code                                    45.3 	1.0.2 	§4.7.3
    Exceptions                              45.3 	1.0.2 	§4.7.5
    SourceFile                              45.3 	1.0.2 	§4.7.10
    LineNumberTable 	                    45.3 	1.0.2 	§4.7.12
    LocalVariableTable 	                    45.3 	1.0.2 	§4.7.13
    InnerClasses                            45.3 	1.1 	§4.7.6
    Synthetic                               45.3 	1.1 	§4.7.8
    Deprecated                              45.3 	1.1 	§4.7.15
    EnclosingMethod 	                    49.0 	5.0 	§4.7.7
    Signature                               49.0 	5.0 	§4.7.9
    SourceDebugExtension                    49.0 	5.0 	§4.7.11
    LocalVariableTypeTable                  49.0 	5.0 	§4.7.14
    RuntimeVisibleAnnotations               49.0 	5.0 	§4.7.16
    RuntimeInvisibleAnnotations             49.0 	5.0 	§4.7.17
    RuntimeVisibleParameterAnnotations   	49.0 	5.0 	§4.7.18
    RuntimeInvisibleParameterAnnotations    49.0 	5.0 	§4.7.19
    AnnotationDefault 	                    49.0 	5.0 	§4.7.22
    StackMapTable 	                        50.0 	6 	    §4.7.4
    BootstrapMethods 	                    51.0 	7 	    §4.7.23
    RuntimeVisibleTypeAnnotations 	        52.0 	8   	§4.7.20
    RuntimeInvisibleTypeAnnotations 	    52.0 	8   	§4.7.21
    MethodParameters 	                    52.0 	8 	    §4.7.24
 */

struct AttributeInfo
{
    U2 attributeNameIndex;
    U4 attributeLength;

    AttributeInfo(U2 nameIndex, U4 length)
        : attributeNameIndex(nameIndex),
          attributeLength(length)
    {
    }

    virtual ~AttributeInfo() = default;
};

class RawAttribute : public AttributeInfo
{
public:
    std::vector<U1> info;

    RawAttribute(U2 nameIndex, U4 length, std::vector<U1> info)
        : AttributeInfo(nameIndex, length),
          info(std::move(info))
    {
    }
};

struct ExceptionTableEntry
{
    U2 startPc;
    U2 endPc;
    U2 handlerPc;
    U2 catchType;
};

class CodeAttribute : public AttributeInfo
{
public:
    U2 maxStack;
    U2 maxLocals;
    U4 codeLength;
    std::vector<U1> code;
    std::vector<ExceptionTableEntry> exceptionTable;
    std::vector<std::unique_ptr<AttributeInfo>> attributes;

    CodeAttribute(U2 nameIndex, U4 length, U2 maxStack, U2 maxLocals, std::vector<U1> code,
                  std::vector<ExceptionTableEntry> exceptionTable,
                  std::vector<std::unique_ptr<AttributeInfo>> attributes)

    : AttributeInfo(nameIndex, length),
      maxStack(maxStack),
      maxLocals(maxLocals),
      codeLength(static_cast<U4>(code.size())),
      code(std::move(code)),
      exceptionTable(std::move(exceptionTable)),
      attributes(std::move(attributes))
    {
    }
};
