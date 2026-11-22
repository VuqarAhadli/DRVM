#include "java/lang/System.hpp"

void JavaSystem::gc(VM& vm)
{
    vm.collectGarbage();
}