#pragma once

#include "IEntity.h"
#include "Components/IComponent.h"

class IObject : public IEntity
{
public:
    virtual ~IObject();

    virtual void AddComponent(IComponent* component);
    virtual void AddComponent(const std::string& componentType);

    IComponent* GetComponent(const std::string& componentType) const;

    template<typename Type>
    Type* GetComponentAs(const std::string& componentType) const;

private:
    std::vector<IComponent*> _components;
};

template<typename Type>
inline Type* IObject::GetComponentAs(const std::string& componentType) const
{
    return (Type*)GetComponent(componentType);
}
