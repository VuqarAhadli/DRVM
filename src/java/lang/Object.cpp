#include "java/lang/Object.hpp"


bool JavaObject::equals(const JavaObject& otherObject) const
{
    return &this->object == &otherObject.object;
}