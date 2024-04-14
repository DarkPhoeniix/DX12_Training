#pragma once

#include "DescriptorHeapDescription.h"

class DescriptorHeap
{
public:
    DescriptorHeap();
    DescriptorHeap(const DescriptorHeapDescription& description);
    ~DescriptorHeap();

    void SetDescription(const DescriptorHeapDescription& description);
    const DescriptorHeapDescription& GetDescription() const;

    void Create();

    D3D12_CPU_DESCRIPTOR_HANDLE GetHeapStartCPUHandle();
    D3D12_GPU_DESCRIPTOR_HANDLE GetHeapStartGPUHandle();

    D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(UINT index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(Resource* resource);

    D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(UINT index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(Resource* resource);

    UINT GetFreeHandleIndex() const;

    ComPtr<ID3D12DescriptorHeap> GetDXDescriptorHeap() const;

    void SetDevice(ComPtr<ID3D12Device2> device);
    ComPtr<ID3D12Device2> GetDevice() const;

private:
    ComPtr<ID3D12DescriptorHeap> _descriptorHeap;
    DescriptorHeapDescription _descriptorHeapDescription;

    std::map<UINT, Resource*> _resourceIndex;

    UINT _heapIncrementSize;

    ComPtr<ID3D12Device2> _DXDevice;
};
