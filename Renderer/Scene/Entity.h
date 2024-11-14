#pragma once

#include "Scene/SceneCache.h"
#include "Scene/ECS/Components.h"

namespace SceneLayer
{
    class Entity
    {
    public:
        Entity();
        explicit Entity(SceneCache* sceneCache, Entity* parent = nullptr);

        IComponent* GetComponent(const std::string_view& name);
        template<typename Type>
        Type* GetComponentAs(const std::string_view& name);

        void AddComponent(const std::shared_ptr<IComponent>& component);
        void ClearComponents();

        void AddChild(std::shared_ptr<Entity> child);

        void SetName(const std::string& name);
        const std::string& GetName() const;

    private:
        std::vector<std::shared_ptr<IComponent>> _components;

        std::vector<std::shared_ptr<Entity>> _children;
        Entity* _parent;

        SceneCache* _sceneCache;

        std::string _name;
    };

    template<typename Type>
    Type* Entity::GetComponentAs(const std::string_view& name)
    {
        return (Type*)GetComponent(name);
    }
} // namespace SceneLayer
