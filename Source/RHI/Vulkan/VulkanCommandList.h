#pragma once

#include "RHI/CommandList.h"

namespace rhi::vulkan
{
    class VulkanCommandList final : public rhi::CommandList
    {
    public:
        VulkanCommandList(const VulkanCommandList&) = delete;
        VulkanCommandList(VulkanCommandList&& other) noexcept;
        ~VulkanCommandList() override;

        VulkanCommandList& operator=(const VulkanCommandList&) = delete;
        VulkanCommandList& operator=(VulkanCommandList&& other) noexcept;

        rhi::CommandListType GetCommandListType() const override;

        void SetPredication(std::shared_ptr<Buffer> buffer, std::uint64_t offset, PredicationOperation operation) override;

        void BeginQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index) override;
        void EndQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index) override;
        void ResolveQueryData(QueryHeap* queryHeap, QueryType type, std::uint32_t index, std::shared_ptr<Buffer> destination, std::uint64_t offset) override;
        void ResolveQueryData(QueryHeap* queryHeap, QueryType type, std::uint32_t index, std::uint32_t numQueries, std::shared_ptr<Buffer> destination, std::uint64_t offset) override;

        void TransitionBarriers(const std::vector<BufferBarrier>& barriers) override;
        void TransitionBarriers(const std::vector<TextureBarrier>& barriers) override;
        void UAVBarrier(std::shared_ptr<Buffer> buffer) override;
        void UAVBarrier(std::shared_ptr<Texture> texture) override;

        void CopyBuffer(std::shared_ptr<Buffer> sourceResource, std::shared_ptr<Buffer> destinationResource) override;
        void CopyBufferRegion(std::shared_ptr<Buffer> sourceResource, std::shared_ptr<Buffer> destinationResource, uint32_t numBytes, uint32_t sourceOffset, uint32_t destinationOffset) override;
        void CopyTexture(std::shared_ptr<Texture> sourceResource, std::shared_ptr<Texture> destinationResource) override;
        void CopyTextureRegion(std::shared_ptr<Texture> sourceResource, std::shared_ptr<Texture> destinationResource, uint32_t numBytes, uint32_t sourceOffset, uint32_t destinationOffset) override;
        void CopyBufferToTexture(std::shared_ptr<Buffer> intermediateBuffer, std::shared_ptr<Texture> destinationTexture, const std::vector<SubresourceData>& subresources) override;

        void SetGraphicsPipelineState(rhi::PipelineState* pipelineState) override;
        void SetComputePipelineState(rhi::PipelineState* pipelineState) override;
        void SetPrimitiveTopology(rhi::PrimitiveTopology primitiveTopology) override;
        void SetVertexBuffer(std::uint32_t slot, const rhi::VertexBufferView& vertexBufferView) override;
        void SetIndexBuffer(const rhi::IndexBufferView& indexBufferView) override;

        void SetRenderTarget(rhi::CPUDescriptor* renderTargetDescriptor, rhi::CPUDescriptor* depthStencilDescriptor) override;
        void SetRenderTargets(const std::vector<rhi::CPUDescriptor>& renderTargetDescriptors, rhi::CPUDescriptor* depthStencilDescriptor) override;
        void SetViewport(const Viewport& viewport, const ScissorRect& scissorRectangle) override;

        void ClearRTV(rhi::CPUDescriptor renderTargetView, const float color[4], ScissorRect* rectangle) override;
        void ClearDSV(rhi::CPUDescriptor depthStencilView, rhi::ClearFlags clearFlags, float depth, std::uint8_t stencil, ScissorRect* rectangle) override;

        void Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t startVertex, std::uint32_t startInstance) override;
        void DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t startIndex, std::uint32_t baseVertex, std::uint32_t startInstance) override;
        void Dispatch(std::uint32_t xThreadGroupsCount, std::uint32_t yThreadGroupsCount, std::uint32_t zThreadGroupsCount) override;
        void ExecuteIndirect(rhi::CommandSignature* commandSignature, std::uint32_t maxCommandCount, std::shared_ptr<Buffer> argumentBuffer, std::shared_ptr<Buffer> countBuffer, std::uint32_t argumentBufferOffset, std::uint32_t countBufferOffset) override;

        void SetDescriptorHeaps(rhi::DescriptorHeap* descriptorHeap) override;
        void SetDescriptorHeaps(const std::vector<rhi::DescriptorHeap*>& descriptorHeaps) override;
        void SetGraphicsConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset) override;
        void SetComputeConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset) override;
        void SetGraphicsConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset) override;
        void SetComputeConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset) override;
        void SetGraphicsCBV(std::uint32_t index, std::uint64_t bufferLocation) override;
        void SetComputeCBV(std::uint32_t index, std::uint64_t bufferLocation) override;
        void SetGraphicsSRV(std::uint32_t index, std::uint64_t bufferLocation) override;
        void SetComputeSRV(std::uint32_t index, std::uint64_t bufferLocation) override;
        void SetGraphicsUAV(std::uint32_t index, std::uint64_t bufferLocation) override;
        void SetComputeUAV(std::uint32_t index, std::uint64_t bufferLocation) override;
        void SetGraphicsDescriptorTable(std::uint32_t index, rhi::GPUDescriptor descriptor) override;
        void SetComputeDescriptorTable(std::uint32_t index, rhi::GPUDescriptor descriptor) override;

        void Reset(rhi::PipelineState* pipelineState) override;
        void Close() override;

        void BeginEvent(const char* name, std::uint8_t color) override;
        void EndEvent() override;
        void SetMarker(const char* name, std::uint8_t color) override;

        void SetName(const std::string& name) override;

        void* GetNative() const override;

    private:
        friend class VulkanDevice;

        VulkanCommandList(rhi::Device* device, rhi::CommandListType type, const std::string& name = "");

        vk::CommandPool   _commandPool;
        vk::CommandBuffer _commandBuffer;
        CommandListType   _type;

        rhi::Device* _device;
    };
} // namespace rhi::vulkan
