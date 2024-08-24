
#include "stdafx.h"

#include "SceneNode.h"

#include "DXObjects/Texture.h"
#include "DXObjects/GraphicsCommandList.h"
#include "Scene/Scene.h"
#include "Volumes/FrustumVolume.h"

using namespace DirectX;

namespace
{
    AABBVolume CalculateAABB(Mesh* mesh, const XMMATRIX& localTransform)
    {
        AABBVolume aabb;

        XMFLOAT4 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 1.0f);
        XMFLOAT4 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), 1.0f);

        if (!mesh)
        {
            return aabb;
        }

        for (const VertexData& vertex : mesh->getVertices())
        {
            if (vertex.Position.x < min.x)
            {
                min.x = vertex.Position.x;
            }
            else if (vertex.Position.x > max.x)
            {
                max.x = vertex.Position.x;
            }

            if (vertex.Position.y < min.y)
            {
                min.y = vertex.Position.y;
            }
            else if (vertex.Position.y > max.y)
            {
                max.y = vertex.Position.y;
            }

            if (vertex.Position.z < min.z)
            {
                min.z = vertex.Position.z;
            }
            else if (vertex.Position.z > max.z)
            {
                max.z = vertex.Position.z;
            }
        }

        aabb.min = DirectX::XMLoadFloat4(&min);
        aabb.max = DirectX::XMLoadFloat4(&max);

        return aabb;
    }
}

SceneNode::SceneNode()
    : ISceneNode()
    , _DXDevice(Core::Device::GetDXDevice())
    , _mesh(nullptr)
    , _albedoTexture(nullptr)
    , _normalTexture(nullptr)
    , _vertexBuffer(nullptr)
    , _indexBuffer(nullptr)
    , _modelMatrix(nullptr)
    , _AABB{}
    , _VBO{}
    , _IBO{}
{
}

SceneNode::SceneNode(Scene* scene, SceneNode* parent)
    : ISceneNode(scene, parent)
    , _DXDevice(Core::Device::GetDXDevice())
    , _mesh(nullptr)
    , _albedoTexture(nullptr)
    , _normalTexture(nullptr)
    , _vertexBuffer(nullptr)
    , _indexBuffer(nullptr)
    , _modelMatrix(nullptr)
    , _AABB{}
    , _VBO{}
    , _IBO{}
{   }

SceneNode::~SceneNode()
{
    _DXDevice = nullptr;

    for (ComPtr<ID3D12Resource> intermediate : intermediates)
    {
        intermediate = nullptr;
    }
}

void SceneNode::Draw(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const
{
    for (const std::shared_ptr<ISceneNode> node : _childNodes)
    {
        node->Draw(commandList, frustum);
    }

    _DrawCurrentNode(commandList, frustum);
}

void SceneNode::DrawAABB(Core::GraphicsCommandList& commandList) const
{
    for (const std::shared_ptr<ISceneNode> node : _childNodes)
    {
        node->DrawAABB(commandList);
    }

    if (!_mesh)
    {
        return;
    }

    commandList.SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);

    commandList.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

    XMMATRIX tr = GetGlobalTransform();
    XMVECTOR min = XMVector4Transform(_AABB.min, tr);
    XMVECTOR max = XMVector4Transform(_AABB.max, tr);

    commandList.SetConstants(0, 3, &min.m128_f32, 0);
    commandList.SetConstants(0, 3, &max.m128_f32, 4);

    commandList.Draw(1);
}

const AABBVolume& SceneNode::GetAABB() const
{
    return _AABB;
}

