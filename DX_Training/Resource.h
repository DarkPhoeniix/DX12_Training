#pragma once

class Resource
{
public:
    Resource(ID3D12Resource* resource);
    ~Resource();

    D3D12_GPU_VIRTUAL_ADDRESS OffsetGPU(unsigned int offset) const;
    void* Map();

    ComPtr<ID3D12Resource> _resource;
};
