#pragma once

#include "DescriptorHeapDescription.h"

namespace dx12
{
    class Resource;

    enum class ResourceViewType
    {
        Unknown,
        RTV,
        DSV,
        CBV,
        SRV,
        UAV
    };

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

        std::uint32_t CopyResourceDescriptor(Resource* resource, ResourceViewType viewType, D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

        D3D12_CPU_DESCRIPTOR_HANDLE GetHeapStartCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetHeapStartGPUHandle();

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleWithOffset(std::uint32_t offset);
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleWithOffset(std::uint32_t offset);

        std::uint32_t Offset();
        std::uint32_t GetCurrentOffset() const;

        void SetDescription(const DescriptorHeapDescription& description);
        const DescriptorHeapDescription& GetDescription() const;

        void SetName(const std::string& name);
        const std::string& GetName() const;

        ComPtr<ID3D12DescriptorHeap> GetDXDescriptorHeap() const;

    private:
        ComPtr<ID3D12DescriptorHeap> _descriptorHeap;
        DescriptorHeapDescription _description;
        UINT _heapIncrementSize;
        std::uint32_t _currentOffset;

        std::string _name;
    };
} // namespace dx12
