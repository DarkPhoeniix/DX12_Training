#include "stdafx.h"

#include "IEntity.h"

IEntity::~IEntity()
{
}

void IEntity::SetName(const std::string& name)
{
    _name = name;
}

const std::string& IEntity::GetName() const
{
    return _name;
}
