#include "stdafx.h"

#include "Scene.h"

#include "DXObjects/Texture.h"
#include "DXObjects/CommandList.h"
#include "Scene/NodeFactory.h"
#include "Scene/Nodes/Camera/Camera.h"
#include "Scene/Nodes/Light/DirectionalLight.h"
#include "Scene/Nodes/Light/PointLight.h"

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
            _sceneGPUData = std::make_shared<Core::Resource>();
            _sceneGPUData->SetResourceDescription(sceneDataDescription);
            _sceneGPUData->CreateCommitedResource(D3D12_RESOURCE_STATE_GENERIC_READ);
        }

        std::shared_ptr<DirectionalLight> directionalLight = std::make_shared<DirectionalLight>(&_cache, nullptr);
        directionalLight->SetName("Directional Light");
        directionalLight->SetDirection(DirectX::XMVectorSet(0.3f, -0.8f, 0.5f, 0.0f));
        directionalLight->SetColor(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));

        _cache.GetLightManager()->AddDirectionalLight(directionalLight);
    }

    Scene::~Scene()
    {   }

    void Scene::Draw(Core::CommandList& commandList)
    {
        // Setup textures
        commandList.SetDescriptorHeaps({ _cache.GetTextureTable()->GetDescriptorHeap().GetDXDescriptorHeap().Get() });
        commandList.SetDescriptorTable(3, _cache.GetTextureTable()->GetDescriptorHeap().GetHeapStartGPUHandle());

        // Setup scene data
        SceneDesc* sceneDesc = (SceneDesc*)_sceneGPUData->Map();
        {
            sceneDesc->ViewProjection = _cache.GetCamera()->ViewProjection();
            sceneDesc->View = _cache.GetCamera()->View();
            sceneDesc->Projection = _cache.GetCamera()->Projection();

            sceneDesc->InvView = DirectX::XMMatrixInverse(nullptr, sceneDesc->View);
            sceneDesc->InvProjection = DirectX::XMMatrixInverse(nullptr, sceneDesc->Projection);

            const SceneLayer::Viewport& viewport = _cache.GetCamera()->GetViewport();
            sceneDesc->WindowSize = { (uint32_t)viewport.GetSize().x, (uint32_t)viewport.GetSize().y };
            sceneDesc->NearFar = { _cache.GetCamera()->GetNearZ(), _cache.GetCamera()->GetFarZ() };

            sceneDesc->LightsNum = _cache.GetLightManager()->GetLightsNum();
        }

        commandList.SetCBV(0, _sceneGPUData->OffsetGPU(0));

        // Setup lights
        _cache.GetLightManager()->SetupLights(commandList);

        for (auto& node : _rootNodes)
        {
            node->Draw(commandList);
        }
    }

    void Scene::DrawAABB(Core::CommandList& commandList)
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

    bool Scene::LoadScene(const std::string& filepath, Core::CommandList& commandList)
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

    void Scene::_UploadTexture(Core::Texture* texture, Core::CommandList& commandList)
    {
        if (_cache.GetTextureTable()->AddResource(texture))
        {
            texture->SetDescriptorHeap(&_cache.GetTextureTable()->GetDescriptorHeap());
            texture->UploadToGPU(commandList);
        }
    }
} // namespace SceneLayer
