#include "stdafx.h"

#include "OcclusionQuery.h"

#include "Scene/Volumes/FrustumVolume.h"

namespace Core
{
    OcclusionQuery::OcclusionQuery()
        : _queryHeap(nullptr)
    {
    }

    OcclusionQuery::~OcclusionQuery()
    {
        _queryHeap = nullptr;
    }

    void OcclusionQuery::Create(int numObjects)
    {
        // Describe and create a heap for occlusion queries.
        D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
        queryHeapDesc.Count = numObjects;
        queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        Core::Device::GetDXDevice()->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&_queryHeap));

        for (int i = 0; i < numObjects; ++i)
        {
            ResourceDescription desc(CD3DX12_RESOURCE_DESC::Buffer(8));
            _queryResults.emplace_back(desc);
            _queryResults.back().CreateCommitedResource(D3D12_RESOURCE_STATE_PREDICATION);
            _queryResults.back().SetName("Query Result " + std::to_string(i));
        }
    }

    void OcclusionQuery::Run(const SceneLayer::ISceneNode* node, CommandList& commandList)
    {
        size_t index = _queryResources.size();
        for (size_t i = 0; i < _queryResources.size(); ++i)
        {
            if (_queryResources[i] == node)
            {
                index = i;
                _queryResources.push_back(node);
                _queryResults.push_back(Resource());
            }
        }

        commandList.BeginQuery(_queryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, index);
        node->Draw(commandList);
        commandList.EndQuery(_queryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, index);

        commandList.TransitionBarrier(_queryResults[index], D3D12_RESOURCE_STATE_COPY_DEST);
        commandList.ResolveQueryData(_queryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, index, _queryResults[index], 0);
        commandList.TransitionBarrier(_queryResults[index], D3D12_RESOURCE_STATE_PREDICATION);
    }

    void OcclusionQuery::SetPredication(const SceneLayer::ISceneNode* node, CommandList& commandList)
    {
        for (size_t i = 0; i < _queryResources.size(); ++i)
        {
            if (_queryResources[i] == node)
            {
                commandList.SetPredication(&_queryResults[i], 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
            }
        }
        commandList.SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
    }
}
