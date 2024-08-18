#include "stdafx.h"

#include "StatisticsQuery.h"

namespace Core
{
    StatisticsQuery::StatisticsQuery()
        : _DXDevice(Core::Device::GetDXDevice())
        , _statQueryHeap(nullptr)
    {
    }

    StatisticsQuery::~StatisticsQuery()
    {
        _DXDevice = nullptr;
        _statQueryHeap = nullptr;
    }

    void StatisticsQuery::Create()
    {
        D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
        queryHeapDesc.Count = 1;
        queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
        _DXDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&_statQueryHeap));

        D3D12_RESOURCE_DESC desc(CD3DX12_RESOURCE_DESC::Buffer(sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS)));
        _statResource.SetResourceDescription(ResourceDescription(desc));
        ComPtr<ID3D12Resource> res;

        CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        _DXDevice->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&res)
        );
        _statResource.InitFromDXResource(res);
    }

    void StatisticsQuery::BeginQuery(GraphicsCommandList& commandList)
    {
        commandList.BeginQuery(_statQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0);
    }

    void StatisticsQuery::EndQuery(GraphicsCommandList& commandList)
    {
        commandList.EndQuery(_statQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0);
    }

    void StatisticsQuery::ResolveQueryData(GraphicsCommandList& commandList)
    {
        commandList.ResolveQueryData(_statQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, _statResource, 0);
    }

    D3D12_QUERY_DATA_PIPELINE_STATISTICS StatisticsQuery::GetStatistics()
    {
        D3D12_QUERY_DATA_PIPELINE_STATISTICS* stat = (D3D12_QUERY_DATA_PIPELINE_STATISTICS*)_statResource.Map();
        return *stat;
    }
} // namespace Core
