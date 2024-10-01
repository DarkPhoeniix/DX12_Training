#pragma once

#include "DXObjects/GraphicsCommandList.h"
#include "Scene/Nodes/ISceneNode.h"

namespace SceneLayer
{
    class ISceneNode;
    class FrustumVolume;
} // namespace SceneLayer

namespace Core
{
    class OcclusionQuery
    {
    public:
        OcclusionQuery();
        ~OcclusionQuery();

        void Create(int numObjects = 256);

        void Run(const SceneLayer::ISceneNode* node, GraphicsCommandList& commandList, const SceneLayer::FrustumVolume& frustum);
        void SetPredication(const SceneLayer::ISceneNode* node, GraphicsCommandList& commandList);

    private:
        ComPtr<ID3D12QueryHeap> _queryHeap;
        std::vector<const SceneLayer::ISceneNode*> _queryResources;
        std::vector<Resource> _queryResults;
    };
} // namespace Core
