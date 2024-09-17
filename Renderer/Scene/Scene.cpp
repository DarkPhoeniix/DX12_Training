#include "stdafx.h"

#include "Scene.h"

#include "DXObjects/Texture.h"
#include "DXObjects/GraphicsCommandList.h"
#include "Scene/SceneNode.h"
#include "Scene/Camera.h"
#include "Volumes/FrustumVolume.h"

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

Scene::Scene()
{
    Core::HeapDescription heapDesc;
    heapDesc.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);
    heapDesc.SetHeapFlags(D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES);
    heapDesc.SetSize(_256MB);
    heapDesc.SetMemoryPoolPreference(D3D12_MEMORY_POOL_UNKNOWN);
    heapDesc.SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY_UNKNOWN);
    heapDesc.SetVisibleNodeMask(1);
    heapDesc.SetCreationNodeMask(1);

    Core::DescriptorHeapDescription descriptorHeapDesc;
    descriptorHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    descriptorHeapDesc.SetNumDescriptors(32);
    descriptorHeapDesc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    descriptorHeapDesc.SetNodeMask(1);

    Core::ResourceDescription desc;
    desc.SetResourceType(Core::EResourceType::Buffer | Core::EResourceType::Dynamic | Core::EResourceType::StrideAlignment);
    desc.SetSize({ sizeof(SceneDesc), 1 });
    desc.SetStride(1);
    desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
    _sceneData = std::make_shared<Core::Resource>();
    _sceneData->SetResourceDescription(desc);
    _sceneData->CreateCommitedResource(D3D12_RESOURCE_STATE_GENERIC_READ);

    _texturesTable = std::make_shared<Core::ResourceTable>(descriptorHeapDesc, heapDesc);
}

Scene::~Scene()
{   }

void Scene::Draw(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum)
{
    commandList.SetDescriptorHeaps({ _texturesTable->GetDescriptorHeap().GetDXDescriptorHeap().Get() });

    SceneDesc* sceneDesc = (SceneDesc*)_sceneData->Map();
    sceneDesc->ViewProjection = _camera->ViewProjection();
    commandList.SetCBV(0, _sceneData->OffsetGPU(0));

    for (auto& node : _rootNodes)
    {
        node->Draw(commandList, frustum);
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
    _camera = &camera;
}

bool Scene::LoadScene(const std::string& filepath, Core::GraphicsCommandList& commandList)
{
    std::ifstream in(filepath, std::ios_base::in | std::ios_base::binary);

    Json::Value root;
    in >> root;

    _name = root["Name"].asCString();

    Json::Value nodes = root["Nodes"];
    for (int i = 0; i < nodes.size(); ++i)
    {
        std::shared_ptr<SceneNode> node = std::make_shared<SceneNode>(this);
        node->LoadNode(_name + '\\' + nodes[i].asCString(), commandList);
        _rootNodes.push_back(node);
    }

    return true;
}

void Scene::_UploadTexture(Core::Texture* texture, Core::GraphicsCommandList& commandList)
{
    if (_texturesTable->AddResource(texture))
    {
        texture->SetDescriptorHeap(&_texturesTable->GetDescriptorHeap());
        texture->UploadToGPU(commandList);
    }
}
