#include "RendererPCH.h"

#include "Entity.h"

namespace scene
{
    Entity::Entity(Entity* parent)
        : _parent(parent)
    {
    }

    const std::vector<std::shared_ptr<IComponent>>& Entity::GetComponents() const
    {
        return _components;
    }

    std::shared_ptr<IComponent> Entity::GetComponent(const std::string_view& name)
    {
        for (const auto& component : _components)
        {
            if (component->ComponentName == name)
            {
                return component;
            }
        }


        return nullptr;
    }

    void Entity::AddComponent(std::shared_ptr<IComponent> component)
    {
        _components.push_back(component);
    }

    void Entity::ClearComponents()
    {
        _components.clear();
    }

    const Transformation& Entity::GetGlobalTransform() const
    {
        return _globalTransformation;
    }

    void Entity::UpdateGlobalTransform(const Transformation* parentTransform)
    {
        _globalTransformation.Transform = GetComponentAs<Transformation>("Transformation")->Transform;
        if (parentTransform)
        {
            _globalTransformation.Transform *= parentTransform->Transform;
        }
    }

    std::vector<std::shared_ptr<Entity>>& Entity::GetChildrenNodes()
    {
        return _children;
    }

    void Entity::AddChild(std::shared_ptr<Entity> child)
    {
        _children.push_back(child);
    }

    void Entity::SetName(const std::string& name)
    {
        _name = name;
    }

    const std::string& Entity::GetName() const
    {
        return _name;
    }
} // namespace scene
