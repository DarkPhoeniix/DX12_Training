#pragma once

#include "CommandList.h"

namespace rhi
{
    class CommandSignature;
    class PipelineState;
} // namespace rhi

namespace rhi::d3d12
{
    class D3D12CommandList final : public rhi::CommandList
    {
    public:
        D3D12CommandList(const D3D12CommandList& other) = delete;
        D3D12CommandList(D3D12CommandList&& other) noexcept;
        virtual ~D3D12CommandList();

        D3D12CommandList& operator=(const D3D12CommandList& other) = delete;
        D3D12CommandList& operator=(D3D12CommandList&& other) noexcept;

        rhi::CommandListType GetCommandListType() const override;

        void SetPredication(std::shared_ptr<Buffer> buffer, std::uint64_t offset, PredicationOperation operation) override;

        void BeginQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index) override;
        void ResolveQueryData(QueryHeap* queryHeap, 
                              QueryType type,
                              std::uint32_t index,
                              std::shared_ptr<Buffer> destination,
                              std::uint64_t offset) override;
        void ResolveQueryData(QueryHeap* queryHeap,
                              QueryType type,
                              std::uint32_t index,
                              std::uint32_t numQueries,
                              std::shared_ptr<Buffer> destination,
                              std::uint64_t offset) override;
        void EndQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index) override;

        void TransitionBarriers(const std::vector<BufferBarrier>& barrier) override;
        void TransitionBarriers(const std::vector<TextureBarrier>& barrier) override;
        void UAVBarrier(std::shared_ptr<Buffer> buffer) override;
        void UAVBarrier(std::shared_ptr<Texture> texture) override;

        void CopyBuffer(std::shared_ptr<Buffer> sourceResource, std::shared_ptr<Buffer> destinationResource) override;
        void CopyBufferRegion(std::shared_ptr<Buffer> sourceResource,
                              std::shared_ptr<Buffer> destinationResource,
                              uint32_t numBytes,
                              uint32_t sourceOffset = 0,
                              uint32_t destinationOffset = 0) override;
        void CopyTexture(std::shared_ptr<Texture> sourceResource, std::shared_ptr<Texture> destinationResource) override;
        void CopyTextureRegion(std::shared_ptr<Texture> sourceResource,
                               std::shared_ptr<Texture> destinationResource,
                               uint32_t numBytes,
                               uint32_t sourceOffset = 0,
                               uint32_t destinationOffset = 0) override;

        void SetGraphicsPipelineState(rhi::PipelineState* pipelineState) override;
        void SetComputePipelineState(rhi::PipelineState* pipelineState) override;
        void SetPrimitiveTopology(rhi::PrimitiveTopology primitiveTopology) override;
        void SetVertexBuffer(std::uint32_t slot, const rhi::VertexBufferView& vertexBufferView) override;
        void SetIndexBuffer(const rhi::IndexBufferView& indexBufferView) override;

        void SetRenderTarget(rhi::CPUDescriptor* renderTargetDescriptor, rhi::CPUDescriptor* depthStencilDescriptor) override;
        void SetRenderTargets(const std::vector<rhi::CPUDescriptor>& renderTargetDescriptors, rhi::CPUDescriptor* depthStencilDescriptor) override;
        void SetViewport(const Viewport& viewport, const ScissorRect& scissorRectangle) override;

        void ClearRTV(rhi::CPUDescriptor renderTargetView, const float color[4], ScissorRect* rectangle = nullptr) override;
        void ClearDSV(rhi::CPUDescriptor depthStencilView,
                      rhi::ClearFlags clearFlags = ClearFlags::Depth,
                      float depth = 1.0f,
                      std::uint8_t stencil = 0,
                      ScissorRect* rectangle = nullptr) override;

        void Draw(std::uint32_t vertexCount,
                  std::uint32_t instanceCount = 1,
                  std::uint32_t startVertex = 0,
                  std::uint32_t startInstance = 0) override;
        void DrawIndexed(std::uint32_t indexCount,
                         std::uint32_t instanceCount = 1,
                         std::uint32_t startIndex = 0,
                         std::uint32_t baseVertex = 0,
                         std::uint32_t startInstance = 0) override;
        void Dispatch(std::uint32_t xThreadGroupsCount = 1,
                      std::uint32_t yThreadGroupsCount = 1,
                      std::uint32_t zThreadGroupsCount = 1) override;
        void ExecuteIndirect(rhi::CommandSignature* commandSignature,
                             std::uint32_t maxCommandCount,
                             std::shared_ptr<Buffer> argumentBuffer,
                             std::shared_ptr<Buffer> countBuffer,
                             std::uint32_t argumentBufferOffset = 0,
                             std::uint32_t countBufferOffset = 0) override;

        void SetDescriptorHeaps(rhi::DescriptorHeap* descriptorHeap) override;
        void SetDescriptorHeaps(const std::vector<rhi::DescriptorHeap*>& descriptorHeaps) override;
        void SetGraphicsConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset = 0) override;
        void SetComputeConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset = 0) override;
        void SetGraphicsConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset = 0) override;
        void SetComputeConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset = 0) override;
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
        friend class D3D12Device;

        D3D12CommandList(rhi::Device* device, rhi::CommandListType type, [[maybe_unused]] const std::string& name = "");

        ComPtr<ID3D12GraphicsCommandList7> _commandList;
        ComPtr<ID3D12CommandAllocator> _commandAllocator;

        CommandListType _type;

        rhi::Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
