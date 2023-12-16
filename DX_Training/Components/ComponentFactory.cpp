#include "stdafx.h"

#include "ComponentFactory.h"

std::map<std::string, ComponentFactory::function>& ComponentFactory::get()
{
    static std::map<std::string, function> types;

    return types;
}
