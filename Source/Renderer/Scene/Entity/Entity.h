#pragma once

#include "Renderer/Scene/Entity/Components/IComponent.h"
#include "Renderer/Scene/Entity/Components/Transformation.h"

namespace scene
{
    class Entity
    {
    public:
        using InstanceID = std::uint32_t;
        constexpr static InstanceID InvalidInstanceID = static_cast<InstanceID>(-1);

        explicit Entity(Entity* parent = nullptr);

        const std::vector<std::shared_ptr<IComponent>>& GetComponents() const;
        std::shared_ptr<IComponent> GetComponent(const std::string_view& name);
        template<typename Type>
        std::shared_ptr<Type> GetComponentAs(const std::string_view& name);

        void AddComponent(std::shared_ptr<IComponent> component);
        void ClearComponents();

        const Transformation& GetGlobalTransform() const;
        void UpdateGlobalTransform(const Transformation* parentTransform = nullptr);

        std::vector<std::shared_ptr<Entity>>& GetChildrenNodes();
        void AddChild(std::shared_ptr<Entity> child);

        void SetInstanceID(InstanceID id);
        InstanceID GetInstanceID() const;

        void SetName(const std::string& name);
        const std::string& GetName() const;

    private:
        std::vector<std::shared_ptr<IComponent>> _components;
        Transformation _globalTransformation;

        std::vector<std::shared_ptr<Entity>> _children;
        Entity* _parent;

        InstanceID _instanceID;
        std::string _name;
    };

    template<typename Type>
    std::shared_ptr<Type> Entity::GetComponentAs(const std::string_view& name)
    {
        return std::static_pointer_cast<Type>(GetComponent(name));
    }
} // namespace scene
