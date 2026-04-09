#pragma once

#include "Descriptor.h"
#include "Format.h"
#include "PipelineStates.h"

#include <vector>
#include <memory>

namespace rhi
{
    class Buffer;
    class CommandSignature;
    class DescriptorHeap;
    class PipelineState;
    class Texture;
    class QueryHeap;
    class BufferBarrier;
    class TextureBarrier;

    enum class CommandListType : uint8_t
    {
        Unknown,
        Graphics,
        Compute,
        Copy
    };

    enum class QueryType : uint8_t
    {
        Occlusion,
        Timestamp,
        PipelineStatistics
    };

    enum class PredicationOperation : uint8_t
    {
        EqualZero,
        NotEqualZero
    };

    enum class ClearFlags : uint8_t
    {
        Depth = 1 << 0,
        Stencil = 1 << 1,
        DepthStencil = Depth | Stencil
    };

    struct VertexBufferView
    {
        std::uint64_t BufferLocation;
        std::uint32_t SizeInBytes;
        std::uint32_t StrideInBytes;
    };

    struct IndexBufferView
    {
        std::uint64_t BufferLocation;
        std::uint32_t SizeInBytes;
        Format Format;
    };

    struct ScissorRect
    {
        std::int32_t Left;
        std::int32_t Top;
        std::int32_t Right;
        std::int32_t Bottom;
    };

    struct Viewport
    {
        float TopLeftX = 0.0f;
        float TopLeftY = 0.0f;
        float Width = 0.0f;
        float Height = 0.0f;
        float MinDepth = 0.0f;
        float MaxDepth = 1.0f;
    };

    class CommandList
    {
    public:
        CommandList() = default;
        CommandList(const CommandList&) = delete;
        CommandList(CommandList&&) noexcept = default;
        virtual ~CommandList() = default;

        CommandList& operator=(const CommandList&) = delete;
        CommandList& operator=(CommandList&&) noexcept = default;

        virtual CommandListType GetCommandListType() const = 0;

        virtual void SetPredication(std::shared_ptr<Buffer> buffer, std::uint64_t offset, PredicationOperation operation) = 0;

        virtual void BeginQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index) = 0;
        virtual void ResolveQueryData(QueryHeap* queryHeap,
                                      QueryType type, std::uint32_t index, 
                                      std::shared_ptr<Buffer> destination, 
                                      std::uint64_t offset) = 0;
        virtual void ResolveQueryData(QueryHeap* queryHeap,
                                      QueryType type, std::uint32_t index, 
                                      std::uint32_t numQueries, std::shared_ptr<Buffer> destination, 
                                      std::uint64_t offset) = 0;
        virtual void EndQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index) = 0;

        virtual void TransitionBarriers(const std::vector<BufferBarrier>& barriers) = 0;
        virtual void TransitionBarriers(const std::vector<TextureBarrier>& barriers) = 0;
        virtual void UAVBarrier(std::shared_ptr<Buffer> buffer) = 0;
        virtual void UAVBarrier(std::shared_ptr<Texture> texture) = 0;

        virtual void CopyBuffer(std::shared_ptr<Buffer> sourceResource, std::shared_ptr<Buffer> destinationResource) = 0;
        virtual void CopyBufferRegion(std::shared_ptr<Buffer> sourceResource, 
                                      std::shared_ptr<Buffer> destinationResource, 
                                      uint32_t numBytes, 
                                      uint32_t sourceOffset = 0, 
                                      uint32_t destinationOffset = 0) = 0;
        virtual void CopyTexture(std::shared_ptr<Texture> sourceResource, std::shared_ptr<Texture> destinationResource) = 0;
        virtual void CopyTextureRegion(std::shared_ptr<Texture> sourceResource, 
                                       std::shared_ptr<Texture> destinationResource, 
                                       uint32_t numBytes,
                                       uint32_t sourceOffset = 0, 
                                       uint32_t destinationOffset = 0) = 0;

        virtual void SetGraphicsPipelineState(PipelineState* pipelineState) = 0;
        virtual void SetComputePipelineState(PipelineState* pipelineState) = 0;
        virtual void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;
        virtual void SetVertexBuffer(std::uint32_t slot, const VertexBufferView& vertexBufferView) = 0;
        virtual void SetIndexBuffer(const IndexBufferView& indexBufferView) = 0;

        virtual void SetRenderTarget(CPUDescriptor* renderTargetDescriptor, CPUDescriptor* depthStencilDescriptor) = 0;
        virtual void SetRenderTargets(const std::vector<CPUDescriptor>& renderTargetDescriptors, CPUDescriptor* depthStencilDescriptor) = 0;
        virtual void SetViewport(const Viewport& viewport, const ScissorRect& scissorRectangle) = 0;

        virtual void ClearRTV(CPUDescriptor renderTargetView, const float color[4], ScissorRect* rectangle = nullptr) = 0;
        virtual void ClearDSV(CPUDescriptor depthStencilView, 
                              ClearFlags clearFlags = ClearFlags::Depth,
                              float depth = 1.0f, 
                              std::uint8_t stencil = 0, 
                              ScissorRect* rectangle = nullptr) = 0;

        virtual void Draw(std::uint32_t vertexCount, 
                          std::uint32_t instanceCount = 1, 
                          std::uint32_t startVertex = 0, 
                          std::uint32_t startInstance = 0) = 0;
        virtual void DrawIndexed(std::uint32_t indexCount, 
                                 std::uint32_t instanceCount = 1, 
                                 std::uint32_t startIndex = 0, 
                                 std::uint32_t baseVertex = 0, 
                                 std::uint32_t startInstance = 0) = 0;
        virtual void Dispatch(std::uint32_t xThreadGroupsCount = 1, 
                              std::uint32_t yThreadGroupsCount = 1, 
                              std::uint32_t zThreadGroupsCount = 1) = 0;
        virtual void ExecuteIndirect(CommandSignature* commandSignature, 
                                     std::uint32_t maxCommandCount, 
                                     std::shared_ptr<Buffer> argumentBuffer,
                                     std::shared_ptr<Buffer> countBuffer, 
                                     std::uint32_t argumentBufferOffset = 0, 
                                     std::uint32_t countBufferOffset = 0) = 0;

        virtual void SetDescriptorHeaps(rhi::DescriptorHeap* descriptorHeap) = 0;
        virtual void SetDescriptorHeaps(const std::vector<DescriptorHeap*>& descriptorHeaps) = 0;
        virtual void SetGraphicsConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset = 0) = 0;
        virtual void SetComputeConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset = 0) = 0;
        virtual void SetGraphicsConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset = 0) = 0;
        virtual void SetComputeConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset = 0) = 0;
        virtual void SetGraphicsCBV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        virtual void SetComputeCBV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        virtual void SetGraphicsSRV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        virtual void SetComputeSRV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        virtual void SetGraphicsUAV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        virtual void SetComputeUAV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        virtual void SetGraphicsDescriptorTable(std::uint32_t index, GPUDescriptor descriptor) = 0;
        virtual void SetComputeDescriptorTable(std::uint32_t index, GPUDescriptor descriptor) = 0;

        virtual void Reset(PipelineState* pipelineState) = 0;

        virtual void Close() = 0;

        virtual void BeginEvent(const char* name, std::uint8_t color = 0) = 0;
        virtual void EndEvent() = 0;

        virtual void SetMarker(const char* name, std::uint8_t color = 0) = 0;

        virtual void SetName(const std::string& name) = 0;

        virtual void* GetNative() const = 0;
    };
} // namespace rhi
