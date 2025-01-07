#include "RendererPCH.h"

#include "Entity.h"

#include "Render/GPUStructs/GPUModelDesc.h"

namespace SceneLayer
{
    Entity::Entity()
        : _sceneCache(nullptr)
        , _parent(nullptr)
    {
        Init();
    }

    Entity::Entity(SceneCache* sceneCache, Entity* parent)
        : _sceneCache(sceneCache)
        , _parent(parent)
    {
        Init();
    }

    IComponent* Entity::GetComponent(const std::string_view& name)
    {
        IComponent* result = nullptr;

        for (const auto& component : _components)
        {
            if (component->ComponentName == name)
            {
                result = component.get();
                break;
            }
        }

        return result;
    }

    void Entity::AddComponent(const std::shared_ptr<IComponent>& component)
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

    dx12::Resource& Entity::GetGPUDesc()
    {
        return _gpuDesc;
    }

    SceneCache* Entity::GetSceneCache()
    {
        return _sceneCache;
    }

    void Entity::Init()
    {
        dx12::ResourceDescription resourceDesc;
        {
            resourceDesc.SetSize({ sizeof(GPUModelDesc), 1 });
            resourceDesc.SetStride(1);
            resourceDesc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
            resourceDesc.SetResourceType(dx12::EResourceType::Dynamic | dx12::EResourceType::Buffer | dx12::EResourceType::Aligned);
        }

        _gpuDesc.CreateCommitedResource(resourceDesc);
        _gpuDesc.SetName(_name);
    }
} // namespace SceneLayer
