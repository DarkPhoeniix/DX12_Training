#pragma once

#include "Scene/SceneCache.h"
#include "Scene/Entity/Components/IComponent.h"
#include "Scene/Entity/Components/Transformation.h"

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

        const Transformation& GetGlobalTransform() const;
        void UpdateGlobalTransform(const Transformation* parentTransform = nullptr);

        std::vector<std::shared_ptr<Entity>>& GetChildrenNodes();
        void AddChild(std::shared_ptr<Entity> child);

        void SetName(const std::string& name);
        const std::string& GetName() const;

        dx12::Resource& GetGPUDesc();

        SceneCache* GetSceneCache();

    private:
        void Init();

        std::vector<std::shared_ptr<IComponent>> _components;
        Transformation _globalTransformation;

        std::vector<std::shared_ptr<Entity>> _children;
        Entity* _parent;

        dx12::Resource _gpuDesc;

        SceneCache* _sceneCache;

        std::string _name;
    };

    template<typename Type>
    Type* Entity::GetComponentAs(const std::string_view& name)
    {
        return (Type*)GetComponent(name);
    }
} // namespace SceneLayer
