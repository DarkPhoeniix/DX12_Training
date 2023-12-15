#include "stdafx.h"

#include "IComponent.h"

#include "Interfaces/IEntity.h"

void IComponent::SetEntity(IEntity* entity)
{
    _parent = entity;
}

IComponent* IComponent::GetSibling(const std::string& componentType) const
{
    return _parent ? _parent->GetComponent(componentType) : nullptr;
}
