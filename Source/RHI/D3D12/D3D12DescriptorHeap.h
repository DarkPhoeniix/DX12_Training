#pragma once

#include "DescriptorHeap.h"

namespace rhi::d3d12
{
    class D3D12DescriptorHeap final : public rhi::DescriptorHeap
    {
    public:
        D3D12DescriptorHeap(const D3D12DescriptorHeap& other) = delete;
        D3D12DescriptorHeap(D3D12DescriptorHeap&& other) noexcept;
        ~D3D12DescriptorHeap();

        D3D12DescriptorHeap& operator=(const D3D12DescriptorHeap& other) = delete;
        D3D12DescriptorHeap& operator=(D3D12DescriptorHeap&& other) noexcept;

        void Reset() override;

        std::uint32_t CopyResourceDescriptor(rhi::CPUDescriptor descriptor) override;

        rhi::CPUDescriptor GetHeapStartCPUHandle() override;
        rhi::GPUDescriptor GetHeapStartGPUHandle() override;

        rhi::CPUDescriptor GetCPUHandleWithOffset(std::uint32_t offset) override;
        rhi::GPUDescriptor GetGPUHandleWithOffset(std::uint32_t offset) override;

        std::uint32_t Offset() override;
        std::uint32_t GetCurrentOffset() const override;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12DescriptorHeap(rhi::Device* device, const rhi::DescriptorHeapDescription& description, [[maybe_unused]] const std::string& name = "");

        ComPtr<ID3D12DescriptorHeap> _descriptorHeap;
        rhi::DescriptorHeapDescription _description;

        UINT _heapIncrementSize;
        std::uint32_t _currentOffset;

        rhi::Device* _device;
#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
