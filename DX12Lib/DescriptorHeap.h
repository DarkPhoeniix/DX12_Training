#pragma once

#include "DescriptorHeapDescription.h"

namespace dx12
{
    class Resource;

    enum class DescriptorHeapType
    {
        RTV,
        DSV,
        CBV_SRV_UAV
    };

    class DescriptorHeap
    {
    public:
        DescriptorHeap();
        DescriptorHeap(const DescriptorHeapDescription& description);
        ~DescriptorHeap();

        void Create();
        void Create(const DescriptorHeapDescription& description);
        void Reset();

        void PlaceResource(Resource* resource);
        void PlaceResourceDescriptor(Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

        D3D12_CPU_DESCRIPTOR_HANDLE GetHeapStartCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetHeapStartGPUHandle();

        D3D12_CPU_DESCRIPTOR_HANDLE GetFreeCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetFreeGPUHandle();

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
        DescriptorHeapDescription _description;
        UINT _heapIncrementSize;

        std::vector<Resource*> _resources;

        std::string _name;
    };
} // namespace dx12
