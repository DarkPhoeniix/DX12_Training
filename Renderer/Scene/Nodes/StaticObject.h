#pragma once

#include "Scene/Mesh.h"
#include "Scene/Material.h"
#include "Scene/Nodes/ISceneNode.h"
#include "Scene/Volumes/AABBVolume.h"

namespace SceneLayer
{
    class StaticObject : public ISceneNode
    {
    public:
        StaticObject();
        StaticObject(SceneCache* cache, ISceneNode* parent = nullptr);
        ~StaticObject();

        const AABBVolume& GetAABB() const;

        // IScene Node
        void Draw(Core::GraphicsCommandList& commandList) const override;
        void DrawAABB(Core::GraphicsCommandList& commandList) const override;

        void LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList) override;

    protected:
        using Base = ISceneNode;

        void _CreateGPUBuffers(Core::GraphicsCommandList& commandList);
        void _UploadData(Core::GraphicsCommandList& commandList,
            ID3D12Resource** destinationResource,
            size_t numElements,
            size_t elementSize,
            const void* bufferData,
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    private:
        std::shared_ptr<Mesh> _mesh;
        std::shared_ptr<Material> _material;
        AABBVolume _AABB;

        std::shared_ptr<Core::Resource> _modelDesc;
        std::shared_ptr<Core::Resource> _vertexBuffer;
        std::shared_ptr<Core::Resource> _indexBuffer;

        D3D12_VERTEX_BUFFER_VIEW _VBO;
        D3D12_INDEX_BUFFER_VIEW _IBO;

        std::vector<ComPtr<ID3D12Resource>> _intermediates;
    };
} // namespace SceneLayer
