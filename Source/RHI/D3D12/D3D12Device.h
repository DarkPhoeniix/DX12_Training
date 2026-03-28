#pragma once

#include "Device.h"
#include "SwapChain.h"

namespace tracking
{
    class IGPUCrashTracker;
} // namespace tracking

namespace rhi::d3d12
{
    // The D3D12Device class encapsulates the DirectX 12 device, adapters, command queues, and swap chain.
    class D3D12Device final : public rhi::Device
    {
    public:
        D3D12Device();
        ~D3D12Device() override;
        // Delete copy constructor to enforce singleton pattern.
        D3D12Device(const D3D12Device& other) = delete;
        // Move constructor.
        D3D12Device(D3D12Device&& other) noexcept;

        // Delete copy assignment to enforce singleton pattern.
        D3D12Device& operator=(const D3D12Device& other) = delete;
        // Move assignment operator.
        D3D12Device& operator=(D3D12Device&& other) noexcept;

        // Checks if enhanced barriers are supported by the device.
        bool IsEnhancedBarriersSupported() override;

        // Binds a swap chain to the device for rendering output.
        void BindSwapChain(rhi::SwapChain& swapChain) override; 

        // Retrieves the compute command queue for GPU compute operations.
        rhi::CommandQueue* GetComputeQueue() override;
        // Retrieves the stream command queue for resource streaming.
        rhi::CommandQueue* GetStreamQueue() override;
        // Retrieves the copy command queue for efficient resource copying.
        rhi::CommandQueue* GetCopyQueue() override;

        // Handles resizing events by updating necessary resources.
        void OnResize(std::uint32_t width, std::uint32_t height) override;
        // Gets the current back buffer resource from the swap chain.
        std::shared_ptr<rhi::Texture> GetBackBuffer() override;

        // Presents the rendered frame to the screen.
        void Present() override;

        std::shared_ptr<rhi::Buffer> CreateBuffer(const rhi::BufferDescription& description, ResourceState initialState) override;
        std::shared_ptr<rhi::Buffer> CreateBuffer(const rhi::BufferDescription& description, rhi::Heap* heap, std::uint64_t offset, ResourceState initialState) override;
        std::shared_ptr<rhi::Buffer> CreateBuffer(void* nativePtr) override;
        std::shared_ptr<rhi::Texture> CreateTexture(const rhi::TextureDescription& description, ResourceState initialState) override;
        std::shared_ptr<rhi::Texture> CreateTexture(const rhi::TextureDescription& description, rhi::Heap* heap, std::uint64_t offset, ResourceState initialState) override;
        std::shared_ptr<rhi::Texture> CreateTexture(void* nativePtr) override;

        std::unique_ptr<rhi::CommandList> CreateCommandList(CommandListType type) override;
        std::unique_ptr<rhi::DescriptorHeap> CreateDescriptorHeap(const DescriptorHeapDescription& description) override;
        std::unique_ptr<rhi::Fence> CreateFence(std::uint64_t initialValue) override;
        std::unique_ptr<rhi::QueryHeap> CreateQueryHeap(const QueryHeapDescription& description) override;
        std::unique_ptr<rhi::Heap> CreateHeap(const HeapDescription& description) override;
        std::unique_ptr<rhi::StatisticsQuery> CreateStatisticsQuery() override;
        std::unique_ptr<rhi::TimestampQuery> CreateTimestampQuery(std::uint32_t timestampsCount) override;
        std::unique_ptr<rhi::CommandSignature> CreateCommandSignature(const std::vector<rhi::IndirectArgumentDescription>& arguments, rhi::PipelineState* pipelineState) override;

        void CreateBufferSRV(std::shared_ptr<rhi::Buffer> resource, rhi::CPUDescriptor& descriptor) override;
        void CreateBufferCBV(std::shared_ptr<rhi::Buffer> resource, rhi::CPUDescriptor& descriptor) override;
        void CreateBufferUAV(std::shared_ptr<rhi::Buffer> resource, rhi::CPUDescriptor& descriptor, std::shared_ptr<rhi::Buffer> counterResource) override;
        void CreateTextureRTV(std::shared_ptr<rhi::Texture> texture, rhi::CPUDescriptor& descriptor) override;
        void CreateTextureDSV(std::shared_ptr<rhi::Texture> texture, rhi::CPUDescriptor& descriptor) override;
        void CreateTextureSRV(std::shared_ptr<rhi::Texture> texture, rhi::CPUDescriptor& descriptor) override;
        void CreateTextureCBV(std::shared_ptr<rhi::Texture> texture, rhi::CPUDescriptor& descriptor) override;
        void CreateTextureUAV(std::shared_ptr<rhi::Texture> texture, rhi::CPUDescriptor& descriptor) override;

        std::uint32_t GetDescriptorHandleIncrementSize(rhi::DescriptorHeapType type) const override;

        rhi::AllocationInfo GetAllocationInfo(const rhi::BufferDescription& description) const override;
        rhi::AllocationInfo GetAllocationInfo(const rhi::TextureDescription& description) const override;

        tracking::IGPUCrashTracker* GetCrashTracker() override;

        void* GetNative() const override;

    private:
        // Creates a DXGI adapter, optionally using WARP (software rasterizer).
        void CreateAdapter(bool userWarp = false);
        // Initializes the DirectX 12 device.
        void CreateDevice();
        // Creates the command queues.
        void CreateQueues();
        // Check features that are not supported on all hardware.
        void CheckFeatureSupport();

        ComPtr<ID3D12Device2> _device;
        ComPtr<IDXGIAdapter4> _adapter;

        bool _enhancedBarriersSupported;

        std::unique_ptr<rhi::CommandQueue> _queueGraphics;
        std::unique_ptr<rhi::CommandQueue> _queueCompute;
        std::unique_ptr<rhi::CommandQueue> _queueCopy;

        // Pointer to the swap chain bound to the device.
        rhi::SwapChain* _swapChain;

        std::unique_ptr<tracking::IGPUCrashTracker> _crashTracker;
    };
} // namespace rhi::d3d12
