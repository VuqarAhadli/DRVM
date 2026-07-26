#pragma once

#include <cstring>
#include <Types.hpp>
#include <string>
#include <vector>
#include <memory>
#include "Common.hpp"
#include "Attribute.hpp"

/**
 * From JVM specification §4.6
 * method_info {
    u2             access_flags;
    u2             name_index;
    u2             descriptor_index;
    u2             attributes_count;
    attribute_info attributes[attributes_count];
    }
 */

class MethodInfo
{
public:
    U2 accessFlags;
    U2 nameIndex;
    U2 descriptorIndex;
    U2 attributesCount;
    std::vector<std::unique_ptr<AttributeInfo>> attributes;
};