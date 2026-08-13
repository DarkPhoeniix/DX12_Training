#pragma once

#include "Device.h"
#include "VulkanHelpers.h"

namespace tracking
{
    class IGPUCrashTracker;
} // namespace tracking

namespace rhi::vulkan
{
    class VulkanDescriptorHeap;

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

        CommandQueue* GetGraphicsQueue() override;
        CommandQueue* GetComputeQueue() override;
        CommandQueue* GetCopyQueue() override;

        void OnResize(std::uint32_t width, std::uint32_t height) override;
        std::shared_ptr<Texture> GetBackBuffer() override;

        void Present() override;

        std::shared_ptr<rhi::Buffer> CreateBuffer(const BufferDescription& description, ResourceState initialState = ResourceState::Common, const std::string& name = "") override;
        std::shared_ptr<rhi::Buffer> CreateBuffer(void* nativePtr, const std::string& name = "") override;
        std::shared_ptr<rhi::Texture> CreateTexture(const TextureDescription& description, ResourceState initialState = ResourceState::Common, const std::string& name = "") override;
        std::shared_ptr<rhi::Texture> CreateTexture(void* nativePtr, const std::string& name = "") override;

        std::unique_ptr<rhi::CommandList> CreateCommandList(CommandListType type, const std::string& name = "") override;
        std::unique_ptr<CommandListPool> CreateCommandListPool() override;
        std::unique_ptr<rhi::DescriptorHeap> CreateDescriptorHeap(const DescriptorHeapDescription& description, const std::string& name = "") override;
        std::unique_ptr<rhi::QueryHeap> CreateQueryHeap(const QueryHeapDescription& description, const std::string& name = "") override;
        std::unique_ptr<rhi::Fence> CreateFence(std::uint64_t initialValue) override;
        std::unique_ptr<rhi::StatisticsQuery> CreateStatisticsQuery(const std::string& name = "") override;
        std::unique_ptr<rhi::TimestampQuery> CreateTimestampQuery(std::uint32_t timestampsCount, const std::string& name = "") override;
        std::unique_ptr<rhi::CommandSignature> CreateCommandSignature(const std::vector<IndirectArgumentDescription>& arguments, PipelineState* pipelineState, const std::string& name = "") override;
        std::unique_ptr<rhi::SwapChain> CreateSwapChain(void* windowHandle, std::uint32_t width, std::uint32_t height, bool vSync) override;
        std::unique_ptr<rhi::PipelineState> CreatePipelineState(const std::string& filepath) override;

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
        vk::Instance GetVulkanInstance() const;
        vk::PhysicalDevice GetPhysicalDevice() const;
        SwapChain* GetSwapChain() const;

    private:
        vk::Instance CreateInstance();
        vk::Device CreateDevice();
        VmaAllocator CreateAllocator();

        void CreateBufferSRV(const BufferView& view, CPUDescriptor& descriptor);
        void CreateBufferCBV(const BufferView& view, CPUDescriptor& descriptor);
        void CreateBufferUAV(const BufferView& view, CPUDescriptor& descriptor);

        void CreateTextureRTV(const TextureView& view, CPUDescriptor& descriptor);
        void CreateTextureDSV(const TextureView& view, CPUDescriptor& descriptor);
        void CreateTextureSRV(const TextureView& view, CPUDescriptor& descriptor);
        void CreateTextureUAV(const TextureView& view, CPUDescriptor& descriptor);

        vk::ImageView CreateImageView(const TextureView& view);

        void EnumerateExtensions() const;
        bool CheckExtensionsSupport(const std::vector<const char*>& extensions) const;
        bool CheckLayersSupport(const std::vector<const char*>& layers) const;
        bool CheckFeatureSupport(const vk::PhysicalDevice& physicalDevice) const;

#if ENABLE_DEVICE_DEBUG
        vk::DebugUtilsMessengerEXT SetupDebugMessenger();
#endif // ENABLE_DEVICE_DEBUG

        vk::Instance _instance;
        vk::PhysicalDevice _physicalDevice;
        vk::Device _logicalDevice;

        VmaAllocator _allocator;

        SwapChain* _swapChain;

        std::unique_ptr<rhi::CommandQueue> _graphicsQueue;
        std::unique_ptr<rhi::CommandQueue> _computeQueue;
        std::unique_ptr<rhi::CommandQueue> _copyQueue;
#if ENABLE_DEVICE_DEBUG
        vk::DebugUtilsMessengerEXT _debugMessenger;
#endif // ENABLE_DEVICE_DEBUG

        std::unique_ptr<tracking::IGPUCrashTracker> _crashTracker;
    };
} // namespace rhi::vulkan
