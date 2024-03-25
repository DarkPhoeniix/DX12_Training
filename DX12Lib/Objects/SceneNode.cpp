#include "stdafx.h"

#include "SceneNode.h"
#include "Scene.h"

#include "Application.h"
#include "FrustumVolume.h"
//#include "TextureLoaderDDS.h"

using namespace DirectX;

//namespace
//{
//    std::string getTextureName(FbxNode* node)
//    {
//        std::string name;
//
//        int mcount = node->GetSrcObjectCount<FbxSurfaceMaterial>();
//        for (int index = 0; index < mcount; index++)
//        {
//            FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)node->GetSrcObject<FbxSurfaceMaterial>(index);
//            if (material)
//            {
//                // This only gets the material of type sDiffuse, you probably need to traverse all Standard Material Property by its name to get all possible textures.
//                FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
//
//                // Check if it's layeredtextures
//                int layered_texture_count = prop.GetSrcObjectCount<FbxLayeredTexture>();
//                if (layered_texture_count > 0)
//                {
//                    for (int j = 0; j < layered_texture_count; j++)
//                    {
//                        FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(prop.GetSrcObject<FbxLayeredTexture>(j));
//                        int lcount = layered_texture->GetSrcObjectCount<FbxTexture>();
//                        for (int k = 0; k < lcount; k++)
//                        {
//                            FbxTexture* texture = FbxCast<FbxTexture>(layered_texture->GetSrcObject<FbxTexture>(k));
//                            // Then, you can get all the properties of the texture, include its name
//                            name = texture->GetName();
//                        }
//                    }
//                }
//                else
//                {
//                    // Directly get textures
//                    int texture_count = prop.GetSrcObjectCount<FbxTexture>();
//                    for (int j = 0; j < texture_count; j++)
//                    {
//                        const FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(j));
//                        // Then, you can get all the properties of the texture, include its name
//                        name = texture->GetName();
//                    }
//                }
//            }
//        }
//
//        return name;
//    }
//}

SceneNode::SceneNode(FbxNode* fbxNode, ComPtr<ID3D12GraphicsCommandList> commandList, SceneNode* parent)
{
    _parent = parent;
    _name = fbxNode->GetName();

    auto device = Application::get().getDevice();

    Logger::Log(LogType::Info, "Parsing node " + _name);

    // Read transform
    {
        FbxAMatrix transform = fbxNode->EvaluateLocalTransform();
        _transform = 
        { 
            (float)transform.mData[0][0], (float)transform.mData[0][1], (float)transform.mData[0][2], (float)transform.mData[0][3],
            (float)transform.mData[1][0], (float)transform.mData[1][1], (float)transform.mData[1][2], (float)transform.mData[1][3],
            (float)transform.mData[2][0], (float)transform.mData[2][1], (float)transform.mData[2][2], (float)transform.mData[2][3],
            (float)transform.mData[3][0], (float)transform.mData[3][1], (float)transform.mData[3][2], (float)transform.mData[3][3],
        };

        EResourceType SRVType = EResourceType::Dynamic | EResourceType::Buffer;

        ResourceDescription desc;
        desc.SetResourceType(SRVType);
        desc.SetSize({ sizeof(XMMATRIX), 1 });
        desc.SetStride(1);
        desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);

        _modelMatrix = std::make_shared<Resource>(device, desc);
        _modelMatrix->CreateCommitedResource(D3D12_RESOURCE_STATE_GENERIC_READ);
        _modelMatrix->SetName(_name + "ModelMatrix");
    }

    // Setup child nodes
    for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
    {
        auto childNode = std::make_shared<SceneNode>(fbxNode->GetChild(childIndex), commandList, this);
        _childNodes.push_back(childNode);
    }

    // Setup mesh
    FbxMesh* fbxMesh = fbxNode->GetMesh();
    if (!fbxMesh)
        return;

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
            _vertexBuffer->SetResource(vertexBuffer);
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
            _indexBuffer->SetResource(indexBuffer);
            _indexBuffer->SetName(_name + "_IB");

            _IBO.BufferLocation = _indexBuffer->OffsetGPU(0);
            _IBO.Format = DXGI_FORMAT_R32_UINT;
            _IBO.SizeInBytes = static_cast<UINT>(_mesh->getIndices().size() * sizeof(_mesh->getIndices()[0]));
        }
    }
}

void SceneNode::Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum) const
{
    for (const std::shared_ptr<ISceneNode> node : _childNodes)
        node->Draw(commandList, frustum);

    _DrawCurrentNode(commandList, frustum);
}

const AABBVolume& SceneNode::getAABB() const
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
    auto device = Application::get().getDevice();

    size_t bufferSize = numElements * elementSize;

    CD3DX12_HEAP_PROPERTIES heapTypeDefault(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferWithFlags = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    Helper::throwIfFailed(device->CreateCommittedResource(
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
        Helper::throwIfFailed(device->CreateCommittedResource(
            &heapTypeUpload,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&intermediateResource)));

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
        return;

    if (!intersect(frustum, _AABB))
    {
        OutputDebugStringA(std::string("Skipped render: " + _name + "\n").c_str());
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
