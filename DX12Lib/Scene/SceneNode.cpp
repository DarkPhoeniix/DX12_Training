
#include "stdafx.h"

#include "SceneNode.h"

#include "DXObjects/Texture.h"
#include "Scene/Scene.h"
#include "Volumes/FrustumVolume.h"

using namespace DirectX;

namespace
{
    XMMATRIX GetNodeLocalTransform(FbxNode* fbxNode)
    {
        FbxAMatrix fbxTransform = fbxNode->EvaluateLocalTransform();
        XMMATRIX transform =
        {
            (float)fbxTransform.mData[0][0], (float)fbxTransform.mData[0][1], (float)fbxTransform.mData[0][2], (float)fbxTransform.mData[0][3],
            (float)fbxTransform.mData[1][0], (float)fbxTransform.mData[1][1], (float)fbxTransform.mData[1][2], (float)fbxTransform.mData[1][3],
            (float)fbxTransform.mData[2][0], (float)fbxTransform.mData[2][1], (float)fbxTransform.mData[2][2], (float)fbxTransform.mData[2][3],
            (float)fbxTransform.mData[3][0], (float)fbxTransform.mData[3][1], (float)fbxTransform.mData[3][2], (float)fbxTransform.mData[3][3],
        };

        return transform;
    }
}

SceneNode::SceneNode()
    : ISceneNode()
    , _DXDevice(Core::Device::GetDXDevice())
    , _mesh(nullptr)
    , _texture(nullptr)
    , _vertexBuffer(nullptr)
    , _indexBuffer(nullptr)
    , _modelMatrix(nullptr)
    , _AABBVertexBuffer(nullptr)
    , _AABBIndexBuffer(nullptr)
    , _AABB{}
    , _AABBVBO{}
    , _AABBIBO{}
    , _VBO{}
    , _IBO{}
{
}

SceneNode::SceneNode(FbxNode* fbxNode, ComPtr<ID3D12GraphicsCommandList> commandList, Scene* scene, SceneNode* parent)
    : ISceneNode(fbxNode->GetName(), scene, parent)
    , _DXDevice(Core::Device::GetDXDevice())
    , _mesh(nullptr)
    , _texture(nullptr)
    , _vertexBuffer(nullptr)
    , _indexBuffer(nullptr)
    , _modelMatrix(nullptr)
    , _AABBVertexBuffer(nullptr)
    , _AABBIndexBuffer(nullptr)
    , _AABB{}
    , _AABBVBO{}
    , _AABBIBO{}
    , _VBO{}
    , _IBO{}
{
    Logger::Log(LogType::Info, "Parsing node " + _name);

    // Read transform
    {
        _transform = GetNodeLocalTransform(fbxNode);

        EResourceType SRVType = EResourceType::Dynamic | EResourceType::Buffer;

        ResourceDescription desc;
        desc.SetResourceType(SRVType);
        desc.SetSize({ sizeof(XMMATRIX), 1 });
        desc.SetStride(1);
        desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);

        _modelMatrix = std::make_shared<Resource>(desc);
        _modelMatrix->CreateCommitedResource(D3D12_RESOURCE_STATE_GENERIC_READ);
        _modelMatrix->SetName(_name + "_ModelMatrix");
    }

    // Setup mesh
    if (FbxMesh* fbxMesh = fbxNode->GetMesh())
    {
        FbxVector4 min, max, center;
        fbxNode->EvaluateGlobalBoundingBoxMinMaxCenter(min, max, center);
        _AABB.min = XMVectorSet(min.mData[0], min.mData[1], min.mData[2], min.mData[3]);
        _AABB.max = XMVectorSet(max.mData[0], max.mData[1], max.mData[2], max.mData[3]);

        if (_mesh = std::make_shared<Mesh>(fbxMesh))
        {
            if (!_mesh->getVertices().empty())
            {
                ComPtr<ID3D12Resource> vertexBuffer;
                _UploadData(commandList, &vertexBuffer, _mesh->getVertices().size(), sizeof(VertexData), _mesh->getVertices().data());
                _vertexBuffer = std::make_shared<Resource>();
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
                _indexBuffer = std::make_shared<Resource>();
                _indexBuffer->InitFromDXResource(indexBuffer);
                _indexBuffer->SetName(_name + "_IB");

                _IBO.BufferLocation = _indexBuffer->OffsetGPU(0);
                _IBO.Format = DXGI_FORMAT_R32_UINT;
                _IBO.SizeInBytes = static_cast<UINT>(_mesh->getIndices().size() * sizeof(_mesh->getIndices()[0]));
            }
        }
    }

    // Setup texture
    {
        std::string texName;
        int materialCount = fbxNode->GetSrcObjectCount<FbxSurfaceMaterial>();
        if (materialCount > 0)
        {
            FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)fbxNode->GetSrcObject<FbxSurfaceMaterial>(0);
            if (material)
            {
                FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

                // Check if it's layeredtextures
                int layeredTextureCount = prop.GetSrcObjectCount<FbxFileTexture>();
                if (layeredTextureCount > 0)
                {
                    FbxFileTexture* texture = prop.GetSrcObject<FbxFileTexture>(0);
                    texName = (const char*)(FbxPathUtils::GetFileName(texture->GetFileName()));
                }
            }
        }

        if (!texName.empty())
            _texture = Texture::LoadFromFile(texName);
    }

    // Setup child nodes
    for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
    {
        auto childNode = std::make_shared<SceneNode>(fbxNode->GetChild(childIndex), commandList, scene, this);
        _childNodes.push_back(childNode);
    }
}

SceneNode::~SceneNode()
{
    _DXDevice = nullptr;

    for (ID3D12Resource* intermediate : intermediates)
    {
        if (intermediate)
            delete intermediate;
        intermediate = nullptr;
    }
}

void SceneNode::Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum) const
{
    for (const std::shared_ptr<ISceneNode> node : _childNodes)
    {
        node->Draw(commandList, frustum);
    }

    _DrawCurrentNode(commandList, frustum);
}

void SceneNode::DrawAABB(ComPtr<ID3D12GraphicsCommandList> commandList) const
{
    for (const std::shared_ptr<ISceneNode> node : _childNodes)
    {
        node->DrawAABB(commandList);
    }

    if (!_mesh)
    {
        return;
    }

    commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

    commandList->SetGraphicsRoot32BitConstants(0, 3, &_AABB.min.m128_f32, 0);
    commandList->SetGraphicsRoot32BitConstants(0, 3, &_AABB.max.m128_f32, 4);

    commandList->DrawInstanced(1, 1, 0, 0);
}

const AABBVolume& SceneNode::GetAABB() const
{
    return _AABB;
}

void SceneNode::_UploadData(ComPtr<ID3D12GraphicsCommandList> commandList, 
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

    ID3D12Resource* intermediateResource;

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

        UpdateSubresources(commandList.Get(),
            *destinationResource, intermediateResource,
            0, 0, 1, &subresourceData);
    }
}

void SceneNode::_DrawCurrentNode(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum) const
{
    if (!_mesh)
    {
        return;
    }

    if (!Intersect(frustum, _AABB))
    {
        return;
    }

    XMMATRIX* modelMatrixData = (XMMATRIX*)_modelMatrix->Map();
    *modelMatrixData = GetGlobalTransform();
    commandList->SetGraphicsRootShaderResourceView(2, _modelMatrix->OffsetGPU(0));

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &_VBO);
    commandList->IASetIndexBuffer(&_IBO);

    commandList->DrawIndexedInstanced(static_cast<UINT>(_mesh->getIndices().size()), 1, 0, 0, 0);
}
