#pragma once

#include "Scene/Mesh.h"
#include "Scene/ISceneNode.h"
#include "Scene/Volumes/AABBVolume.h"
#include "Utility/Blob.h"

#include <fbxsdk.h>

class Camera;
class Heap;
class DescriptorHeap;

class SceneNode : public ISceneNode
{
public:
    SceneNode() = default;
    SceneNode(FbxNode* node, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12Device2> device = nullptr, SceneNode* parent = nullptr);
    ~SceneNode();

    void SetDevice(ComPtr<ID3D12Device2> device);
    ComPtr<ID3D12Device2> GetDevice() const;

    void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum) const override;
    void DrawAABB(ComPtr<ID3D12GraphicsCommandList> commandList) const override;

    void UploadTextures(ComPtr<ID3D12GraphicsCommandList> commandList, Heap& heap, DescriptorHeap& descriptorHeap) override;

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

    std::shared_ptr<Resource> _AABBVertexBuffer = nullptr;
    std::shared_ptr<Resource> _AABBIndexBuffer = nullptr;

    D3D12_VERTEX_BUFFER_VIEW _AABBVBO;
    D3D12_INDEX_BUFFER_VIEW _AABBIBO;
    
    std::vector<ID3D12Resource*> intermediates;

    Resource _resource;
    Base::Blob _textureBlob;
    D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle;

    ComPtr<ID3D12Device2> _DXDevice;
};

