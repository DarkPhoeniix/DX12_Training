#include "stdafx.h"

#include "IEntity.h"

IEntity::~IEntity()
{
    for (int i = 0; i < _components.size(); ++i)
        delete _components[i];
}

void IEntity::SetName(const std::string& name)
{
    _name = name;
}

const std::string& IEntity::GetName() const
{
    return _name;
}

IComponent* IEntity::GetComponent(const std::string& componentType) const
{
    IComponent* result = nullptr;

    for (IComponent* component : _components)
    {
        const std::string& type = component->GetType();
        if (type == componentType)
        {
            result = component;
            break;
        }
    }

    return result;
}

void IEntity::AddComponent(IComponent* component)
{
    _components.push_back(component);
    _components.back()->SetEntity(this);
}

void IEntity::AddComponent(const std::string& componentType)
{
    _components.push_back(ComponentFactory::Create<IComponent>(componentType));
    _components.back()->SetEntity(this);
}
