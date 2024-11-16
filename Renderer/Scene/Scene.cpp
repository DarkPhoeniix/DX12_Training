#include "stdafx.h"

#include "Scene.h"

#include "DXObjects/Texture.h"
#include "DXObjects/CommandList.h"
#include "Scene/NodeFactory.h"
#include "Scene/Nodes/Camera/Camera.h"
#include "Scene/Nodes/Light/DirectionalLight.h"
#include "Scene/ECS/EntityLoader.h"

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
        Core::ResourceDescription sceneDataDescription;
        {
            sceneDataDescription.SetResourceType(Core::EResourceType::Buffer | Core::EResourceType::Dynamic | Core::EResourceType::StrideAlignment);
            sceneDataDescription.SetSize({ sizeof(SceneDesc), 1 });
            sceneDataDescription.SetStride(1);
            sceneDataDescription.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);

            _gpuDesc.SetResourceDescription(sceneDataDescription);
            _gpuDesc.CreateCommitedResource(D3D12_RESOURCE_STATE_GENERIC_READ);
        }

        std::shared_ptr<DirectionalLight> directionalLight = std::make_shared<DirectionalLight>(&_cache, nullptr);
        directionalLight->SetName("Directional Light");
        directionalLight->SetDirection(DirectX::XMVectorSet(-0.7f, -0.8f, -0.5f, 0.0f));
        directionalLight->SetColor(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));

        _cache.GetLightManager()->AddDirectionalLight(directionalLight);
    }

    Scene::~Scene()
    {   }

    std::vector<std::shared_ptr<Entity>>& Scene::GetRootNodes()
    {
        return _rootNodes;
    }

    SceneCache& Scene::GetCache()
    {
        return _cache;
    }

    void Scene::SetCamera(Camera& camera)
    {
        _cache.SetCamera(&camera);
    }

    Core::Resource& Scene::GetGPUDesc()
    {
        return _gpuDesc;
    }

    bool Scene::LoadScene(const std::string& filepath, Core::CommandList& commandList)
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
