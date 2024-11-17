#pragma once

#include "CommandList.h"

namespace SceneLayer
{
    class ISceneNode;
    class FrustumVolume;
} // namespace SceneLayer

namespace dx12
{
    class OcclusionQuery
    {
    public:
        OcclusionQuery();
        ~OcclusionQuery();

        void Create(int numObjects = 256);

        //void Run(const SceneLayer::ISceneNode* node, CommandList& commandList);
        //void SetPredication(const SceneLayer::ISceneNode* node, CommandList& commandList);

    private:
        ComPtr<ID3D12QueryHeap> _queryHeap;
        std::vector<const SceneLayer::ISceneNode*> _queryResources;
        std::vector<Resource> _queryResults;
    };
} // namespace dx12
