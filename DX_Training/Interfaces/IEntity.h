#pragma once

class IEntity
{
public:
    virtual ~IEntity();

    virtual void AddComponent(IComponent* component);
    virtual void AddComponent(const std::string& componentType);

    void SetName(const std::string& name);
    const std::string& GetName() const;

    IComponent* GetComponent(const std::string& componentType) const;

    template<typename Type>
    Type* GetComponentAs(const std::string& componentType) const;

protected:
    std::vector<IComponent*> _components;
    std::string _name;
};

template<typename Type>
inline Type* IEntity::GetComponentAs(const std::string& componentType) const
{
    return (Type*)GetComponent(componentType);
}
