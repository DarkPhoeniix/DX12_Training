#include "RendererPCH.h"

#include "Scene.h"

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

namespace scene
{
    void Scene::AddRootNode(std::shared_ptr<Entity> entity)
    {
        _rootNodes.push_back(entity);
    }

    std::vector<std::shared_ptr<Entity>>& Scene::GetRootNodes()
    {
        return _rootNodes;
    }

    const std::vector<std::shared_ptr<Entity>>& Scene::GetRootNodes() const
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

    std::vector<std::shared_ptr<Entity>> Scene::FilterNodesByComponent(const std::string& componentName) const
    {
        std::vector<std::shared_ptr<Entity>> nodes;

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
                nodes.push_back(currentEntity);
            }
        }

        return nodes;
    }

    SceneCache& Scene::GetCache()
    {
        return _cache;
    }

    void Scene::SetName(const std::string& name)
    {
        _name = name;
    }

    const std::string Scene::GetName() const
    {
        return _name;
    }
} // namespace scene
