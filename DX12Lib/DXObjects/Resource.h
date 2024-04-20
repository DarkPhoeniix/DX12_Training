#pragma once

#include "ResourceDescription.h"

namespace Core
{
    class Resource
    {
    public:
        Resource();
        Resource(ResourceDescription resourceDesc);
        ~Resource();

        void InitFromDXResource(ComPtr<ID3D12Resource> resource);
        ComPtr<ID3D12Resource> GetDXResource() const;
        ComPtr<ID3D12Resource>& GetDXResource();

        void SetName(const std::string& name);
        std::string GetName() const;

        void SetResourceDescription(const ResourceDescription& resourceDesc);
        ResourceDescription GetResourceDescription() const;

        void SetCurrentState(D3D12_RESOURCE_STATES state);
        D3D12_RESOURCE_STATES GetCurrentState() const;

        D3D12_GPU_VIRTUAL_ADDRESS OffsetGPU(unsigned int offset) const;
        void* Map();

        D3D12_RESOURCE_BARRIER CreateBarrierAlias(Resource* old) const;

        ComPtr<ID3D12Resource> CreateCommitedResource(D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST);
        ComPtr<ID3D12Resource> CreatePlacedResource(ComPtr<ID3D12Heap> heap, unsigned int offset, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST);

    protected:
        ComPtr<ID3D12Device2> _DXDevice;

        ComPtr<ID3D12Resource> _resource;
        std::string _name;

        ResourceDescription _resourceDesc;

        D3D12_RESOURCE_STATES _initialState;
        D3D12_RESOURCE_STATES _currentState;
    };
} // namespace Core
