#pragma once

#include <fbxsdk.h>

struct VertexData
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT2 UV;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT3 Color;
};

class SceneNode
{
public:
    SceneNode() = default;
    SceneNode(FbxNode* node, ComPtr<ID3D12GraphicsCommandList> commandList);
    ~SceneNode() = default;

    void Draw(ComPtr<ID3D12GraphicsCommandList> commandList);

protected:
    void _UploadData(ComPtr<ID3D12GraphicsCommandList> commandList,
        ID3D12Resource** destinationResource,
        size_t numElements,
        size_t elementSize,
        const void* bufferData,
        D3D12_RESOURCE_FLAGS flagss = D3D12_RESOURCE_FLAG_NONE);
    void _DrawCurrentNode(ComPtr<ID3D12GraphicsCommandList> commandList);

private:
    std::vector<std::shared_ptr<SceneNode>> _childNodes = {};

    std::vector<VertexData> _rawVertexData;
    std::vector<WORD> _rawIndexData;
    DirectX::XMMATRIX _transform;

    std::shared_ptr<Resource> _vertexBuffer = nullptr;
    std::shared_ptr<Resource> _indexBuffer = nullptr;

    D3D12_VERTEX_BUFFER_VIEW _VBO;
    D3D12_INDEX_BUFFER_VIEW _IBO;
};

