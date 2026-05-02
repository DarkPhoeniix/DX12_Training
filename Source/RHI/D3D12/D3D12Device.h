#pragma once

#include "Device.h"

namespace tracking
{
    class IGPUCrashTracker;
} // namespace tracking

namespace rhi::d3d12
{
    class D3D12Device final : public rhi::Device
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

        void BindSwapChain(rhi::SwapChain* swapChain) override; 

        CommandQueue* GetQueue(rhi::CommandListType type) override;
        rhi::CommandQueue* GetGraphicsQueue() override;
        rhi::CommandQueue* GetComputeQueue() override;
        rhi::CommandQueue* GetCopyQueue() override;

        void OnResize(std::uint32_t width, std::uint32_t height) override;
        std::shared_ptr<rhi::Texture> GetBackBuffer() override;

        void Present() override;

        std::shared_ptr<rhi::Buffer> CreateBuffer(const rhi::BufferDescription& description, ResourceState initialState, const std::string& name) override;
        std::shared_ptr<rhi::Buffer> CreateBuffer(const rhi::BufferDescription& description, rhi::Heap* heap, std::uint64_t offset, ResourceState initialState, const std::string& name) override;
        std::shared_ptr<rhi::Buffer> CreateBuffer(void* nativePtr, const std::string& name) override;
        std::shared_ptr<rhi::Texture> CreateTexture(const rhi::TextureDescription& description, ResourceState initialState, const std::string& name) override;
        std::shared_ptr<rhi::Texture> CreateTexture(const rhi::TextureDescription& description, rhi::Heap* heap, std::uint64_t offset, ResourceState initialState, const std::string& name) override;
        std::shared_ptr<rhi::Texture> CreateTexture(void* nativePtr, const std::string& name) override;

        std::unique_ptr<rhi::CommandList> CreateCommandList(CommandListType type, const std::string& name) override;
        std::unique_ptr<rhi::DescriptorHeap> CreateDescriptorHeap(const DescriptorHeapDescription& description, const std::string& name) override;
        std::unique_ptr<rhi::Fence> CreateFence(std::uint64_t initialValue) override;
        std::unique_ptr<rhi::QueryHeap> CreateQueryHeap(const QueryHeapDescription& description, const std::string& name) override;
        std::unique_ptr<rhi::Heap> CreateHeap(const HeapDescription& description, const std::string& name) override;
        std::unique_ptr<rhi::StatisticsQuery> CreateStatisticsQuery(const std::string& name) override;
        std::unique_ptr<rhi::TimestampQuery> CreateTimestampQuery(std::uint32_t timestampsCount, const std::string& name) override;
        std::unique_ptr<rhi::CommandSignature> CreateCommandSignature(const std::vector<rhi::IndirectArgumentDescription>& arguments, rhi::PipelineState* pipelineState, const std::string& name) override;
        std::unique_ptr<rhi::SwapChain> CreateSwapChain(void* windowHandle, std::uint32_t width, std::uint32_t height, bool vSync) override;
        std::unique_ptr<rhi::PipelineState> CreatePipelineState(const std::string& filepath) override;

        void CreateBufferView(const rhi::BufferView& view, CPUDescriptor& descriptor) override;
        void CreateBufferSRV(std::shared_ptr<rhi::Buffer> resource, rhi::CPUDescriptor& descriptor) override;
        void CreateBufferCBV(std::shared_ptr<rhi::Buffer> resource, rhi::CPUDescriptor& descriptor) override;
        void CreateBufferUAV(std::shared_ptr<rhi::Buffer> resource, rhi::CPUDescriptor& descriptor, std::shared_ptr<rhi::Buffer> counterResource) override;

        void CreateTextureView(const rhi::TextureView& view, CPUDescriptor& descriptor) override;
        void CreateTextureRTV(std::shared_ptr<rhi::Texture> texture, rhi::CPUDescriptor& descriptor) override;
        void CreateTextureDSV(std::shared_ptr<rhi::Texture> texture, rhi::CPUDescriptor& descriptor) override;
        void CreateTextureSRV(std::shared_ptr<rhi::Texture> texture, rhi::CPUDescriptor& descriptor) override;
        void CreateTextureUAV(std::shared_ptr<rhi::Texture> texture, rhi::CPUDescriptor& descriptor) override;

        std::uint32_t GetDescriptorHandleIncrementSize(rhi::DescriptorHeapType type) const override;

        rhi::AllocationInfo GetAllocationInfo(const rhi::BufferDescription& description) const override;
        rhi::AllocationInfo GetAllocationInfo(const rhi::TextureDescription& description) const override;

        const AdapterInfo& QueryAdapterInfo() override;

        tracking::IGPUCrashTracker* GetCrashTracker() override;

        void* GetNative() const override;

    private:
        void CreateAdapter(bool userWarp = false);
        void CreateDevice();
        void CreateQueues();
        void CheckFeatureSupport();

        void CreateBufferSRV(const BufferView& view, CPUDescriptor& descriptor);
        void CreateBufferCBV(const BufferView& view, CPUDescriptor& descriptor);
        void CreateBufferUAV(const BufferView& view, CPUDescriptor& descriptor);

        void CreateTextureRTV(const TextureView& view, CPUDescriptor& descriptor);
        void CreateTextureDSV(const TextureView& view, CPUDescriptor& descriptor);
        void CreateTextureSRV(const TextureView& view, CPUDescriptor& descriptor);
        void CreateTextureUAV(const TextureView& view, CPUDescriptor& descriptor);

        ComPtr<ID3D12Device2> _device;
        ComPtr<IDXGIAdapter4> _adapter;

        bool _enhancedBarriersSupported;

        AdapterInfo _adapterInfo;

        std::unique_ptr<rhi::CommandQueue> _queueGraphics;
        std::unique_ptr<rhi::CommandQueue> _queueCompute;
        std::unique_ptr<rhi::CommandQueue> _queueCopy;

        rhi::SwapChain* _swapChain;

        std::unique_ptr<tracking::IGPUCrashTracker> _crashTracker;
    };
} // namespace rhi::d3d12
