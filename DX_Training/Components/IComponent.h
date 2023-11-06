#pragma once

class IEntity;

class IComponent
{
public:
    virtual ~IComponent() = default;

    virtual const std::string& GetType() const = 0;

    void SetEntity(IEntity* entity);
    IComponent* GetSibling(const std::string& componentType) const;

protected:
    IEntity* _parent;
};
