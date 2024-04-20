#pragma once

#include "DescriptorHeapDescription.h"

class DescriptorHeap
{
public:
    DescriptorHeap();
    DescriptorHeap(const DescriptorHeapDescription& description);
    ~DescriptorHeap();

    void Create(const std::string& name = "");
    void PlaceResource(Resource* resource);

    D3D12_CPU_DESCRIPTOR_HANDLE GetHeapStartCPUHandle();
    D3D12_GPU_DESCRIPTOR_HANDLE GetHeapStartGPUHandle();

    D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(Resource* resource);
    D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(Resource* resource);

    void SetDescription(const DescriptorHeapDescription& description);
    const DescriptorHeapDescription& GetDescription() const;

    ComPtr<ID3D12DescriptorHeap> GetDXDescriptorHeap() const;

private:
    ComPtr<ID3D12Device2> _DXDevice;

    ComPtr<ID3D12DescriptorHeap> _descriptorHeap;
    DescriptorHeapDescription _descriptorHeapDescription;
    UINT _heapIncrementSize;

    std::map<UINT, Resource*> _resourceIndex;

    std::string _name;
};
