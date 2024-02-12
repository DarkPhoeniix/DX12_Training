#pragma once

#include "ResourceDescription.h"

class Resource
{
public:
    Resource(ResourceDescription resourceDesc, ComPtr<ID3D12Device> device);
    ~Resource();

    void SetResource(ComPtr<ID3D12Resource> resource);
    ComPtr<ID3D12Resource> GetResource() const;

    void SetResourceDescription(const ResourceDescription& resourceDesc);
    ResourceDescription GetResourceDescription() const;

    D3D12_GPU_VIRTUAL_ADDRESS OffsetGPU(unsigned int offset) const;
    void* Map();

    ComPtr<ID3D12Resource> CreateCommitedResource();
    ComPtr<ID3D12Resource> CreatePlacedResource(ComPtr<ID3D12Heap> heap, unsigned int offset);

private:
    ComPtr<ID3D12Resource> _resource;

    ResourceDescription _resourceDesc;

    D3D12_RESOURCE_STATES _initialState;
    D3D12_RESOURCE_STATES _currentState;

    ComPtr<ID3D12Device> _device;
};
