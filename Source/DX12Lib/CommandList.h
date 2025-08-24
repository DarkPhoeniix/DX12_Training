#pragma once

#include "PipelineState.h"

namespace scene
{
    // Forward declaration for Viewport class used in setting viewports for the command list.
    class Viewport;
} // namespace scene

namespace dx12
{
    class ResourceBarrier;

    // Enum representing the different types of DirectX 12 command lists.
    enum class CommandListType
    {
        Unknown,      // Undefined or uninitialized command list type.
        Graphics,     // Command list for rendering graphics (e.g., draw calls).
        Compute,      // Command list for compute operations (e.g., dispatch compute shaders).
        Copy          // Command list for copying resources (e.g., buffer/resource copy).
    };

    // Represents a wrapper for DirectX 12 command list that encapsulates the functionality of recording commands for the GPU.
    class CommandList
    {
    public:
        // Default constructor initializes a CommandList object.
        CommandList();
        // Constructor that initializes a CommandList from an existing DirectX 12 command list.
        CommandList(ComPtr<ID3D12GraphicsCommandList> DXCommandList);
        // Copy constructor.
        CommandList(const CommandList& other);
        // Move constructor.
        CommandList(CommandList&& other) noexcept;
        // Destructor for cleaning up the command list resources.
        ~CommandList();

        // Copy assignment operator.
        CommandList& operator=(const CommandList& other);
        // Move assignment operator.
        CommandList& operator=(CommandList&& other) noexcept;

        // Returns the type of command list (Graphics/Compute/Copy).
        CommandListType GetCommandListType() const;

        // Sets the underlying DirectX command list.
        void SetDXCommandList(ComPtr<ID3D12GraphicsCommandList> commandList);
        // Retrieves the underlying DirectX command list.
        ComPtr<ID3D12GraphicsCommandList> GetDXCommandList() const;
        // Retrieves a reference to the underlying DirectX command list.
        ComPtr<ID3D12GraphicsCommandList>& GetDXCommandList();

        // Sets the predication (conditional execution) for the command list using a buffer and offset.
        void SetPredication(std::shared_ptr<Resource> buffer, std::uint64_t offset, D3D12_PREDICATION_OP operation);

        // Begins recording a query for GPU information).
        void BeginQuery(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, std::uint32_t index);
        // Resolves query data into a destination buffer.
        void ResolveQueryData(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, std::uint32_t index, Resource& destination, std::uint64_t offset);
        // Ends a previously started query.
        void EndQuery(ComPtr<ID3D12QueryHeap> queryHeap, D3D12_QUERY_TYPE type, std::uint32_t index);

        // Sets a resource transition barrier (to change resource states between pipeline stages).
        void TransitionBarrier(const ResourceBarrier& barrier);
        void TransitionBarriers(std::vector<ResourceBarrier>& barrier);
        // Sets a resource transition barrier (to change resource states between pipeline stages).
        // Before state is a current resource state
        void TransitionBarrier(Resource& resource, D3D12_RESOURCE_STATES stateAfter, std::uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
        // Sets an aliasing barrier (for aliasing resource states between different resource usages).
        void AliasingBarrier(std::shared_ptr<Resource> beforeResource = nullptr, std::shared_ptr<Resource> afterResource = nullptr);
        // Sets an UAV barrier for the specified resource (all UAV accesses must complete before any future UAV accesses can begin)
        void UAVBarrier(std::shared_ptr<Resource> resource);

        // Copies a resource from a source to a destination.
        void CopyResource(Resource& sourceResource, Resource& destinationResource);
        void CopyBufferRegion(Resource& sourceResource, Resource& destinationResource, uint32_t numBytes, uint32_t sourceOffset = 0, uint32_t destinationOffset = 0);

        // Sets the primitive topology for the Input Assembly stage (e.g., points, lines, triangles).
        // This defines how the GPU will interpret the vertex data for each draw call.
        void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology);

        // Sets the vertex buffer for the Input Assembly stage. The buffer contains vertex data
        // for drawing geometric primitives.
        void SetVertexBuffer(std::uint32_t slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView);