void SceneNode::LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList)
{
    Logger::Log(LogType::Info, "Parsing node " + filepath);

    std::ifstream in(filepath, std::ios_base::in | std::ios_base::binary);
    Json::Value root;
    in >> root;

    _name = root["Name"].asCString();

    _transform = XMMatrixSet(
        root["Transform"]["r0"]["x"].asFloat(), root["Transform"]["r0"]["y"].asFloat(), root["Transform"]["r0"]["z"].asFloat(), root["Transform"]["r0"]["w"].asFloat(),
        root["Transform"]["r1"]["x"].asFloat(), root["Transform"]["r1"]["y"].asFloat(), root["Transform"]["r1"]["z"].asFloat(), root["Transform"]["r1"]["w"].asFloat(),
        root["Transform"]["r2"]["x"].asFloat(), root["Transform"]["r2"]["y"].asFloat(), root["Transform"]["r2"]["z"].asFloat(), root["Transform"]["r2"]["w"].asFloat(),
        root["Transform"]["r3"]["x"].asFloat(), root["Transform"]["r3"]["y"].asFloat(), root["Transform"]["r3"]["z"].asFloat(), root["Transform"]["r3"]["w"].asFloat()
    );

    if (!root["Mesh"].isNull())
    {
        _mesh = std::make_shared<Mesh>();
        _mesh->LoadMesh(_scene->_name + '\\' + root["Mesh"].asCString());
    }

    _AABB = CalculateAABB(_mesh.get(), _transform);

    if (!root["Material"].isNull())
    {
        std::ifstream inMat(_scene->_name + '\\' + root["Material"].asCString(), std::ios_base::in | std::ios_base::binary);
        Json::Value mat;
        inMat >> mat;
        if (_albedoTexture = std::move(Core::Texture::LoadFromFile(mat["Albedo"].asCString())))
        {
            _scene->_UploadTexture(_albedoTexture.get(), commandList);
        }
        if (_normalTexture = std::move(Core::Texture::LoadFromFile(mat["Normal"].asCString())))
        {
            _scene->_UploadTexture(_normalTexture.get(), commandList);
        }
    }

    auto children = root["Nodes"];
    for (int i = 0; i < children.size(); ++i)
    {
        std::shared_ptr<SceneNode> child = std::make_shared<SceneNode>(_scene, this);
        child->LoadNode(_scene->_name + '\\' + children[i].asCString(), commandList);
        _childNodes.push_back(child);
    }

    {
        Core::EResourceType SRVType = Core::EResourceType::Dynamic | Core::EResourceType::Buffer;

        Core::ResourceDescription desc;
        desc.SetResourceType(SRVType);
        desc.SetSize({ sizeof(XMMATRIX), 1 });
        desc.SetStride(1);
        desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);

        _modelMatrix = std::make_shared<Core::Resource>(desc);
        _modelMatrix->CreateCommitedResource(D3D12_RESOURCE_STATE_GENERIC_READ);
        _modelMatrix->SetName(_name + "_ModelMatrix");
    }


    if (_mesh)
    {
        if (!_mesh->getVertices().empty())
        {
            ComPtr<ID3D12Resource> vertexBuffer;
            _UploadData(commandList, &vertexBuffer, _mesh->getVertices().size(), sizeof(VertexData), _mesh->getVertices().data());
            _vertexBuffer = std::make_shared<Core::Resource>();
            _vertexBuffer->InitFromDXResource(vertexBuffer);
            _vertexBuffer->SetName(_name + "_VB");

            _VBO.BufferLocation = _vertexBuffer->OffsetGPU(0);
            _VBO.SizeInBytes = static_cast<UINT>(_mesh->getVertices().size() * sizeof(_mesh->getVertices()[0]));
            _VBO.StrideInBytes = sizeof(VertexData);
        }

        if (!_mesh->getIndices().empty())
        {
            ComPtr<ID3D12Resource> indexBuffer;
            _UploadData(commandList, &indexBuffer, _mesh->getIndices().size(), sizeof(UINT), _mesh->getIndices().data());
            _indexBuffer = std::make_shared<Core::Resource>();
            _indexBuffer->InitFromDXResource(indexBuffer);
            _indexBuffer->SetName(_name + "_IB");

            _IBO.BufferLocation = _indexBuffer->OffsetGPU(0);
            _IBO.Format = DXGI_FORMAT_R32_UINT;
            _IBO.SizeInBytes = static_cast<UINT>(_mesh->getIndices().size() * sizeof(_mesh->getIndices()[0]));
        }
    }
}

void SceneNode::_UploadData(Core::GraphicsCommandList& commandList,
    ID3D12Resource** destinationResource,
    size_t numElements,
    size_t elementSize,
    const void* bufferData,
    D3D12_RESOURCE_FLAGS flags)
{
    size_t bufferSize = numElements * elementSize;

    CD3DX12_HEAP_PROPERTIES heapTypeDefault(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferWithFlags = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    Helper::throwIfFailed(_DXDevice->CreateCommittedResource(
        &heapTypeDefault,
        D3D12_HEAP_FLAG_NONE,
        &bufferWithFlags,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(destinationResource)));

    CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC buffer = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    ComPtr<ID3D12Resource> intermediateResource;

    if (bufferData)
    {
        Helper::throwIfFailed(_DXDevice->CreateCommittedResource(
            &heapTypeUpload,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&intermediateResource)));
        intermediates.push_back(intermediateResource);

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.GetDXCommandList().Get(),
            *destinationResource, intermediateResource.Get(),
            0, 0, 1, &subresourceData);
    }
}

void SceneNode::_DrawCurrentNode(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const
{
    if (!_mesh)
    {
        return;
    }

    if (!Intersect(frustum, _AABB))
    {
        return;
    }

    if (_albedoTexture)
    {
        commandList.SetDescriptorHeaps({ _scene->_texturesTable->GetDescriptorHeap().GetDXDescriptorHeap().Get() });

        commandList.SetDescriptorTable(3, _scene->_texturesTable->GetResourceGPUHandle(_albedoTexture->GetName()));
        commandList.SetDescriptorTable(4, _scene->_texturesTable->GetResourceGPUHandle(_normalTexture->GetName())); 
    }

    XMMATRIX* modelMatrixData = (XMMATRIX*)_modelMatrix->Map();
    *modelMatrixData = GetGlobalTransform();
    commandList.SetSRV(1, _modelMatrix->OffsetGPU(0));

    commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList.SetVertexBuffer(0, _VBO);
    commandList.SetIndexBuffer(_IBO);

    commandList.DrawIndexed(_mesh->getIndices().size());
}
