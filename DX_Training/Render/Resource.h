#pragma once

#include "ResourceDescription.h"

class Resource
{
public:
    Resource() = default;
    Resource(ComPtr<ID3D12Device> device, ResourceDescription resourceDesc);
    ~Resource();

    void SetResource(ComPtr<ID3D12Resource> resource);
    ComPtr<ID3D12Resource> GetResource() const;

    void SetName(const std::string& name);
    std::string GetName() const;

    void SetResourceDescription(const ResourceDescription& resourceDesc);
    ResourceDescription GetResourceDescription() const;

    D3D12_GPU_VIRTUAL_ADDRESS OffsetGPU(unsigned int offset) const;
    void* Map();

    ComPtr<ID3D12Resource> CreateCommitedResource(D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST);
    ComPtr<ID3D12Resource> CreatePlacedResource(ComPtr<ID3D12Heap> heap, unsigned int offset, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST);

private:
    ComPtr<ID3D12Resource> _resource = nullptr;
    std::string _name;

    ResourceDescription _resourceDesc;

    D3D12_RESOURCE_STATES _initialState;
    D3D12_RESOURCE_STATES _currentState;

    ComPtr<ID3D12Device> _device;
};
