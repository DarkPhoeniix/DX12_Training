#pragma once

#include "CommandList.h"
#include "Descriptor.h"
#include "DescriptorHeap.h"
#include "ResourceCommon.h"

namespace tracking
{
    class IGPUCrashTracker;
}

namespace rhi
{
    class CommandQueue;
    class CommandSignature;
    class PipelineState;
    class Fence;
    class Buffer;
    class Texture;
    class DescriptorHeap;
    class QueryHeap;
    class Heap;
    class SwapChain;
    class StatisticsQuery;
    class TimestampQuery;
    struct BufferDescription;
    struct TextureDescription;
    struct IndirectArgumentDescription;
    struct DescriptorHeapDescription;
    struct QueryHeapDescription;
    struct HeapDescription;

    enum class BackendAPI
    {
        D3D12,
        Vulkan
    };

    class Device
    {
    public:
        Device() = default;
        Device(const Device&) = delete;
        Device(Device&&) noexcept = default;
        virtual ~Device() = default;

        Device& operator=(const Device&) = delete;
        Device& operator=(Device&&) noexcept = default;

        virtual bool IsEnhancedBarriersSupported() = 0;

        virtual void BindSwapChain(SwapChain& swapChain) = 0;

        virtual CommandQueue* GetComputeQueue() = 0;
        virtual CommandQueue* GetStreamQueue() = 0;
        virtual CommandQueue* GetCopyQueue() = 0;

        virtual void OnResize(std::uint32_t width, std::uint32_t height) = 0;
        virtual std::shared_ptr<Texture> GetBackBuffer() = 0;

        virtual void Present() = 0;

        virtual std::shared_ptr<Buffer> CreateBuffer(const BufferDescription& description, ResourceState initialState = ResourceState::Common) = 0;
        virtual std::shared_ptr<Buffer> CreateBuffer(const BufferDescription& description, Heap* heap, std::uint64_t offset, ResourceState initialState = ResourceState::Common) = 0;
        virtual std::shared_ptr<Buffer> CreateBuffer(void* nativePtr) = 0;
        virtual std::shared_ptr<Texture> CreateTexture(const TextureDescription& description, ResourceState initialState = ResourceState::Common) = 0;
        virtual std::shared_ptr<Texture> CreateTexture(const TextureDescription& description, Heap* heap, std::uint64_t offset, ResourceState initialState = ResourceState::Common) = 0;
        virtual std::shared_ptr<Texture> CreateTexture(void* nativePtr) = 0;

        virtual std::unique_ptr<CommandList> CreateCommandList(CommandListType type) = 0;
        virtual std::unique_ptr<DescriptorHeap> CreateDescriptorHeap(const DescriptorHeapDescription& description) = 0;
        virtual std::unique_ptr<QueryHeap> CreateQueryHeap(const QueryHeapDescription& description) = 0;
        virtual std::unique_ptr<Fence> CreateFence(std::uint64_t initialValue) = 0;
        virtual std::unique_ptr<Heap> CreateHeap(const HeapDescription& description) = 0;
        virtual std::unique_ptr<StatisticsQuery> CreateStatisticsQuery() = 0;
        virtual std::unique_ptr<TimestampQuery> CreateTimestampQuery(std::uint32_t timestampsCount) = 0;
        virtual std::unique_ptr<CommandSignature> CreateCommandSignature(const std::vector<IndirectArgumentDescription>& arguments, PipelineState* pipelineState) = 0;

        virtual void CreateBufferSRV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor) = 0;
        virtual void CreateBufferCBV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor) = 0;
        virtual void CreateBufferUAV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor, std::shared_ptr<Buffer> counterResource = nullptr) = 0;
        virtual void CreateTextureRTV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) = 0;
        virtual void CreateTextureDSV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) = 0;
        virtual void CreateTextureSRV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) = 0;
        virtual void CreateTextureCBV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) = 0;
        virtual void CreateTextureUAV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) = 0;

        virtual std::uint32_t GetDescriptorHandleIncrementSize(DescriptorHeapType type) const = 0;

        virtual AllocationInfo GetAllocationInfo(const BufferDescription& description) const = 0;
        virtual AllocationInfo GetAllocationInfo(const TextureDescription& description) const = 0;

        virtual tracking::IGPUCrashTracker* GetCrashTracker() = 0;

        virtual void* GetNative() const = 0;
    };

    static std::unique_ptr<Device> CreateDevice(BackendAPI backend);
} // namespace rhi
