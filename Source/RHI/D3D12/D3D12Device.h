#pragma once

#include "Device.h"

namespace tracking
{
    class IGPUCrashTracker;
} // namespace tracking

namespace D3D12MA
{
    class Allocator;
} // namespace D3D12MA

namespace rhi::d3d12
{
    using NativeDevice = ID3D12Device10;

    class D3D12Device final : public Device
    {
    public:
        D3D12Device();
        ~D3D12Device() override;
        D3D12Device(const D3D12Device& other) = delete;
        D3D12Device(D3D12Device&& other) noexcept;

        D3D12Device& operator=(const D3D12Device& other) = delete;
        D3D12Device& operator=(D3D12Device&& other) noexcept;

        BackendAPI GetBackend() const override;

        bool IsEnhancedBarriersSupported() override;

        void BindSwapChain(SwapChain* swapChain) override;

        CommandQueue* GetGraphicsQueue() override;
        CommandQueue* GetComputeQueue() override;
        CommandQueue* GetCopyQueue() override;

        void OnResize(std::uint32_t width, std::uint32_t height) override;
        std::shared_ptr<Texture> GetBackBuffer() override;

        void Present() override;

        std::shared_ptr<Buffer> CreateBuffer(const BufferDescription& description, ResourceState initialState, const std::string& name) override;
        std::shared_ptr<Buffer> CreateBuffer(void* nativePtr, const std::string& name) override;
        std::shared_ptr<Texture> CreateTexture(const TextureDescription& description, ResourceState initialState, const std::string& name) override;
        std::shared_ptr<Texture> CreateTexture(void* nativePtr, const std::string& name) override;

        std::unique_ptr<CommandList> CreateCommandList(CommandListType type, const std::string& name) override;
        std::unique_ptr<CommandListPool> CreateCommandListPool() override;
        std::unique_ptr<DescriptorHeap> CreateDescriptorHeap(const DescriptorHeapDescription& description, const std::string& name) override;
        std::unique_ptr<Fence> CreateFence(std::uint64_t initialValue) override;
        std::unique_ptr<QueryHeap> CreateQueryHeap(const QueryHeapDescription& description, const std::string& name) override;
        std::unique_ptr<StatisticsQuery> CreateStatisticsQuery(const std::string& name) override;
        std::unique_ptr<TimestampQuery> CreateTimestampQuery(std::uint32_t timestampsCount, const std::string& name) override;
        std::unique_ptr<CommandSignature> CreateCommandSignature(const std::vector<IndirectArgumentDescription>& arguments, PipelineState* pipelineState, const std::string& name) override;
        std::unique_ptr<SwapChain> CreateSwapChain(void* windowHandle, std::uint32_t width, std::uint32_t height, bool vSync) override;
        std::unique_ptr<PipelineState> CreatePipelineState(const std::string& filepath) override;

        void CreateBufferView(const BufferView& view, CPUDescriptor& descriptor) override;
        void CreateBufferSRV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor) override;
        void CreateBufferCBV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor) override;
        void CreateBufferUAV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor, std::shared_ptr<Buffer> counterResource) override;

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
        void CreateAdapter(bool userWarp = false);
        void CreateDevice();
        void CreateAllocator();
        void CreateQueues();
        void CheckFeatureSupport();

        void CreateBufferSRV(const BufferView& view, CPUDescriptor& descriptor);
        void CreateBufferCBV(const BufferView& view, CPUDescriptor& descriptor);
        void CreateBufferUAV(const BufferView& view, CPUDescriptor& descriptor);

        void CreateTextureRTV(const TextureView& view, CPUDescriptor& descriptor);
        void CreateTextureDSV(const TextureView& view, CPUDescriptor& descriptor);
        void CreateTextureSRV(const TextureView& view, CPUDescriptor& descriptor);
        void CreateTextureUAV(const TextureView& view, CPUDescriptor& descriptor);

        ComPtr<NativeDevice> _device;
        ComPtr<IDXGIAdapter4> _adapter;
        ComPtr<D3D12MA::Allocator> _allocator;

        bool _enhancedBarriersSupported;

        AdapterInfo _adapterInfo;

        std::unique_ptr<CommandQueue> _queueGraphics;
        std::unique_ptr<CommandQueue> _queueCompute;
        std::unique_ptr<CommandQueue> _queueCopy;

        SwapChain* _swapChain;

        std::unique_ptr<tracking::IGPUCrashTracker> _crashTracker;
    };
} // namespace rhi::d3d12
