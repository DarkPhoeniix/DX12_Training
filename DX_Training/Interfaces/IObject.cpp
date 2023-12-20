#include "stdafx.h"

#include "IObject.h"

IObject::~IObject()
{
    for (int i = 0; i < _components.size(); ++i)
        delete _components[i];
}

IComponent* IObject::GetComponent(const std::string& componentType) const
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

void IObject::AddComponent(IComponent* component)
{
    _components.push_back(component);
    _components.back()->SetEntity(this);
}

void IObject::AddComponent(const std::string& componentType)
{
    _components.push_back(ComponentFactory::Create<IComponent>(componentType));
    _components.back()->SetEntity(this);
}
