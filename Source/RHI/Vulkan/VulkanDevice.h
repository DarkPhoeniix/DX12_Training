#pragma once

#include "Device.h"

namespace tracking
{
    class IGPUCrashTracker;
} // namespace tracking

namespace vma
{
    class Allocator;
} // namespace vma

namespace rhi::vulkan
{
    class VulkanDevice final : public rhi::Device
    {
    public:
        VulkanDevice();
        VulkanDevice(const VulkanDevice& other) = delete;
        VulkanDevice(VulkanDevice&& other) noexcept;
        ~VulkanDevice() override;

        VulkanDevice& operator=(const VulkanDevice& other) = delete;
        VulkanDevice& operator=(VulkanDevice&& other) noexcept;

        BackendAPI GetBackend() const override;

        bool IsEnhancedBarriersSupported() override;

        void BindSwapChain(SwapChain* swapChain) override;

        CommandQueue* GetQueue(rhi::CommandListType type) override;
        CommandQueue* GetGraphicsQueue() override;
        CommandQueue* GetComputeQueue() override;
        CommandQueue* GetCopyQueue() override;

        void OnResize(std::uint32_t width, std::uint32_t height) override;
        std::shared_ptr<Texture> GetBackBuffer() override;

        void Present() override;

        std::shared_ptr<Buffer> CreateBuffer(const BufferDescription& description, ResourceState initialState = ResourceState::Common, const std::string& name = "") override;
        std::shared_ptr<Buffer> CreateBuffer(void* nativePtr, const std::string& name = "") override;
        std::shared_ptr<Texture> CreateTexture(const TextureDescription& description, ResourceState initialState = ResourceState::Common, const std::string& name = "") override;
        std::shared_ptr<Texture> CreateTexture(void* nativePtr, const std::string& name = "") override;

        std::unique_ptr<CommandList> CreateCommandList(CommandListType type, const std::string& name = "") override;
        std::unique_ptr<CommandListPool> CreateCommandListPool() override;
        std::unique_ptr<DescriptorHeap> CreateDescriptorHeap(const DescriptorHeapDescription& description, const std::string& name = "") override;
        std::unique_ptr<QueryHeap> CreateQueryHeap(const QueryHeapDescription& description, const std::string& name = "") override;
        std::unique_ptr<Fence> CreateFence(std::uint64_t initialValue) override;
        std::unique_ptr<StatisticsQuery> CreateStatisticsQuery(const std::string& name = "") override;
        std::unique_ptr<TimestampQuery> CreateTimestampQuery(std::uint32_t timestampsCount, const std::string& name = "") override;
        std::unique_ptr<CommandSignature> CreateCommandSignature(const std::vector<IndirectArgumentDescription>& arguments, PipelineState* pipelineState, const std::string& name = "") override;
        std::unique_ptr<SwapChain> CreateSwapChain(void* windowHandle, std::uint32_t width, std::uint32_t height, bool vSync) override;
        std::unique_ptr<PipelineState> CreatePipelineState(const std::string& filepath) override;

        void CreateBufferView(const BufferView& view, CPUDescriptor& descriptor) override;
        void CreateBufferSRV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor) override;
        void CreateBufferCBV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor) override;
        void CreateBufferUAV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor, std::shared_ptr<Buffer> counterResource = nullptr) override;

        void CreateTextureView(const TextureView& view, CPUDescriptor& descriptor) override;
        void CreateTextureRTV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) override;
        void CreateTextureDSV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) override;
        void CreateTextureSRV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) override;
        void CreateTextureUAV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) override;

        std::uint32_t GetDescriptorHandleIncrementSize(DescriptorHeapType type) const override;

        AllocationInfo GetAllocationInfo(const BufferDescription& description) const override;
        AllocationInfo GetAllocationInfo(const TextureDescription& description) const override;

        const AdapterInfo& QueryAdapterInfo() override;

        AllocatorStats QueryAllocatorStats() const override;

        tracking::IGPUCrashTracker* GetCrashTracker() override;

        void* GetNative() const override;

    private:
        vk::Instance CreateInstance();
        vk::Device CreateDevice();
        void EnumerateExtensions() const;
        bool CheckExtensionsSupport(const std::vector<const char*>& extensions) const;
        bool CheckLayersSupport(const std::vector<const char*>& layers) const;

        vk::Instance _instance;
        vk::Device _device;
    };
} // namespace rhi::vulkan
