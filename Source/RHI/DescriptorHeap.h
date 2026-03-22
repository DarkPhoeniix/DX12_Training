#pragma once

#include "Descriptor.h"

namespace rhi
{
    enum class DescriptorHeapType
    {
        RTV,
        DSV,
        CBV_SRV_UAV
    };

    struct DescriptorHeapDescription
    {
        DescriptorHeapType Type = DescriptorHeapType::CBV_SRV_UAV;
        std::uint32_t NumDescriptors = 0;
        bool ShaderVisible = false;
        std::uint32_t Flags = 0;
    };

    class DescriptorHeap
    {
    public:
        DescriptorHeap() = default;
        DescriptorHeap(const DescriptorHeap&) = delete;
        DescriptorHeap(DescriptorHeap&&) noexcept = default;
        virtual ~DescriptorHeap() = default;

        DescriptorHeap& operator=(const DescriptorHeap&) = delete;
        DescriptorHeap& operator=(DescriptorHeap&&) noexcept = default;

        virtual void Reset() = 0;

        virtual std::uint32_t CopyResourceDescriptor(CPUDescriptor descriptor) = 0;

        virtual CPUDescriptor GetHeapStartCPUHandle() = 0;
        virtual GPUDescriptor GetHeapStartGPUHandle() = 0;

        virtual CPUDescriptor GetCPUHandleWithOffset(std::uint32_t offset) = 0;
        virtual GPUDescriptor GetGPUHandleWithOffset(std::uint32_t offset) = 0;

        virtual std::uint32_t Offset() = 0;
        virtual std::uint32_t GetCurrentOffset() const = 0;

        virtual void* GetNative() const = 0;
    };
} // namespace rhi