        // Sets the index buffer for the Input Assembly stage. The index buffer contains indices
        // into the vertex buffer to define which vertices form the primitives.
        void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& indexBufferView);

        // Sets the render target and depth stencil views for the Output Merger stage.
        // These views determine where the final rendering output is written to.
        void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptor, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor);
        // Sets multiple render target views (RTVs) for the Output Merger stage,
        // allowing the output of multiple render targets (e.g., for deferred rendering).
        void SetRenderTargets(const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor);
        // Sets the viewport for rendering, defining the area of the render target that will receive the output.
        void SetViewport(const scene::Viewport& viewport);

        // Sets the pipeline state object (PSO) for the current command list. The PSO contains
        // all the graphics or compute pipeline state information, such as shaders, blending, rasterization state, etc.
        void SetPipelineState(const PipelineState& rootSignature);

        // Clears the render target view (RTV) to a specified color.
        // Optionally, the viewport can be passed to define the region to clear.
        void ClearRTV(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float color[4], scene::Viewport* viewport = nullptr);

        // Clears the depth stencil view (DSV) with specified flags, depth value, and stencil value.
        // This is used to reset depth and stencil information.
        void ClearDSV(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH, float depth = 1.0f, std::uint8_t stencil = 0, scene::Viewport* viewport = nullptr);

        // Issues a draw call to render a set number of vertices.
        // Optionally, instance count and offsets can be used for instanced drawing.
        void Draw(std::uint32_t vertexCount, std::uint32_t instanceCount = 1, std::uint32_t startVertex = 0, std::uint32_t startInstance = 0);
        // Issues a draw call with indexed vertices. The index buffer is used to reference vertices in the vertex buffer.
        void DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount = 1, std::uint32_t startIndex = 0, std::uint32_t baseVertex = 0, std::uint32_t startInstance = 0);

        // Dispatches a compute shader with specified thread group counts in the X, Y, and Z dimensions.
        void Dispatch(std::uint32_t xThreadGroupsCount = 1, std::uint32_t yThreadGroupsCount = 1, std::uint32_t zThreadGroupsCount = 1);

        // TODO: comment ExecuteIndirect
        void ExecuteIndirect(ComPtr<ID3D12CommandSignature> cmdSignature, std::uint32_t maxCommandCount, Resource& argumentBuffer, std::shared_ptr<Resource> countBuffer, std::uint32_t argumentBufferOffset = 0, std::uint32_t countBufferOffset = 0);

        // Sets the descriptor heaps for the command list. Descriptor heaps are used to manage resources like buffers, textures, etc.
        void SetDescriptorHeaps(const std::vector<ID3D12DescriptorHeap*> descriptorHeaps);
        // Sets a constant value for a shader at a specific index in the command list.
        void SetConstant(std::uint32_t index, std::uint32_t data, std::uint32_t offset = 0);
        // Sets multiple constant values for a shader at a specific index in the command list.
        void SetConstants(std::uint32_t index, std::uint32_t numValues, const void* data, std::uint32_t offset = 0);
        // Sets a constant buffer view (CBV) for a shader at a specified index.
        void SetCBV(std::uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
        // Sets a shader resource view (SRV) for a shader at a specified index.
        void SetSRV(std::uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
        // Sets an unordered access view (UAV) for a shader at a specified index. UAVs allow shaders to read/write data.
        void SetUAV(std::uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
        // Sets a descriptor table for a shader at a specified index. This enables the use of multiple descriptors in a single call.
        void SetDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE descriptor);

        // Resets the command list by associating it with a new command allocator and pipeline state.
        void Reset(ID3D12CommandAllocator* commandAllocator, ID3D12PipelineState* pipelineState);

        // Closes the command list, signaling that no further commands can be recorded.
        void Close();

        // Sets the debug name for the command list.
        void SetName(const std::string& name);
        // Gets the debug name of the command list.
        std::string GetName() const;

    private:
        // Raw DirectX 12 command list.
        ComPtr<ID3D12GraphicsCommandList> _commandList;
        // Type of the current command list
        CommandListType _type;

        // Debug name.
        std::string _name;
    };
} // namespace dx12
