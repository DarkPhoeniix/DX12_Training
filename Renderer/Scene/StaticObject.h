#pragma once

#include "Scene/Mesh.h"
#include "Scene/Material.h"
#include "Scene/ISceneNode.h"
#include "Scene/Volumes/AABBVolume.h"

#include <fbxsdk.h>

class Scene;

class StaticObject : public ISceneNode
{
public:
    StaticObject();
    StaticObject(Scene* scene, StaticObject* parent = nullptr);
    ~StaticObject();

    void Draw(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const override;
    void DrawAABB(Core::GraphicsCommandList& commandList) const override;

    const AABBVolume& GetAABB() const;

    void LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList) override;

protected:
    void _UploadData(Core::GraphicsCommandList& commandList,
                     ID3D12Resource** destinationResource,
                     size_t numElements,
                     size_t elementSize,
                     const void* bufferData,
                     D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
    void _DrawCurrentNode(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const;

private:
    std::shared_ptr<Mesh> _mesh;
    std::shared_ptr<Material> _material;
    AABBVolume _AABB;

    std::shared_ptr<Core::Texture> _albedoTexture;
    std::shared_ptr<Core::Texture> _normalTexture;

    std::shared_ptr<Core::Resource> _modelMatrix;
    std::shared_ptr<Core::Resource> _modelDesc;
    std::shared_ptr<Core::Resource> _vertexBuffer;
    std::shared_ptr<Core::Resource> _indexBuffer;

    D3D12_VERTEX_BUFFER_VIEW _VBO;
    D3D12_INDEX_BUFFER_VIEW _IBO;
    
    std::vector<ComPtr<ID3D12Resource>> intermediates;
};
