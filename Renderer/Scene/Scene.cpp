#include "stdafx.h"

#include "Scene.h"

#include "CommandList.h"

#include "Scene/Camera.h"
#include "Scene/ECS/EntityLoader.h"

#include <queue>

namespace
{
    struct SceneDesc
    {
        DirectX::XMMATRIX ViewProjection = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX View = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX Projection = DirectX::XMMatrixIdentity();
        
        DirectX::XMMATRIX InvView = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX InvProjection = DirectX::XMMatrixIdentity();

        DirectX::XMVECTOR EyePosition = DirectX::XMVectorZero();
        DirectX::XMVECTOR EyeDirection = DirectX::XMVectorZero();

        DirectX::XMUINT2 WindowSize = { 0, 0 };
        DirectX::XMFLOAT2 NearFar = { 0, 0 };

        UINT LightsNum = 0;
    };
} // namespace unnamed

namespace SceneLayer
{
    Scene::Scene()
    {
        dx12::ResourceDescription sceneDataDescription;
        {
            sceneDataDescription.SetResourceType(dx12::EResourceType::Buffer | dx12::EResourceType::Dynamic | dx12::EResourceType::Aligned);
            sceneDataDescription.SetSize({ sizeof(SceneDesc), 1 });
            sceneDataDescription.SetStride(1);
            sceneDataDescription.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);

            _gpuDesc.CreateCommitedResource(sceneDataDescription, D3D12_RESOURCE_STATE_GENERIC_READ);
        }
    }

    Scene::~Scene()
    {   }

    std::vector<std::shared_ptr<Entity>>& Scene::GetRootNodes()
    {
        return _rootNodes;
    }

    std::shared_ptr<Entity> Scene::FindNodeByName(const std::string& name) const
    {
        std::shared_ptr<Entity> currentEntity = nullptr;

        std::queue<std::shared_ptr<Entity>> entities;
        for (const auto& rootNode : _rootNodes)
        {
            entities.push(rootNode);
        }

        while (!entities.empty())
        {
            currentEntity = entities.front();
            entities.pop();

            if (currentEntity->GetName() == name)
            {
                return currentEntity;
            }
        }

        return nullptr;
    }

    std::shared_ptr<Entity> Scene::FindNodeByComponentName(const std::string& componentName) const
    {
        std::shared_ptr<Entity> currentEntity = nullptr;

        std::queue<std::shared_ptr<Entity>> entities;
        for (const auto& rootNode : _rootNodes)
        {
            entities.push(rootNode);
        }

        while (!entities.empty())
        {
            currentEntity = entities.front();
            entities.pop();

            if (currentEntity->GetComponent(componentName))
            {
                return currentEntity;
            }
        }

        return nullptr;
    }

    SceneCache& Scene::GetCache()
    {
        return _cache;
    }

    void Scene::SetCamera(Camera& camera)
    {
        _cache.SetCamera(&camera);
    }

    dx12::Resource& Scene::GetGPUDesc()
    {
        return _gpuDesc;
    }

    bool Scene::LoadScene(const std::string& filepath, dx12::CommandList& commandList)
    {
        std::ifstream in(filepath, std::ifstream::in | std::ifstream::binary);

        Json::Value root;
        in >> root;

        _name = root["Name"].asCString();

        // Parse children nodes
        for (auto& node : root["Nodes"])
        {
            Helpers::EntityLoader loader(std::filesystem::path(filepath).parent_path().string() + '/' + node.asString());
            
            _rootNodes.push_back(loader.LoadEntity(&_cache));
        }

        return true;
    }
} // namespace SceneLayer
