#pragma once

#include "DescriptorHeapDescription.h"

namespace Core
{
    class DescriptorHeap
    {
    public:
        DescriptorHeap();
        DescriptorHeap(const DescriptorHeapDescription& description);
        ~DescriptorHeap();

        void Create();
        void PlaceResource(Resource* resource);

        D3D12_CPU_DESCRIPTOR_HANDLE GetHeapStartCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetHeapStartGPUHandle();

        D3D12_GPU_DESCRIPTOR_HANDLE GetHeapGPUHandle(size_t offset = 0);

        D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(Resource* resource);
        D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(Resource* resource);
        UINT GetResourceIndex(Resource* resource);

        void SetDescription(const DescriptorHeapDescription& description);
        const DescriptorHeapDescription& GetDescription() const;

        void SetName(const std::string& name);
        const std::string& GetName() const;

        ComPtr<ID3D12DescriptorHeap> GetDXDescriptorHeap() const;

    private:
        ComPtr<ID3D12DescriptorHeap> _descriptorHeap;
        DescriptorHeapDescription _descriptorHeapDescription;
        UINT _heapIncrementSize;

        std::map<UINT, Resource*> _resourceIndex;

        std::string _name;
    };
} // namespace Core
