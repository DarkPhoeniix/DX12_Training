#include "stdafx.h"

#include "ComponentFactory.h"

std::map<std::string_view, ComponentFactory::function>& ComponentFactory::get()
{
    static std::map<std::string_view, function> types;
    return types;
}
