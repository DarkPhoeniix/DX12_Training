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

    // CommandListType represents the type of command list. It is used to specify the intended usage of the command list and determine the appropriate 
    // command queue for execution
    enum class CommandListType : uint8_t
    {
        Unknown,
        Graphics,
        Compute,
        Copy
    };

    // QueryType represents the type of query that can be performed on the GPU. It is used to specify the type of data that should be collected when using 
    // query heaps for performance measurements and other data collection
    enum class QueryType : uint8_t
    {
        Occlusion,
        Timestamp,
        PipelineStatistics
    };

    // PredicationOperation represents the type of predication operation that can be applied to a command list. It is used to specify the condition under which
    // the GPU should execute or skip certain commands based on the value of a buffer resource
    enum class PredicationOperation : uint8_t
    {
        EqualZero,
        NotEqualZero
    };

    // ClearFlags represents the flags that can be used to specify which aspects of a depth-stencil view should be cleared
    enum class ClearFlags : uint8_t
    {
        Depth = 1 << 0,
        Stencil = 1 << 1,
        DepthStencil = Depth | Stencil
    };

    // VertexBufferView represents the view of a vertex buffer resource. It is used to specify how vertex data is organized and accessed by the GPU
    struct VertexBufferView
    {
        std::uint64_t BufferLocation;
        std::uint32_t SizeInBytes;
        std::uint32_t StrideInBytes;
    };

    // IndexBufferView represents the view of an index buffer resource. It is used to specify how index data is organized and accessed by the GPU, 
    // including the format of the indices
    struct IndexBufferView
    {
        std::uint64_t BufferLocation;
        std::uint32_t SizeInBytes;
        Format Format;
    };

    // ScissorRect represents a rectangular region that defines the scissor area for rendering operations. It is used to specify the portion of the render target
    // that should be affected by rendering commands, allowing for efficient rendering of specific areas of the screen
    struct ScissorRect
    {
        std::int32_t Left;
        std::int32_t Top;
        std::int32_t Right;
        std::int32_t Bottom;
    };

    // Viewport represents the viewport for rendering operations. It is used to specify the area of the render target that should be rendered to
    struct Viewport
    {
        float TopLeftX = 0.0f;
        float TopLeftY = 0.0f;
        float Width = 0.0f;
        float Height = 0.0f;
        float MinDepth = 0.0f;
        float MaxDepth = 1.0f;
    };

    // CommandList is an abstract interface representing a command list, which is used to record GPU commands for execution
    class CommandList
    {
    public:
        CommandList() = default;
        CommandList(const CommandList&) = delete;
        CommandList(CommandList&&) noexcept = default;
        virtual ~CommandList() = default;

        CommandList& operator=(const CommandList&) = delete;
        CommandList& operator=(CommandList&&) noexcept = default;

        // Retrieves the type of the command list, which indicates its intended usage and determines the appropriate command queue for execution
        virtual CommandListType GetCommandListType() const = 0;

        // Sets predication for the command list, allowing the GPU to conditionally execute or skip certain commands based on the value of a buffer resource
        virtual void SetPredication(std::shared_ptr<Buffer> buffer, std::uint64_t offset, PredicationOperation operation) = 0;

        // Begin query operation on the GPU, allowing the application to start collecting data for a specific query type using a query heap. 
        // The index parameter specifies the slot in the query heap where the query should be recorded
        virtual void BeginQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index) = 0;
        // End query operation on the GPU, allowing the application to stop collecting data for a specific query type using a query heap.
        // The index parameter specifies the slot in the query heap where the query should be recorded.
        virtual void EndQuery(QueryHeap* queryHeap, QueryType type, std::uint32_t index) = 0;
        // Resolves query data from the GPU, allowing the application to retrieve the result of a specific query type from a query heap and store it in a destination buffer
        virtual void ResolveQueryData(QueryHeap* queryHeap,
                                      QueryType type, 
                                      std::uint32_t index, 
                                      std::shared_ptr<Buffer> destination, 
                                      std::uint64_t destinationOffset) = 0;
        // Resolves query data from the GPU, allowing the application to retrieve the results of a specific query type from a query heap and store it in a destination buffer
        virtual void ResolveQueryData(QueryHeap* queryHeap,
                                      QueryType type, 
                                      std::uint32_t index, 
                                      std::uint32_t numQueries, 
                                      std::shared_ptr<Buffer> destination, 
                                      std::uint64_t destinationOffset) = 0;

        // Transitions the resource states of the specified buffer resources, allowing the application to manage resource state transitions for proper synchronization and usage
        virtual void TransitionBarriers(const std::vector<BufferBarrier>& barriers) = 0;
        // Transitions the resource states of the specified texture resources, allowing the application to manage resource state transitions for proper synchronization and usage
        virtual void TransitionBarriers(const std::vector<TextureBarrier>& barriers) = 0;
        // Inserts a UAV barrier for the specified buffer resource, ensuring that all UAV accesses to the buffer are completed before any subsequent commands that access the buffer are executed
        virtual void UAVBarrier(std::shared_ptr<Buffer> buffer) = 0;
        // Inserts a UAV barrier for the specified texture resource, ensuring that all UAV accesses to the texture are completed before any subsequent commands that access the texture are executed
        virtual void UAVBarrier(std::shared_ptr<Texture> texture) = 0;

        // Copies data between resources, allowing the application to transfer data from a source buffer to a destination buffer. 
        // The CopyBuffer method perform a full copy of the resource, with optional offsets for both the source and destination
        virtual void CopyBuffer(std::shared_ptr<Buffer> sourceResource, std::shared_ptr<Buffer> destinationResource) = 0;
        // Copies a specific region of data between buffer, allowing the application to transfer a specified number of bytes from a source buffer 
        // to a destination buffer, with optional offsets for both the source and destination
        virtual void CopyBufferRegion(std::shared_ptr<Buffer> sourceResource, 
                                      std::shared_ptr<Buffer> destinationResource, 
                                      uint32_t numBytes, 
                                      uint32_t sourceOffset = 0, 
                                      uint32_t destinationOffset = 0) = 0;
        // Copies data between resources, allowing the application to transfer data from a source texture to a destination texture. 
        // The CopyTexture method performs a full copy of the resource
        virtual void CopyTexture(std::shared_ptr<Texture> sourceResource, std::shared_ptr<Texture> destinationResource) = 0;
        // Copies a specific region of data between textures, allowing the application to transfer a specified number of bytes from a source texture
        // to a destination texture, with optional offsets for both the source and destination
        virtual void CopyTextureRegion(std::shared_ptr<Texture> sourceResource, 
                                       std::shared_ptr<Texture> destinationResource, 
                                       uint32_t numBytes,
                                       uint32_t sourceOffset = 0, 
                                       uint32_t destinationOffset = 0) = 0;

        // Sets the graphics pipeline state for the command list, allowing the application to specify the configuration of the graphics pipeline
        virtual void SetGraphicsPipelineState(PipelineState* pipelineState) = 0;
        // Sets the compute pipeline state for the command list, allowing the application to specify the configuration of the compute pipeline
        virtual void SetComputePipelineState(PipelineState* pipelineState) = 0;
        // Sets the primitive topology for the command list, allowing the application to specify how vertex data should be interpreted and assembled into primitives for rendering
        virtual void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;
        // Sets the vertex buffer for the command list, allowing the application to specify the vertex buffer resource and its layout
        virtual void SetVertexBuffer(std::uint32_t slot, const VertexBufferView& vertexBufferView) = 0;
        // Sets the index buffer for the command list, allowing the application to specify the index buffer resource and its layout
        virtual void SetIndexBuffer(const IndexBufferView& indexBufferView) = 0;

        // Sets the optional render target and optional depth-stencil view for the command list, allowing the application to specify the output targets
        virtual void SetRenderTarget(CPUDescriptor* renderTargetDescriptor, CPUDescriptor* depthStencilDescriptor) = 0;
        // Sets multiple render targets and an optional depth-stencil view for the command list, allowing the application to specify multiple output targets
        virtual void SetRenderTargets(const std::vector<CPUDescriptor>& renderTargetDescriptors, CPUDescriptor* depthStencilDescriptor) = 0;
        // Sets the viewport and scissor rectangle for the command list, allowing the application to specify the portion of the render target to draw to
        virtual void SetViewport(const Viewport& viewport, const ScissorRect& scissorRectangle) = 0;

        // Clears the render target view with the specified color, allowing the application to reset the contents of the render target
        virtual void ClearRTV(CPUDescriptor renderTargetView, const float color[4], ScissorRect* rectangle = nullptr) = 0;
        // Clears the depth-stencil view with the specified depth and stencil values, allowing the application to reset the contents of the target
        virtual void ClearDSV(CPUDescriptor depthStencilView, 
                              ClearFlags clearFlags = ClearFlags::Depth,
                              float depth = 1.0f, 
                              std::uint8_t stencil = 0, 
                              ScissorRect* rectangle = nullptr) = 0;

        // Issues draw calls for non-indexed geometry, allowing the application to render primitives based on vertex data without using an index buffer
        virtual void Draw(std::uint32_t vertexCount, 
                          std::uint32_t instanceCount = 1, 
                          std::uint32_t startVertex = 0, 
                          std::uint32_t startInstance = 0) = 0;
        // Issues draw calls for indexed geometry, allowing the application to render primitives based on vertex data using an index buffer
        virtual void DrawIndexed(std::uint32_t indexCount, 
                                 std::uint32_t instanceCount = 1, 
                                 std::uint32_t startIndex = 0, 
                                 std::uint32_t baseVertex = 0, 
                                 std::uint32_t startInstance = 0) = 0;
        // Issues dispatch calls for compute shaders, allowing the application to execute compute workloads on the GPU by specifying the number of thread 
        // groups to dispatch in each dimension (X, Y, Z)
        virtual void Dispatch(std::uint32_t xThreadGroupsCount = 1, 
                              std::uint32_t yThreadGroupsCount = 1, 
                              std::uint32_t zThreadGroupsCount = 1) = 0;
        // Issues indirect draw calls, allowing the application to execute commands based on parameters stored in a buffer resource
        virtual void ExecuteIndirect(CommandSignature* commandSignature, 
                                     std::uint32_t maxCommandCount, 
                                     std::shared_ptr<Buffer> argumentBuffer,
                                     std::shared_ptr<Buffer> countBuffer, 
                                     std::uint32_t argumentBufferOffset = 0, 
                                     std::uint32_t countBufferOffset = 0) = 0;

        // Sets the descriptor heap for the command list.
        // Allows the application to specify the descriptor heap that should be used for shader resource binding and rendering operations
        virtual void SetDescriptorHeaps(rhi::DescriptorHeap* descriptorHeap) = 0;
        // Sets multiple descriptor heaps for the command list.
        // Allows the application to specify multiple descriptor heaps that should be used for shader resource binding and rendering operations
        virtual void SetDescriptorHeaps(const std::vector<DescriptorHeap*>& descriptorHeaps) = 0;
        // Sets a single graphics shader constant value for the command list, allowing the application to specify constant data that can be accessed in shaders
        virtual void SetGraphicsConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset = 0) = 0;
        // Sets a single compute shader constant value for the command list, allowing the application to specify constant data that can be accessed in shaders
        virtual void SetComputeConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset = 0) = 0;
        // Sets multiple graphics shader constant values for the command list, allowing the application to specify constant data that can be accessed in shaders
        virtual void SetGraphicsConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset = 0) = 0;
        // Sets multiple compute shader constant values for the command list, allowing the application to specify constant data that can be accessed in shaders
        virtual void SetComputeConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset = 0) = 0;
        // Sets a graphics constant buffer view (CBV) for the command list.
        // Allows the application to specify a buffer resource that can be accessed as a constant buffer in shaders
        virtual void SetGraphicsCBV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        // Sets a compute constant buffer view (CBV) for the command list.
        // Allows the application to specify a buffer resource that can be accessed as a constant buffer in shaders
        virtual void SetComputeCBV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        // Sets a graphics shader resource view (SRV) for the command list.
        // Allows the application to specify a buffer or texture resource that can be accessed as a read-only resource in shaders
        virtual void SetGraphicsSRV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        // Sets a compute shader resource view (SRV) for the command list.
        // Allows the application to specify a buffer or texture resource that can be accessed as a read-only resource in shaders
        virtual void SetComputeSRV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        // Sets a graphics unordered access view (UAV) for the command list.
        // Allows the application to specify a buffer or texture resource that can be accessed as a read-write resource in shaders
        virtual void SetGraphicsUAV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        // Sets a compute unordered access view (UAV) for the command list. 
        // Allows the application to specify a buffer or texture resource that can be accessed as a read-write resource in shaders
        virtual void SetComputeUAV(std::uint32_t index, std::uint64_t bufferLocation) = 0;
        // Sets a graphics descriptor table for the command list.
        // Allows the application to specify a GPU descriptor that can be used for shader resource binding and rendering operations
        virtual void SetGraphicsDescriptorTable(std::uint32_t index, GPUDescriptor descriptor) = 0;
        // Sets a compute descriptor table for the command list.
        // Allows the application to specify a GPU descriptor that can be used for shader resource binding and rendering operations
        virtual void SetComputeDescriptorTable(std::uint32_t index, GPUDescriptor descriptor) = 0;

        // Resets the command list, allowing the application to reuse the command list for recording new commands. 
        // The pipeline state parameter specifies the initial pipeline state to set when resetting the command list
        virtual void Reset(PipelineState* pipelineState) = 0;

        // Closes the command list, indicating that recording of commands is complete and the command list is ready for execution
        virtual void Close() = 0;

        // Begins a new event for GPU debugging and profiling purposes.
        // Allows the application to annotate the command list with a named event that can be visualized in GPU debugging tools.
        virtual void BeginEvent(const char* name, std::uint8_t color = 0) = 0;
        // Ends the current event for GPU debugging and profiling purposes
        virtual void EndEvent() = 0;

        // Sets a marker for GPU debugging and profiling purposes.
        // Allows the application to annotate the command list with a named marker that can be visualized in GPU debugging tools
        virtual void SetMarker(const char* name, std::uint8_t color = 0) = 0;

        // Sets the name of the command list for debugging purposes.
        // Allows the application to assign a readable name to the command list that can be visualized in GPU debugging tools
        virtual void SetName(const std::string& name) = 0;

        // Retrieves the native command list object, allowing the application to access the underlying API-specific command list
        virtual void* GetNative() const = 0;
    };
} // namespace rhi
