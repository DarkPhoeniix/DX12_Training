#pragma once

#include "Scene/Mesh.h"
#include "Scene/ISceneNode.h"
#include "Scene/Volumes/AABBVolume.h"

#include <fbxsdk.h>

class Camera;
class Heap;
class DescriptorHeap;

class SceneNode : public ISceneNode
{
public:
    SceneNode();
    SceneNode(FbxNode* node, ComPtr<ID3D12GraphicsCommandList> commandList, SceneNode* parent = nullptr);
    ~SceneNode();

    void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum) const override;
    void DrawAABB(ComPtr<ID3D12GraphicsCommandList> commandList) const override;

    const AABBVolume& GetAABB() const;

protected:
    void _UploadData(ComPtr<ID3D12GraphicsCommandList> commandList,
                     ID3D12Resource** destinationResource,
                     size_t numElements,
                     size_t elementSize,
                     const void* bufferData,
                     D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
    void _DrawCurrentNode(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum) const;

private:
    ComPtr<ID3D12Device2> _DXDevice;

    std::shared_ptr<Mesh> _mesh;
    AABBVolume _AABB;

    std::shared_ptr<Resource> _modelMatrix;
    std::shared_ptr<Resource> _vertexBuffer;
    std::shared_ptr<Resource> _indexBuffer;

    D3D12_VERTEX_BUFFER_VIEW _VBO;
    D3D12_INDEX_BUFFER_VIEW _IBO;

    std::shared_ptr<Resource> _AABBVertexBuffer;
    std::shared_ptr<Resource> _AABBIndexBuffer;

    D3D12_VERTEX_BUFFER_VIEW _AABBVBO;
    D3D12_INDEX_BUFFER_VIEW _AABBIBO;
    
    std::vector<ID3D12Resource*> intermediates;
};

