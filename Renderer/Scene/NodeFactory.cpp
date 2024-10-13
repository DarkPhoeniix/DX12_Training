#include "stdafx.h"

#include "NodeFactory.h"

std::map<std::string_view, NodeFactory::function>& NodeFactory::get()
{
    static std::map<std::string_view, function> types;
    return types;
}
