#include "stdafx.h"

#include "Scene.h"

#include "DXObjects/Texture.h"
#include "DXObjects/GraphicsCommandList.h"
#include "Scene/NodeFactory.h"
#include "Scene/Nodes/Camera/Camera.h"
#include "Scene/Nodes/Light/DirectionalLight.h"
#include "Scene/Nodes/Light/PointLight.h"

namespace
{
    struct SceneDesc
    {
        DirectX::XMMATRIX ViewProjection = DirectX::XMMatrixIdentity();
        DirectX::XMVECTOR EyePosition = DirectX::XMVectorZero();
        DirectX::XMVECTOR EyeDirection = DirectX::XMVectorZero();

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
            _sceneGPUData = std::make_shared<Core::Resource>();
            _sceneGPUData->SetResourceDescription(sceneDataDescription);
            _sceneGPUData->CreateCommitedResource(D3D12_RESOURCE_STATE_GENERIC_READ);
        }

        std::shared_ptr<DirectionalLight> directionalLight = std::make_shared<DirectionalLight>(&_cache, nullptr);
        directionalLight->SetName("Directional Light");
        directionalLight->SetDirection(DirectX::XMVectorSet(0.5f, -1.0f, 0.2f, 0.0f));
        directionalLight->SetColor(DirectX::XMVectorSet(0.0f, 1.0f, 0.5f, 1.0f));

        std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>(&_cache, nullptr);
        DirectX::XMMATRIX tr = DirectX::XMMatrixIdentity() * DirectX::XMMatrixTranslation(20.0f, 100.0f, -20.0f);
        pointLight->SetName("Point Light");
        pointLight->SetLocalTransform(tr);
        pointLight->SetRange(500.0f);
        pointLight->SetIntensity(100.0f);
        pointLight->SetColor(DirectX::XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f));

        _cache.GetLightManager()->AddDirectionalLight(directionalLight);
        _cache.GetLightManager()->AddPointLight(pointLight);
    }

    Scene::~Scene()
    {   }

    void Scene::Draw(Core::GraphicsCommandList& commandList)
    {
        // Setup textures
        commandList.SetDescriptorHeaps({ _cache.GetTextureTable()->GetDescriptorHeap().GetDXDescriptorHeap().Get()});
        commandList.SetDescriptorTable(3, _cache.GetTextureTable()->GetDescriptorHeap().GetHeapStartGPUHandle());

        // Setup scene data
        SceneDesc* sceneDesc = (SceneDesc*)_sceneGPUData->Map();
        sceneDesc->ViewProjection = _cache.GetCamera()->ViewProjection();
        sceneDesc->LightsNum = _cache.GetLightManager()->GetLightsNum();
        commandList.SetCBV(0, _sceneGPUData->OffsetGPU(0));

        // Setup lights
        _cache.GetLightManager()->SetupLights(commandList);

        for (auto& node : _rootNodes)
        {
            node->Draw(commandList);
        }
    }

    void Scene::DrawAABB(Core::GraphicsCommandList& commandList)
    {
        for (auto& node : _rootNodes)
        {
            node->DrawAABB(commandList);
        }
    }

    void Scene::SetCamera(Camera& camera)
    {
        _cache.SetCamera(&camera);
    }

    bool Scene::LoadScene(const std::string& filepath, Core::GraphicsCommandList& commandList)
    {
        std::ifstream in(filepath, std::ifstream::in | std::ifstream::binary);

        Json::Value root;
        in >> root;

        _name = root["Name"].asCString();

        // Parse children nodes
        for (auto& node : root["Nodes"])
        {
            // TODO: redundant file opening...
            std::string nodeFilepath = std::filesystem::path(filepath).parent_path().string() + '/' + node.asCString();
            std::ifstream nodeIn(nodeFilepath, std::ifstream::in | std::ifstream::binary);

            Json::Value root;
            nodeIn >> root;

            std::shared_ptr<ISceneNode> child = std::shared_ptr<ISceneNode>(NodeFactory::Create<ISceneNode>(root["Type"].asCString()));
            child->SetSceneCache(&_cache);
            child->LoadNode(nodeFilepath, commandList);

            _rootNodes.push_back(child);
        }

        return true;
    }

    void Scene::_UploadTexture(Core::Texture* texture, Core::GraphicsCommandList& commandList)
    {
        if (_cache.GetTextureTable()->AddResource(texture))
        {
            texture->SetDescriptorHeap(&_cache.GetTextureTable()->GetDescriptorHeap());
            texture->UploadToGPU(commandList);
        }
    }
} // namespace SceneLayer
