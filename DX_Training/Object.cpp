#include "pch.h"
#include "Object.h"

void Object::SetName(const std::string& name)
{
    _name = name;
}

std::string Object::GetName() const
{
    return _name;
}
