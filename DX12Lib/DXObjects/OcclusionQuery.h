#pragma once

#include "DXObjects/GraphicsCommandList.h"
#include "Scene/ISceneNode.h"

class ISceneNode;
class FrustumVolume;

namespace Core
{
    class OcclusionQuery
    {
    public:
        OcclusionQuery();
        ~OcclusionQuery();

        void Create(int numObjects = 256);

        void Run(const ISceneNode* node, GraphicsCommandList& commandList, const FrustumVolume& frustum);
        void SetPredication(const ISceneNode* node, GraphicsCommandList& commandList);

    private:
        ComPtr<ID3D12Device2> _DXDevice;

        ComPtr<ID3D12QueryHeap> _queryHeap;
        std::vector<const ISceneNode*> _queryResources;
        std::vector<Resource> _queryResults;
    };
}
