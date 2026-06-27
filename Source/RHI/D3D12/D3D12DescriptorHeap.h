#pragma once

#include "DescriptorHeap.h"

namespace rhi::d3d12
{
    class D3D12DescriptorHeap final : public DescriptorHeap
    {
    public:
        D3D12DescriptorHeap(const D3D12DescriptorHeap& other) = delete;
        D3D12DescriptorHeap(D3D12DescriptorHeap&& other) noexcept;
        ~D3D12DescriptorHeap();

        D3D12DescriptorHeap& operator=(const D3D12DescriptorHeap& other) = delete;
        D3D12DescriptorHeap& operator=(D3D12DescriptorHeap&& other) noexcept;

        void Reset() override;

        std::uint32_t CopyResourceDescriptor(CPUDescriptor descriptor) override;

        CPUDescriptor GetHeapStartCPUHandle() override;
        GPUDescriptor GetHeapStartGPUHandle() override;

        CPUDescriptor GetCPUHandleWithOffset(std::uint32_t offset) override;
        GPUDescriptor GetGPUHandleWithOffset(std::uint32_t offset) override;

        std::uint32_t Offset() override;
        std::uint32_t GetCurrentOffset() const override;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12DescriptorHeap(Device* device, const DescriptorHeapDescription& description, [[maybe_unused]] const std::string& name = "");

        ComPtr<ID3D12DescriptorHeap> _descriptorHeap;
        DescriptorHeapDescription _description;

        UINT _heapIncrementSize;
        std::uint32_t _currentOffset;

        Device* _device;
#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
