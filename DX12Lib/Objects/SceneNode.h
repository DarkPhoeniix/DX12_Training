#pragma once

#include "Mesh.h"
#include "Interfaces/ISceneNode.h"
#include "AABBVolume.h"

#include <fbxsdk.h>

class SceneNode : public ISceneNode
{
public:
    SceneNode() = default;
    SceneNode(FbxNode* node, ComPtr<ID3D12GraphicsCommandList> commandList, SceneNode* parent = nullptr);
    ~SceneNode() = default;

    void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum) const override;

    const AABBVolume& getAABB() const;

protected:
    void _UploadData(ComPtr<ID3D12GraphicsCommandList> commandList,
        ID3D12Resource** destinationResource,
        size_t numElements,
        size_t elementSize,
        const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
    void _DrawCurrentNode(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum) const;

private:
    std::shared_ptr<Mesh> _mesh = nullptr;
    AABBVolume _AABB;

    std::shared_ptr<Resource> _modelMatrix = nullptr;
    std::shared_ptr<Resource> _vertexBuffer = nullptr;
    std::shared_ptr<Resource> _indexBuffer = nullptr;

    D3D12_VERTEX_BUFFER_VIEW _VBO;
    D3D12_INDEX_BUFFER_VIEW _IBO;
};

